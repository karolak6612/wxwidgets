#include "editor_logic/commands/DeleteSelectionCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/selection/SelectionManager.h"
#include "core/editor/EditorControllerInterface.h" // For getMap()->notifyTileChanged

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT
#include <set>     // For std::set to get unique positions from selection

namespace RME_COMMANDS {

DeleteSelectionCommand::DeleteSelectionCommand(
    RME::core::Map* map,
    const QList<RME::core::Position>& selectedPositions,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    // m_selectionManager(selectionManager), // Removed
    m_controller(controller),
    m_affectedPositions(selectedPositions), // Store selected positions
    m_firstRun(true) // Still useful for one-time state capture in redo if needed
{
    Q_ASSERT(m_map);
    // Q_ASSERT(m_selectionManager); // Removed
    Q_ASSERT(m_controller);

    // Text will be set more accurately in redo based on actual affected tiles
    setText(QObject::tr("Delete Selection"));
}

void DeleteSelectionCommand::redo() {
    // m_affectedPositions is now set by the constructor.
    // The m_firstRun flag is used to ensure m_undoneTileStates is captured only once.

    if (m_affectedPositions.isEmpty()) {
        // If no positions were passed in (e.g. selection was empty when command created)
        // then there's nothing to do.
        setText(QObject::tr("Delete Selection (nothing selected)"));
        m_firstRun = false; // Mark as "run" so it doesn't try to capture empty state again if merged.
        return;
    }

    if (m_firstRun) {
        // Capture the state of the tiles to be affected *before* clearing them.
        // This capture should only happen once.
        for (const RME::core::Position& pos : m_affectedPositions) {
            RME::core::Tile* tile = m_map->getTile(pos);
            if (tile) {
                // Ensure not to insert if already present (e.g. if mergeWith logic was more complex)
                if (!m_undoneTileStates.contains(pos)) {
                    m_undoneTileStates.insert(pos, tile->deepCopy());
                }
            }
        }
        m_firstRun = false;
    }

    // If there's nothing in the undo state (e.g., all selected positions were invalid or pointed to null tiles)
    // and m_affectedPositions was also empty initially, then nothing to do.
    // This check is slightly redundant if m_affectedPositions.isEmpty() is checked first,
    // but good for safety if m_undoneTileStates could be empty for other reasons.
    if (m_undoneTileStates.isEmpty() && m_affectedPositions.isEmpty()) {
         setText(QObject::tr("Delete Selection (no valid tiles in selection)"));
        return;
    }

    // Proceed to clear the tiles.
    // m_affectedPositions holds the list of tiles that were part of the selection
    // when the command was created.
    int actuallyClearedCount = 0;
    for (const RME::core::Position& pos : m_affectedPositions) {
        RME::core::Tile* tile = m_map->getTileForEditing(pos);
        if (tile) {
            // Clear all contents - similar to aggressive eraser
            tile->setGround(nullptr);
            tile->clearItems();
            tile->setSpawn(nullptr);
            tile->setCreature(nullptr);
            if (m_controller && m_controller->getMap()) { // Ensure controller and map are valid
                m_controller->getMap()->notifyTileChanged(pos);
            }
        }
    }
    setText(QObject::tr("Delete Selection (%1 tile(s))").arg(m_affectedPositions.size()));
}

void DeleteSelectionCommand::undo() {
    if (m_undoneTileStates.isEmpty()) {
        // This implies that redo() didn't capture anything, possibly because m_affectedPositions was empty.
        // Or, if redo() was never called (e.g. command pushed and then immediately undone before first redo).
        // In such a case, there's no state to restore.
        return;
    }

    for (auto it = m_undoneTileStates.begin(); it != m_undoneTileStates.end(); ++it) {
        const RME::core::Position& pos = it.key();
        std::unique_ptr<RME::core::Tile>& originalTileStateCopy = it.value(); // This is the stored deep copy

        RME::core::Tile* tileOnMap = m_map->getOrCreateTile(pos);
        if (tileOnMap && originalTileStateCopy) {
            // Restore contents from the copy
            tileOnMap->setGround(originalTileStateCopy->getGround() ? originalTileStateCopy->getGround()->deepCopy() : nullptr);
            tileOnMap->clearItems(); // Clear current items before adding back old ones
            for (const auto& item_ptr : originalTileStateCopy->getItems()) {
                if(item_ptr) tileOnMap->addItem(item_ptr->deepCopy());
            }
            tileOnMap->setSpawn(originalTileStateCopy->getSpawn() ? originalTileStateCopy->getSpawn()->deepCopy() : nullptr);
            tileOnMap->setCreature(originalTileStateCopy->getCreature() ? originalTileStateCopy->getCreature()->deepCopy() : nullptr);

            if (m_controller && m_controller->getMap()) {
                 m_controller->getMap()->notifyTileChanged(pos);
            }
        }
    }
    setText(QObject::tr("Undo Delete Selection (%1 tile(s))").arg(m_undoneTileStates.size()));
}

bool DeleteSelectionCommand::mergeWith(const QUndoCommand *other) {
    if (other->id() != id()) {
        return false;
    }
    // Cast to concrete type to access its members, if needed for more complex merge.
    // const DeleteSelectionCommand *otherCmd = static_cast<const DeleteSelectionCommand*>(other);

    // A simple merge strategy: if this command hasn't run its redo yet (m_firstRun is true)
    // and the 'other' command (which would be the command just pushed before this one on stack)
    // has affected some positions, we can try to absorb its selection.
    // However, QUndoStack calls redo() on the new command *before* attempting to merge.
    // So, m_firstRun will be false when mergeWith is called if redo was executed.

    // If this command's redo has already populated m_affectedPositions and m_undoneTileStates,
    // merging becomes more complex: potentially unioning selections and undo states.
    // For now, a simple approach is to prevent merging to avoid complexity.
    // Each "Delete Selection" action will be a distinct undoable step.
    return false;
}

} // namespace RME_COMMANDS
