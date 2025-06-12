#ifndef RME_DELETECOMMAND_H
#define RME_DELETECOMMAND_H

#include <QUndoCommand>
#include <QMap>
#include <QSet> // Not strictly needed here, but often useful for commands
#include <memory> // For std::unique_ptr
#include "core/Position.h"
// Forward declarations
namespace RME {
    class Map;
    class SelectionManager;
    class Tile;
}

namespace RME_COMMANDS {

const int DeleteCommandId = 1002; // Unique ID for this command type

class DeleteCommand : public QUndoCommand {
public:
    DeleteCommand(
        RME::Map* map,
        RME::SelectionManager* selectionManager,
        QUndoCommand* parent = nullptr
    );
    ~DeleteCommand() override;

    void undo() override;
    void redo() override;

    int id() const override { return DeleteCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Merging delete is less common

private:
    RME::Map* m_map;
    RME::SelectionManager* m_selectionManager; // To clear selection after delete

    // Stores the state of tiles *before* redo() deleted them.
    QMap<RME::Position, std::unique_ptr<RME::Tile>> m_deletedTiles;
    // To remember which tiles were part of the selection, for undo re-selection (optional)
    QList<RME::Position> m_selectedPositionsForUndo;
};

} // namespace RME_COMMANDS
#endif // RME_DELETECOMMAND_H
