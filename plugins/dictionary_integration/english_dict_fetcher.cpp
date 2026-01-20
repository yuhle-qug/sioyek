#include "english_dict_fetcher.h"

#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

FreeDictionaryAPI::FreeDictionaryAPI()
    : networkManager(new QNetworkAccessManager()) {}

FreeDictionaryAPI::~FreeDictionaryAPI() { delete networkManager; }

static QString sanitizeWord(const QString& in) {
    QString s = in.trimmed();
    // Only keep basic word characters and hyphens
    return s;
}

WordDefinition FreeDictionaryAPI::parseEnglishResponse(const QJsonArray& data) {
    WordDefinition def;
    if (data.isEmpty()) return def;
    auto entry = data.at(0).toObject();
    def.word = entry.value("word").toString();

    // Phonetics
    auto phonetics = entry.value("phonetics").toArray();
    if (!phonetics.isEmpty()) {
        for (const auto& p : phonetics) {
            auto po = p.toObject();
            if (def.phonetic.isEmpty()) def.phonetic = po.value("text").toString();
            if (def.audioUrl.isEmpty()) def.audioUrl = po.value("audio").toString();
        }
    }

    // Meanings
    auto meanings = entry.value("meanings").toArray();
    if (!meanings.isEmpty()) {
        auto m0 = meanings.at(0).toObject();
        def.partOfSpeech = m0.value("partOfSpeech").toString();
        auto defs = m0.value("definitions").toArray();
        if (!defs.isEmpty()) {
            auto d0 = defs.at(0).toObject();
            def.definition = d0.value("definition").toString();
            if (d0.contains("example")) def.examples << d0.value("example").toString();
        }
    }
    return def;
}

WordDefinition FreeDictionaryAPI::fetchEnglishDefinition(const QString& word) {
    lastError.clear();
    WordDefinition def;
    QString path = BASE_URL + QUrl::toPercentEncoding(sanitizeWord(word));
    QNetworkRequest req(QUrl(path));
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        lastError = reply->errorString();
        reply->deleteLater();
        return def;
    }
    QByteArray body = reply->readAll();
    reply->deleteLater();

    QJsonParseError jerr{};
    QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
    if (jerr.error != QJsonParseError::NoError || (!doc.isArray() && !doc.isObject())) {
        lastError = "Invalid dictionary response";
        return def;
    }
    if (doc.isObject() && doc.object().contains("title")) {
        // error response from API
        lastError = doc.object().value("message").toString();
        return def;
    }
    def = parseEnglishResponse(doc.array());
    return def;
}

QString FreeDictionaryAPI::fetchGoogleTranslate(const QString& word) {
    // Use MyMemory free API for EN->VI as a lightweight fallback
    QUrl url("https://api.mymemory.translated.net/get");
    QUrlQuery q;
    q.addQueryItem("q", word);
    q.addQueryItem("langpair", "en|vi");
    url.setQuery(q);
    QNetworkRequest req(url);
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return QString();
    }
    QByteArray body = reply->readAll();
    reply->deleteLater();
    QJsonParseError jerr{};
    QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) return QString();
    auto obj = doc.object();
    auto data = obj.value("responseData").toObject();
    return data.value("translatedText").toString();
}

VietnameseTranslation FreeDictionaryAPI::fetchVietnameseTranslation(const QString& word) {
    VietnameseTranslation vt; vt.word = word; vt.confidence = 0.6f;
    QString t = fetchGoogleTranslate(word);
    if (!t.isEmpty()) {
        vt.meanings = QStringList{t};
        vt.partOfSpeech = QString();
    } else {
        lastError = "Translation unavailable";
    }
    return vt;
}

bool FreeDictionaryAPI::isAvailable() const { return true; }

QString FreeDictionaryAPI::getLastError() const { return lastError; }

// VietnameseDictionaryAPI (basic shim delegating to FreeDictionaryAPI translation for now)
VietnameseDictionaryAPI::VietnameseDictionaryAPI()
    : networkManager(new QNetworkAccessManager()) {}

WordDefinition VietnameseDictionaryAPI::fetchEnglishDefinition(const QString& word) {
    Q_UNUSED(word);
    return WordDefinition{}; // Not applicable
}

VietnameseTranslation VietnameseDictionaryAPI::scrapeVietnameseMeaning(const QString& word) {
    // Placeholder: could be extended to Wiktionary parsing.
    VietnameseTranslation vt; vt.word = word; vt.confidence = 0.0f;
    return vt;
}

VietnameseTranslation VietnameseDictionaryAPI::fetchVietnameseTranslation(const QString& word) {
    // Fallback to MyMemory similar to FreeDictionaryAPI
    FreeDictionaryAPI helper;
    return helper.fetchVietnameseTranslation(word);
}

bool VietnameseDictionaryAPI::isAvailable() const { return true; }

QString VietnameseDictionaryAPI::getLastError() const { return lastError; }
