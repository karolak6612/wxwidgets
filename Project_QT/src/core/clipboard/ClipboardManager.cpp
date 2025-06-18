#include "ClipboardManager.h"
#include "Project_QT/src/core/selection/SelectionManager.h"
#include "Project_QT/src/core/map/Map.h"
#include "Project_QT/src/core/Tile.h"
#include "Project_QT/src/core/Item.h"
#include "Project_QT/src/core/Creature.h"
#include "Project_QT/src/core/Spawn.h"
#include "Project_QT/src/core/ItemType.h" // Assuming ItemType holds item attributes
#include "Project_QT/src/core/actions/DeleteCommand.h"
#include "Project_QT/src/core/actions/PasteCommand.h"
#include "Project_QT/src/core/selection/SelectionManager.h" // For getting detailed selection for delete

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QUndoStack> // For cut/paste
#include <limits>   // For std::numeric_limits
#include <QDebug>   // For logging

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

        // Items - For now, if tile is selected, copy all items
        // TODO: Implement per-item selection when Item selection state is available
        if (selectionManager.isSelected(tile)) {
            // Copy ground item if it exists
            if (tile->getGround()) {
                RME::ClipboardItemData groundData;
                groundData.id = tile->getGround()->getID();
                groundData.subType = tile->getGround()->getSubtype();
                groundData.attributes = QVariantMap(); // Convert from Item attributes if needed
                // Copy common attributes
                if (tile->getGround()->hasAttribute("uid")) {
                    groundData.attributes["uid"] = tile->getGround()->getAttribute("uid");
                }
                if (tile->getGround()->hasAttribute("aid")) {
                    groundData.attributes["aid"] = tile->getGround()->getAttribute("aid");
                }
                if (tile->getGround()->hasAttribute("text")) {
                    groundData.attributes["text"] = tile->getGround()->getAttribute("text");
                }
                tileData.items.append(groundData);
            }
            
            // Copy stacked items
            QList<RME::Item*> allItems = tile->getAllItems();
            for (RME::Item* item : allItems) {
                if (item && item != tile->getGround()) { // Skip ground item as it's handled above
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
                    tileData.items.append(itemData);
                }
            }
        }

        // Creature - For now, if tile is selected, copy creature
        // TODO: Implement per-creature selection when Creature selection state is available
        if (selectionManager.isSelected(tile) && tile->hasCreature()) {
            RME::core::creatures::Creature* creature = tile->getCreature();
            if (creature) {
                tileData.hasCreature = true;
                tileData.creature.name = creature->getName();
                // TODO: Add other creature data like outfit, direction, etc. when needed
            }
        }

        // Spawn - For now, if tile is selected, copy spawn data
        // TODO: Implement per-spawn selection when Spawn selection state is available
        if (selectionManager.isSelected(tile) && tile->isSpawnTile()) {
            tileData.hasSpawn = true;
            tileData.spawn.radius = tile->getSpawnRadius();
            tileData.spawn.creatureNames = tile->getSpawnCreatureList();
            // TODO: Add spawn interval and other spawn properties when needed
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
    // Important: If copySelection modifies any state that DeleteCommand relies on,
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
