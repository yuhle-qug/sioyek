#ifndef QUIZLET_EXPORTER_H
#define QUIZLET_EXPORTER_H

#include <QString>
#include <QVector>
#include "vocabulary_manager.h"

/**
 * @brief Exporter for Quizlet-compatible formats
 */
class QuizletExporter {
public:
    QuizletExporter();
    
    /**
     * @brief Export vocabulary to Quizlet format (CSV)
     * @param vocabularyList List of vocabulary entries to export
     * @param title Study set title
     * @return CSV content ready to import to Quizlet
     */
    QString exportToQuizletCSV(const QVector<VocabularyEntry>& vocabularyList,
                              const QString& title = "Vocabulary");
    
    /**
     * @brief Export to Anki .txt format
     * @param vocabularyList List of vocabulary entries
     * @return Content in Anki format
     */
    QString exportToAnkiFormat(const QVector<VocabularyEntry>& vocabularyList);
    
    /**
     * @brief Export to Memrise format
     * @param vocabularyList List of vocabulary entries
     * @return Content in Memrise CSV format
     */
    QString exportToMemriseFormat(const QVector<VocabularyEntry>& vocabularyList);
    
    /**
     * @brief Save export to file
     * @param content The content to export
     * @param filePath Path where to save
     * @param format Export format (quizlet, anki, memrise, csv)
     * @return true if successful
     */
    bool saveToFile(const QString& content, const QString& filePath, 
                   const QString& format = "csv");
    
    /**
     * @brief Generate Quizlet study set HTML
     * @param vocabularyList Vocabulary entries
     * @param title Study set title
     * @return HTML content that can be imported
     */
    QString generateQuizletHTML(const QVector<VocabularyEntry>& vocabularyList,
                               const QString& title = "Vocabulary");
    
    /**
     * @brief Get supported export formats
     */
    static QStringList getSupportedFormats() {
        return {"csv", "quizlet", "anki", "memrise", "html"};
    }
    
    /**
     * @brief Get format description
     */
    static QString getFormatDescription(const QString& format);
    
    /**
     * @brief Get last error
     */
    QString getLastError() const;
    
private:
    QString lastError;
    
    /**
     * @brief Escape CSV fields
     */
    QString escapeCSV(const QString& field);
    
    /**
     * @brief Convert entry to CSV row
     */
    QString entryToCSVRow(const VocabularyEntry& entry);
    
    /**
     * @brief Build CSV header
     */
    QString buildCSVHeader();
};

#endif // QUIZLET_EXPORTER_H
