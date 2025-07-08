#include "editor_logic/commands/RemoveCreatureCommand.h"
#include "core/Tile.h"
#include "core/creatures/Creature.h" // For std::unique_ptr<Creature>
#include "core/editor/EditorControllerInterface.h" // For notifyTileChanged

#include <QString> // For setText
#include <QDebug>  // For qWarning (if needed later)

namespace RME {
namespace editor_logic {
namespace commands {

RemoveCreatureCommand::RemoveCreatureCommand(
    RME::core::Tile* tile,
    RME::editor_logic::EditorControllerInterface* editorController,
    QUndoCommand* parent
) : BaseCommand(editorController, QString(), parent),
    m_tile(tile),
    m_removedCreature(nullptr),
    m_wasCreaturePresent(false) // Set by the first redo if a creature is found
{
    if (!m_tile || !m_editorController) {
        // Set an invalid state if parameters are null
        setText("Invalid Remove Creature Command (null tile or controller)");
        // m_wasCreaturePresent remains false, so isValid() will be false.
        return;
    }
    // Initial text; might be updated in redo() if a creature is found and named.
    setText(QString("Remove Creature from (%1,%2,%3)")
                .arg(m_tile->getPosition().x)
                .arg(m_tile->getPosition().y)
                .arg(m_tile->getPosition().z));
}

RemoveCreatureCommand::~RemoveCreatureCommand() {
    // m_removedCreature (std::unique_ptr) automatically handles deletion
    // if it owns a Creature object (i.e., if it's not nullptr).
}

void RemoveCreatureCommand::redo() {
    if (!m_tile || !m_editorController) { // Check for invalid state from constructor
        return;
    }

    // The redo action is to remove the creature from the tile.
    // If m_removedCreature is not null, it means undo() was called,
    // which put m_removedCreature back onto the tile. So, we pop it again.
    // If m_removedCreature is null, this is the first time redo() is called.

    std::unique_ptr<RME::core::creatures::Creature> creatureCurrentlyOnTile = m_tile->popCreature();

    if (!m_wasCreaturePresent && !m_removedCreature) { // First execution of redo for this command instance
        m_removedCreature = std::move(creatureCurrentlyOnTile);
        if (m_removedCreature) {
            m_wasCreaturePresent = true; // A creature was actually found and removed
            setText(QString("Remove Creature: %1 from (%2,%3,%4)")
                        .arg(m_removedCreature->getName()) // Assuming Creature has getName()
                        .arg(m_tile->getPosition().x)
                        .arg(m_tile->getPosition().y)
                        .arg(m_tile->getPosition().z));
        } else {
            // No creature was on the tile to begin with.
            m_wasCreaturePresent = false;
            setText(QString("Remove Creature (none found) from (%1,%2,%3)")
                        .arg(m_tile->getPosition().x)
                        .arg(m_tile->getPosition().y)
                        .arg(m_tile->getPosition().z));
            // Command is effectively a no-op if nothing was there.
        }
    } else { // This is a subsequent redo (after an undo)
        // creatureCurrentlyOnTile should be the same creature as previously in m_removedCreature.
        // We move it back to m_removedCreature.
        m_removedCreature = std::move(creatureCurrentlyOnTile);
        // m_wasCreaturePresent and text are already set from the first redo.
    }

    if (m_wasCreaturePresent) { // Only notify if a change occurred
        m_editorController->notifyTileChanged(m_tile->getPosition());
    }
}

void RemoveCreatureCommand::undo() {
    if (!m_tile || !m_editorController) { // Check for invalid state
        return;
    }

    // Undo should only proceed if a creature was actually removed by this command.
    if (m_wasCreaturePresent) {
        // m_removedCreature should hold the creature that was taken off the tile by redo().
        // If it's null here, it means either redo() didn't find a creature (m_wasCreaturePresent would be false),
        // or it was already moved to the tile in a previous undo (which shouldn't happen if QUndoStack works correctly).
        // Or, it was moved from m_removedCreature in redo() itself and not restored.
        // The current redo logic: if m_removedCreature exists (from previous undo), it's popped and moved to itself.
        // This means m_removedCreature should always hold the creature after redo if one was present.
        if (m_removedCreature) {
            m_tile->setCreature(std::move(m_removedCreature)); // Give creature back to tile. m_removedCreature is now null.
            m_editorController->notifyTileChanged(m_tile->getPosition());
        } else {
            // This state (m_wasCreaturePresent is true, but m_removedCreature is null)
            // implies an issue with the command's state management, as a creature
            // was supposedly removed but is not available to be restored.
            // qWarning("RemoveCreatureCommand::undo: Creature was marked as present, but no creature to restore.");
        }
    }
    // If !m_wasCreaturePresent, redo() found no creature, so undo() does nothing.
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
