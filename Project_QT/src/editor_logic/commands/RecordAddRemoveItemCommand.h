#ifndef RME_RECORDADDREMOVEITEMCOMMAND_H
#define RME_RECORDADDREMOVEITEMCOMMAND_H

#include <QUndoCommand>
#include <memory>   // For std::unique_ptr
#include <QString>    // For command text
#include <optional> // For std::optional if storing index

#include "core/Position.h" // For RME::core::Position

// Forward declare RME::core::Item and RME::core::Tile
namespace RME {
namespace core {
    class Item;
    class Tile;
    namespace editor { // Forward declare EditorControllerInterface
        class EditorControllerInterface;
    }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RecordAddRemoveItemCommandId = toInt(CommandId::RecordAddRemoveItem);

enum class ItemChangeOperation { Add, Remove };

class RecordAddRemoveItemCommand : public QUndoCommand {
public:
    // Constructor for Adding an item
    RecordAddRemoveItemCommand(
        RME::core::Tile* tile,
        std::unique_ptr<RME::core::Item> itemToAdd, // Item to be added
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    // Constructor for Removing an item
    RecordAddRemoveItemCommand(
        RME::core::Tile* tile,
        RME::core::Item* itemToRemove, // Item to be removed (raw pointer, command will copy)
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~RecordAddRemoveItemCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RecordAddRemoveItemCommandId; }

    // Getters for test verification
    ItemChangeOperation getOperation() const { return m_operation; }
    uint16_t getItemIdForOperation() const {
        // For Add, m_itemForAddRedo_RemoveUndo holds the item added (its copy).
        // For Remove, m_itemForAddRedo_RemoveUndo holds a copy of the removed item.
        // m_itemIdForRemove is also specifically stored for remove operations.
        if (m_operation == ItemChangeOperation::Add && m_itemForAddRedo_RemoveUndo) {
            return m_itemForAddRedo_RemoveUndo->getID();
        } else if (m_operation == ItemChangeOperation::Remove) {
            return m_itemIdForRemove; // This was the ID of the item targeted for removal.
        }
        return 0; // Should not happen if command is well-formed
    }
    const RME::core::Item* getItemForAddRedo_RemoveUndo() const { return m_itemForAddRedo_RemoveUndo.get(); }

private:
    RME::core::Tile* m_tile;
    RME::core::editor::EditorControllerInterface* m_controller;
    RME::core::Position m_tilePosition;
    ItemChangeOperation m_operation;

    // State for add/remove
    std::unique_ptr<RME::core::Item> m_itemForAddRedo_RemoveUndo; // Stores the item for add (redo) / remove (undo)
    // For remove, we also need to know which item was removed if multiple of same type exist.
    // Storing a copy of the removed item is good. For exact stack order, an index might be needed.
    // For RAW brush, removing the topmost matching ID is usually sufficient.
    // If item pointer stability is an issue after removal, an ID and potentially index might be better.
    // Let's assume Tile::removeItem(Item*) works by pointer identity for now.
    // If item was removed, m_itemForAddRedo_RemoveUndo holds its copy for undo.
    // If item was added, m_itemForAddRedo_RemoveUndo holds it for redo.

    // For remove operation, we need to store the raw pointer of the item that was on the tile to remove it by pointer in redo.
    // This is tricky because that pointer is only valid as long as the item is on the tile.
    // For `redo` of remove, we need to find it again if not by pointer. Let's assume Tile::removeItem(Item*) is robust.
    // For `undo` of remove, we add back `m_itemForAddRedo_RemoveUndo`.
    RME::core::Item* m_rawItemPtrForRemoveRedo; // Only used if operation is Remove, for redo.
                                                // This pointer is to the item *on the tile* before it's removed by redo.
                                                // This is problematic. Let's simplify: for remove, we store ID and find it.
    uint16_t m_itemIdForRemove; // Store ID for removal robustness
    // int m_itemIndexForRemove; // Optional: if stack order is critical

    QString m_commandTextBase;

    void initializeCommandText();
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_RECORDADDREMOVEITEMCOMMAND_H
