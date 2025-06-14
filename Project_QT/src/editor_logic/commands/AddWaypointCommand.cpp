#include "commands/AddWaypointCommand.h"
#include "core/waypoints/WaypointManager.h" // RME::core::WaypointManager
#include "core/waypoints/Waypoint.h"       // RME::core::Waypoint struct/class
#include <QDebug>
#include <QObject> // For QObject::tr

namespace RME {
namespace editor_logic {
namespace commands {

AddWaypointCommand::AddWaypointCommand(
    RME::core::WaypointManager* waypointManager,
    const QString& waypointName,
    const RME::core::Position& position,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_waypointManager(waypointManager),
    m_waypointName(waypointName),
    m_position(position),
    m_replacedWaypoint(nullptr),
    m_wasReplacement(false) {
    // setText will be set in redo()
}

void AddWaypointCommand::undo() {
    if (!m_waypointManager) {
        qWarning("AddWaypointCommand::undo: WaypointManager is null.");
        return;
    }

    // Remove the added/updated waypoint
    bool removed = m_waypointManager->removeWaypoint(m_waypointName);
    if (!removed) {
        // This might happen if undo is called without a successful redo, or other error
        // It's also possible the waypoint was already removed by another operation if commands are not managed carefully.
        qWarning("AddWaypointCommand::undo: Failed to remove waypoint '%s'. It might have been already removed or name changed.", qPrintable(m_waypointName));
    }

    // If this add operation replaced an existing waypoint, restore the old one.
    if (m_wasReplacement && m_replacedWaypoint) {
        // addWaypoint handles decrementing count for the (now removed) one, and incrementing for this one.
        m_waypointManager->addWaypoint(std::move(m_replacedWaypoint));
        // m_replacedWaypoint is now null as ownership has been transferred.
    }

    // The text should ideally reflect what was undone.
    // If it was a replacement, "Undo Replace Waypoint..." might be more accurate.
    // For simplicity, using a general undo text.
    setText(QObject::tr("Undo Add/Replace Waypoint '%1'").arg(m_waypointName));
}

void AddWaypointCommand::redo() {
    if (!m_waypointManager) {
        qWarning("AddWaypointCommand::redo: WaypointManager is null.");
        return;
    }

    // Check if a waypoint with this name already exists, to handle replacement logic for undo
    RME::core::Waypoint* existingWp = m_waypointManager->getWaypoint(m_waypointName);
    if (existingWp) {
        m_wasReplacement = true;
        // Create a copy of the existing waypoint for undo.
        // This copy stores its state *before* it's implicitly removed by the addWaypoint call below.
        m_replacedWaypoint = std::make_unique<RME::core::Waypoint>(existingWp->name, existingWp->position);
        // Note: WaypointManager::addWaypoint is expected to handle the removal of 'existingWp'
        // (including decrementing its tile count) before adding the new one.
    } else {
        m_wasReplacement = false;
        m_replacedWaypoint.reset(); // Ensure no old data is kept
    }

    auto newWaypoint = std::make_unique<RME::core::Waypoint>(m_waypointName, m_position);
    bool added = m_waypointManager->addWaypoint(std::move(newWaypoint)); // This might replace an existing one

    if (added) {
        if (m_wasReplacement) {
            setText(QObject::tr("Replace Waypoint '%1' at (%2,%3,%4)")
                .arg(m_waypointName)
                .arg(m_position.x)
                .arg(m_position.y)
                .arg(m_position.z));
        } else {
            setText(QObject::tr("Add Waypoint '%1' at (%2,%3,%4)")
                .arg(m_waypointName)
                .arg(m_position.x)
                .arg(m_position.y)
                .arg(m_position.z));
        }
    } else {
        // This case implies addWaypoint itself failed (e.g., empty name, though constructor should catch this).
        // If addWaypoint failed, m_wasReplacement and m_replacedWaypoint should be reset
        // to ensure undo doesn't try to restore something that wasn't actually replaced.
        setText(QObject::tr("Add Waypoint '%1' Failed").arg(m_waypointName));
        m_wasReplacement = false;
        m_replacedWaypoint.reset();
        qWarning("AddWaypointCommand::redo: m_waypointManager->addWaypoint failed for '%s'.", qPrintable(m_waypointName));
    }
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
