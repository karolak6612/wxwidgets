#include "ChatManager.h"
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

namespace RME {
namespace core {
namespace network {

ChatManager::ChatManager(QObject* parent)
    : QObject(parent)
    , m_maxMessagesPerMinute(10)
    , m_maxMessageLength(500)
    , m_spamCleanupTimer(new QTimer(this))
    , m_muteTimer(new QTimer(this))
    , m_maxHistorySize(1000)
    , m_nextMessageId(1)
{
    // Initialize filters
    m_enabledFilters[ChatFilter::None] = true;
    m_enabledFilters[ChatFilter::ProfanityFilter] = true;
    m_enabledFilters[ChatFilter::SpamFilter] = true;
    m_enabledFilters[ChatFilter::LinkFilter] = false;
    
    // Setup timers
    m_spamCleanupTimer->setInterval(60000); // 1 minute
    connect(m_spamCleanupTimer, &QTimer::timeout, this, &ChatManager::onSpamCleanup);
    m_spamCleanupTimer->start();
    
    m_muteTimer->setInterval(60000); // 1 minute
    connect(m_muteTimer, &QTimer::timeout, this, &ChatManager::onMuteTimeout);
    m_muteTimer->start();
    
    initializeProfanityFilter();
    
    qInfo() << "ChatManager: Initialized with spam limits:" << m_maxMessagesPerMinute 
            << "messages/minute, max length:" << m_maxMessageLength;
}

ChatManager::~ChatManager() = default;

uint32_t ChatManager::sendMessage(uint32_t senderId, const QString& senderName, 
                                 const QString& content, MessageType type, uint32_t targetUserId)
{
    // Check if user is muted
    if (isUserMuted(senderId) && type == MessageType::Normal) {
        qWarning() << "ChatManager: Muted user" << senderId << "attempted to send message";
        return 0;
    }
    
    // Check spam limits for normal messages
    if (type == MessageType::Normal && !checkSpamLimits(senderId, content)) {
        emit spamDetected(senderId, content);
        return 0;
    }
    
    // Create message
    ChatMessage message;
    message.messageId = generateMessageId();
    message.senderId = senderId;
    message.senderName = senderName;
    message.content = content;
    message.originalContent = content;
    message.type = type;
    message.timestamp = QDateTime::currentDateTime();
    message.targetUserId = targetUserId;
    message.isFiltered = false;
    
    // Apply filters for normal messages
    if (type == MessageType::Normal) {
        QString filteredContent = filterMessage(content, senderId);
        if (filteredContent != content) {
            message.content = filteredContent;
            message.isFiltered = true;
            emit messageFiltered(senderId, content, filteredContent);
        }
    }
    
    // Add to history
    addToHistory(message);
    
    // Emit signal
    emit messageReceived(message);
    
    qDebug() << "ChatManager: Message sent by" << senderName << "(" << senderId << "):" << message.content;
    
    return message.messageId;
}

void ChatManager::receiveMessage(const ChatMessage& message)
{
    addToHistory(message);
    emit messageReceived(message);
}

void ChatManager::enableFilter(ChatFilter filter, bool enabled)
{
    m_enabledFilters[filter] = enabled;
    qInfo() << "ChatManager: Filter" << static_cast<int>(filter) << (enabled ? "enabled" : "disabled");
}

bool ChatManager::isFilterEnabled(ChatFilter filter) const
{
    return m_enabledFilters.value(filter, false);
}

QString ChatManager::filterMessage(const QString& content, uint32_t senderId)
{
    QString filtered = content;
    
    // Apply profanity filter
    if (isFilterEnabled(ChatFilter::ProfanityFilter)) {
        filtered = applyProfanityFilter(filtered);
    }
    
    // Apply link filter
    if (isFilterEnabled(ChatFilter::LinkFilter)) {
        filtered = applyLinkFilter(filtered);
    }
    
    return filtered;
}

QList<ChatManager::ChatMessage> ChatManager::getMessageHistory(int maxMessages) const
{
    if (maxMessages <= 0 || maxMessages >= m_messageHistory.size()) {
        return m_messageHistory;
    }
    
    // Return the last maxMessages
    return m_messageHistory.mid(m_messageHistory.size() - maxMessages);
}

QList<ChatManager::ChatMessage> ChatManager::getWhisperHistory(uint32_t userId1, uint32_t userId2, int maxMessages) const
{
    QPair<uint32_t, uint32_t> key = (userId1 < userId2) ? qMakePair(userId1, userId2) : qMakePair(userId2, userId1);
    
    auto it = m_whisperHistory.find(key);
    if (it == m_whisperHistory.end()) {
        return QList<ChatMessage>();
    }
    
    const QList<ChatMessage>& history = it.value();
    if (maxMessages <= 0 || maxMessages >= history.size()) {
        return history;
    }
    
    return history.mid(history.size() - maxMessages);
}

void ChatManager::clearHistory()
{
    m_messageHistory.clear();
    m_whisperHistory.clear();
    qInfo() << "ChatManager: Message history cleared";
}

void ChatManager::muteUser(uint32_t userId, int durationMinutes)
{
    UserChatState& state = m_userStates[userId];
    state.isMuted = true;
    
    if (durationMinutes > 0) {
        state.muteExpiry = QDateTime::currentDateTime().addSecs(durationMinutes * 60);
    } else {
        state.muteExpiry = QDateTime(); // Permanent mute
    }
    
    qInfo() << "ChatManager: User" << userId << "muted for" 
            << (durationMinutes > 0 ? QString::number(durationMinutes) + " minutes" : "permanently");
    
    emit userMuted(userId, durationMinutes);
}

void ChatManager::unmuteUser(uint32_t userId)
{
    auto it = m_userStates.find(userId);
    if (it != m_userStates.end()) {
        it.value().isMuted = false;
        it.value().muteExpiry = QDateTime();
        
        qInfo() << "ChatManager: User" << userId << "unmuted";
        emit userUnmuted(userId);
    }
}

bool ChatManager::isUserMuted(uint32_t userId) const
{
    auto it = m_userStates.find(userId);
    if (it == m_userStates.end()) {
        return false;
    }
    
    const UserChatState& state = it.value();
    if (!state.isMuted) {
        return false;
    }
    
    // Check if temporary mute has expired
    if (state.muteExpiry.isValid() && state.muteExpiry <= QDateTime::currentDateTime()) {
        return false;
    }
    
    return true;
}

void ChatManager::setSpamLimits(int messagesPerMinute, int maxMessageLength)
{
    m_maxMessagesPerMinute = messagesPerMinute;
    m_maxMessageLength = maxMessageLength;
    
    qInfo() << "ChatManager: Spam limits updated -" << messagesPerMinute 
            << "messages/minute, max length:" << maxMessageLength;
}

bool ChatManager::checkSpamLimits(uint32_t senderId, const QString& content)
{
    // Check message length
    if (content.length() > m_maxMessageLength) {
        qWarning() << "ChatManager: Message too long from user" << senderId 
                   << "(" << content.length() << ">" << m_maxMessageLength << ")";
        return false;
    }
    
    // Check spam filter
    if (isFilterEnabled(ChatFilter::SpamFilter) && isSpam(senderId, content)) {
        return false;
    }
    
    // Update user's message history
    UserChatState& state = m_userStates[senderId];
    QDateTime now = QDateTime::currentDateTime();
    state.recentMessages.append(now);
    
    return true;
}

void ChatManager::sendSystemMessage(const QString& content)
{
    sendMessage(0, "System", content, MessageType::System);
}

void ChatManager::sendAnnouncement(const QString& content)
{
    sendMessage(0, "Server", content, MessageType::Announcement);
}

void ChatManager::sendUserJoinedMessage(const QString& username)
{
    sendSystemMessage(QString("%1 joined the server").arg(username));
}

void ChatManager::sendUserLeftMessage(const QString& username)
{
    sendSystemMessage(QString("%1 left the server").arg(username));
}

void ChatManager::onMuteTimeout()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (auto it = m_userStates.begin(); it != m_userStates.end(); ++it) {
        UserChatState& state = it.value();
        if (state.isMuted && state.muteExpiry.isValid() && state.muteExpiry <= now) {
            state.isMuted = false;
            state.muteExpiry = QDateTime();
            
            qInfo() << "ChatManager: Temporary mute expired for user" << it.key();
            emit userUnmuted(it.key());
        }
    }
}

void ChatManager::onSpamCleanup()
{
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-60); // 1 minute ago
    
    for (auto it = m_userStates.begin(); it != m_userStates.end(); ++it) {
        UserChatState& state = it.value();
        
        // Remove old messages
        auto messageIt = state.recentMessages.begin();
        while (messageIt != state.recentMessages.end()) {
            if (*messageIt < cutoff) {
                messageIt = state.recentMessages.erase(messageIt);
            } else {
                ++messageIt;
            }
        }
    }
}

QString ChatManager::applyProfanityFilter(const QString& content)
{
    QString filtered = content;
    
    for (const QString& word : m_profanityWords) {
        QRegularExpression regex("\\b" + QRegularExpression::escape(word) + "\\b", 
                                QRegularExpression::CaseInsensitiveOption);
        QString replacement = QString("*").repeated(word.length());
        filtered.replace(regex, replacement);
    }
    
    return filtered;
}

QString ChatManager::applyLinkFilter(const QString& content)
{
    // Remove URLs
    QRegularExpression urlRegex(R"(https?://[^\s]+)", QRegularExpression::CaseInsensitiveOption);
    return content.replace(urlRegex, "[LINK REMOVED]");
}

bool ChatManager::isSpam(uint32_t senderId, const QString& content)
{
    UserChatState& state = m_userStates[senderId];
    
    // Check message rate
    if (state.recentMessages.size() >= m_maxMessagesPerMinute) {
        qWarning() << "ChatManager: Spam detected - too many messages from user" << senderId;
        return true;
    }
    
    // Check for repeated content
    int duplicateCount = 0;
    for (int i = m_messageHistory.size() - 1; i >= 0 && i >= m_messageHistory.size() - 10; --i) {
        const ChatMessage& msg = m_messageHistory[i];
        if (msg.senderId == senderId && msg.content == content) {
            duplicateCount++;
            if (duplicateCount >= 3) {
                qWarning() << "ChatManager: Spam detected - repeated content from user" << senderId;
                return true;
            }
        }
    }
    
    return false;
}

uint32_t ChatManager::generateMessageId()
{
    return m_nextMessageId++;
}

void ChatManager::addToHistory(const ChatMessage& message)
{
    // Add to main history
    m_messageHistory.append(message);
    
    // Add to whisper history if it's a whisper
    if (message.type == MessageType::Whisper && message.targetUserId != 0) {
        uint32_t userId1 = message.senderId;
        uint32_t userId2 = message.targetUserId;
        QPair<uint32_t, uint32_t> key = (userId1 < userId2) ? qMakePair(userId1, userId2) : qMakePair(userId2, userId1);
        
        m_whisperHistory[key].append(message);
        
        // Limit whisper history size
        QList<ChatMessage>& whisperList = m_whisperHistory[key];
        if (whisperList.size() > 100) {
            whisperList.removeFirst();
        }
    }
    
    // Cleanup old messages
    cleanupOldMessages();
}

void ChatManager::cleanupOldMessages()
{
    if (m_messageHistory.size() > m_maxHistorySize) {
        int toRemove = m_messageHistory.size() - m_maxHistorySize;
        m_messageHistory.erase(m_messageHistory.begin(), m_messageHistory.begin() + toRemove);
    }
}

void ChatManager::initializeProfanityFilter()
{
    loadProfanityWords();
    qInfo() << "ChatManager: Profanity filter initialized with" << m_profanityWords.size() << "words";
}

void ChatManager::loadProfanityWords()
{
    // Basic profanity list - in a real implementation, this would be loaded from a file
    m_profanityWords = QStringList{
        "damn", "hell", "crap", "stupid", "idiot", "moron", "dumb"
        // Add more words as needed
    };
}

} // namespace network
} // namespace core
} // namespace RME