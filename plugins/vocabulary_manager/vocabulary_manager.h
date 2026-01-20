#ifndef VOCABULARY_MANAGER_H
#define VOCABULARY_MANAGER_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QSqlDatabase>
#include <memory>

/**
 * @brief Represents a single vocabulary entry
 */
struct VocabularyEntry {
    int id;
    QString word;
    QString englishDefinition;
    QString vietnameseTranslation;
    QString partOfSpeech;
    QStringList examples;
    QDateTime dateAdded;
    int timesReviewed;
    QString learningStatus;  // 'new', 'learning', 'mastered'
    QDateTime lastReviewDate;
    QString sourceDocument;
    int pageNumber;
};

/**
 * @brief Manages vocabulary database and learning features
 */
class VocabularyManager {
public:
    VocabularyManager(const QString& databasePath = "vocabulary.db");
    ~VocabularyManager();
    
    /**
     * @brief Add new word to vocabulary
     * @return true if successfully added
     */
    bool addWord(const VocabularyEntry& entry);
    
    /**
     * @brief Update existing vocabulary entry
     */
    bool updateWord(const VocabularyEntry& entry);
    
    /**
     * @brief Remove word from vocabulary
     */
    bool removeWord(int wordId);
    
    /**
     * @brief Get word by ID
     */
    VocabularyEntry getWord(int wordId);
    
    /**
     * @brief Search words by term
     */
    QVector<VocabularyEntry> searchWords(const QString& searchTerm);
    
    /**
     * @brief Get all words in vocabulary
     */
    QVector<VocabularyEntry> getAllWords();
    
    /**
     * @brief Get words by learning status
     */
    QVector<VocabularyEntry> getWordsByStatus(const QString& status);
    
    /**
     * @brief Get statistics about vocabulary
     */
    struct VocabularyStats {
        int totalWords;
        int newWords;
        int learningWords;
        int masteredWords;
        int totalReviews;
        QDateTime lastReviewDate;
    };
    VocabularyStats getStatistics();
    
    /**
     * @brief Update learning status of a word
     */
    bool updateLearningStatus(int wordId, const QString& newStatus);
    
    /**
     * @brief Record a review session
     */
    bool recordReview(int wordId);
    
    /**
     * @brief Get words due for review (spaced repetition)
     */
    QVector<VocabularyEntry> getWordsForReview(int maxCount = 20);
    
    /**
     * @brief Export vocabulary to CSV format
     */
    QString exportToCSV();
    
    /**
     * @brief Export vocabulary to Quizlet format
     */
    QString exportToQuizlet();
    
    /**
     * @brief Import vocabulary from CSV
     */
    bool importFromCSV(const QString& csvContent);
    
    /**
     * @brief Clear all vocabulary
     */
    bool clearAll();
    
    /**
     * @brief Get last error message
     */
    QString getLastError() const;
    
private:
    QSqlDatabase database;
    QString databasePath;
    QString lastError;
    
    /**
     * @brief Initialize database schema
     */
    bool initializeDatabase();
    
    /**
     * @brief Upgrade database schema if needed
     */
    bool upgradeDatabase();
};

#endif // VOCABULARY_MANAGER_H
