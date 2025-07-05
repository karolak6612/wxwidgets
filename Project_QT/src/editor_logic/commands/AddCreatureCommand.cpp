#include "editor_logic/commands/AddCreatureCommand.h"
#include "core/Tile.h"
#include "core/creatures/Creature.h"
#include "core/assets/CreatureData.h"
#include "editor_logic/EditorControllerInterface.h" // For notifyTileChanged

#include <QString> // For setText
#include <QDebug> // For potential qWarning/qDebug

namespace RME {
namespace editor_logic {
namespace commands {

AddCreatureCommand::AddCreatureCommand(
    RME::core::Tile* tile,
    const RME::core::assets::CreatureData* creatureData,
    RME::editor_logic::EditorControllerInterface* editorController,
    QUndoCommand* parent
) : BaseCommand(editorController, QObject::tr("Add Creature"), parent),
    m_tile(tile),
    m_creatureData(creatureData),
    m_previousCreature(nullptr),
    m_addedCreature(nullptr)
{
    if (!m_tile || !m_creatureData) {
        setErrorText("add creature - invalid parameters");
        return;
    }
    setText(QString("Add Creature: %1 to (%2,%3,%4)")
                .arg(m_creatureData->name)
                .arg(m_tile->getPosition().x)
                .arg(m_tile->getPosition().y)
                .arg(m_tile->getPosition().z)
    );
}

AddCreatureCommand::~AddCreatureCommand() {
    // unique_ptrs handle their memory automatically.
    // If a creature was popped from the tile and not given back (e.g., command destroyed mid-stack),
    // it's correctly deleted.
}

void AddCreatureCommand::redo() {
    if (!validateMembers() || !m_tile || !m_creatureData) {
        setErrorText("redo add creature");
        return;
    }

    // This is the first time redo is called for this command instance.
    // Capture the state of the tile *before* this command makes its change.
    // m_previousCreature will store whatever was on the tile.
    // If m_addedCreature is already set, it means undo() was called, and then redo() again.
    // In that case, m_previousCreature should have already been captured.
    if (!m_previousCreature && !m_addedCreature) { // True only on the very first redo execution.
        m_previousCreature = m_tile->popCreature();
    } else {
        // On subsequent redos (after an undo), the tile should currently hold m_previousCreature.
        // We pop it, but effectively discard it because m_previousCreature already holds the canonical "previous" state.
        // The creature we are about to place is in m_addedCreature (put there by undo).
        m_tile->popCreature().reset();
    }

    // Ensure m_addedCreature holds the creature instance we want to place.
    // If undo() was called, it moved the creature from the tile into m_addedCreature.
    // If this is the first run, m_addedCreature is null, so we create it.
    if (!m_addedCreature) {
        m_addedCreature = std::make_unique<RME::core::creatures::Creature>(m_creatureData, m_tile->getPosition());
    }

    m_tile->setCreature(std::move(m_addedCreature)); // Place the new/re-added creature. m_addedCreature is now null.
    notifyMapChanged(m_tile->getPosition());
}

void AddCreatureCommand::undo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("undo add creature");
        return;
    }

    // When undoing, the tile currently has the creature placed by redo().
    // We take this creature back from the tile and store it in m_addedCreature.
    // Then, we restore m_previousCreature (which was captured before the first redo) to the tile.
    m_addedCreature = m_tile->popCreature(); // Take back the creature that redo() placed.
    m_tile->setCreature(std::move(m_previousCreature)); // Restore original. m_previousCreature is now null.
    notifyMapChanged(m_tile->getPosition());
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
