#include "editor_logic/commands/DeleteCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/selection/SelectionManager.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/Item.h"    // For Tile methods that might take/return Item
#include "core/Spawn.h"   // For Tile methods related to Spawn
#include "core/Creature.h"// For Tile methods related to Creature

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT

namespace RME {
namespace core {
namespace actions {

DeleteCommand::DeleteCommand(
    RME::core::Map* map,
    RME::core::selection::SelectionManager* selectionManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QString(), parent),
    m_map(map),
    m_selectionManager(selectionManager),
    m_hadSelectionToDelete(false)
{
    if (!m_map || !m_selectionManager) {
        qWarning("DeleteCommand: Initialization with null map or selection manager.");
        setErrorText("Delete");
        return;
    }
    // Text set in redo based on whether action is performed.
}

void DeleteCommand::redo() {
    // Capture selection before modification
    m_previouslySelectedTiles = m_selectionManager->getCurrentSelectedTilesList();

    if (m_previouslySelectedTiles.isEmpty()) {
        m_hadSelectionToDelete = false;
        setText(QObject::tr("Delete (nothing selected)"));
        return;
    }

    m_hadSelectionToDelete = true;
    m_originalTileData.clear(); // Clear any previous state if redo is called multiple times

    for (RME::core::Tile* tile : m_previouslySelectedTiles) {
        if (!tile) continue;
        RME::core::Position pos = tile->getPosition();
        // Store a deep copy of the tile's state BEFORE clearing it
        m_originalTileData.insert(pos, RME::core::data_transfer::TileData::fromTile(tile));

        // Clear the tile's contents on the actual map
        tile->setGround(nullptr);
        tile->clearItems(); // Assumes this deletes/removes all items
        tile->setSpawn(nullptr);
        tile->setCreature(nullptr);

        // Notify that the tile has changed
        notifyMapChanged(pos);
    }

    // After deleting contents, the selection should be cleared.
    m_selectionManager->clearSelectionInternal();

    setText(QObject::tr("Delete Selection (%1 tile(s))").arg(m_previouslySelectedTiles.size()));
}

void DeleteCommand::undo() {
    if (!m_hadSelectionToDelete) {
        setText(QObject::tr("Undo Delete (no action taken)"));
        return; // Nothing was deleted by redo(), so nothing to undo.
    }

    for (auto it = m_originalTileData.constBegin(); it != m_originalTileData.constEnd(); ++it) {
        const RME::core::Position& pos = it.key();
        const RME::core::data_transfer::TileData& dataToRestore = it.value();

        RME::core::Tile* tileOnMap = m_map->getOrCreateTile(pos); // Ensure tile exists
        if (tileOnMap) {
            dataToRestore.applyToTile(tileOnMap); // Restore contents
            notifyMapChanged(pos);
        }
    }

    // Restore the selection state
    m_selectionManager->setSelectedTilesInternal(m_previouslySelectedTiles);

    setText(QObject::tr("Undo Delete Selection (%1 tile(s))").arg(m_previouslySelectedTiles.size()));
}

} // namespace actions
} // namespace core
} // namespace RME
