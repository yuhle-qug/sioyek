#include "gemini_client.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

GeminiClient::GeminiClient(const QString& key)
    : networkManager(new QNetworkAccessManager()), apiKey(key) {
    tokenStats = {0, 0, 0.0f};
}

GeminiClient::~GeminiClient() {
    delete networkManager;
}

void GeminiClient::setApiKey(const QString& key) { apiKey = key; }

bool GeminiClient::isAvailable() const { return !apiKey.isEmpty(); }

QString GeminiClient::getLastError() const { return lastError; }

GeminiClient::TokenStats GeminiClient::getTokenStats() const { return tokenStats; }

QString GeminiClient::buildSystemPrompt(const QString& task) {
    return QString(
        "You are an assistant helping with vocabulary learning. "
        "Task: %1. Return concise, beginner-friendly explanations."
    ).arg(task);
}

GeminiResponse GeminiClient::makeRequest(const QString& prompt, bool /*streaming*/) {
    GeminiResponse resp{false, QString(), QString(), 0};
    lastError.clear();
    if (apiKey.isEmpty()) {
        resp.error = "Gemini API key is empty";
        lastError = resp.error;
        return resp;
    }

    // Build request URL with key
    QUrl url(BASE_URL);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    url.setQuery(query);

    // Build JSON payload per Generative Language API v1beta generateContent
    QJsonObject contentPart;
    contentPart.insert("text", prompt);
    QJsonArray parts; parts.append(contentPart);
    QJsonObject contentObj; contentObj.insert("parts", parts);
    QJsonArray contents; contents.append(contentObj);

    QJsonObject root;
    root.insert("contents", contents);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QNetworkReply* reply = networkManager->post(req, QJsonDocument(root).toJson());
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        resp.error = reply->errorString();
        lastError = resp.error;
        reply->deleteLater();
        return resp;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError jerr{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
        resp.error = QString("Invalid JSON: %1").arg(jerr.errorString());
        lastError = resp.error;
        return resp;
    }

    QJsonObject obj = doc.object();
    resp.content = parseResponse(obj);
    resp.success = !resp.content.isEmpty();
    // Token usage may be present in usageMetadata.totalTokenCount
    if (obj.contains("usageMetadata")) {
        auto usage = obj.value("usageMetadata").toObject();
        int total = usage.value("totalTokenCount").toInt(0);
        resp.tokensUsed = total;
        tokenStats.totalTokensUsed += total;
        tokenStats.requestCount += 1;
        if (tokenStats.requestCount) {
            tokenStats.averageTokensPerRequest = static_cast<float>(tokenStats.totalTokensUsed) / tokenStats.requestCount;
        }
    }
    return resp;
}

QString GeminiClient::parseResponse(const QJsonObject& response) {
    // Typical structure: candidates[0].content.parts[].text
    if (!response.contains("candidates")) return QString();
    auto candidates = response.value("candidates").toArray();
    if (candidates.isEmpty()) return QString();
    auto content = candidates.at(0).toObject().value("content").toObject();
    auto parts = content.value("parts").toArray();
    QString out;
    for (const auto& p : parts) {
        auto t = p.toObject().value("text").toString();
        if (!t.isEmpty()) {
            if (!out.isEmpty()) out += "\n";
            out += t;
        }
    }
    return out.trimmed();
}

GeminiResponse GeminiClient::explainTerm(const QString& term, const QString& context) {
    QString prompt = buildSystemPrompt("Explain the term succinctly") +
                     "\nTerm: " + term;
    if (!context.isEmpty()) prompt += "\nContext: " + context;
    return makeRequest(prompt);
}

GeminiResponse GeminiClient::translateAndExplain(const QString& englishTerm, const QString& englishDefinition) {
    QString prompt = buildSystemPrompt("Translate to Vietnamese and explain briefly") +
                     "\nTerm: " + englishTerm;
    if (!englishDefinition.isEmpty()) prompt += "\nDefinition: " + englishDefinition;
    prompt += "\nOutput format: Vietnamese translation first, then a one-sentence explanation.";
    return makeRequest(prompt);
}

GeminiResponse GeminiClient::getContextualExplanation(const QString& term, const QString& documentContext) {
    QString prompt = buildSystemPrompt("Give context-aware explanation suitable for a student") +
                     "\nTerm: " + term;
    if (!documentContext.isEmpty()) prompt += "\nDocument context: " + documentContext;
    return makeRequest(prompt);
}

GeminiResponse GeminiClient::generateLearningNotes(const QStringList& words) {
    QString prompt = buildSystemPrompt("Create brief learning notes for each word") +
                     "\nWords: " + words.join(", ") +
                     "\nFor each word: definition, example, quick tip.";
    return makeRequest(prompt);
}
