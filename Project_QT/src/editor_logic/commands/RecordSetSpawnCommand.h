#ifndef RME_RECORDSETSPAWNCOMMAND_H
#define RME_RECORDSETSPAWNCOMMAND_H

#include <QUndoCommand>
#include <memory> // For std::unique_ptr
#include <QString> // For command text

#include "core/Position.h" // For RME::core::Position

// Forward declare RME::core::Spawn and RME::Tile to avoid full includes in header
namespace RME {
namespace core {
    class Spawn;
    class Tile;
    namespace editor { // Forward declare EditorControllerInterface
        class EditorControllerInterface;
    }
} // namespace core
} // namespace RME


namespace RME {
namespace editor_logic {
namespace commands { // Target namespace

const int RecordSetSpawnCommandId = 1006; // Choose a unique ID

class RecordSetSpawnCommand : public QUndoCommand {
public:
    RecordSetSpawnCommand(
        RME::core::Tile* tile,
        std::unique_ptr<RME::core::Spawn> newSpawn, // The state *after* the change
        std::unique_ptr<RME::core::Spawn> oldSpawn, // The state *before* the change
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );
    ~RecordSetSpawnCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RecordSetSpawnCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Optional

    // Getters for test verification
    const RME::core::Spawn* getSpawnForUndoState() const { return m_spawnStateForUndo.get(); }
    const RME::core::Spawn* getSpawnForRedoState() const { return m_spawnStateForRedo.get(); }

private:
    RME::core::Tile* m_tile;
    std::unique_ptr<RME::core::Spawn> m_spawnStateForRedo; // Stores the state to apply on redo
    std::unique_ptr<RME::core::Spawn> m_spawnStateForUndo; // Stores the state to apply on undo
    RME::core::editor::EditorControllerInterface* m_controller;

    RME::core::Position m_tilePosition;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_RECORDSETSPAWNCOMMAND_H
