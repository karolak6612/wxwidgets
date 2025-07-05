#include "editor_logic/commands/SetHouseExitCommand.h"
#include "core/houses/Houses.h" // Adjusted path
#include "core/houses/HouseData.h" // Added for HouseData
#include "core/map/Map.h"      // Adjusted path
#include <QDebug>          // For qWarning if needed
#include <QString>         // For QString

// Ensure RME namespace is used or members are fully qualified if not within a RME namespace block.
// The provided code doesn't use a namespace block for the command implementation,
// so RME::core::houses::House, RME::Map, RME::core::Position will be used.

SetHouseExitCommand::SetHouseExitCommand(quint32 houseId,
                                       const RME::core::Position& newExitPos,
                                       RME::core::houses::Houses* housesManager,
                                       RME::core::Map* map,
                                       QUndoCommand* parent)
    : BaseCommand(nullptr, QString(), parent),
      m_houseId(houseId),
      m_housesManager(housesManager),
      m_map(map),
      m_newExitPos(newExitPos) {
    if (!m_housesManager) {
        qWarning() << "SetHouseExitCommand: Houses manager pointer is null.";
        setErrorText("Set House Exit");
        return;
    }
    if (!m_map) {
        qWarning() << "SetHouseExitCommand: Map pointer is null.";
        setErrorText("Set House Exit");
        return;
    }
    
    // Get house data to store current exit
    RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
    if (!house) {
        qWarning() << "SetHouseExitCommand: House with ID" << m_houseId << "not found.";
        setText("Invalid Set House Exit Command (house not found)");
        return;
    }
    
    m_oldExitPos = house->entryPoint; // Store current exit before any changes

    // Set command text for undo/redo stack display
    setText(QString("Set House '%1' Exit to (%2, %3, %4)")
                .arg(house->name)
                .arg(m_newExitPos.x)
                .arg(m_newExitPos.y)
                .arg(m_newExitPos.z));
}

void SetHouseExitCommand::undo() {
    if (!m_housesManager || !m_map) {
        qWarning() << "SetHouseExitCommand::undo: Invalid command state (null houses manager or map).";
        return;
    }

    RME::core::Position previouslyAppliedExit = m_newExitPos; // The exit that redo() set

    // Set house exit via Houses manager
    m_housesManager->setHouseExit(m_houseId, m_oldExitPos);

    // Notify map about tile changes for redraw
    if (m_oldExitPos.isValid()) {
        m_map->notifyTileChanged(m_oldExitPos);
    }
    if (previouslyAppliedExit.isValid() && previouslyAppliedExit != m_oldExitPos) {
        m_map->notifyTileChanged(previouslyAppliedExit);
    }
    m_map->setChanged(true); // Mark map as changed
}

void SetHouseExitCommand::redo() {
    if (!m_housesManager || !m_map) {
        qWarning() << "SetHouseExitCommand::redo: Invalid command state (null houses manager or map).";
        return;
    }

    RME::core::Position exitBeforeThisRedo = m_oldExitPos; // The exit that undo() might have set (or initial state)

    // Set house exit via Houses manager
    m_housesManager->setHouseExit(m_houseId, m_newExitPos);

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
