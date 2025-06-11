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
            // tileData.groundItemID = tile->getGround() ? tile->getGround()->getID() : 0; // Example
            tileData.houseId = tile->getHouseId();     // Assumes Tile::getHouseId
            tileData.tileFlags = tile->getFlags();   // Assumes Tile::getFlags for map flags
            // tileData.zoneIds = ...; // Requires zone system access from tile
        }

        // Items
        // const RME::ItemVector& items = tile->getItems(); // Assumes Tile::getItems()
        // for (RME::Item* item : items) {
        //     if (item && selectionManager.isSelected(tile, item)) { // Check if item is selected
        //         RME::ClipboardItemData itemData;
        //         itemData.id = item->getID();
        //         itemData.subType = item->getSubType(); // Or count, etc.
        //         // itemData.attributes = item->getAttributes(); // Assuming Item::getAttributes
        //         tileData.items.append(itemData);
        //     }
        // }

        // Creature
        // RME::Creature* creature = tile->getCreature(); // Assumes Tile::getCreature
        // if (creature && selectionManager.isSelected(tile, creature)) {
        //     tileData.hasCreature = true;
        //     tileData.creature.name = creature->getName(); // Assumes Creature::getName
        //     // Populate other creature data
        // }

        // Spawn
        // RME::Spawn* spawn = tile->getSpawn(); // Assumes Tile::getSpawn
        // if (spawn && selectionManager.isSelected(tile, spawn)) {
        //     tileData.hasSpawn = true;
        //     tileData.spawn.radius = spawn->getRadius(); // Assumes Spawn::getRadius
        //     // Populate spawn creature list
        // }

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

    // 1. Capture the state of selected elements for the DeleteCommand *before* copying.
    //    Copying might alter selection state conceptually or if it involves temporary objects.
    //    The DeleteCommand needs to know exactly what was selected on the live map.
    RME::ClipboardContent dataToDeleteContent;
    // This should be populated similarly to copySelection, but based on current live selection.
    // It represents what will be removed.
    // Let's assume copySelection already did this and the clipboard now holds this data.
    // Or, SelectionManager provides a method to get this data directly.

    // For now, let's assume copySelection correctly captures what to delete.
    // The data for deletion must be *what is currently selected on the map*.
    RME::Position ignoredRefPos; // Not used for delete command's data capture
    QList<RME::DeletedTileData> elementsToDeleteData;

    // Iterate through selected tiles from SelectionManager
    // For each selected tile, and for each selected element on it, create a DeletedTileData entry.
    // This is similar to copySelection's logic but ensures positions are absolute.
    for (RME::Tile* tile : selectedTiles) {
        if (!tile) continue;
        RME::DeletedTileData tileDeleteData;
        tileDeleteData.relativePosition = tile->getPosition(); // Store ABSOLUTE position

        bool tileHasSelectedContent = false;
        if (selectionManager.isSelected(tile)) {
            tileDeleteData.hasGround = true;
            // tileDeleteData.groundItemID = tile->getGround() ? tile->getGround()->getID() : 0;
            tileDeleteData.houseId = tile->getHouseId();
            tileDeleteData.tileFlags = tile->getFlags();
            tileHasSelectedContent = true;
        }
        // For Items, Creature, Spawn - if they are selected, add their data to tileDeleteData.items etc.
        // ... (similar logic as in copySelection for populating items, creature, spawn based on selectionManager.isSelected(...)) ...
        // if (tileHasSelectedContent || !tileDeleteData.items.isEmpty() || tileDeleteData.hasCreature || tileDeleteData.hasSpawn) {
             elementsToDeleteData.append(tileDeleteData);
        // }
    }
     if (elementsToDeleteData.isEmpty()) {
        qDebug() << "ClipboardManager::cutSelection - No elements marked for deletion based on current selection.";
        // Potentially, copySelection might still have put something on clipboard if logic differs.
        // But if nothing is selected to be deleted, maybe don't proceed with delete command.
        // For now, proceed if clipboard got data.
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

    if (elementsToDeleteData.isEmpty()){
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
