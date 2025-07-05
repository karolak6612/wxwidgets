#include "commands/RenameWaypointCommand.h"
#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include <QDebug>

namespace RME {
namespace core {
namespace actions {

RenameWaypointCommand::RenameWaypointCommand(
    RME::core::waypoints::WaypointManager* manager,
    const QString& oldName,
    const QString& newName,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Rename Waypoint"), parent),
    m_waypointManager(manager),
    m_oldName(oldName),
    m_newName(newName),
    m_oldWaypointExisted(false),
    m_newNameConflicted(false) {
    
    if (!m_waypointManager) {
        qWarning("RenameWaypointCommand: WaypointManager is null");
        return;
    }
    
    if (m_oldName.isEmpty() || m_newName.isEmpty()) {
        qWarning("RenameWaypointCommand: Old or new name is empty");
        return;
    }
    
    if (m_oldName == m_newName) {
        qWarning("RenameWaypointCommand: Old and new names are the same");
        return;
    }
    
    // Check if old waypoint exists
    RME::core::waypoints::Waypoint* oldWaypoint = m_waypointManager->getWaypoint(m_oldName);
    if (oldWaypoint) {
        m_oldWaypointExisted = true;
        m_waypointPosition = oldWaypoint->getPosition();
        
        // Check if new name conflicts with existing waypoint
        RME::core::waypoints::Waypoint* conflictWaypoint = m_waypointManager->getWaypoint(m_newName);
        if (conflictWaypoint) {
            m_newNameConflicted = true;
            setText(QObject::tr("Rename waypoint '%1' to '%2' (replaces existing)").arg(m_oldName, m_newName));
        } else {
            setText(QObject::tr("Rename waypoint '%1' to '%2'").arg(m_oldName, m_newName));
        }
    } else {
        setText(QObject::tr("Rename waypoint '%1' to '%2' (source not found)").arg(m_oldName, m_newName));
    }
}

void RenameWaypointCommand::redo() {
    if (!validateMembers() || !m_waypointManager) {
        setErrorText("redo waypoint rename");
        return;
    }
    
    if (!m_oldWaypointExisted) {
        qWarning() << "RenameWaypointCommand::redo: Old waypoint" << m_oldName << "does not exist";
        return;
    }
    
    // Get the old waypoint
    RME::core::waypoints::Waypoint* oldWaypoint = m_waypointManager->getWaypoint(m_oldName);
    if (!oldWaypoint) {
        qWarning() << "RenameWaypointCommand::redo: Old waypoint" << m_oldName << "not found during redo";
        return;
    }
    
    // Store position for consistency check
    RME::core::Position currentPosition = oldWaypoint->getPosition();
    if (currentPosition != m_waypointPosition) {
        qWarning() << "RenameWaypointCommand::redo: Waypoint position changed since command creation";
        m_waypointPosition = currentPosition; // Update stored position
    }
    
    // Create new waypoint with new name at same position
    auto newWaypoint = std::make_unique<RME::core::waypoints::Waypoint>(m_newName, m_waypointPosition);
    
    // Add new waypoint (this may replace existing waypoint with new name)
    bool addSuccess = m_waypointManager->addWaypoint(std::move(newWaypoint));
    if (!addSuccess) {
        qWarning() << "RenameWaypointCommand::redo: Failed to add waypoint with new name" << m_newName;
        return;
    }
    
    // Remove old waypoint
    bool removeSuccess = m_waypointManager->removeWaypoint(m_oldName);
    if (!removeSuccess) {
        qWarning() << "RenameWaypointCommand::redo: Failed to remove old waypoint" << m_oldName;
        // Try to clean up by removing the newly added waypoint
        m_waypointManager->removeWaypoint(m_newName);
        return;
    }
    
    qDebug() << "RenameWaypointCommand::redo: Renamed waypoint from" << m_oldName << "to" << m_newName;
}

void RenameWaypointCommand::undo() {
    if (!validateMembers() || !m_waypointManager) {
        setErrorText("undo waypoint rename");
        return;
    }
    
    // Get the current waypoint with new name
    RME::core::waypoints::Waypoint* newWaypoint = m_waypointManager->getWaypoint(m_newName);
    if (!newWaypoint) {
        qWarning() << "RenameWaypointCommand::undo: Waypoint with new name" << m_newName << "not found";
        return;
    }
    
    // Verify position consistency
    RME::core::Position currentPosition = newWaypoint->getPosition();
    if (currentPosition != m_waypointPosition) {
        qWarning() << "RenameWaypointCommand::undo: Waypoint position changed since rename";
        m_waypointPosition = currentPosition; // Update for consistency
    }
    
    // Create waypoint with old name at same position
    auto oldWaypoint = std::make_unique<RME::core::waypoints::Waypoint>(m_oldName, m_waypointPosition);
    
    // Add old waypoint back
    bool addSuccess = m_waypointManager->addWaypoint(std::move(oldWaypoint));
    if (!addSuccess) {
        qWarning() << "RenameWaypointCommand::undo: Failed to restore waypoint with old name" << m_oldName;
        return;
    }
    
    // Remove waypoint with new name
    bool removeSuccess = m_waypointManager->removeWaypoint(m_newName);
    if (!removeSuccess) {
        qWarning() << "RenameWaypointCommand::undo: Failed to remove waypoint with new name" << m_newName;
        // Try to clean up by removing the restored waypoint
        m_waypointManager->removeWaypoint(m_oldName);
        return;
    }
    
    qDebug() << "RenameWaypointCommand::undo: Restored waypoint name from" << m_newName << "back to" << m_oldName;
}

} // namespace actions
} // namespace core
} // namespace RME