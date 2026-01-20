#include "dictionary_api.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief Implementation of English Dictionary API using Free Dictionary API
 */
class FreeDictionaryAPI : public DictionaryAPI {
public:
    FreeDictionaryAPI();
    ~FreeDictionaryAPI();
    
    WordDefinition fetchEnglishDefinition(const QString& word) override;
    VietnameseTranslation fetchVietnameseTranslation(const QString& word) override;
    bool isAvailable() const override;
    QString getLastError() const override;
    
private:
    QNetworkAccessManager* networkManager;
    const QString BASE_URL = "https://api.dictionaryapi.dev/api/v2/entries/en/";
    const QString GOOGLE_TRANSLATE_API = "https://translate.googleapis.com/translate_a/element.js";
    
    /**
     * @brief Parse JSON response from Free Dictionary API
     */
    WordDefinition parseEnglishResponse(const QJsonArray& data);
    
    /**
     * @brief Fetch Vietnamese translation using Google Translate
     */
    QString fetchGoogleTranslate(const QString& word);
};

/**
 * @brief Vietnamese-specific dictionary implementation
 */
class VietnameseDictionaryAPI : public DictionaryAPI {
public:
    VietnameseDictionaryAPI();
    
    WordDefinition fetchEnglishDefinition(const QString& word) override;
    VietnameseTranslation fetchVietnameseTranslation(const QString& word) override;
    bool isAvailable() const override;
    QString getLastError() const override;
    
private:
    QNetworkAccessManager* networkManager;
    
    /**
     * @brief Scrape Vietnamese meaning from Wiktionary or other sources
     */
    VietnameseTranslation scrapeVietnameseMeaning(const QString& word);
};

#endif // ENGLISH_DICT_FETCHER_H
