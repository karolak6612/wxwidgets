#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "core/clipboard/ClipboardData.h" // For ClipboardTileData, if we store what was deleted
                                                         // Alternatively, operate directly on Tile pointers.
                                                         // Let's store data for restoration.

namespace RME {
class Map;
class Tile; // Forward declaration
struct SelectionManager; // For context or detailed selection info, if needed beyond simple tile list

// Structure to hold information about what was deleted for undo purposes.
// This is similar to ClipboardTileData but focused on elements present before deletion.
using DeletedTileData = ClipboardTileData; // Reuse for simplicity, represents state before deletion.

class DeleteCommand : public QUndoCommand {
public:
    // Constructor takes the map and a list of data representing the elements to be deleted.
    // This data should be captured *before* deletion occurs.
    DeleteCommand(
        Map* map,
        const QList<DeletedTileData>& itemsToDelete, // Data of elements as they were before deletion
        const QString& text = "Delete Selection",
        QUndoCommand* parent = nullptr
    );

    ~DeleteCommand() override;

    void undo() override; // Re-inserts the deleted elements
    void redo() override; // Performs the deletion

private:
    Map* m_map; // Non-owning
    QList<DeletedTileData> m_deletedData; // Stores what was deleted, including their original relative positions if applicable for restoration
                                          // If deletion is based on live selection, this might store Tile* and Item* pointers
                                          // along with their state. Storing data like ClipboardTileData is safer for undo.
    // If we base deletion on live selection, we'd need to capture that state.
    // For 'cut', the clipboard data itself (ClipboardContent) can represent what's being deleted.
};

} // namespace RME
#endif // DELETECOMMAND_H
