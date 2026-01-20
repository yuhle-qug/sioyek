#ifndef DICTIONARY_API_H
#define DICTIONARY_API_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

/**
 * @brief Structure to hold word definition data
 */
struct WordDefinition {
    QString word;
    QString partOfSpeech;
    QString definition;
    QStringList examples;
    QString phonetic;
    QString audioUrl;
};

/**
 * @brief Structure for Vietnamese translation
 */
struct VietnameseTranslation {
    QString word;
    QStringList meanings;  // Multiple possible meanings
    QString partOfSpeech;
    float confidence;  // 0.0 to 1.0
};

/**
 * @brief Abstract base class for dictionary API implementations
 */
class DictionaryAPI {
public:
    virtual ~DictionaryAPI() = default;
    
    /**
     * @brief Fetch English definition from API
     * @param word The English word to look up
     * @return WordDefinition structure with results, or empty if not found
     */
    virtual WordDefinition fetchEnglishDefinition(const QString& word) = 0;
    
    /**
     * @brief Fetch Vietnamese translation
     * @param word The English word to translate
     * @return VietnameseTranslation structure with results
     */
    virtual VietnameseTranslation fetchVietnameseTranslation(const QString& word) = 0;
    
    /**
     * @brief Check if API is available/responsive
     * @return true if API is accessible
     */
    virtual bool isAvailable() const = 0;
    
    /**
     * @brief Get error message from last operation
     * @return Error string
     */
    virtual QString getLastError() const = 0;
    
protected:
    QString lastError;
};

#endif // DICTIONARY_API_H
