#ifndef RME_UPDATESPAWNCOMMAND_H
#define RME_UPDATESPAWNCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
#include "core/assets/SpawnData.h" // RME::core::assets::SpawnData

// Forward declarations
namespace RME {
namespace core {
    class Map;
    // class Tile; // Not directly manipulated by this command for its own state, only via Map
    namespace editor {
        class EditorControllerInterface;
    }
} // namespace core

namespace editor_logic {
namespace commands {

class UpdateSpawnCommand : public QUndoCommand {
public:
    UpdateSpawnCommand(
        RME::core::Map* map,
        // const RME::core::Position& spawnCenterPos, // The center position of the spawn being updated
        const RME::core::assets::SpawnData& oldSpawnData, // A copy of the spawn data *before* changes
        const RME::core::assets::SpawnData& newSpawnData, // A copy of the spawn data *after* changes
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~UpdateSpawnCommand() override = default;

    void undo() override;
    void redo() override;

private:
    RME::core::Map* m_map;
    // RME::core::Position m_spawnCenterPos; // Center from oldSpawnData can be used
    RME::core::assets::SpawnData m_oldSpawnData; // Store a copy of the old state
    RME::core::assets::SpawnData m_newSpawnData; // Store a copy of the new state
    RME::core::editor::EditorControllerInterface* m_controller;

    // Helper to apply a state
    bool applySpawnData(const RME::core::assets::SpawnData& dataToApply, const RME::core::assets::SpawnData& originalDataToFind);
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_UPDATESPAWNCOMMAND_H
