#ifndef RME_REMOVESPAWNCOMMAND_H
#define RME_REMOVESPAWNCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
#include "core/spawns/Spawn.h"
#include "core/actions/CommandIds.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    namespace spawns { class SpawnManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RemoveSpawnCommandId = toInt(CommandId::RemoveSpawn);

class RemoveSpawnCommand : public QUndoCommand {
public:
    RemoveSpawnCommand(
        const RME::core::Position& position,
        RME::core::spawns::SpawnManager* spawnManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~RemoveSpawnCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RemoveSpawnCommandId; }

private:
    RME::core::Position m_position;
    RME::core::spawns::Spawn m_backupSpawn;
    RME::core::spawns::SpawnManager* m_spawnManager;
    RME::core::editor::EditorControllerInterface* m_controller;
    
    bool m_hasBackup = false;
    bool m_hasBeenExecuted = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_REMOVESPAWNCOMMAND_H