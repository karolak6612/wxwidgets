#ifndef RME_ADDSPAWNCOMMAND_H
#define RME_ADDSPAWNCOMMAND_H

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

constexpr int AddSpawnCommandId = toInt(CommandId::AddSpawn);

class AddSpawnCommand : public QUndoCommand {
public:
    AddSpawnCommand(
        const RME::core::spawns::Spawn& spawn,
        RME::core::spawns::SpawnManager* spawnManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~AddSpawnCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return AddSpawnCommandId; }

private:
    RME::core::spawns::Spawn m_spawn;
    RME::core::spawns::SpawnManager* m_spawnManager;
    RME::core::editor::EditorControllerInterface* m_controller;
    
    bool m_hasBeenExecuted = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_ADDSPAWNCOMMAND_H