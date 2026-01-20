#include "vocab_integration.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#ifdef SIOYEK_QT6
#include <QRegularExpression>
#endif

#include "pdf_viewer/main_widget.h"

VocabIntegration& VocabIntegration::instance() {
    static VocabIntegration inst;
    return inst;
}

VocabIntegration::VocabIntegration() {
    dictionary.reset(new FreeDictionaryAPI());
    vocabManager.reset(new VocabularyManager("vocabulary.db"));
    configureFromEnv();
}

VocabIntegration::~VocabIntegration() {
    if (panel) panel->deleteLater();
}

void VocabIntegration::configureFromEnv() {
    // Optional: load Gemini key from environment variable GEMINI_API_KEY
    QByteArray envKey = qgetenv("GEMINI_API_KEY");
    if (!envKey.isEmpty()) {
        gemini.reset(new GeminiClient(QString::fromUtf8(envKey)));
    }
}

VocabularyManager* VocabIntegration::vocab() { return vocabManager.get(); }

static bool isLikelySingleWord(const QString& s) {
    if (s.trimmed().isEmpty()) return false;
#ifdef SIOYEK_QT6
    QRegularExpression re("\\s");
    if (s.contains(re)) return false;
#else
    if (s.contains(QRegExp("\\s"))) return false;
#endif
    if (s.size() > 48) return false;
    return true;
}

void VocabIntegration::ensurePanel(MainWidget* parent) {
    if (panel) return;
    panel = new QWidget(parent, Qt::Tool);
    panel->setWindowTitle("Dictionary");
    panel->setMinimumWidth(360);
    panel->setMinimumHeight(260);
    auto* layout = new QVBoxLayout(panel);
    auto* text = new QTextEdit(panel);
    text->setObjectName("dict_text");
    text->setReadOnly(true);
    layout->addWidget(text);
}

void VocabIntegration::updatePanel(const WordDefinition& enDef,
                                   const VietnameseTranslation& vi,
                                   const QString& geminiNote) {
    if (!panel) return;
    auto* text = panel->findChild<QTextEdit*>("dict_text");
    if (!text) return;
    QString content;
    content += "<b>Word:</b> " + enDef.word.toHtmlEscaped() +
               (enDef.phonetic.isEmpty() ? QString() : (" &nbsp; <i>" + enDef.phonetic.toHtmlEscaped() + "</i>")) + "<br>";
    if (!enDef.partOfSpeech.isEmpty()) {
        content += "<b>POS:</b> " + enDef.partOfSpeech.toHtmlEscaped() + "<br>";
    }
    if (!enDef.definition.isEmpty()) {
        content += "<b>EN:</b> " + enDef.definition.toHtmlEscaped() + "<br>";
    }
    if (!vi.meanings.isEmpty()) {
        content += "<b>VI:</b> " + vi.meanings.join(", ").toHtmlEscaped() + "<br>";
    }
    if (!geminiNote.trimmed().isEmpty()) {
        content += "<b>Gemini:</b> " + geminiNote.toHtmlEscaped() + "<br>";
    }
    text->setHtml(content);
}

void VocabIntegration::onTextHighlighted(MainWidget* mainWidget,
                                         const std::wstring& selectedTextW,
                                         const std::wstring& documentPathW,
                                         int pageNumber) {
    QString selected = QString::fromStdWString(selectedTextW).trimmed();
    if (!isLikelySingleWord(selected)) return; // only auto-lookup single words

    // Fetch EN definition
    WordDefinition en = dictionary->fetchEnglishDefinition(selected);
    VietnameseTranslation vi;
    if (!en.word.isEmpty()) {
        vi = dictionary->fetchVietnameseTranslation(en.word);
    }

    // Optional Gemini translate/explain
    QString gnote;
    if (gemini && gemini->isAvailable()) {
        auto gr = gemini->translateAndExplain(en.word.isEmpty() ? selected : en.word,
                                              en.definition);
        if (gr.success) gnote = gr.content;
    }

    // Persist into vocabulary DB
    if (!en.word.isEmpty() || !vi.word.isEmpty()) {
        VocabularyEntry entry{};
        entry.word = en.word.isEmpty() ? selected : en.word;
        entry.englishDefinition = en.definition;
        entry.vietnameseTranslation = vi.meanings.join(", ");
        entry.partOfSpeech = en.partOfSpeech;
        entry.examples = en.examples;
        entry.sourceDocument = QString::fromStdWString(documentPathW);
        entry.pageNumber = pageNumber;
        vocabManager->addWord(entry);
    }

    // Show panel at right side of main window
    if (!mainWidget) return;
    ensurePanel(mainWidget);
    updatePanel(en, vi, gnote);
    // Position panel to the right side of the main widget
    QRect mw = mainWidget->geometry();
    int width = 380;
    int height = qMax(260, mw.height() - 100);
    panel->resize(width, height);
    panel->move(mw.x() + mw.width() - width, mw.y() + 50);
    panel->show();
    panel->raise();
}
