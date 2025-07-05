#ifndef RME_CHAT_MANAGER_H
#define RME_CHAT_MANAGER_H

#include "core/network/live_packets.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QTimer>

namespace RME {
namespace core {
namespace network {

/**
 * @brief Manages chat functionality for live collaboration
 */
class ChatManager : public QObject {
    Q_OBJECT

public:
    enum class MessageType {
        Normal,         // Regular chat message
        System,         // System notification
        Whisper,        // Private message
        Announcement,   // Server announcement
        Warning,        // Warning message
        Error          // Error message
    };

    enum class ChatFilter {
        None,           // No filtering
        ProfanityFilter,// Filter profanity
        SpamFilter,     // Filter spam
        LinkFilter      // Filter links
    };

    struct ChatMessage {
        uint32_t messageId;
        uint32_t senderId;
        QString senderName;
        QString content;
        MessageType type;
        QDateTime timestamp;
        NetworkColor senderColor;
        uint32_t targetUserId; // For whispers, 0 for public
        bool isFiltered;
        QString originalContent; // Before filtering
    };

    explicit ChatManager(QObject* parent = nullptr);
    ~ChatManager() override;

    // Message handling
    uint32_t sendMessage(uint32_t senderId, const QString& senderName, 
                        const QString& content, MessageType type = MessageType::Normal,
                        uint32_t targetUserId = 0);
    void receiveMessage(const ChatMessage& message);
    
    // Message filtering
    void enableFilter(ChatFilter filter, bool enabled);
    bool isFilterEnabled(ChatFilter filter) const;
    QString filterMessage(const QString& content, uint32_t senderId);
    
    // Message history
    QList<ChatMessage> getMessageHistory(int maxMessages = 100) const;
    QList<ChatMessage> getWhisperHistory(uint32_t userId1, uint32_t userId2, int maxMessages = 50) const;
    void clearHistory();
    
    // User management
    void muteUser(uint32_t userId, int durationMinutes = 0); // 0 = permanent
    void unmuteUser(uint32_t userId);
    bool isUserMuted(uint32_t userId) const;
    
    // Spam protection
    void setSpamLimits(int messagesPerMinute, int maxMessageLength);
    bool checkSpamLimits(uint32_t senderId, const QString& content);
    
    // System messages
    void sendSystemMessage(const QString& content);
    void sendAnnouncement(const QString& content);
    void sendUserJoinedMessage(const QString& username);
    void sendUserLeftMessage(const QString& username);
    
    // Configuration
    void setMaxHistorySize(int maxSize) { m_maxHistorySize = maxSize; }
    int getMaxHistorySize() const { return m_maxHistorySize; }

signals:
    void messageReceived(const ChatMessage& message);
    void messageFiltered(uint32_t senderId, const QString& originalContent, const QString& filteredContent);
    void userMuted(uint32_t userId, int durationMinutes);
    void userUnmuted(uint32_t userId);
    void spamDetected(uint32_t senderId, const QString& content);

private slots:
    void onMuteTimeout();
    void onSpamCleanup();

private:
    // Message processing
    QString applyProfanityFilter(const QString& content);
    QString applyLinkFilter(const QString& content);
    bool isSpam(uint32_t senderId, const QString& content);
    
    // Helper methods
    uint32_t generateMessageId();
    void addToHistory(const ChatMessage& message);
    void cleanupOldMessages();
    
    // Message storage
    QList<ChatMessage> m_messageHistory;
    QMap<QPair<uint32_t, uint32_t>, QList<ChatMessage>> m_whisperHistory;
    
    // User state
    struct UserChatState {
        bool isMuted;
        QDateTime muteExpiry;
        QList<QDateTime> recentMessages; // For spam detection
        int warningCount;
    };
    QMap<uint32_t, UserChatState> m_userStates;
    
    // Filtering
    QMap<ChatFilter, bool> m_enabledFilters;
    QStringList m_profanityWords;
    QStringList m_allowedDomains;
    
    // Spam protection
    int m_maxMessagesPerMinute;
    int m_maxMessageLength;
    QTimer* m_spamCleanupTimer;
    QTimer* m_muteTimer;
    
    // Configuration
    int m_maxHistorySize;
    uint32_t m_nextMessageId;
    
    void initializeProfanityFilter();
    void loadProfanityWords();
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_CHAT_MANAGER_H