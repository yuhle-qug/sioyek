#ifndef VOCAB_INTEGRATION_H
#define VOCAB_INTEGRATION_H

#include <QObject>
#include <QString>
#include <memory>

#include "plugins/dictionary_integration/dictionary_api.h"
#include "plugins/dictionary_integration/english_dict_fetcher.h"
#include "plugins/gemini_integration/gemini_client.h"
#include "plugins/vocabulary_manager/vocabulary_manager.h"

class QWidget;
class MainWidget;

class VocabIntegration : public QObject {
    Q_OBJECT
public:
    static VocabIntegration& instance();

    void configureFromEnv();

    // Triggered when the user highlights/selects text; will lookup and optionally show UI panel.
    void onTextHighlighted(MainWidget* mainWidget, const std::wstring& selectedText,
                           const std::wstring& documentPath, int pageNumber);

    // Accessors
    VocabularyManager* vocab();

private:
    VocabIntegration();
    ~VocabIntegration();
    VocabIntegration(const VocabIntegration&) = delete;
    VocabIntegration& operator=(const VocabIntegration&) = delete;

    std::unique_ptr<DictionaryAPI> dictionary;
    std::unique_ptr<GeminiClient> gemini;
    std::unique_ptr<VocabularyManager> vocabManager;

    QWidget* panel = nullptr; // lazy-created panel window

    void ensurePanel(MainWidget* parent);
    void updatePanel(const WordDefinition& enDef,
                     const VietnameseTranslation& vi,
                     const QString& geminiNote);
};

#endif // VOCAB_INTEGRATION_H