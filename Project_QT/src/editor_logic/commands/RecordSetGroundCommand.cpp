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
namespace core {
namespace actions {

RecordSetGroundCommand::RecordSetGroundCommand(
    RME::core::Tile* tile,
    std::unique_ptr<RME::core::Item> newGround,
    std::unique_ptr<RME::core::Item> oldGround,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Set Ground"), parent),
    m_tile(tile)
{
    Q_ASSERT(m_tile);

    // Store deep copies of the passed states. The command now owns these copies.
    m_groundStateForRedo = newGround ? newGround->deepCopy() : nullptr;
    m_groundStateForUndo = oldGround ? oldGround->deepCopy() : nullptr;

    m_tilePosition = m_tile->getPosition();

    // Set initial command text (redo action description)
    if (m_groundStateForRedo) {
        const auto& itemDb = getController()->getAssetManager()->getItemDatabase();
        QString itemName = itemDb.getItemData(m_groundStateForRedo->getID()).name;
        if (itemName.isEmpty()) itemName = QString("ID: %1").arg(m_groundStateForRedo->getID());
        m_commandTextBase = QObject::tr("Set Ground (%1)").arg(itemName);
    } else {
        m_commandTextBase = QObject::tr("Clear Ground");
    }
    setText(m_commandTextBase + QObject::tr(" at (%1,%2,%3)").arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
}

void RecordSetGroundCommand::undo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("undo ground operation");
        return;
    }

    m_tile->setGround(m_groundStateForUndo ? m_groundStateForUndo->deepCopy() : nullptr);
    notifyMapChanged(m_tilePosition);
    logUndo(m_commandTextBase, m_tilePosition);
}

void RecordSetGroundCommand::redo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("redo ground operation");
        return;
    }

    m_tile->setGround(m_groundStateForRedo ? m_groundStateForRedo->deepCopy() : nullptr);
    notifyMapChanged(m_tilePosition);
    logRedo(m_commandTextBase, m_tilePosition);
}

} // namespace actions
} // namespace core
} // namespace RME
