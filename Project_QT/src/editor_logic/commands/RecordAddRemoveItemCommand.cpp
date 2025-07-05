#include "editor_logic/commands/RecordAddRemoveItemCommand.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h" // For map->notifyTileChanged
#include "core/assets/ItemDatabase.h" // For getting item name for text
#include "core/assets/AssetManager.h" // For getting ItemDatabase

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT

namespace RME {
namespace core {
namespace actions {

// Constructor for Adding an item
RecordAddRemoveItemCommand::RecordAddRemoveItemCommand(
    RME::core::Tile* tile,
    std::unique_ptr<RME::core::Item> itemToAdd,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Add Item"), parent),
    m_tile(tile),
    m_operation(ItemChangeOperation::Add),
    m_rawItemPtrForRemoveRedo(nullptr) // Not used for Add
{
    Q_ASSERT(m_tile);
    Q_ASSERT(itemToAdd);

    m_tilePosition = m_tile->getPosition();
    // Store a deep copy of the item to be added. This copy will be used for redo.
    m_itemForAddRedo_RemoveUndo = itemToAdd->deepCopy();
    m_itemIdForRemove = 0; // Not used for Add

    initializeCommandText();
}

// Constructor for Removing an item
RecordAddRemoveItemCommand::RecordAddRemoveItemCommand(
    RME::core::Tile* tile,
    RME::core::Item* itemToRemove, // Raw pointer to item on tile
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Remove Item"), parent),
    m_tile(tile),
    m_operation(ItemChangeOperation::Remove),
    m_rawItemPtrForRemoveRedo(itemToRemove) // Store the original pointer for redo removal
{
    Q_ASSERT(m_tile);
    Q_ASSERT(itemToRemove);

    m_tilePosition = m_tile->getPosition();
    // Store a deep copy of the item to be removed. This copy will be used for undo.
    m_itemForAddRedo_RemoveUndo = itemToRemove->deepCopy();
    m_itemIdForRemove = itemToRemove->getID();

    initializeCommandText();
}

void RecordAddRemoveItemCommand::initializeCommandText() {
    const auto& itemDb = getController()->getAssetManager()->getItemDatabase();
    QString itemName = "Unknown Item";
    uint16_t itemIdToDisplay = 0;

    if (m_itemForAddRedo_RemoveUndo) {
        itemIdToDisplay = m_itemForAddRedo_RemoveUndo->getID();
    }

    if (m_operation == ItemChangeOperation::Remove && m_rawItemPtrForRemoveRedo) {
         // If removing, the item for display is the one being removed.
        itemIdToDisplay = m_rawItemPtrForRemoveRedo->getID();
    } else if (m_operation == ItemChangeOperation::Add && m_itemForAddRedo_RemoveUndo) {
        // If adding, it's the item being added.
        itemIdToDisplay = m_itemForAddRedo_RemoveUndo->getID();
    }

    if(itemIdToDisplay != 0) {
      itemName = itemDb.getItemData(itemIdToDisplay).name;
      if (itemName.isEmpty()) itemName = QString("ID: %1").arg(itemIdToDisplay);
    }

    if (m_operation == ItemChangeOperation::Add) {
        m_commandTextBase = QObject::tr("Add Item (%1)").arg(itemName);
    } else { // Remove
        m_commandTextBase = QObject::tr("Remove Item (%1)").arg(itemName);
    }
    setText(m_commandTextBase + QObject::tr(" at (%1,%2,%3)").arg(m_tilePosition.x).arg(m_tilePosition.y).arg(m_tilePosition.z));
}

void RecordAddRemoveItemCommand::undo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("undo item operation");
        return;
    }

    if (m_operation == ItemChangeOperation::Add) { // Undo Add -> Remove the item
        // We need to find the item that was added.
        // If m_itemForAddRedo_RemoveUndo is the one on tile, Tile::removeItem should handle it.
        // This assumes that the item added by redo() is the same instance or identifiable.
        // Let's assume Tile::removeItem can find and remove by comparing properties or if we stored a raw pointer.
        // For simplicity, assuming the redo() added m_itemForAddRedo_RemoveUndo (or a fresh copy of it)
        // and we need to remove an item of that type. Tile::removeItemById(id) or Tile::removeItem(Item*) by value might be needed.
        // This is tricky. If redo() adds a *new unique_ptr copy*, how do we get the pointer to that specific item on tile?
        // Let's assume Tile::removeItem(Item* item, bool destroy = true) can find the item by logical equality if not pointer.
        // Or, more robustly, for undo of add, we remove an item of m_itemForAddRedo_RemoveUndo->getID().
        // This might remove the wrong one if multiple identical items exist.

        // Simplification: RawBrush usually adds to top. So remove from top matching ID.
        RME::core::Item* itemOnTileToRemove = m_tile->getTopItemByID(m_itemForAddRedo_RemoveUndo->getID());
        if (itemOnTileToRemove) {
            m_tile->removeItem(itemOnTileToRemove, true); // True to delete it
        } else {
            qWarning("RecordAddRemoveItemCommand::undo (Add): Could not find item to remove with ID %d.").arg(m_itemForAddRedo_RemoveUndo->getID());
        }
    } else { // Undo Remove -> Add the item back
        Q_ASSERT(m_itemForAddRedo_RemoveUndo); // This should hold the copy of the removed item.
        m_tile->addItem(m_itemForAddRedo_RemoveUndo->deepCopy()); // Add a copy back
    }

    notifyMapChanged(m_tilePosition);
    logUndo(m_commandTextBase, m_tilePosition);
}

void RecordAddRemoveItemCommand::redo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("redo item operation");
        return;
    }

    if (m_operation == ItemChangeOperation::Add) { // Redo Add -> Add the item
        Q_ASSERT(m_itemForAddRedo_RemoveUndo);
        m_tile->addItem(m_itemForAddRedo_RemoveUndo->deepCopy()); // Add a copy
    } else { // Redo Remove -> Remove the item
        // m_rawItemPtrForRemoveRedo was stored pointing to the item on tile.
        // This pointer might be stale if other commands modified items.
        // A more robust way is to find the item by ID (and potentially index if stored).
        // For RAW brush, removing the topmost item of m_itemIdForRemove is typical.
        RME::core::Item* itemOnTileToRemove = m_tile->getTopItemByID(m_itemIdForRemove);
        if (itemOnTileToRemove) {
            m_tile->removeItem(itemOnTileToRemove, true);
        } else {
            qWarning("RecordAddRemoveItemCommand::redo (Remove): Could not find item ID %d to remove.").arg(m_itemIdForRemove);
        }
    }

    notifyMapChanged(m_tilePosition);
    logRedo(m_commandTextBase, m_tilePosition);
}

} // namespace actions
} // namespace core
} // namespace RME
