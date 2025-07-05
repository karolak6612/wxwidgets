#ifndef RME_CONFLICT_RESOLVER_H
#define RME_CONFLICT_RESOLVER_H

#include "core/network/live_packets.h"
#include "core/Position.h"
#include <QObject>
#include <QMap>
#include <QTimer>
#include <QDateTime>

namespace RME {
namespace core {
namespace network {

/**
 * @brief Handles conflict resolution for simultaneous map edits
 * 
 * This class manages conflicts that occur when multiple users edit the same
 * tile simultaneously, providing various resolution strategies.
 */
class ConflictResolver : public QObject {
    Q_OBJECT

public:
    enum class ResolutionStrategy {
        LastWriteWins,      // Most recent change takes precedence
        FirstWriteWins,     // First change takes precedence
        PriorityBased,      // Based on user priority/permissions
        Manual              // Require manual resolution
    };

    enum class ConflictType {
        SimultaneousEdit,   // Multiple users editing same tile
        VersionMismatch,    // Different base versions
        PermissionConflict, // Permission-based conflicts
        DataCorruption      // Data integrity issues
    };

    struct ConflictInfo {
        ConflictType type;
        Position position;
        uint32_t originalPeerId;
        uint32_t conflictingPeerId;
        TileChange originalChange;
        TileChange conflictingChange;
        QDateTime timestamp;
        bool resolved;
    };

    explicit ConflictResolver(QObject* parent = nullptr);
    ~ConflictResolver() override;

    // Configuration
    void setResolutionStrategy(ResolutionStrategy strategy);
    ResolutionStrategy getResolutionStrategy() const { return m_strategy; }
    
    void setConflictTimeout(int timeoutMs);
    int getConflictTimeout() const { return m_conflictTimeoutMs; }

    // Conflict detection and resolution
    bool detectConflict(const TileChange& incomingChange, uint32_t peerId);
    ConflictInfo resolveConflict(const ConflictInfo& conflict);
    
    // Pending changes management
    void addPendingChange(const TileChange& change, uint32_t peerId);
    void removePendingChange(const Position& position);
    bool hasPendingChange(const Position& position) const;
    
    // Conflict history
    QList<ConflictInfo> getConflictHistory() const { return m_conflictHistory; }
    void clearConflictHistory();

signals:
    void conflictDetected(const ConflictInfo& conflict);
    void conflictResolved(const ConflictInfo& conflict, const TileChange& resolvedChange);
    void manualResolutionRequired(const ConflictInfo& conflict);

private slots:
    void onConflictTimeout();

private:
    // Resolution strategies
    TileChange resolveLastWriteWins(const ConflictInfo& conflict);
    TileChange resolveFirstWriteWins(const ConflictInfo& conflict);
    TileChange resolvePriorityBased(const ConflictInfo& conflict);
    
    // Helper methods
    bool isConflicting(const TileChange& change1, const TileChange& change2);
    int getUserPriority(uint32_t peerId);
    void addToHistory(const ConflictInfo& conflict);

    ResolutionStrategy m_strategy;
    int m_conflictTimeoutMs;
    
    // Pending changes tracking
    struct PendingChange {
        TileChange change;
        uint32_t peerId;
        QDateTime timestamp;
    };
    QMap<Position, PendingChange> m_pendingChanges;
    
    // Conflict tracking
    QList<ConflictInfo> m_activeConflicts;
    QList<ConflictInfo> m_conflictHistory;
    
    // Timeout management
    QTimer* m_timeoutTimer;
    
    // User priorities (for priority-based resolution)
    QMap<uint32_t, int> m_userPriorities;
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_CONFLICT_RESOLVER_H