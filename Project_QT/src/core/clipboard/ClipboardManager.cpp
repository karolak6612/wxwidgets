#include "ClipboardManager.h"
#include "core/selection/SelectionManager.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/creatures/Creature.h"
#include "core/spawns/Spawn.h"
#include "core/ItemType.h" // Assuming ItemType holds item attributes
#include "core/actions/DeleteCommand.h"
#include "core/actions/PasteCommand.h"
#include "core/selection/SelectionManager.h" // For getting detailed selection for delete

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QUndoStack> // For cut/paste
#include <limits>   // For std::numeric_limits
#include <QDebug>   // For logging
#include <QSize>    // For bounding box calculations
#include <climits>  // For INT_MAX, INT_MIN

ClipboardManager::ClipboardManager(QObject *parent) : QObject(parent) {}

void ClipboardManager::copySelection(const RME::SelectionManager& selectionManager, const RME::Map& map) {
    const QSet<RME::Tile*>& selectedTiles = selectionManager.getSelectedTiles();
    if (selectedTiles.isEmpty()) {
        qDebug() << "ClipboardManager: No tiles selected to copy.";
        return;
    }

    RME::ClipboardContent clipboardContent;
    RME::Position copyRefPos(std::numeric_limits<int16_t>::max(),
                             std::numeric_limits<int16_t>::max(),
                             std::numeric_limits<int8_t>::max());
    bool firstTile = true;

    // First pass: determine the top-left-most position (copyRefPos)
    for (RME::Tile* tile : selectedTiles) {
        if (!tile) continue;
        // Only consider tiles that are themselves selected or contain selected elements.
        // The selectionManager.getSelectedTiles() should ideally give us this.
        // For copy, we iterate what SelectionManager considers selected.

        const RME::Position& tilePos = tile->getPosition();
        if (firstTile) {
            copyRefPos = tilePos;
            firstTile = false;
        } else {
            if (tilePos.x < copyRefPos.x) copyRefPos.x = tilePos.x;
            if (tilePos.y < copyRefPos.y) copyRefPos.y = tilePos.y;
            if (tilePos.z < copyRefPos.z) copyRefPos.z = tilePos.z; // Or handle floor logic differently if needed
        }
    }
    if (firstTile) { // No valid tiles found
        qDebug() << "ClipboardManager: No valid tiles in selection to determine reference position.";
        return;
    }


    for (RME::Tile* tile : selectedTiles) {
        if (!tile) continue;

        // We only copy data from a tile if the tile itself is selected or it contains selected items/creatures/spawn
        // The wxwidgets version iterates editor.selection and then checks tile->ground->isSelected() or item->isSelected() etc.
        // Our SelectionManager::isSelected(tile) or SelectionManager::isSelected(tile, item) should be the source of truth.

        RME::ClipboardTileData tileData;
        tileData.relativePosition = tile->getPosition() - copyRefPos;

        // Ground data (assuming tile selection implies ground selection, or specific ground selection check)
        if (selectionManager.isSelected(tile)) { // Check if the tile itself (ground) is selected
            tileData.hasGround = true;
            if (tile->getGround()) {
                tileData.groundItemID = tile->getGround()->getID();
            }
            tileData.houseId = tile->getHouseId();
            tileData.tileFlags = static_cast<uint32_t>(tile->getMapFlags()); // Convert TileMapFlags to uint32_t
        }

        // Items - Implement per-item selection when Item selection state is available
        // Check if the tile itself is selected (which implies all items are selected)
        // or check individual item selection states
        bool tileSelected = selectionManager.isSelected(tile);
        
        // Copy ground item if it exists and is selected
        if (tile->getGround()) {
            bool groundSelected = tileSelected || selectionManager.isItemSelected(tile, tile->getGround());
            if (groundSelected) {
                RME::ClipboardItemData groundData = createItemClipboardData(tile->getGround());
                tileData.items.append(groundData);
                
                // If ground is selected, mark tile as having ground data
                if (!tileData.hasGround) {
                    tileData.hasGround = true;
                    tileData.groundItemID = tile->getGround()->getID();
                    tileData.houseId = tile->getHouseId();
                    tileData.tileFlags = static_cast<uint32_t>(tile->getMapFlags());
                }
            }
        }
        
        // Copy stacked items (check individual selection for each item)
        QList<RME::Item*> allItems = tile->getAllItems();
        for (RME::Item* item : allItems) {
            if (item && item != tile->getGround()) { // Skip ground item as it's handled above
                bool itemSelected = tileSelected || selectionManager.isItemSelected(tile, item);
                if (itemSelected) {
                    RME::ClipboardItemData itemData = createItemClipboardData(item);
                    tileData.items.append(itemData);
                }
            }
        }

        // Creature - Implement per-creature selection when Creature selection state is available
        if (tile->hasCreature()) {
            RME::core::creatures::Creature* creature = tile->getCreature();
            if (creature) {
                bool creatureSelected = tileSelected || selectionManager.isCreatureSelected(tile, creature);
                if (creatureSelected) {
                    tileData.hasCreature = true;
                    tileData.creature = createCreatureClipboardData(creature);
                }
            }
        }

        // Spawn - Implement per-spawn selection when Spawn selection state is available
        if (tile->isSpawnTile()) {
            // For now, assume tile-level selection includes spawn data
            // In a full implementation, we'd check individual spawn selection
            bool spawnSelected = tileSelected;
            
            // If tile has spawn integration, check its selection state
            if (tile->hasSpawn()) {
                RME::core::spawns::Spawn spawn = tile->getSpawn();
                spawnSelected = tileSelected || spawn.isSelected();
            }
            
            if (spawnSelected) {
                tileData.hasSpawn = true;
                tileData.spawn = createSpawnClipboardData(tile);
            }
        }

        // Only add tileData if it has some selected content
        if (tileData.hasGround || !tileData.items.isEmpty() || tileData.hasCreature || tileData.hasSpawn) {
            clipboardContent.tiles.append(tileData);
        }
    }

    if (clipboardContent.tiles.isEmpty()) {
        qDebug() << "ClipboardManager: No selected elements found to copy.";
        return;
    }

    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << clipboardContent;

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(RME::RME_CLIPBOARD_MIME_TYPE, byteArray);
    QApplication::clipboard()->setMimeData(mimeData);

    qDebug() << "ClipboardManager: Copied" << clipboardContent.tiles.count() << "tiles' data to clipboard.";
}

bool ClipboardManager::canPaste() const {
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (mimeData) {
        return mimeData->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE);
    }
    return false;
}

RME::ClipboardContent ClipboardManager::getPasteData() const {
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (mimeData && mimeData->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE)) {
        QByteArray byteArray = mimeData->data(RME::RME_CLIPBOARD_MIME_TYPE);
        QDataStream stream(&byteArray, QIODevice::ReadOnly);
        RME::ClipboardContent content;
        stream >> content;
        if (stream.status() == QDataStream::Ok) {
            return content;
        } else {
            qWarning() << "ClipboardManager: Error deserializing clipboard data.";
        }
    }
    return RME::ClipboardContent(); // Return empty content
}

// --- Stubs for cut/paste which will use DeleteCommand and PasteCommand (Step 4) ---
void ClipboardManager::cutSelection(RME::SelectionManager& selectionManager, RME::Map& map, QUndoStack* undoStack) {
    if (selectionManager.getSelectedTiles().isEmpty()) {
         qDebug() << "ClipboardManager: No selection to cut.";
        return;
    }
    const QSet<RME::Tile*>& selectedTiles = selectionManager.getSelectedTiles();
    if (selectedTiles.isEmpty()) {
         qDebug() << "ClipboardManager: No selection to cut.";
        return;
    }

    // 1. Capture data for DeleteCommand *before* copySelection
    RME::ClipboardContent elementsToDeleteData; // Use ClipboardContent for consistency
    
    for (RME::Tile* tile : selectedTiles) {
        if (!tile) continue;

        RME::ClipboardTileData tileDeleteData;
        tileDeleteData.relativePosition = tile->getPosition(); // Absolute position for delete

        if (selectionManager.isSelected(tile)) {
            tileDeleteData.hasGround = true;
            if (tile->getGround()) {
                tileDeleteData.groundItemID = tile->getGround()->getID();
            }
            tileDeleteData.houseId = tile->getHouseId();
            tileDeleteData.tileFlags = static_cast<uint32_t>(tile->getMapFlags());
            
            // Copy items for deletion
            if (tile->getGround()) {
                RME::ClipboardItemData groundData;
                groundData.id = tile->getGround()->getID();
                groundData.subType = tile->getGround()->getSubtype();
                tileDeleteData.items.append(groundData);
            }
            
            QList<RME::Item*> allItems = tile->getAllItems();
            for (RME::Item* item : allItems) {
                if (item && item != tile->getGround()) {
                    RME::ClipboardItemData itemData;
                    itemData.id = item->getID();
                    itemData.subType = item->getSubtype();
                    tileDeleteData.items.append(itemData);
                }
            }
            
            // Copy creature for deletion
            if (tile->hasCreature()) {
                tileDeleteData.hasCreature = true;
                tileDeleteData.creature.name = tile->getCreature()->getName();
            }
            
            // Copy spawn for deletion
            if (tile->isSpawnTile()) {
                tileDeleteData.hasSpawn = true;
                tileDeleteData.spawn.radius = tile->getSpawnRadius();
                tileDeleteData.spawn.creatureNames = tile->getSpawnCreatureList();
                tileDeleteData.spawn.spawnTime = tile->getSpawnIntervalSeconds();
            }
        }

        if (tileDeleteData.hasGround || !tileDeleteData.items.isEmpty() || tileDeleteData.hasCreature || tileDeleteData.hasSpawn) {
            elementsToDeleteData.tiles.append(tileDeleteData);
        }
    }

    if (elementsToDeleteData.tiles.isEmpty()) {
        qDebug() << "ClipboardManager::cutSelection - No elements marked for deletion based on current selection.";
        return;
    }


    // 2. Perform the copy operation (puts data on system clipboard)
    copySelection(selectionManager, map);
    // Critical: Ensure copySelection doesn't modify state that DeleteCommand depends on
    // then data for DeleteCommand must be captured *before* copySelection.
    // Assuming SelectionManager's state is the source of truth and copySelection is read-only w.r.t map state.

    if (QApplication::clipboard()->mimeData() == nullptr || !QApplication::clipboard()->mimeData()->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE)) {
        qDebug() << "ClipboardManager::cutSelection - Copy operation failed or produced no data. Aborting cut.";
        return;
    }

    if (elementsToDeleteData.tiles.isEmpty()){
         qDebug() << "ClipboardManager::cutSelection - No actual data captured for deletion command based on current selection. Aborting delete part of cut.";
        return;
    }

    // 3. Create and push DeleteCommand
    // The DeleteCommand should operate on the elements that were just copied.
    // It can get this from the clipboardContent or from a fresh query of selectionManager.
    // Using elementsToDeleteData captured above is more direct.
    RME::DeleteCommand* cmd = new RME::DeleteCommand(&map, elementsToDeleteData, "Cut");
    undoStack->push(cmd);
    qDebug() << "ClipboardManager: Cut operation - copy done, DeleteCommand pushed.";
}

void ClipboardManager::paste(RME::Map& map, const RME::Position& targetPosition, QUndoStack* undoStack) {
    if (!canPaste()) {
        qDebug() << "ClipboardManager: No data to paste or invalid format.";
        return;
    }
    RME::ClipboardContent pasteData = getPasteData();
    if (pasteData.tiles.isEmpty()) {
        qDebug() << "ClipboardManager: Clipboard data is empty.";
        return;
    }

    RME::PasteCommand* cmd = new RME::PasteCommand(&map, targetPosition, pasteData, "Paste");
    undoStack->push(cmd);
    qDebug() << "ClipboardManager: Paste operation - PasteCommand pushed.";
}

// Helper methods for creating clipboard data
RME::ClipboardItemData ClipboardManager::createItemClipboardData(const RME::Item* item) const {
    RME::ClipboardItemData itemData;
    itemData.id = item->getID();
    itemData.subType = item->getSubtype();
    itemData.attributes = QVariantMap();
    
    // Copy common attributes
    if (item->hasAttribute("uid")) {
        itemData.attributes["uid"] = item->getAttribute("uid");
    }
    if (item->hasAttribute("aid")) {
        itemData.attributes["aid"] = item->getAttribute("aid");
    }
    if (item->hasAttribute("text")) {
        itemData.attributes["text"] = item->getAttribute("text");
    }
    if (item->hasAttribute("description")) {
        itemData.attributes["description"] = item->getAttribute("description");
    }
    if (item->hasAttribute("charges")) {
        itemData.attributes["charges"] = item->getAttribute("charges");
    }
    if (item->hasAttribute("count")) {
        itemData.attributes["count"] = item->getAttribute("count");
    }
    
    // Copy container contents if this is a container
    if (item->isContainer()) {
        const RME::ContainerItem* container = dynamic_cast<const RME::ContainerItem*>(item);
        if (container) {
            QVariantList containerItems;
            const auto& contents = container->getContents();
            for (const RME::Item* contentItem : contents) {
                if (contentItem) {
                    // Recursively create clipboard data for container contents
                    RME::ClipboardItemData contentData = createItemClipboardData(contentItem);
                    QVariantMap contentMap;
                    contentMap["id"] = contentData.id;
                    contentMap["subType"] = contentData.subType;
                    contentMap["attributes"] = contentData.attributes;
                    containerItems.append(contentMap);
                }
            }
            if (!containerItems.isEmpty()) {
                itemData.attributes["containerContents"] = containerItems;
            }
        }
    }
    
    // Copy special item properties
    if (item->isDoor()) {
        const RME::DoorItem* door = dynamic_cast<const RME::DoorItem*>(item);
        if (door) {
            itemData.attributes["doorId"] = door->getDoorID();
            itemData.attributes["isOpen"] = door->isOpen();
        }
    }
    
    if (item->isTeleport()) {
        const RME::TeleportItem* teleport = dynamic_cast<const RME::TeleportItem*>(item);
        if (teleport) {
            const RME::core::Position& dest = teleport->getDestination();
            QVariantMap destMap;
            destMap["x"] = dest.x;
            destMap["y"] = dest.y;
            destMap["z"] = dest.z;
            itemData.attributes["teleportDestination"] = destMap;
        }
    }
    
    return itemData;
}

RME::ClipboardCreatureData ClipboardManager::createCreatureClipboardData(const RME::core::creatures::Creature* creature) const {
    RME::ClipboardCreatureData creatureData;
    creatureData.name = creature->getName();
    
    // Copy outfit data
    const RME::core::creatures::Outfit& outfit = creature->getOutfit();
    creatureData.lookType = outfit.lookType;
    creatureData.head = outfit.head;
    creatureData.body = outfit.body;
    creatureData.legs = outfit.legs;
    creatureData.feet = outfit.feet;
    creatureData.addons = outfit.addons;
    creatureData.mount = outfit.mount;
    
    // Copy direction and other properties
    creatureData.direction = static_cast<uint8_t>(creature->getDirection());
    creatureData.isNpc = creature->isNpc();
    
    // Copy additional attributes
    if (creature->hasAttribute("spawnTime")) {
        creatureData.attributes["spawnTime"] = creature->getAttribute("spawnTime");
    }
    if (creature->hasAttribute("script")) {
        creatureData.attributes["script"] = creature->getAttribute("script");
    }
    if (creature->hasAttribute("health")) {
        creatureData.attributes["health"] = creature->getAttribute("health");
    }
    if (creature->hasAttribute("maxHealth")) {
        creatureData.attributes["maxHealth"] = creature->getAttribute("maxHealth");
    }
    
    return creatureData;
}

RME::ClipboardSpawnData ClipboardManager::createSpawnClipboardData(const RME::Tile* tile) const {
    RME::ClipboardSpawnData spawnData;
    
    if (!tile->isSpawnTile()) {
        return spawnData; // Return empty spawn data if tile is not a spawn tile
    }
    
    // Copy basic spawn data from tile
    spawnData.radius = tile->getSpawnRadius();
    spawnData.spawnTime = tile->getSpawnIntervalSeconds();
    spawnData.creatureNames = tile->getSpawnCreatureList();
    
    // Set default values for advanced spawn properties
    spawnData.despawnRange = 2; // Default value
    spawnData.despawnRadius = 1; // Default value
    
    // Convert creature names to detailed creature spawn entries
    for (const QString& creatureName : spawnData.creatureNames) {
        RME::ClipboardSpawnData::CreatureSpawnEntry entry;
        entry.name = creatureName;
        entry.chance = 100; // Default chance
        entry.max = 1; // Default max
        spawnData.creatures.append(entry);
    }
    
    // If the tile has spawn integration, use that for more detailed information
    if (tile->hasSpawn()) {
        RME::core::spawns::Spawn tileSpawn = tile->getSpawn();
        spawnData.radius = tileSpawn.getRadius();
        spawnData.spawnTime = tileSpawn.getIntervalSeconds();
        spawnData.creatureNames = tileSpawn.getCreatureTypes();
        
        // Update creature entries with spawn information
        spawnData.creatures.clear();
        for (const QString& creatureName : tileSpawn.getCreatureTypes()) {
            RME::ClipboardSpawnData::CreatureSpawnEntry entry;
            entry.name = creatureName;
            entry.chance = 100; // Default chance - Spawn doesn't have individual chances
            entry.max = 1; // Default max
            spawnData.creatures.append(entry);
        }
        
        // Add spawn selection state if available
        if (tileSpawn.isSelected()) {
            spawnData.attributes["selected"] = true;
        }
    }
    
    return spawnData;
}

// Advanced clipboard operations
QString ClipboardManager::getClipboardStatistics() const {
    ClipboardStats stats = analyzeClipboardData();
    
    QString result;
    result += QString("Clipboard Statistics:\n");
    result += QString("- Total Tiles: %1\n").arg(stats.totalTiles);
    result += QString("- Total Items: %1 (%2 unique types)\n").arg(stats.totalItems).arg(stats.uniqueItemTypes);
    result += QString("- Total Creatures: %1 (%2 unique types)\n").arg(stats.totalCreatures).arg(stats.uniqueCreatureTypes);
    result += QString("- Total Spawns: %1\n").arg(stats.totalSpawns);
    result += QString("- Bounding Box: %1x%2\n").arg(stats.boundingBox.width()).arg(stats.boundingBox.height());
    result += QString("- Format Version: %1\n").arg(stats.formatVersion);
    
    return result;
}

bool ClipboardManager::validateClipboardData() const {
    if (!canPaste()) {
        return false;
    }
    
    RME::ClipboardContent content = getPasteData();
    if (content.tiles.isEmpty()) {
        return false;
    }
    
    // Validate each tile's data
    for (const auto& tileData : content.tiles) {
        // Check for valid position ranges
        if (tileData.relativePosition.x < -1000 || tileData.relativePosition.x > 1000 ||
            tileData.relativePosition.y < -1000 || tileData.relativePosition.y > 1000 ||
            tileData.relativePosition.z < 0 || tileData.relativePosition.z > 15) {
            qWarning() << "ClipboardManager: Invalid position in clipboard data:" << tileData.relativePosition.x << tileData.relativePosition.y << tileData.relativePosition.z;
            return false;
        }
        
        // Validate item IDs
        for (const auto& itemData : tileData.items) {
            if (itemData.id == 0 || itemData.id > 65535) {
                qWarning() << "ClipboardManager: Invalid item ID in clipboard data:" << itemData.id;
                return false;
            }
        }
        
        // Validate creature data
        if (tileData.hasCreature) {
            if (tileData.creature.name.isEmpty()) {
                qWarning() << "ClipboardManager: Empty creature name in clipboard data";
                return false;
            }
            if (tileData.creature.direction > 3) {
                qWarning() << "ClipboardManager: Invalid creature direction:" << tileData.creature.direction;
                return false;
            }
        }
        
        // Validate spawn data
        if (tileData.hasSpawn) {
            if (tileData.spawn.radius == 0 || tileData.spawn.radius > 50) {
                qWarning() << "ClipboardManager: Invalid spawn radius:" << tileData.spawn.radius;
                return false;
            }
            if (tileData.spawn.creatures.isEmpty() && tileData.spawn.creatureNames.isEmpty()) {
                qWarning() << "ClipboardManager: Spawn with no creatures in clipboard data";
                return false;
            }
        }
    }
    
    return true;
}

void ClipboardManager::compressClipboardData() {
    // This is a placeholder for future compression implementation
    // Could implement:
    // 1. Remove duplicate tile data
    // 2. Compress item stacks
    // 3. Optimize position encoding
    // 4. Use binary compression algorithms
    
    qDebug() << "ClipboardManager::compressClipboardData - Not yet implemented";
}

ClipboardManager::ClipboardStats ClipboardManager::analyzeClipboardData() const {
    ClipboardStats stats;
    stats.totalTiles = 0;
    stats.totalItems = 0;
    stats.totalCreatures = 0;
    stats.totalSpawns = 0;
    stats.uniqueItemTypes = 0;
    stats.uniqueCreatureTypes = 0;
    stats.boundingBox = QSize(0, 0);
    stats.formatVersion = "1.0";
    
    if (!canPaste()) {
        return stats;
    }
    
    RME::ClipboardContent content = getPasteData();
    if (content.tiles.isEmpty()) {
        return stats;
    }
    
    QSet<uint16_t> uniqueItems;
    QSet<QString> uniqueCreatures;
    int minX = INT_MAX, maxX = INT_MIN;
    int minY = INT_MAX, maxY = INT_MIN;
    
    for (const auto& tileData : content.tiles) {
        stats.totalTiles++;
        
        // Count items and track unique types
        for (const auto& itemData : tileData.items) {
            stats.totalItems++;
            uniqueItems.insert(itemData.id);
        }
        
        // Count creatures and track unique types
        if (tileData.hasCreature) {
            stats.totalCreatures++;
            uniqueCreatures.insert(tileData.creature.name);
        }
        
        // Count spawns
        if (tileData.hasSpawn) {
            stats.totalSpawns++;
            // Add spawn creatures to unique creature list
            for (const auto& creatureName : tileData.spawn.creatureNames) {
                uniqueCreatures.insert(creatureName);
            }
            for (const auto& creature : tileData.spawn.creatures) {
                uniqueCreatures.insert(creature.name);
            }
        }
        
        // Calculate bounding box
        const auto& pos = tileData.relativePosition;
        minX = qMin(minX, static_cast<int>(pos.x));
        maxX = qMax(maxX, static_cast<int>(pos.x));
        minY = qMin(minY, static_cast<int>(pos.y));
        maxY = qMax(maxY, static_cast<int>(pos.y));
    }
    
    stats.uniqueItemTypes = uniqueItems.size();
    stats.uniqueCreatureTypes = uniqueCreatures.size();
    
    if (stats.totalTiles > 0) {
        stats.boundingBox = QSize(maxX - minX + 1, maxY - minY + 1);
    }
    
    return stats;
}
