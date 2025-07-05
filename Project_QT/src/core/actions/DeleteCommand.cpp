#include "DeleteCommand.h"
#include "Project_QT/src/core/map/Map.h"
#include "Project_QT/src/core/Tile.h"
#include "Project_QT/src/core/Item.h"
#include "Project_QT/src/core/Creature.h"
#include "Project_QT/src/core/Spawn.h"
#include <QDebug>

namespace RME {

DeleteCommand::DeleteCommand(
    Map* map,
    const QList<DeletedTileData>& itemsToDelete,
    const QString& text,
    QUndoCommand* parent)
    : QUndoCommand(parent), m_map(map), m_deletedData(itemsToDelete)
{
    setText(text);
    Q_ASSERT(m_map);
}

DeleteCommand::~DeleteCommand() {}

void DeleteCommand::undo() {
    // Re-insert the elements based on m_deletedData
    // This is essentially a paste operation of the m_deletedData
    qDebug() << "DeleteCommand: Undoing deletion of" << m_deletedData.count() << "elements' data.";
    for (const DeletedTileData& data : m_deletedData) {
        // The relativePosition in DeletedTileData here should be the original absolute position.
        // When capturing for delete, ensure Position is absolute.
        Tile* tile = m_map->getOrCreateTile(data.relativePosition); // relativePosition is absolute here
        if (!tile) continue;

        if (data.hasGround) {
            // tile->setGround(data.groundItemID); // Example, depends on how ground is set
            tile->setHouseId(data.houseId);
            tile->setFlags(data.tileFlags);
            // Restore zone IDs etc.
        } else {
            // If the ground was part of the deletion (e.g. whole tile selected),
            // then undoing means restoring it. If ground was NOT part of deletion,
            // then undoing delete should not add ground if there was none.
            // This logic needs to be robust based on what "deleting a tile" means.
        }

        for (const ClipboardItemData& itemData : data.items) {
            // Item* item = RME::Item::create(itemData.id); // Assuming Item::create factory
            // if (item) {
            //    item->setSubType(itemData.subType);
            //    item->setAttributes(itemData.attributes);
            //    tile->addItem(item); // May need to consider order
            // }
        }
        if (data.hasCreature) {
            // Creature* creature = RME::Creature::create(data.creature.name); // Assuming factory
            // if (creature) {
            //    // set creature properties
            //    tile->addCreature(creature);
            // }
        }
        if (data.hasSpawn) {
            // Spawn* spawn = RME::Spawn::create(); // Assuming factory
            // if (spawn) {
            //    spawn->setRadius(data.spawn.radius);
            //    // set spawn creatures
            //    tile->setSpawn(spawn);
            // }
        }
        m_map->markTileDirty(tile->getPosition());
    }
    // TODO: Trigger map changed signals/updates
}

void DeleteCommand::redo() {
    // Perform the deletion from the map
    // Iterate m_deletedData to know what to remove. The positions are absolute.
    qDebug() << "DeleteCommand: Redoing deletion of" << m_deletedData.count() << "elements' data.";
    for (const DeletedTileData& data : m_deletedData) {
        Tile* tile = m_map->getTile(data.relativePosition); // relativePosition is absolute
        if (!tile) continue;

        // If data.hasGround is true, it means the ground/tile itself was part of the deletion.
        // This could mean removing all items, creature, spawn, and then clearing ground properties,
        // or even removing the tile object if it becomes empty and the map supports sparse tiles.
        if (data.hasGround) {
            // tile->clearItems();
            // tile->removeCreature();
            // tile->removeSpawn();
            // tile->setGround(0); // Or equivalent of clearing ground
            // tile->setHouseId(0);
            // tile->setFlags(TILESTATE_NONE); // from wxWidgets
            // tile->clearZoneId();
            qDebug() << "  Deleting ground and all content from tile at" << data.relativePosition;
        } else {
            // Only delete specific items/creatures/spawns listed in data, not the whole ground.
            // for (const ClipboardItemData& itemData : data.items) {
            //    Item* itemToRemove = tile->findItem(itemData.id, itemData.attributes); // Needs a way to identify the exact item
            //    if (itemToRemove) tile->removeItem(itemToRemove, true); // true to delete
            // }
            // if (data.hasCreature && tile->getCreature() && tile->getCreature()->getName() == data.creature.name) {
            //    tile->removeCreature();
            // }
            // if (data.hasSpawn && tile->getSpawn()) { // Simplified check
            //    tile->removeSpawn();
            // }
            qDebug() << "  Deleting specific elements from tile at" << data.relativePosition;
        }
        if (tile->isEmptyAndClean()) { // Assuming Tile::isEmptyAndClean() checks if it has no content and no special state
             // m_map->removeTile(tile->getPosition()); // If map supports removing empty tiles
        } else {
            m_map->markTileDirty(tile->getPosition());
        }
    }
    // TODO: Trigger map changed signals/updates
    // TODO: Handle "automagic" border updates similar to wxWidgets cut if needed
}

} // namespace RME
