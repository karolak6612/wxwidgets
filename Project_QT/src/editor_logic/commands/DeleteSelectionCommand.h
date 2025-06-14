#ifndef RME_DELETESELECTIONCOMMAND_H
#define RME_DELETESELECTIONCOMMAND_H

#include <QUndoCommand>
#include <QMap>
#include <memory>      // For std::unique_ptr
#include <QString>     // For command text

#include "core/Position.h" // For RME::core::Position

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    class Map;
    namespace selection { class SelectionManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME_COMMANDS {

const int DeleteSelectionCommandId = 1010; // Choose a unique ID

class DeleteSelectionCommand : public QUndoCommand {
public:
    DeleteSelectionCommand(
        RME::core::Map* map,
        const QList<RME::core::Position>& selectedPositions,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~DeleteSelectionCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return DeleteSelectionCommandId; }
    bool mergeWith(const QUndoCommand *other) override;

    // For testing
    const QMap<RME::core::Position, std::unique_ptr<RME::core::Tile>>& getUndoneTileStates() const { return m_undoneTileStates; }

private:
    RME::core::Map* m_map;
    // RME::core::selection::SelectionManager* m_selectionManager; // Removed
    RME::core::editor::EditorControllerInterface* m_controller; // For map notifications

    // Stores deep copies of the tiles *before* their contents were cleared by redo()
    QMap<RME::core::Position, std::unique_ptr<RME::core::Tile>> m_undoneTileStates;
    QList<RME::core::Position> m_affectedPositions; // To ensure consistent iteration order for notifications
    bool m_firstRun;
};

} // namespace RME_COMMANDS
#endif // RME_DELETESELECTIONCOMMAND_H
