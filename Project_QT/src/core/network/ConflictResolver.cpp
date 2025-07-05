#include "ConflictResolver.h"
#include <QDebug>
#include <algorithm>

namespace RME {
namespace core {
namespace network {

ConflictResolver::ConflictResolver(QObject* parent)
    : QObject(parent)
    , m_strategy(ResolutionStrategy::LastWriteWins)
    , m_conflictTimeoutMs(5000) // 5 seconds default
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(false);
    m_timeoutTimer->setInterval(1000); // Check every second
    connect(m_timeoutTimer, &QTimer::timeout, this, &ConflictResolver::onConflictTimeout);
    m_timeoutTimer->start();
}

ConflictResolver::~ConflictResolver() = default;

void ConflictResolver::setResolutionStrategy(ResolutionStrategy strategy)
{
    m_strategy = strategy;
    qInfo() << "ConflictResolver: Strategy changed to" << static_cast<int>(strategy);
}

void ConflictResolver::setConflictTimeout(int timeoutMs)
{
    m_conflictTimeoutMs = timeoutMs;
    qInfo() << "ConflictResolver: Timeout set to" << timeoutMs << "ms";
}

bool ConflictResolver::detectConflict(const TileChange& incomingChange, uint32_t peerId)
{
    // Check if there's a pending change for the same position
    auto it = m_pendingChanges.find(incomingChange.position);
    if (it != m_pendingChanges.end()) {
        const PendingChange& pendingChange = it.value();
        
        // Check if it's from a different peer
        if (pendingChange.peerId != peerId) {
            // Check if the changes are actually conflicting
            if (isConflicting(pendingChange.change, incomingChange)) {
                ConflictInfo conflict;
                conflict.type = ConflictType::SimultaneousEdit;
                conflict.position = incomingChange.position;
                conflict.originalPeerId = pendingChange.peerId;
                conflict.conflictingPeerId = peerId;
                conflict.originalChange = pendingChange.change;
                conflict.conflictingChange = incomingChange;
                conflict.timestamp = QDateTime::currentDateTime();
                conflict.resolved = false;
                
                m_activeConflicts.append(conflict);
                emit conflictDetected(conflict);
                
                qWarning() << "ConflictResolver: Conflict detected at position" 
                          << incomingChange.position.x << "," << incomingChange.position.y
                          << "between peers" << pendingChange.peerId << "and" << peerId;
                
                return true;
            }
        }
    }
    
    return false;
}

ConflictResolver::ConflictInfo ConflictResolver::resolveConflict(const ConflictInfo& conflict)
{
    ConflictInfo resolvedConflict = conflict;
    TileChange resolvedChange;
    
    switch (m_strategy) {
        case ResolutionStrategy::LastWriteWins:
            resolvedChange = resolveLastWriteWins(conflict);
            break;
            
        case ResolutionStrategy::FirstWriteWins:
            resolvedChange = resolveFirstWriteWins(conflict);
            break;
            
        case ResolutionStrategy::PriorityBased:
            resolvedChange = resolvePriorityBased(conflict);
            break;
            
        case ResolutionStrategy::Manual:
            emit manualResolutionRequired(conflict);
            return resolvedConflict; // Return unresolved
    }
    
    resolvedConflict.resolved = true;
    addToHistory(resolvedConflict);
    
    // Remove from active conflicts
    m_activeConflicts.removeAll(conflict);
    
    emit conflictResolved(resolvedConflict, resolvedChange);
    
    qInfo() << "ConflictResolver: Conflict resolved at position" 
            << conflict.position.x << "," << conflict.position.y
            << "using strategy" << static_cast<int>(m_strategy);
    
    return resolvedConflict;
}

void ConflictResolver::addPendingChange(const TileChange& change, uint32_t peerId)
{
    PendingChange pendingChange;
    pendingChange.change = change;
    pendingChange.peerId = peerId;
    pendingChange.timestamp = QDateTime::currentDateTime();
    
    m_pendingChanges[change.position] = pendingChange;
    
    qDebug() << "ConflictResolver: Added pending change at position" 
             << change.position.x << "," << change.position.y 
             << "from peer" << peerId;
}

void ConflictResolver::removePendingChange(const Position& position)
{
    if (m_pendingChanges.remove(position) > 0) {
        qDebug() << "ConflictResolver: Removed pending change at position" 
                 << position.x << "," << position.y;
    }
}

bool ConflictResolver::hasPendingChange(const Position& position) const
{
    return m_pendingChanges.contains(position);
}

void ConflictResolver::clearConflictHistory()
{
    m_conflictHistory.clear();
    qInfo() << "ConflictResolver: Conflict history cleared";
}

void ConflictResolver::onConflictTimeout()
{
    QDateTime now = QDateTime::currentDateTime();
    
    // Check for timed out pending changes
    auto it = m_pendingChanges.begin();
    while (it != m_pendingChanges.end()) {
        if (it.value().timestamp.msecsTo(now) > m_conflictTimeoutMs) {
            qDebug() << "ConflictResolver: Pending change timed out at position" 
                     << it.key().x << "," << it.key().y;
            it = m_pendingChanges.erase(it);
        } else {
            ++it;
        }
    }
    
    // Check for timed out active conflicts
    auto conflictIt = m_activeConflicts.begin();
    while (conflictIt != m_activeConflicts.end()) {
        if (conflictIt->timestamp.msecsTo(now) > m_conflictTimeoutMs) {
            qWarning() << "ConflictResolver: Active conflict timed out at position" 
                       << conflictIt->position.x << "," << conflictIt->position.y;
            
            // Auto-resolve using current strategy
            ConflictInfo resolvedConflict = resolveConflict(*conflictIt);
            conflictIt = m_activeConflicts.erase(conflictIt);
        } else {
            ++conflictIt;
        }
    }
}

TileChange ConflictResolver::resolveLastWriteWins(const ConflictInfo& conflict)
{
    // The conflicting change (more recent) wins
    qInfo() << "ConflictResolver: Resolving with LastWriteWins - conflicting change wins";
    return conflict.conflictingChange;
}

TileChange ConflictResolver::resolveFirstWriteWins(const ConflictInfo& conflict)
{
    // The original change (first) wins
    qInfo() << "ConflictResolver: Resolving with FirstWriteWins - original change wins";
    return conflict.originalChange;
}

TileChange ConflictResolver::resolvePriorityBased(const ConflictInfo& conflict)
{
    int originalPriority = getUserPriority(conflict.originalPeerId);
    int conflictingPriority = getUserPriority(conflict.conflictingPeerId);
    
    if (originalPriority > conflictingPriority) {
        qInfo() << "ConflictResolver: Resolving with PriorityBased - original peer has higher priority";
        return conflict.originalChange;
    } else if (conflictingPriority > originalPriority) {
        qInfo() << "ConflictResolver: Resolving with PriorityBased - conflicting peer has higher priority";
        return conflict.conflictingChange;
    } else {
        // Equal priority, fall back to last write wins
        qInfo() << "ConflictResolver: Equal priority, falling back to LastWriteWins";
        return conflict.conflictingChange;
    }
}

bool ConflictResolver::isConflicting(const TileChange& change1, const TileChange& change2)
{
    // Changes are conflicting if they affect the same position
    // and have different tile data
    if (change1.position != change2.position) {
        return false;
    }
    
    // Compare tile data to see if they're actually different
    return change1.newTileDataOtbm != change2.newTileDataOtbm;
}

int ConflictResolver::getUserPriority(uint32_t peerId)
{
    // Return user priority, default to 0 if not set
    return m_userPriorities.value(peerId, 0);
}

void ConflictResolver::addToHistory(const ConflictInfo& conflict)
{
    m_conflictHistory.append(conflict);
    
    // Limit history size to prevent memory growth
    const int maxHistorySize = 1000;
    if (m_conflictHistory.size() > maxHistorySize) {
        m_conflictHistory.removeFirst();
    }
}

} // namespace network
} // namespace core
} // namespace RME