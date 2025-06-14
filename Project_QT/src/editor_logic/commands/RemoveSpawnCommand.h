#ifndef RME_REMOVESPAWNCOMMAND_H
#define RME_REMOVESPAWNCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
#include "core/assets/SpawnData.h" // RME::core::assets::SpawnData
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
    namespace editor {
        class EditorControllerInterface;
    }
} // namespace core

namespace editor_logic {
namespace commands {

class RemoveSpawnCommand : public QUndoCommand {
public:
    RemoveSpawnCommand(
        RME::core::Map* map,
        const RME::core::Position& spawnCenterPos, // Position of the tile hosting the spawn ref
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~RemoveSpawnCommand() override = default;

    void undo() override;
    void redo() override;

    bool wasSpawnEffectivelyRemoved() const { return m_spawnEffectivelyRemoved; }

private:
    RME::core::Map* m_map;
    RME::core::Position m_centerPosition; // Position of the tile whose spawn ref is cleared
    RME::core::editor::EditorControllerInterface* m_controller;

    std::unique_ptr<RME::core::assets::SpawnData> m_removedSpawnDataCopy;
    RME::core::assets::SpawnData* m_originalSpawnRefOnTile = nullptr;
    bool m_spawnEffectivelyRemoved = false;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_REMOVESPAWNCOMMAND_H
