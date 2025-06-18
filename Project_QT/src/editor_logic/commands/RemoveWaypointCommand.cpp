#include "commands/RemoveWaypointCommand.h"
#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include <QDebug>

namespace RME {
namespace core {
namespace actions {

RemoveWaypointCommand::RemoveWaypointCommand(
    RME::core::waypoints::WaypointManager* manager,
    const QString& waypointName,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_waypointManager(manager),
    m_waypointName(waypointName),
    m_waypointExisted(false) {
    
    if (!m_waypointManager) {
        qWarning("RemoveWaypointCommand: WaypointManager is null");
        return;
    }
    
    if (m_waypointName.isEmpty()) {
        qWarning("RemoveWaypointCommand: Waypoint name is empty");
        return;
    }
    
    // Check if waypoint exists and store its data for undo
    RME::core::waypoints::Waypoint* existing = m_waypointManager->getWaypoint(m_waypointName);
    if (existing) {
        m_waypointExisted = true;
        m_waypointPosition = existing->getPosition();
        // Create a copy for undo (we'll store the actual waypoint during redo)
        setText(QObject::tr("Remove waypoint '%1'").arg(m_waypointName));
    } else {
        setText(QObject::tr("Remove waypoint '%1' (not found)").arg(m_waypointName));
    }
}

void RemoveWaypointCommand::redo() {
    if (!m_waypointManager) {
        qWarning("RemoveWaypointCommand::redo: WaypointManager is null");
        return;
    }
    
    if (!m_waypointExisted) {
        qDebug() << "RemoveWaypointCommand::redo: Waypoint" << m_waypointName << "does not exist, nothing to remove";
        return;
    }
    
    // Get the waypoint before removing it (for undo)
    RME::core::waypoints::Waypoint* waypoint = m_waypointManager->getWaypoint(m_waypointName);
    if (waypoint) {
        // Create a copy of the waypoint for undo
        m_removedWaypoint = std::make_unique<RME::core::waypoints::Waypoint>(
            waypoint->getName(), waypoint->getPosition());
        
        // Remove the waypoint from the manager
        bool success = m_waypointManager->removeWaypoint(m_waypointName);
        if (success) {
            qDebug() << "RemoveWaypointCommand::redo: Removed waypoint" << m_waypointName;
        } else {
            qWarning() << "RemoveWaypointCommand::redo: Failed to remove waypoint" << m_waypointName;
            m_removedWaypoint.reset(); // Clear the stored waypoint if removal failed
        }
    } else {
        qWarning() << "RemoveWaypointCommand::redo: Waypoint" << m_waypointName << "not found during redo";
    }
}

void RemoveWaypointCommand::undo() {
    if (!m_waypointManager) {
        qWarning("RemoveWaypointCommand::undo: WaypointManager is null");
        return;
    }
    
    if (!m_removedWaypoint) {
        qDebug() << "RemoveWaypointCommand::undo: No waypoint to restore";
        return;
    }
    
    // Create a new waypoint with the stored data
    auto restoredWaypoint = std::make_unique<RME::core::waypoints::Waypoint>(
        m_removedWaypoint->getName(), m_removedWaypoint->getPosition());
    
    // Add the waypoint back to the manager
    bool success = m_waypointManager->addWaypoint(std::move(restoredWaypoint));
    if (success) {
        qDebug() << "RemoveWaypointCommand::undo: Restored waypoint" << m_waypointName;
    } else {
        qWarning() << "RemoveWaypointCommand::undo: Failed to restore waypoint" << m_waypointName;
    }
}

} // namespace actions
} // namespace core
} // namespace RME