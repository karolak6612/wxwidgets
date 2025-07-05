#ifndef RME_ADD_CREATURE_COMMAND_H
#define RME_ADD_CREATURE_COMMAND_H

#include "BaseCommand.h"
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME {
namespace core {
class Tile;
namespace creatures {
class Creature;
}
namespace assets {
struct CreatureData;
}
} // namespace core

namespace editor_logic {
class EditorControllerInterface; // For notifying tile changed

namespace commands {

class AddCreatureCommand : public BaseCommand {
public:
    AddCreatureCommand(
        RME::core::Tile* tile,
        const RME::core::assets::CreatureData* creatureData,
        RME::editor_logic::EditorControllerInterface* editorController, // To notify tile changes
        QUndoCommand* parent = nullptr
    );

    ~AddCreatureCommand() override;

    void undo() override;
    void redo() override;

private:
    RME::core::Tile* m_tile;
    const RME::core::assets::CreatureData* m_creatureData; // The type of creature to add

    // Store the creature that was on the tile before this command (if any)
    // and the creature that this command places, to manage their lifecycle.
    std::unique_ptr<RME::core::creatures::Creature> m_previousCreature;
    std::unique_ptr<RME::core::creatures::Creature> m_addedCreature; // Only used if redo creates it and undo needs to pop it
};

} // namespace commands
} // namespace editor_logic
} // namespace RME

#endif // RME_ADD_CREATURE_COMMAND_H
