#ifndef RME_AUTHENTICATION_MANAGER_H
#define RME_AUTHENTICATION_MANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QCryptographicHash>

namespace RME {
namespace core {
namespace network {

/**
 * @brief Manages user authentication and permissions for live collaboration
 */
class AuthenticationManager : public QObject {
    Q_OBJECT

public:
    enum class UserRole {
        Guest,          // Read-only access
        Editor,         // Can edit map
        Moderator,      // Can kick users, manage permissions
        Administrator   // Full access
    };

    enum class AuthResult {
        Success,
        InvalidCredentials,
        UserBanned,
        ServerFull,
        InternalError
    };

    struct UserInfo {
        uint32_t userId;
        QString username;
        UserRole role;
        QDateTime lastLogin;
        QDateTime sessionStart;
        QString ipAddress;
        bool isOnline;
        int priority; // For conflict resolution
    };

    explicit AuthenticationManager(QObject* parent = nullptr);
    ~AuthenticationManager() override;

    // Authentication
    AuthResult authenticateUser(const QString& username, const QString& password, 
                               const QString& ipAddress, uint32_t& outUserId);
    bool validateSession(uint32_t userId, const QString& sessionToken);
    void logoutUser(uint32_t userId);
    
    // User management
    bool addUser(const QString& username, const QString& password, UserRole role);
    bool removeUser(const QString& username);
    bool changeUserRole(const QString& username, UserRole newRole);
    bool changeUserPassword(const QString& username, const QString& newPassword);
    
    // Session management
    QString generateSessionToken(uint32_t userId);
    void invalidateSession(uint32_t userId);
    void cleanupExpiredSessions();
    
    // User queries
    UserInfo getUserInfo(uint32_t userId) const;
    UserInfo getUserInfo(const QString& username) const;
    QList<UserInfo> getOnlineUsers() const;
    QList<UserInfo> getAllUsers() const;
    
    // Permissions
    bool hasPermission(uint32_t userId, const QString& permission) const;
    bool canEditMap(uint32_t userId) const;
    bool canKickUsers(uint32_t userId) const;
    bool canManageUsers(uint32_t userId) const;
    
    // Ban management
    void banUser(const QString& username, const QString& reason = QString());
    void unbanUser(const QString& username);
    bool isUserBanned(const QString& username) const;
    
    // Configuration
    void setMaxUsers(int maxUsers) { m_maxUsers = maxUsers; }
    int getMaxUsers() const { return m_maxUsers; }
    
    void setSessionTimeout(int timeoutMinutes) { m_sessionTimeoutMinutes = timeoutMinutes; }
    int getSessionTimeout() const { return m_sessionTimeoutMinutes; }

signals:
    void userLoggedIn(uint32_t userId, const QString& username);
    void userLoggedOut(uint32_t userId, const QString& username);
    void userBanned(const QString& username, const QString& reason);
    void userUnbanned(const QString& username);
    void sessionExpired(uint32_t userId);

private:
    // Password hashing
    QString hashPassword(const QString& password, const QString& salt) const;
    QString generateSalt() const;
    bool verifyPassword(const QString& password, const QString& hash, const QString& salt) const;
    
    // User ID generation
    uint32_t generateUserId();
    
    // Data structures
    struct UserData {
        uint32_t userId;
        QString username;
        QString passwordHash;
        QString salt;
        UserRole role;
        QDateTime created;
        QDateTime lastLogin;
        bool isBanned;
        QString banReason;
    };
    
    struct SessionData {
        uint32_t userId;
        QString token;
        QDateTime created;
        QDateTime lastAccess;
        QString ipAddress;
    };
    
    QMap<QString, UserData> m_users; // username -> UserData
    QMap<uint32_t, UserData*> m_usersByID; // userId -> UserData*
    QMap<uint32_t, SessionData> m_sessions; // userId -> SessionData
    QMap<QString, QDateTime> m_bannedIPs; // IP -> ban time
    
    uint32_t m_nextUserId;
    int m_maxUsers;
    int m_sessionTimeoutMinutes;
    
    // Default permissions by role
    QMap<UserRole, QStringList> m_rolePermissions;
    
    void initializeDefaultPermissions();
    void loadUsers(); // Load from persistent storage
    void saveUsers(); // Save to persistent storage
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_AUTHENTICATION_MANAGER_H