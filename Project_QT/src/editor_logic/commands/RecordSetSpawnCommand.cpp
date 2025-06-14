#include "editor_logic/commands/RecordSetSpawnCommand.h"
#include "core/Tile.h"
#include "core/Spawn.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h"


#include <QObject>
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

RecordSetSpawnCommand::RecordSetSpawnCommand(
    RME::core::Tile* tile,
    std::unique_ptr<RME::core::Spawn> newSpawn,
    std::unique_ptr<RME::core::Spawn> oldSpawn,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_tile(tile),
    m_controller(controller)
{
    Q_ASSERT(m_tile);
    Q_ASSERT(m_controller);

    // Store deep copies of the passed states.
    // The command now owns these copies.
    m_spawnStateForRedo = newSpawn ? newSpawn->deepCopy() : nullptr;
    m_spawnStateForUndo = oldSpawn ? oldSpawn->deepCopy() : nullptr;

    m_tilePosition = m_tile->getPosition();

    // Initial text will be set by the first redo() call.
    // If the command is pushed directly onto the stack and redo() isn't called immediately,
    // consider setting a default text here or ensure redo() is always called by the command framework.
    if (m_spawnStateForRedo) {
         setText(QObject::tr("Set Spawn (Radius: %1) at (%2,%3,%4)")
            .arg(m_spawnStateForRedo->getRadius())
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    } else {
        setText(QObject::tr("Clear Spawn at (%1,%2,%3)")
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    }
}

void RecordSetSpawnCommand::undo() {
    if (!m_tile) {
        qWarning("RecordSetSpawnCommand::undo: Tile pointer is null.");
        return;
    }
    if (!m_controller || !m_controller->getMap()) {
        qWarning("RecordSetSpawnCommand::undo: Controller or map is null.");
        return;
    }

    m_tile->setSpawn(m_spawnStateForUndo ? m_spawnStateForUndo->deepCopy() : nullptr);

    m_controller->getMap()->notifyTileChanged(m_tilePosition);

    // Update text to reflect the undone action, possibly indicating what was restored.
    if (m_spawnStateForUndo) {
        setText(QObject::tr("Undo Set Spawn (Restored Radius: %1) at (%2,%3,%4)")
            .arg(m_spawnStateForUndo->getRadius())
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    } else {
        setText(QObject::tr("Undo Clear Spawn (Restored Nothing) at (%1,%2,%3)")
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    }
}

void RecordSetSpawnCommand::redo() {
    if (!m_tile) {
        qWarning("RecordSetSpawnCommand::redo: Tile pointer is null.");
        return;
    }
     if (!m_controller || !m_controller->getMap()) {
        qWarning("RecordSetSpawnCommand::redo: Controller or map is null.");
        return;
    }

    m_tile->setSpawn(m_spawnStateForRedo ? m_spawnStateForRedo->deepCopy() : nullptr);

    m_controller->getMap()->notifyTileChanged(m_tilePosition);

    if (m_spawnStateForRedo) {
         setText(QObject::tr("Set Spawn (Radius: %1) at (%2,%3,%4)")
            .arg(m_spawnStateForRedo->getRadius())
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    } else {
        setText(QObject::tr("Clear Spawn at (%1,%2,%3)")
            .arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
    }
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
