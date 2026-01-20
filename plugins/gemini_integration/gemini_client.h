#ifndef GEMINI_CLIENT_H
#define GEMINI_CLIENT_H

#include <QString>
#include <QJsonObject>
#include <QNetworkAccessManager>

/**
 * @brief Structure for Gemini API response
 */
struct GeminiResponse {
    bool success;
    QString content;
    QString error;
    int tokensUsed;
};

/**
 * @brief Client for Google Gemini API integration
 */
class GeminiClient {
public:
    /**
     * @brief Constructor
     * @param apiKey Your Gemini API key
     */
    GeminiClient(const QString& apiKey);
    ~GeminiClient();
    
    /**
     * @brief Set API key
     */
    void setApiKey(const QString& apiKey);
    
    /**
     * @brief Request explanation for a term
     * @param term The word/term to explain
     * @param context Optional context (e.g., from surrounding text in PDF)
     * @return GeminiResponse with explanation
     */
    GeminiResponse explainTerm(const QString& term, const QString& context = "");
    
    /**
     * @brief Request Vietnamese translation with explanation
     * @param englishTerm The English term
     * @param englishDefinition The English definition
     * @return GeminiResponse with Vietnamese translation and explanation
     */
    GeminiResponse translateAndExplain(const QString& englishTerm, 
                                       const QString& englishDefinition);
    
    /**
     * @brief Request context-aware explanation
     * Used for complex terms in academic papers
     * @param term The term to explain
     * @param documentContext The context from the PDF (e.g., surrounding paragraph)
     * @return GeminiResponse with detailed explanation
     */
    GeminiResponse getContextualExplanation(const QString& term, 
                                           const QString& documentContext);
    
    /**
     * @brief Generate vocabulary learning notes
     * @param words List of words to create notes for
     * @return GeminiResponse with learning notes
     */
    GeminiResponse generateLearningNotes(const QStringList& words);
    
    /**
     * @brief Check if API is available/authenticated
     */
    bool isAvailable() const;
    
    /**
     * @brief Get last error
     */
    QString getLastError() const;
    
    /**
     * @brief Get token usage stats
     */
    struct TokenStats {
        int totalTokensUsed;
        int requestCount;
        float averageTokensPerRequest;
    };
    TokenStats getTokenStats() const;
    
private:
    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString lastError;
    TokenStats tokenStats;
    
    const QString BASE_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent";
    const QString MODEL_NAME = "gemini-pro";
    
    /**
     * @brief Build system prompt for vocabulary learning
     */
    QString buildSystemPrompt(const QString& task);
    
    /**
     * @brief Make synchronous API request
     */
    GeminiResponse makeRequest(const QString& prompt, bool streaming = false);
    
    /**
     * @brief Parse API response
     */
    QString parseResponse(const QJsonObject& response);
};

#endif // GEMINI_CLIENT_H
