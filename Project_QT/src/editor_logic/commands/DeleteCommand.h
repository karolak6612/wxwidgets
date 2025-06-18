#ifndef RME_DELETECOMMAND_H
#define RME_DELETECOMMAND_H

#include <QUndoCommand>
#include <QMap>
#include <QList>
#include <memory>      // For std::unique_ptr within TileData
#include <QString>     // For command text

#include "core/Position.h"
#include "core/data_transfer/TileData.h" // For RME::core::data_transfer::TileData
#include "core/actions/CommandIds.h"

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    class Map;
    namespace selection { class SelectionManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int DeleteCommandId = toInt(CommandId::Delete);

class DeleteCommand : public QUndoCommand {
public:
    DeleteCommand(
        RME::core::Map* map,
        RME::core::selection::SelectionManager* selectionManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~DeleteCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return DeleteCommandId; }

    // For testing
    const QMap<RME::core::Position, RME::core::data_transfer::TileData>& getOriginalTileData() const {
        return m_originalTileData;
    }
    const QList<RME::core::Tile*>& getPreviouslySelectedTiles() const {
        return m_previouslySelectedTiles;
    }

private:
    RME::core::Map* m_map;
    RME::core::selection::SelectionManager* m_selectionManager;
    RME::core::editor::EditorControllerInterface* m_controller;

    QMap<RME::core::Position, RME::core::data_transfer::TileData> m_originalTileData;
    QList<RME::core::Tile*> m_previouslySelectedTiles; // Raw pointers, tile objects must persist
    bool m_hadSelectionToDelete;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_DELETECOMMAND_H
