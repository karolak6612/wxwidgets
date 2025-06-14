#ifndef RME_ADDSPAWNCOMMAND_H
#define RME_ADDSPAWNCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
// Assuming SpawnData.h defines RME::core::assets::SpawnData
#include "core/assets/SpawnData.h"
#include <memory>

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

class AddSpawnCommand : public QUndoCommand {
public:
    AddSpawnCommand(
        RME::core::Map* map,
        const RME::core::assets::SpawnData& spawnData, // Data for the new spawn
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~AddSpawnCommand() override = default;

    void undo() override;
    void redo() override;

private:
    RME::core::Map* m_map;
    RME::core::assets::SpawnData m_spawnData; // Store a copy of the spawn data to add
    RME::core::Position m_centerPosition;
    RME::core::editor::EditorControllerInterface* m_controller;

    RME::core::assets::SpawnData* m_oldSpawnRefOnTile = nullptr;
    bool m_spawnAddedToMapList = false;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_ADDSPAWNCOMMAND_H
