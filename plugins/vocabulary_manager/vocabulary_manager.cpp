#include "vocabulary_manager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QTextStream>
#include <QDebug>

VocabularyManager::VocabularyManager(const QString& dbPath)
    : databasePath(dbPath) {
    
    // Initialize SQLite database
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databasePath);
    
    if (!database.open()) {
        lastError = "Failed to open database: " + database.lastError().text();
        qWarning() << lastError;
        return;
    }
    
    initializeDatabase();
}

VocabularyManager::~VocabularyManager() {
    if (database.isOpen()) {
        database.close();
    }
}

bool VocabularyManager::initializeDatabase() {
    QSqlQuery query;
    
    // Create vocabulary table
    if (!query.exec("CREATE TABLE IF NOT EXISTS vocabulary ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "word TEXT UNIQUE NOT NULL,"
                   "english_definition TEXT,"
                   "vietnamese_translation TEXT,"
                   "part_of_speech TEXT,"
                   "examples TEXT,"
                   "date_added DATETIME DEFAULT CURRENT_TIMESTAMP,"
                   "times_reviewed INTEGER DEFAULT 0,"
                   "learning_status TEXT DEFAULT 'new',"
                   "last_review_date DATETIME,"
                   "source_document TEXT,"
                   "page_number INTEGER"
                   ")")) {
        lastError = "Failed to create vocabulary table: " + query.lastError().text();
        return false;
    }
    
    // Create document highlights table
    if (!query.exec("CREATE TABLE IF NOT EXISTS document_highlights ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "word_id INTEGER NOT NULL,"
                   "document_path TEXT,"
                   "page_number INTEGER,"
                   "highlight_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
                   "FOREIGN KEY(word_id) REFERENCES vocabulary(id)"
                   ")")) {
        lastError = "Failed to create document_highlights table: " + query.lastError().text();
        return false;
    }
    
    // Create learning statistics table
    if (!query.exec("CREATE TABLE IF NOT EXISTS learning_stats ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "word_id INTEGER NOT NULL,"
                   "review_date DATETIME,"
                   "correct BOOLEAN,"
                   "response_time_ms INTEGER,"
                   "FOREIGN KEY(word_id) REFERENCES vocabulary(id)"
                   ")")) {
        lastError = "Failed to create learning_stats table: " + query.lastError().text();
        return false;
    }
    
    return true;
}

bool VocabularyManager::addWord(const VocabularyEntry& entry) {
    QSqlQuery query;
    query.prepare("INSERT INTO vocabulary (word, english_definition, vietnamese_translation, "
                 "part_of_speech, examples, learning_status, source_document, page_number) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entry.word);
    query.addBindValue(entry.englishDefinition);
    query.addBindValue(entry.vietnameseTranslation);
    query.addBindValue(entry.partOfSpeech);
    query.addBindValue(entry.examples.join("|"));
    query.addBindValue("new");
    query.addBindValue(entry.sourceDocument);
    query.addBindValue(entry.pageNumber);
    
    if (!query.exec()) {
        lastError = "Failed to add word: " + query.lastError().text();
        return false;
    }
    return true;
}

bool VocabularyManager::updateWord(const VocabularyEntry& entry) {
    QSqlQuery query;
    query.prepare("UPDATE vocabulary SET english_definition=?, vietnamese_translation=?, "
                 "part_of_speech=?, examples=?, learning_status=? WHERE id=?");
    
    query.addBindValue(entry.englishDefinition);
    query.addBindValue(entry.vietnameseTranslation);
    query.addBindValue(entry.partOfSpeech);
    query.addBindValue(entry.examples.join("|"));
    query.addBindValue(entry.learningStatus);
    query.addBindValue(entry.id);
    
    if (!query.exec()) {
        lastError = "Failed to update word: " + query.lastError().text();
        return false;
    }
    return true;
}

bool VocabularyManager::removeWord(int wordId) {
    QSqlQuery query;
    query.prepare("DELETE FROM vocabulary WHERE id=?");
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        lastError = "Failed to remove word: " + query.lastError().text();
        return false;
    }
    return true;
}

VocabularyEntry VocabularyManager::getWord(int wordId) {
    VocabularyEntry entry;
    QSqlQuery query;
    query.prepare("SELECT * FROM vocabulary WHERE id=?");
    query.addBindValue(wordId);
    
    if (query.exec() && query.next()) {
        entry.id = wordId;
        entry.word = query.value("word").toString();
        entry.englishDefinition = query.value("english_definition").toString();
        entry.vietnameseTranslation = query.value("vietnamese_translation").toString();
        entry.partOfSpeech = query.value("part_of_speech").toString();
        entry.examples = query.value("examples").toString().split("|");
        entry.dateAdded = query.value("date_added").toDateTime();
        entry.timesReviewed = query.value("times_reviewed").toInt();
        entry.learningStatus = query.value("learning_status").toString();
        entry.lastReviewDate = query.value("last_review_date").toDateTime();
        entry.sourceDocument = query.value("source_document").toString();
        entry.pageNumber = query.value("page_number").toInt();
    }
    return entry;
}

QVector<VocabularyEntry> VocabularyManager::searchWords(const QString& searchTerm) {
    QVector<VocabularyEntry> results;
    QSqlQuery query;
    query.prepare("SELECT id FROM vocabulary WHERE word LIKE ? OR english_definition LIKE ? "
                 "OR vietnamese_translation LIKE ? LIMIT 100");
    
    QString searchPattern = "%" + searchTerm + "%";
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    
    if (query.exec()) {
        while (query.next()) {
            results.append(getWord(query.value(0).toInt()));
        }
    }
    return results;
}

QVector<VocabularyEntry> VocabularyManager::getAllWords() {
    QVector<VocabularyEntry> results;
    QSqlQuery query("SELECT id FROM vocabulary ORDER BY date_added DESC");
    
    if (query.exec()) {
        while (query.next()) {
            results.append(getWord(query.value(0).toInt()));
        }
    }
    return results;
}

QVector<VocabularyEntry> VocabularyManager::getWordsByStatus(const QString& status) {
    QVector<VocabularyEntry> results;
    QSqlQuery query;
    query.prepare("SELECT id FROM vocabulary WHERE learning_status=? ORDER BY date_added DESC");
    query.addBindValue(status);
    
    if (query.exec()) {
        while (query.next()) {
            results.append(getWord(query.value(0).toInt()));
        }
    }
    return results;
}

VocabularyManager::VocabularyStats VocabularyManager::getStatistics() {
    VocabularyStats stats = {0, 0, 0, 0, 0, QDateTime()};
    
    QSqlQuery query;
    
    if (query.exec("SELECT COUNT(*) FROM vocabulary")) {
        if (query.next()) stats.totalWords = query.value(0).toInt();
    }
    
    if (query.exec("SELECT COUNT(*) FROM vocabulary WHERE learning_status='new'")) {
        if (query.next()) stats.newWords = query.value(0).toInt();
    }
    
    if (query.exec("SELECT COUNT(*) FROM vocabulary WHERE learning_status='learning'")) {
        if (query.next()) stats.learningWords = query.value(0).toInt();
    }
    
    if (query.exec("SELECT COUNT(*) FROM vocabulary WHERE learning_status='mastered'")) {
        if (query.next()) stats.masteredWords = query.value(0).toInt();
    }
    
    if (query.exec("SELECT COUNT(*) FROM learning_stats")) {
        if (query.next()) stats.totalReviews = query.value(0).toInt();
    }
    
    if (query.exec("SELECT MAX(review_date) FROM learning_stats")) {
        if (query.next()) stats.lastReviewDate = query.value(0).toDateTime();
    }
    
    return stats;
}

bool VocabularyManager::updateLearningStatus(int wordId, const QString& newStatus) {
    QSqlQuery query;
    query.prepare("UPDATE vocabulary SET learning_status=? WHERE id=?");
    query.addBindValue(newStatus);
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        lastError = "Failed to update learning status: " + query.lastError().text();
        return false;
    }
    return true;
}

bool VocabularyManager::recordReview(int wordId) {
    QSqlQuery query;
    query.prepare("UPDATE vocabulary SET times_reviewed=times_reviewed+1, last_review_date=CURRENT_TIMESTAMP WHERE id=?");
    query.addBindValue(wordId);
    
    if (!query.exec()) {
        lastError = "Failed to record review: " + query.lastError().text();
        return false;
    }
    return true;
}

QVector<VocabularyEntry> VocabularyManager::getWordsForReview(int maxCount) {
    QVector<VocabularyEntry> results;
    QSqlQuery query;
    
    // Simple spaced repetition: prioritize new words, then words not reviewed recently
    query.prepare("SELECT id FROM vocabulary WHERE learning_status IN ('new', 'learning') "
                 "ORDER BY CASE WHEN learning_status='new' THEN 0 ELSE 1 END, "
                 "last_review_date ASC LIMIT ?");
    query.addBindValue(maxCount);
    
    if (query.exec()) {
        while (query.next()) {
            results.append(getWord(query.value(0).toInt()));
        }
    }
    return results;
}

QString VocabularyManager::exportToCSV() {
    QString csv;
    csv += "Word,English Definition,Vietnamese Translation,Part of Speech,Examples,Date Added,Learning Status\n";
    
    QSqlQuery query("SELECT * FROM vocabulary ORDER BY date_added DESC");
    while (query.next()) {
        // Escape and format CSV
        auto escapeField = [](const QString& field) {
            QString escaped = field;
            if (escaped.contains(",") || escaped.contains("\"") || escaped.contains("\n")) {
                escaped.replace("\"", "\"\"");
                escaped = "\"" + escaped + "\"";
            }
            return escaped;
        };
        
        csv += escapeField(query.value("word").toString()) + ",";
        csv += escapeField(query.value("english_definition").toString()) + ",";
        csv += escapeField(query.value("vietnamese_translation").toString()) + ",";
        csv += escapeField(query.value("part_of_speech").toString()) + ",";
        csv += escapeField(query.value("examples").toString()) + ",";
        csv += query.value("date_added").toDateTime().toString("yyyy-MM-dd HH:mm:ss") + ",";
        csv += query.value("learning_status").toString() + "\n";
    }
    
    return csv;
}

QString VocabularyManager::exportToQuizlet() {
    return exportToCSV();  // Quizlet uses CSV format
}

bool VocabularyManager::importFromCSV(const QString& csvContent) {
    // TODO: Implement CSV import
    return false;
}

bool VocabularyManager::clearAll() {
    QSqlQuery query;
    if (!query.exec("DELETE FROM vocabulary") || !query.exec("DELETE FROM document_highlights")) {
        lastError = "Failed to clear vocabulary: " + query.lastError().text();
        return false;
    }
    return true;
}

QString VocabularyManager::getLastError() const {
    return lastError;
}
