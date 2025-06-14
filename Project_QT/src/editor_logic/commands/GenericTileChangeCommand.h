#ifndef RME_GENERICTILECHANGECOMMAND_H
#define RME_GENERICTILECHANGECOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    class Map; // For notifying map of changes
    namespace editor {
        class EditorControllerInterface;
    }
} // namespace core

namespace editor_logic {
namespace commands {

class GenericTileChangeCommand : public QUndoCommand {
public:
    GenericTileChangeCommand(
        RME::core::Map* map, // To get the live tile and notify
        const RME::core::Position& tilePos,
        std::unique_ptr<RME::core::Tile> oldTileState, // Deep copy of tile before change
        std::unique_ptr<RME::core::Tile> newTileState, // Deep copy of tile after change
        RME::core::editor::EditorControllerInterface* controller, // To get asset manager for item creation
        QUndoCommand* parent = nullptr
    );

    ~GenericTileChangeCommand() override = default;

    void undo() override;
    void redo() override;

private:
    bool applyState(const RME::core::Tile* stateToApply); // Helper to apply a state to the live tile

    RME::core::Map* m_map;
    RME::core::Position m_tilePosition;
    std::unique_ptr<RME::core::Tile> m_oldTileState;
    std::unique_ptr<RME::core::Tile> m_newTileState;
    RME::core::editor::EditorControllerInterface* m_controller;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_GENERICTILECHANGECOMMAND_H
