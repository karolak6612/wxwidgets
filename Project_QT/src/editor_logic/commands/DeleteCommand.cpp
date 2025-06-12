#include "commands/DeleteCommand.h"
#include "core/map/Map.h"
#include "core/selection/SelectionManager.h"
#include "core/Tile.h"           // For RME::Tile
#include <QDebug>                // For qWarning (optional)
#include <QList>                 // For QList<RME::Tile*>

namespace RME_COMMANDS {

DeleteCommand::DeleteCommand(
    RME::Map* map,
    RME::SelectionManager* selectionManager,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    m_selectionManager(selectionManager) {
    setText(QObject::tr("Delete Selection")); // Initial text
}

DeleteCommand::~DeleteCommand() {
    // m_deletedTiles unique_ptrs will auto-delete.
}

void DeleteCommand::undo() {
    if (!m_map) {
        qWarning("DeleteCommand::undo(): Map is null.");
        return;
    }
    if (m_deletedTiles.isEmpty()) { // Nothing was deleted, or undo already processed.
        return;
    }

    // QList<RME::Tile*> restoredTilesForSelection; // Optional: for re-selecting after undo
    for (auto it = m_deletedTiles.begin(); it != m_deletedTiles.end(); ++it) {
        const RME::Position& pos = it.key();
        std::unique_ptr<RME::Tile>& originalTileState = it.value();

        if (originalTileState) { // Should always be true if redo stored something
            // Assumes Map::setTile takes ownership of the unique_ptr.
            m_map->setTile(pos, std::move(originalTileState));
            // RME::Tile* liveTile = m_map->getTile(pos);
            // if (liveTile) {
            //     restoredTilesForSelection.append(liveTile);
            // }
        }
        m_map->notifyTileChanged(pos);
    }
    // m_deletedTiles unique_ptrs are now invalid (moved from). It will be repopulated by the next redo().

    // Optionally, re-select the restored tiles
    // if (m_selectionManager && !m_selectedPositionsForUndo.isEmpty()) {
    //     m_selectionManager->startSelectionChange();
    //     for (const RME::Position& pos : m_selectedPositionsForUndo) {
    //         RME::Tile* tile = m_map->getTile(pos);
    //         if (tile) m_selectionManager->addSelectedTile(tile); // Assuming addSelectedTile exists
    //     }
    //     m_selectionManager->finishSelectionChange("Undo Delete");
    // }
    // m_selectedPositionsForUndo.clear(); // Clear after use
    setText(QObject::tr("Undo Delete %n tile(s)", "", m_selectedPositionsForUndo.count()));
}

void DeleteCommand::redo() {
    if (!m_map || !m_selectionManager) {
        qWarning("DeleteCommand::redo(): Map or SelectionManager is null.");
        return;
    }

    // Get selected tiles from SelectionManager. This should be a copy
    // or a list of positions, as the tiles themselves will be removed from the map.
    // Assuming getSelectedTiles() returns a list/set of tile pointers that are currently on the map.
    const QSet<RME::Tile*>& selectedTiles = m_selectionManager->getSelectedTiles();

    if (selectedTiles.isEmpty()) {
        setText(QObject::tr("Delete (nothing selected)"));
        return; // Nothing to do
    }

    m_deletedTiles.clear();
    m_selectedPositionsForUndo.clear();

    for (RME::Tile* tile : selectedTiles) {
        if (tile) {
            m_deletedTiles.insert(tile->getPosition(), tile->deepCopy());
            m_selectedPositionsForUndo.append(tile->getPosition()); // Store pos for potential re-selection on undo

            // To "delete" the tile, we replace it with nullptr using the conceptual setTile method.
            // This assumes BaseMap::setTile(pos, nullptr) correctly removes/empties the tile.
            // Using setTile with nullptr to be consistent with BrushStrokeCommand's removal.
            m_map->setTile(tile->getPosition(), nullptr);
            m_map->notifyTileChanged(tile->getPosition());
        }
    }

    m_selectionManager->clear();
    setText(QObject::tr("Delete %n tile(s)", "", m_deletedTiles.count()));
}

} // namespace RME_COMMANDS
