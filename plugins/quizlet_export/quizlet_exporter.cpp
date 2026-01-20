#include "quizlet_exporter.h"
#include <QFile>
#include <QTextStream>

QuizletExporter::QuizletExporter() {}

QString QuizletExporter::escapeCSV(const QString& field) {
    QString f = field;
    if (f.contains('"')) f.replace("\"", "\"\"");
    if (f.contains(',') || f.contains('\n') || f.contains('"')) {
        f = '"' + f + '"';
    }
    return f;
}

QString QuizletExporter::buildCSVHeader() {
    // For Quizlet import, two columns are typical: Term, Definition
    return "Term,Definition\n";
}

QString QuizletExporter::entryToCSVRow(const VocabularyEntry& entry) {
    QString term = entry.word;
    QString def;
    if (!entry.englishDefinition.isEmpty()) def += entry.englishDefinition;
    if (!entry.vietnameseTranslation.isEmpty()) {
        if (!def.isEmpty()) def += " \u2014 ";
        def += entry.vietnameseTranslation;
    }
    return escapeCSV(term) + "," + escapeCSV(def) + "\n";
}

QString QuizletExporter::exportToQuizletCSV(const QVector<VocabularyEntry>& vocabularyList,
                                            const QString& /*title*/) {
    QString csv = buildCSVHeader();
    for (const auto& e : vocabularyList) csv += entryToCSVRow(e);
    return csv;
}

QString QuizletExporter::exportToAnkiFormat(const QVector<VocabularyEntry>& vocabularyList) {
    // Anki plain text: term\tdefinition per line
    QString out;
    for (const auto& e : vocabularyList) {
        QString def;
        if (!e.englishDefinition.isEmpty()) def += e.englishDefinition;
        if (!e.vietnameseTranslation.isEmpty()) {
            if (!def.isEmpty()) def += " \u2014 ";
            def += e.vietnameseTranslation;
        }
        out += e.word + "\t" + def + "\n";
    }
    return out;
}

QString QuizletExporter::exportToMemriseFormat(const QVector<VocabularyEntry>& vocabularyList) {
    // Similar to CSV but can be the same two-column format.
    return exportToQuizletCSV(vocabularyList, "");
}

bool QuizletExporter::saveToFile(const QString& content, const QString& filePath, const QString& /*format*/) {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        lastError = "Failed to open file for writing";
        return false;
    }
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts << content;
    return true;
}

QString QuizletExporter::generateQuizletHTML(const QVector<VocabularyEntry>& vocabularyList,
                                             const QString& title) {
    QString html = "<html><head><meta charset=\"utf-8\"><title>" + title + "</title></head><body>";
    html += "<table border=1><tr><th>Term</th><th>Definition</th></tr>";
    for (const auto& e : vocabularyList) {
        QString def;
        if (!e.englishDefinition.isEmpty()) def += e.englishDefinition;
        if (!e.vietnameseTranslation.isEmpty()) {
            if (!def.isEmpty()) def += " \u2014 ";
            def += e.vietnameseTranslation;
        }
        html += "<tr><td>" + e.word.toHtmlEscaped() + "</td><td>" + def.toHtmlEscaped() + "</td></tr>";
    }
    html += "</table></body></html>";
    return html;
}

QString QuizletExporter::getFormatDescription(const QString& format) {
    if (format == "csv" || format == "quizlet") return "Two columns: Term,Definition";
    if (format == "anki") return "Plain text with tab: Term\tDefinition";
    if (format == "html") return "Simple HTML table";
    if (format == "memrise") return "CSV similar to Quizlet";
    return "Unknown format";
}

QString QuizletExporter::getLastError() const { return lastError; }
