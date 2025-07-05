#include "AuthenticationManager.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QUuid>

namespace RME {
namespace core {
namespace network {

AuthenticationManager::AuthenticationManager(QObject* parent)
    : QObject(parent)
    , m_nextUserId(1000) // Start from 1000 to avoid conflicts
    , m_maxUsers(50)
    , m_sessionTimeoutMinutes(60) // 1 hour default
{
    initializeDefaultPermissions();
    loadUsers();
    
    // Add default admin user if no users exist
    if (m_users.isEmpty()) {
        addUser("admin", "admin", UserRole::Administrator);
        qInfo() << "AuthenticationManager: Created default admin user (admin/admin)";
    }
}

AuthenticationManager::~AuthenticationManager()
{
    saveUsers();
}

AuthenticationManager::AuthResult AuthenticationManager::authenticateUser(
    const QString& username, const QString& password, const QString& ipAddress, uint32_t& outUserId)
{
    // Check if server is full
    if (getOnlineUsers().size() >= m_maxUsers) {
        qWarning() << "AuthenticationManager: Server full, rejecting user" << username;
        return AuthResult::ServerFull;
    }
    
    // Check if user exists
    auto userIt = m_users.find(username);
    if (userIt == m_users.end()) {
        qWarning() << "AuthenticationManager: User not found:" << username;
        return AuthResult::InvalidCredentials;
    }
    
    UserData& userData = userIt.value();
    
    // Check if user is banned
    if (userData.isBanned) {
        qWarning() << "AuthenticationManager: Banned user attempted login:" << username;
        return AuthResult::UserBanned;
    }
    
    // Verify password
    if (!verifyPassword(password, userData.passwordHash, userData.salt)) {
        qWarning() << "AuthenticationManager: Invalid password for user:" << username;
        return AuthResult::InvalidCredentials;
    }
    
    // Update user data
    userData.lastLogin = QDateTime::currentDateTime();
    outUserId = userData.userId;
    
    // Create session
    QString sessionToken = generateSessionToken(userData.userId);
    SessionData session;
    session.userId = userData.userId;
    session.token = sessionToken;
    session.created = QDateTime::currentDateTime();
    session.lastAccess = session.created;
    session.ipAddress = ipAddress;
    
    m_sessions[userData.userId] = session;
    
    qInfo() << "AuthenticationManager: User authenticated:" << username << "(" << userData.userId << ")";
    emit userLoggedIn(userData.userId, username);
    
    return AuthResult::Success;
}

bool AuthenticationManager::validateSession(uint32_t userId, const QString& sessionToken)
{
    auto sessionIt = m_sessions.find(userId);
    if (sessionIt == m_sessions.end()) {
        return false;
    }
    
    SessionData& session = sessionIt.value();
    
    // Check token
    if (session.token != sessionToken) {
        return false;
    }
    
    // Check if session expired
    QDateTime now = QDateTime::currentDateTime();
    if (session.lastAccess.addSecs(m_sessionTimeoutMinutes * 60) < now) {
        m_sessions.remove(userId);
        emit sessionExpired(userId);
        return false;
    }
    
    // Update last access
    session.lastAccess = now;
    return true;
}

void AuthenticationManager::logoutUser(uint32_t userId)
{
    auto sessionIt = m_sessions.find(userId);
    if (sessionIt != m_sessions.end()) {
        auto userIt = m_usersByID.find(userId);
        QString username = (userIt != m_usersByID.end()) ? userIt.value()->username : "Unknown";
        
        m_sessions.remove(userId);
        qInfo() << "AuthenticationManager: User logged out:" << username << "(" << userId << ")";
        emit userLoggedOut(userId, username);
    }
}

bool AuthenticationManager::addUser(const QString& username, const QString& password, UserRole role)
{
    if (m_users.contains(username)) {
        qWarning() << "AuthenticationManager: User already exists:" << username;
        return false;
    }
    
    UserData userData;
    userData.userId = generateUserId();
    userData.username = username;
    userData.salt = generateSalt();
    userData.passwordHash = hashPassword(password, userData.salt);
    userData.role = role;
    userData.created = QDateTime::currentDateTime();
    userData.isBanned = false;
    
    m_users[username] = userData;
    m_usersByID[userData.userId] = &m_users[username];
    
    qInfo() << "AuthenticationManager: User added:" << username << "with role" << static_cast<int>(role);
    return true;
}

bool AuthenticationManager::removeUser(const QString& username)
{
    auto userIt = m_users.find(username);
    if (userIt == m_users.end()) {
        return false;
    }
    
    uint32_t userId = userIt.value().userId;
    
    // Remove from sessions if online
    m_sessions.remove(userId);
    
    // Remove from maps
    m_usersByID.remove(userId);
    m_users.erase(userIt);
    
    qInfo() << "AuthenticationManager: User removed:" << username;
    return true;
}

bool AuthenticationManager::changeUserRole(const QString& username, UserRole newRole)
{
    auto userIt = m_users.find(username);
    if (userIt == m_users.end()) {
        return false;
    }
    
    userIt.value().role = newRole;
    qInfo() << "AuthenticationManager: Changed role for user" << username << "to" << static_cast<int>(newRole);
    return true;
}

bool AuthenticationManager::changeUserPassword(const QString& username, const QString& newPassword)
{
    auto userIt = m_users.find(username);
    if (userIt == m_users.end()) {
        return false;
    }
    
    UserData& userData = userIt.value();
    userData.salt = generateSalt();
    userData.passwordHash = hashPassword(newPassword, userData.salt);
    
    qInfo() << "AuthenticationManager: Password changed for user:" << username;
    return true;
}

QString AuthenticationManager::generateSessionToken(uint32_t userId)
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces) + QString::number(userId);
}

void AuthenticationManager::invalidateSession(uint32_t userId)
{
    m_sessions.remove(userId);
}

void AuthenticationManager::cleanupExpiredSessions()
{
    QDateTime now = QDateTime::currentDateTime();
    auto it = m_sessions.begin();
    
    while (it != m_sessions.end()) {
        if (it.value().lastAccess.addSecs(m_sessionTimeoutMinutes * 60) < now) {
            uint32_t userId = it.key();
            it = m_sessions.erase(it);
            emit sessionExpired(userId);
        } else {
            ++it;
        }
    }
}

AuthenticationManager::UserInfo AuthenticationManager::getUserInfo(uint32_t userId) const
{
    UserInfo info;
    auto userIt = m_usersByID.find(userId);
    if (userIt != m_usersByID.end()) {
        const UserData* userData = userIt.value();
        info.userId = userData->userId;
        info.username = userData->username;
        info.role = userData->role;
        info.lastLogin = userData->lastLogin;
        info.isOnline = m_sessions.contains(userId);
        info.priority = static_cast<int>(userData->role); // Role as priority
        
        if (info.isOnline) {
            const SessionData& session = m_sessions[userId];
            info.sessionStart = session.created;
            info.ipAddress = session.ipAddress;
        }
    }
    return info;
}

AuthenticationManager::UserInfo AuthenticationManager::getUserInfo(const QString& username) const
{
    auto userIt = m_users.find(username);
    if (userIt != m_users.end()) {
        return getUserInfo(userIt.value().userId);
    }
    return UserInfo();
}

QList<AuthenticationManager::UserInfo> AuthenticationManager::getOnlineUsers() const
{
    QList<UserInfo> onlineUsers;
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        onlineUsers.append(getUserInfo(it.key()));
    }
    return onlineUsers;
}

QList<AuthenticationManager::UserInfo> AuthenticationManager::getAllUsers() const
{
    QList<UserInfo> allUsers;
    for (auto it = m_users.begin(); it != m_users.end(); ++it) {
        allUsers.append(getUserInfo(it.value().userId));
    }
    return allUsers;
}

bool AuthenticationManager::hasPermission(uint32_t userId, const QString& permission) const
{
    auto userIt = m_usersByID.find(userId);
    if (userIt == m_usersByID.end()) {
        return false;
    }
    
    UserRole role = userIt.value()->role;
    return m_rolePermissions[role].contains(permission);
}

bool AuthenticationManager::canEditMap(uint32_t userId) const
{
    return hasPermission(userId, "edit_map");
}

bool AuthenticationManager::canKickUsers(uint32_t userId) const
{
    return hasPermission(userId, "kick_users");
}

bool AuthenticationManager::canManageUsers(uint32_t userId) const
{
    return hasPermission(userId, "manage_users");
}

void AuthenticationManager::banUser(const QString& username, const QString& reason)
{
    auto userIt = m_users.find(username);
    if (userIt != m_users.end()) {
        UserData& userData = userIt.value();
        userData.isBanned = true;
        userData.banReason = reason;
        
        // Kick user if online
        m_sessions.remove(userData.userId);
        
        qInfo() << "AuthenticationManager: User banned:" << username << "Reason:" << reason;
        emit userBanned(username, reason);
    }
}

void AuthenticationManager::unbanUser(const QString& username)
{
    auto userIt = m_users.find(username);
    if (userIt != m_users.end()) {
        UserData& userData = userIt.value();
        userData.isBanned = false;
        userData.banReason.clear();
        
        qInfo() << "AuthenticationManager: User unbanned:" << username;
        emit userUnbanned(username);
    }
}

bool AuthenticationManager::isUserBanned(const QString& username) const
{
    auto userIt = m_users.find(username);
    return (userIt != m_users.end()) ? userIt.value().isBanned : false;
}

QString AuthenticationManager::hashPassword(const QString& password, const QString& salt) const
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData((password + salt).toUtf8());
    return hash.result().toHex();
}

QString AuthenticationManager::generateSalt() const
{
    QByteArray salt;
    for (int i = 0; i < 16; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt.toHex();
}

bool AuthenticationManager::verifyPassword(const QString& password, const QString& hash, const QString& salt) const
{
    return hashPassword(password, salt) == hash;
}

uint32_t AuthenticationManager::generateUserId()
{
    return m_nextUserId++;
}

void AuthenticationManager::initializeDefaultPermissions()
{
    // Guest permissions
    m_rolePermissions[UserRole::Guest] = QStringList{
        "view_map"
    };
    
    // Editor permissions
    m_rolePermissions[UserRole::Editor] = QStringList{
        "view_map",
        "edit_map",
        "chat"
    };
    
    // Moderator permissions
    m_rolePermissions[UserRole::Moderator] = QStringList{
        "view_map",
        "edit_map",
        "chat",
        "kick_users",
        "moderate_chat"
    };
    
    // Administrator permissions
    m_rolePermissions[UserRole::Administrator] = QStringList{
        "view_map",
        "edit_map",
        "chat",
        "kick_users",
        "moderate_chat",
        "manage_users",
        "server_admin"
    };
}

void AuthenticationManager::loadUsers()
{
    // In a full implementation, this would load from a database or file
    // For now, we'll just initialize with empty data
    qInfo() << "AuthenticationManager: User data loaded";
}

void AuthenticationManager::saveUsers()
{
    // In a full implementation, this would save to a database or file
    // For now, we'll just log the action
    qInfo() << "AuthenticationManager: User data saved";
}

} // namespace network
} // namespace core
} // namespace RME