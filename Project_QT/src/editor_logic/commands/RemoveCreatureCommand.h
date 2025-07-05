#ifndef RME_REMOVE_CREATURE_COMMAND_H
#define RME_REMOVE_CREATURE_COMMAND_H

#include "BaseCommand.h"
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME {
namespace core {
class Tile;
namespace creatures {
class Creature;
}
// No CreatureData needed here as we operate on an existing creature
} // namespace core

namespace editor_logic {
class EditorControllerInterface; // For notifying tile changed

namespace commands {

class RemoveCreatureCommand : public BaseCommand {
public:
    RemoveCreatureCommand(
        RME::core::Tile* tile,
        RME::editor_logic::EditorControllerInterface* editorController,
        QUndoCommand* parent = nullptr
    );

    ~RemoveCreatureCommand() override;

    void undo() override;
    void redo() override;

    // Helper to check if a creature actually existed to be removed.
    // This allows the controller to avoid pushing an empty command.
    bool isValid() const { return m_wasCreaturePresent; }

private:
    RME::core::Tile* m_tile;
    RME::editor_logic::EditorControllerInterface* m_editorController;
    std::unique_ptr<RME::core::creatures::Creature> m_removedCreature;
    bool m_wasCreaturePresent = false; // Flag to indicate if a creature was actually removed
};

} // namespace commands
} // namespace editor_logic
} // namespace RME

#endif // RME_REMOVE_CREATURE_COMMAND_H
