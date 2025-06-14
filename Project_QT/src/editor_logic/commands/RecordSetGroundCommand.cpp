#include "editor_logic/commands/RecordSetGroundCommand.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h" // For map->notifyTileChanged
#include "core/assets/ItemDatabase.h" // For getting item name for text
#include "core/assets/AssetManager.h" // For getting ItemDatabase

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace editor_logic {
namespace commands {

RecordSetGroundCommand::RecordSetGroundCommand(
    RME::core::Tile* tile,
    std::unique_ptr<RME::core::Item> newGround,
    std::unique_ptr<RME::core::Item> oldGround,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_tile(tile),
    m_controller(controller)
{
    Q_ASSERT(m_tile);
    Q_ASSERT(m_controller);
    Q_ASSERT(m_controller->getAssetManager()); // Needed for item names

    // Store deep copies of the passed states. The command now owns these copies.
    m_groundStateForRedo = newGround ? newGround->deepCopy() : nullptr;
    m_groundStateForUndo = oldGround ? oldGround->deepCopy() : nullptr;

    m_tilePosition = m_tile->getPosition();

    // Set initial command text (redo action description)
    if (m_groundStateForRedo) {
        const auto& itemDb = m_controller->getAssetManager()->getItemDatabase();
        QString itemName = itemDb.getItemData(m_groundStateForRedo->getID()).name;
        if (itemName.isEmpty()) itemName = QString("ID: %1").arg(m_groundStateForRedo->getID());
        m_commandTextBase = QObject::tr("Set Ground (%1)").arg(itemName);
    } else {
        m_commandTextBase = QObject::tr("Clear Ground");
    }
    setText(m_commandTextBase + QObject::tr(" at (%1,%2,%3)").arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
}

void RecordSetGroundCommand::undo() {
    if (!m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("RecordSetGroundCommand::undo: Tile, controller, or map is null.");
        return;
    }

    m_tile->setGround(m_groundStateForUndo ? m_groundStateForUndo->deepCopy() : nullptr);
    m_controller->getMap()->notifyTileChanged(m_tilePosition);

    // Update text to reflect the undone action
    setText(QObject::tr("Undo: ") + m_commandTextBase + QObject::tr(" at (%1,%2,%3)").arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
}

void RecordSetGroundCommand::redo() {
    if (!m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("RecordSetGroundCommand::redo: Tile, controller, or map is null.");
        return;
    }

    m_tile->setGround(m_groundStateForRedo ? m_groundStateForRedo->deepCopy() : nullptr);
    m_controller->getMap()->notifyTileChanged(m_tilePosition);

    // Set text to reflect the redone action
    setText(m_commandTextBase + QObject::tr(" at (%1,%2,%3)").arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
