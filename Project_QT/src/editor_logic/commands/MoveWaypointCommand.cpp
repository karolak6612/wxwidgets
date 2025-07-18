#include "commands/MoveWaypointCommand.h"
#include "core/waypoints/WaypointManager.h" // RME::core::WaypointManager
#include <QDebug>
#include <QObject> // For QObject::tr

namespace RME {
namespace core {
namespace actions {

MoveWaypointCommand::MoveWaypointCommand(
    RME::core::WaypointManager* waypointManager,
    const QString& waypointName,
    const RME::core::Position& oldPosition,
    const RME::core::Position& newPosition,
    RME::core::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Move Waypoint"), parent),
    m_waypointManager(waypointManager),
    m_waypointName(waypointName),
    m_oldPosition(oldPosition),
    m_newPosition(newPosition) {
    // Set initial text for the command. This will be displayed in undo/redo views.
    setText(QObject::tr("Move Waypoint '%1' to (%2,%3,%4)")
        .arg(m_waypointName)
        .arg(m_newPosition.x)
        .arg(m_newPosition.y)
        .arg(m_newPosition.z));
}

void MoveWaypointCommand::undo() {
    if (!validateMembers() || !m_waypointManager) {
        setErrorText("undo waypoint move");
        return;
    }

    bool success = m_waypointManager->updateWaypointPosition(m_waypointName, m_oldPosition);
    if (!success) {
        qWarning() << "MoveWaypointCommand::undo: Failed to move waypoint" << m_waypointName << "back to original position" << m_oldPosition.toString() << ". Waypoint might not exist anymore.";
        // Consider how to handle this failure. If the waypoint was deleted by another command,
        // this command should ideally be invalidated or handled by the command stack.
        // For now, just log. The state might become inconsistent if this happens.
    }
    // The text for "Undo X" is often set automatically by QUndoView if not explicitly changed here.
    // If explicit control is needed:
    // setText(QObject::tr("Undo Move Waypoint '%1' from (%2,%3,%4) to (%3,%4,%5)")
    //     .arg(m_waypointName)...);
}

void MoveWaypointCommand::redo() {
    if (!validateMembers() || !m_waypointManager) {
        setErrorText("redo waypoint move");
        return;
    }

    bool success = m_waypointManager->updateWaypointPosition(m_waypointName, m_newPosition);
    if (!success) {
        qWarning() << "MoveWaypointCommand::redo: Failed to move waypoint" << m_waypointName << "to new position" << m_newPosition.toString() << ". Waypoint might not exist.";
        // Similar to undo, if waypoint doesn't exist, command might be stale.
    }
    // Text was set in constructor for the initial redo.
    // If mergeWith updates m_newPosition, it should also update the text.
}

bool MoveWaypointCommand::mergeWith(const QUndoCommand* command) {
    if (command->id() != this->id()) { // Check if it's also a MoveWaypointCommand
        return false;
    }
    const MoveWaypointCommand* nextCommand = static_cast<const MoveWaypointCommand*>(command);

    // Merge if the next command moves the *same* waypoint.
    if (m_waypointName == nextCommand->m_waypointName) {
        // The new position for the merged command becomes the target of the *next* command.
        // The old position of *this* command remains the true starting point of the entire sequence.
        m_newPosition = nextCommand->m_newPosition;

        // Update the command text to reflect the final position of the merged move.
        setText(QObject::tr("Move Waypoint '%1' to (%2,%3,%4)")
            .arg(m_waypointName)
            .arg(m_newPosition.x)
            .arg(m_newPosition.y)
            .arg(m_newPosition.z));
        return true;
    }
    return false;
}

} // namespace actions
} // namespace core
} // namespace RME
