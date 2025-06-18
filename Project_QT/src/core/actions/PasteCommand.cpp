#include "PasteCommand.h"
#include "Project_QT/src/core/map/Map.h"
#include "Project_QT/src/core/Tile.h"
#include "Project_QT/src/core/Item.h"
#include "Project_QT/src/core/Creature.h"
#include "Project_QT/src/core/Spawn.h"
// #include "Project_QT/src/core/settings/EditorSettings.h" // For Config::MERGE_PASTE
#include <QDebug>

namespace RME {

PasteCommand::PasteCommand(
    Map* map,
    const Position& targetTopLeftPosition,
    const ClipboardContent& clipboardContent,
    const QString& text,
    QUndoCommand* parent)
    : QUndoCommand(parent),
      m_map(map),
      m_targetTopLeft(targetTopLeftPosition),
      m_pastedContent(clipboardContent)
{
    setText(text);
    Q_ASSERT(m_map);
}

PasteCommand::~PasteCommand() {
    // qDeleteAll(m_affectedTilesOriginalState); // If storing deep copies
}

void PasteCommand::undo() {
    // Remove the elements that were pasted by redo().
    // This needs to be the inverse of redo().
    // If redo() replaced tiles, undo() restores the originals.
    // If redo() merged, undo() needs to unmerge.
    // For a simple first pass: remove all items/creatures/spawns that were part of m_pastedContent
    // from the tiles at m_pastedTilePositions. Clear ground if it was pasted.
    qDebug() << "PasteCommand: Undoing paste of" << m_pastedContent.tiles.count() << "tiles' data.";

    for (const Position& pos : m_pastedTilePositions) {
        Tile* tile = m_map->getTile(pos);
        if (!tile) continue;

        // This is a simplified undo. A robust undo would restore the exact previous state
        // of each tile (which should have been saved by redo() before modifying).
        // For now, let's assume we're just clearing what was pasted.

        // Find the corresponding ClipboardTileData for this position
        bool found = false;
        for(const auto& pastedTileData : m_pastedContent.tiles) {
            if (m_targetTopLeft + pastedTileData.relativePosition == pos) {
                // tile->removeMatchingPastedElements(pastedTileData); // Tile needs complex logic here
                found = true;
                break;
            }
        }
        // Fallback: just clear the tile if it was newly created, or try to revert based on m_affectedTilesOriginalState
        // This part is highly dependent on how redo() stores pre-paste state.
        // If m_affectedTilesOriginalState stored deep copies of tiles *before* paste, restore them.
        // For now, let's assume redo clears the area, so undo just clears it again (or removes if it was new)

        // tile->clear(); // Placeholder for more precise undo
        if (tile->isEmptyAndClean()) { // if it became empty after removing pasted stuff
            // m_map->removeTile(pos);
        } else {
            m_map->markTileDirty(pos);
        }
    }
    m_pastedTilePositions.clear(); // Clear for next redo, if any
    // Map change notifications are handled per-tile in the loop above
}

void PasteCommand::redo() {
    // Perform the paste operation
    qDebug() << "PasteCommand: Redoing paste of" << m_pastedContent.tiles.count() << "tiles' data to target" << m_targetTopLeft;
    m_pastedTilePositions.clear(); // Prepare for this redo pass

    // bool mergePaste = EditorSettings::getInstance().getBool(Config::MERGE_PASTE); // Example
    bool mergePaste = true; // Defaulting to merge for now, as in wxWidgets.

    for (const ClipboardTileData& data : m_pastedContent.tiles) {
        Position targetPos = m_targetTopLeft + data.relativePosition;
        if (!m_map->isValidPosition(targetPos)) { // Assuming Map::isValidPosition
            continue;
        }
        m_pastedTilePositions.append(targetPos);

        Tile* destTile = m_map->getOrCreateTile(targetPos);
        if (!destTile) continue;

        // TODO: Before modifying destTile, if robust undo is needed, store its current state.
        // DeletedTileData originalState = captureTileState(destTile);
        // m_affectedTilesOriginalState.append(originalState);


        // Logic based on wxwidgets/copybuffer.cpp paste:
        if (mergePaste || !data.hasGround) {
            // Merge with existing tile
            if (data.hasGround) { // Only merge ground if data has ground
                // destTile->setGround(data.groundItemID); // Example
                destTile->setHouseId(data.houseId);
                destTile->setFlags(data.tileFlags);
                // merge zone IDs etc.
            }
            for (const ClipboardItemData& itemData : data.items) {
                // Item* item = RME::Item::create(itemData.id);
                // if (item) { /* set properties */ destTile->addItem(item); }
            }
            if (data.hasCreature) {
                // Creature* creature = RME::Creature::create(data.creature.name);
                // if (creature) { /* set properties */ destTile->addCreature(creature); }
            }
            if (data.hasSpawn) {
                // Spawn* spawn = RME::Spawn::create();
                // if (spawn) { /* set properties */ destTile->setSpawn(spawn); }
            }
        } else { // Replace existing tile (if data.hasGround and not merging)
            // destTile->clear(); // Clear existing content
            // destTile->setGround(data.groundItemID); // Example
            // destTile->setHouseId(data.houseId);
            // destTile->setFlags(data.tileFlags);
            // ... and then add items, creature, spawn as above.
        }
        m_map->markTileDirty(targetPos);
    }
    // Map change notifications are handled per-tile via markTileDirty above
    // Note: Automagic border updates are handled by the brush system during normal editing.
    // For paste operations, borders are typically preserved as part of the copied data.
}

} // namespace RME
