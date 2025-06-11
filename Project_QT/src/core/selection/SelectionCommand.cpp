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
                    change.tile->setSelected(applyCurrentState ? change.currentState : change.previousState);
                }
                break;
            case SelectionManager::SelectionChange::TargetType::ITEM:
                if (change.item) {
                    change.item->setSelected(applyCurrentState ? change.currentState : change.previousState);
                }
                break;
            case SelectionManager::SelectionChange::TargetType::CREATURE:
                if (change.creature) {
                    change.creature->setSelected(applyCurrentState ? change.currentState : change.previousState);
                }
                break;
            case SelectionManager::SelectionChange::TargetType::SPAWN:
                if (change.spawn) {
                    change.spawn->setSelected(applyCurrentState ? change.currentState : change.previousState);
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

    QSet<Tile*> involvedTiles;
    for (const auto& change : m_changes) {
        if (change.tile) {
            involvedTiles.insert(change.tile);
        }
    }

    // Directly manipulate m_selectedTiles for now. This is a bit of a hack.
    // Ideally, SelectionManager would have a method to rebuild/update this.
    // For each involved tile, check if it should be in the set.
    for (Tile* tile : involvedTiles) {
        if (tile->hasSelectedElements()) { // Assumes Tile::hasSelectedElements() checks self + children
            m_selectionManager->m_selectedTiles.insert(tile);
        } else {
            m_selectionManager->m_selectedTiles.remove(tile);
        }
    }
    // Emitting a signal from SelectionManager that selection changed might be good practice here.
    // e.g., m_selectionManager->emitSelectionChanged();
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
