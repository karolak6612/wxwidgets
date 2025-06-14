#include "editor_logic/commands/SetHouseExitCommand.h"
#include "../../core/houses/House.h" // Adjusted path
#include "../../core/map/Map.h"      // Adjusted path
#include <QDebug>          // For qWarning if needed
#include <QString>         // For QString

namespace RME {
namespace editor_logic {
namespace commands {

// Ensure RME namespace is used or members are fully qualified if not within a RME namespace block.
// The provided code doesn't use a namespace block for the command implementation,
// so RME::core::houses::House, RME::Map, RME::core::Position will be used.

SetHouseExitCommand::SetHouseExitCommand(RME::core::houses::House* house,
                                       const RME::core::Position& newExitPos,
                                       RME::Map* map,
                                       QUndoCommand* parent)
    : QUndoCommand(parent),
      m_house(house),
      m_map(map),
      m_newExitPos(newExitPos) {
    if (!m_house) {
        qWarning("SetHouseExitCommand: House pointer is null.");
        // Potentially set an error state or make command invalid
        setText("Invalid Set House Exit Command (null house)");
        return;
    }
    if (!m_map) {
        qWarning("SetHouseExitCommand: Map pointer is null.");
        setText("Invalid Set House Exit Command (null map)");
        return;
    }
    m_oldExitPos = m_house->getExitPos(); // Store current exit before any changes

    // Set command text for undo/redo stack display
    // Assuming m_house->getName() returns QString or compatible.
    // And Position members x, y, z are suitable for arg().
    setText(QString("Set House '%1' Exit to (%2, %3, %4)")
                .arg(m_house->getName()) // Assuming getName() is available and returns QString
                .arg(m_newExitPos.x)
                .arg(m_newExitPos.y)
                .arg(m_newExitPos.z));
}

void SetHouseExitCommand::undo() {
    if (!m_house || !m_map) {
        qWarning("SetHouseExitCommand::undo: Invalid command state (null house or map).");
        return;
    }

    RME::core::Position previouslyAppliedExit = m_newExitPos; // The exit that redo() set

    m_house->setExit(m_oldExitPos); // Assumes House::setExit handles map tile updates internally or via signals

    // Notify map about tile changes for redraw
    // These notifications are crucial if House::setExit doesn't automatically signal the main map view
    // or if direct map interaction is preferred for commands.
    if (m_oldExitPos.isValid()) {
        m_map->notifyTileChanged(m_oldExitPos);
    }
    if (previouslyAppliedExit.isValid() && previouslyAppliedExit != m_oldExitPos) {
        m_map->notifyTileChanged(previouslyAppliedExit);
    }
    m_map->setChanged(true); // Mark map as changed
}

void SetHouseExitCommand::redo() {
    if (!m_house || !m_map) {
        qWarning("SetHouseExitCommand::redo: Invalid command state (null house or map).");
        return;
    }

    RME::core::Position exitBeforeThisRedo = m_oldExitPos; // The exit that undo() might have set (or initial state)

    m_house->setExit(m_newExitPos); // Assumes House::setExit handles map tile updates

    // Notify map about tile changes for redraw
    if (m_newExitPos.isValid()) {
        m_map->notifyTileChanged(m_newExitPos);
    }
    if (exitBeforeThisRedo.isValid() && exitBeforeThisRedo != m_newExitPos) {
        m_map->notifyTileChanged(exitBeforeThisRedo);
    }
    m_map->setChanged(true); // Mark map as changed
}

// Optional merge logic (example, may need adjustment)
// int SetHouseExitCommand::id() const {
//     // Return a unique ID for this command type, plus house ID for merging
//     return 1001; // Arbitrary ID for SetHouseExitCommand
// }

// bool SetHouseExitCommand::mergeWith(const QUndoCommand *other) {
//     // if (other->id() != id()) {
//     //     return false;
//     // }
//     // const SetHouseExitCommand *otherCmd = static_cast<const SetHouseExitCommand*>(other);
//     // if (otherCmd->m_house != m_house) {
//     //     return false;
//     // }
//     // m_newExitPos = otherCmd->m_newExitPos; // The new command's exit becomes the final one
//     // setText(QString("Set House '%1' Exit to (%2, %3, %4)") // Update text
//     //             .arg(m_house->getName())
//     //             .arg(m_newExitPos.x)
//     //             .arg(m_newExitPos.y)
//     //             .arg(m_newExitPos.z));
//     // return true;
//     return false; // Default to no merge for simplicity now
// }

} // namespace commands
} // namespace editor_logic
} // namespace RME
