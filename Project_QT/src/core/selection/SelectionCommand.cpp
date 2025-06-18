#include "SelectionCommand.h"
#include "Project_QT/src/core/Tile.h"
#include "Project_QT/src/core/Item.h"
#include "Project_QT/src/core/Creature.h"
#include "Project_QT/src/core/Spawn.h"
#include "Project_QT/src/core/map/Map.h" // If Map class methods are needed
#include <QDebug>

namespace RME {

SelectionCommand::SelectionCommand(
    SelectionManager* selectionManager,
    Map* map,
    const QList<SelectionManager::SelectionChange>& changes,
    const QString& text,
    QUndoCommand* parent)
    : QUndoCommand(parent),
      m_selectionManager(selectionManager),
      m_map(map),
      m_changes(changes) // Store a copy
{
    setText(text);
    Q_ASSERT(m_selectionManager);
    // m_map can be null if not strictly needed by element->setSelected()
}

SelectionCommand::~SelectionCommand() {
    // m_changes will be cleaned up automatically (QList of structs/objects)
}

void SelectionCommand::applyChanges(bool applyCurrentState) {
    // This function applies either the 'currentState' (for redo) or 'previousState' (for undo)
    // from each SelectionChange struct to the actual game objects.
    for (const auto& change : m_changes) {
        switch (change.type) {
            case SelectionManager::SelectionChange::TargetType::TILE:
                if (change.tile) {
                    bool newState = applyCurrentState ? change.currentState : change.previousState;
                    if (newState) {
                        change.tile->addStateFlag(RME::TileStateFlag::SELECTED);
                    } else {
                        change.tile->removeStateFlag(RME::TileStateFlag::SELECTED);
                    }
                }
                break;
            case SelectionManager::SelectionChange::TargetType::ITEM:
                if (change.item) {
                    // TODO: Implement item selection when Item class supports it
                    // For now, items are selected as part of tile selection
                }
                break;
            case SelectionManager::SelectionChange::TargetType::CREATURE:
                if (change.creature) {
                    // TODO: Implement creature selection when Creature class supports it
                    // For now, creatures are selected as part of tile selection
                }
                break;
            case SelectionManager::SelectionChange::TargetType::SPAWN:
                if (change.spawn) {
                    // TODO: Implement spawn selection when spawn selection state is available
                    // For now, spawns are selected as part of tile selection
                }
                break;
        }
    }

    // After applying changes, SelectionManager's internal state (m_selectedTiles)
    // needs to be updated. This is crucial.
    // A simple way: iterate all tiles that were part of any change.
    // For each such unique tile, check its new state (and its children's state)
    // and add/remove it from m_selectionManager->m_selectedTiles.
    // This logic is best encapsulated in SelectionManager itself.
    // Let's assume SelectionManager has a method like:
    // void SelectionManager::resyncSelectedTilesSet(const QSet<Tile*>& involvedTiles);

    // Collect all tiles that were affected by changes
    QList<RME::Tile*> tilesToUpdate;
    QList<RME::Tile*> tilesToRemove;
    
    for (const auto& change : m_changes) {
        if (change.tile) {
            // Check if tile should be selected based on its current state
            if (change.tile->hasStateFlag(RME::TileStateFlag::SELECTED)) {
                tilesToUpdate.append(change.tile);
            } else {
                tilesToRemove.append(change.tile);
            }
        }
    }

    // Update SelectionManager's internal state using public methods
    if (!tilesToUpdate.isEmpty()) {
        m_selectionManager->addTilesToSelectionInternal(tilesToUpdate);
    }
    if (!tilesToRemove.isEmpty()) {
        m_selectionManager->removeTilesFromSelectionInternal(tilesToRemove);
    }
}

void SelectionCommand::undo() {
    applyChanges(false); // Apply previous state
    qDebug() << "SelectionCommand: Undoing -" << text();
}

void SelectionCommand::redo() {
    applyChanges(true); // Apply current state (which is the "new" state this command created)
    qDebug() << "SelectionCommand: Redoing -" << text();
}

} // namespace RME
