#include "editor_logic/commands/SetHouseExitCommand.h"
#include "core/houses/House.h"
#include "core/map/Map.h" // For controller->getMap()->notifyTileChanged
#include "core/editor/EditorControllerInterface.h"
#include "core/Tile.h" // For checking tile flags if needed, though House::setExit handles it

#include <QObject> // For tr()
#include <QDebug>  // For Q_ASSERT, qWarning

namespace RME_COMMANDS {

SetHouseExitCommand::SetHouseExitCommand(
    RME::core::houses::House* house,
    const RME::core::Position& newExitPos,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_house(house),
    m_controller(controller),
    m_newExitPos(newExitPos)
{
    Q_ASSERT(m_house);
    Q_ASSERT(m_controller);
    Q_ASSERT(m_controller->getMap()); // Map is needed for notifications

    // Capture the old exit position *before* any change is made by redo()
    m_oldExitPos = m_house->getExitPos();

    // Set initial text (redo action description)
    if (m_newExitPos.isValid()) {
        setText(QObject::tr("Set House %1 Exit to (%2)").arg(m_house->getId()).arg(m_newExitPos.toString()));
    } else {
        setText(QObject::tr("Clear House %1 Exit").arg(m_house->getId()));
    }
}

void SetHouseExitCommand::redo() {
    if (!m_house || !m_controller || !m_controller->getMap()) {
        qWarning("SetHouseExitCommand::redo: Invalid members.");
        // Set text to indicate error or that command is invalid.
        setText(QObject::tr("Set House Exit (Error)"));
        return;
    }
    RME::core::Map* map = m_controller->getMap();

    // The House::setExit method already handles unflagging old exit tile and flagging new one.
    // It also updates m_house->m_exitPos internally.
    // m_oldExitPos was captured at construction (which was house's exit *before* this command's first redo).

    m_house->setExit(m_newExitPos);

    // Notify changes for both old and new exit positions if they were valid and different.
    if (m_oldExitPos.isValid() && m_oldExitPos != m_newExitPos) {
        map->notifyTileChanged(m_oldExitPos);
    }
    if (m_newExitPos.isValid()) { // Always notify new exit if valid, even if same as old (in case flag was wrong)
        map->notifyTileChanged(m_newExitPos);
    }

    // Update text to reflect redone action (already set in constructor for first redo, but good to ensure)
    if (m_newExitPos.isValid()) {
        setText(QObject::tr("Set House %1 Exit to (%2)").arg(m_house->getId()).arg(m_newExitPos.toString()));
    } else {
        setText(QObject::tr("Clear House %1 Exit").arg(m_house->getId()));
    }
}

void SetHouseExitCommand::undo() {
    if (!m_house || !m_controller || !m_controller->getMap()) {
        qWarning("SetHouseExitCommand::undo: Invalid members.");
        return;
    }
    RME::core::Map* map = m_controller->getMap();

    // The current exit of the house is m_newExitPos (set by redo).
    // We want to revert it to m_oldExitPos.
    m_house->setExit(m_oldExitPos);

    // Notify changes for both positions as their flags might have flipped.
    if (m_newExitPos.isValid() && m_newExitPos != m_oldExitPos) {
        map->notifyTileChanged(m_newExitPos);
    }
    if (m_oldExitPos.isValid()) { // Always notify old exit if valid, even if same as new (in case flag was wrong)
        map->notifyTileChanged(m_oldExitPos);
    }

    // Update text for undo stack
    setText(QObject::tr("Undo Set House %1 Exit").arg(m_house->getId()));
}

} // namespace RME_COMMANDS
