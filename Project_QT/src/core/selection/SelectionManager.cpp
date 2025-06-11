#include "SelectionManager.h"
#include "Project_QT/src/core/Tile.h"
#include "Project_QT/src/core/Item.h"
#include "Project_QT/src/core/Creature.h"
#include "Project_QT/src/core/Spawn.h"
#include "Project_QT/src/core/map/Map.h"
#include "SelectionCommand.h" // Add this include

#include <QUndoStack>
#include <QDebug>

namespace RME {

SelectionManager::SelectionManager(Map* map, QUndoStack* undoStack, QObject *parent) :
    QObject(parent),
    m_map(map),
    m_undoStack(undoStack),
    m_selectionChangeActive(false)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_undoStack);
}

SelectionManager::~SelectionManager()
{
}

void SelectionManager::startSelectionChange() {
    if (m_selectionChangeActive) {
        qWarning() << "SelectionManager::startSelectionChange called while a change is already active. Clearing pending changes.";
        m_pendingChanges.clear();
    }
    m_selectionChangeActive = true;
    m_pendingChanges.clear();
}

void SelectionManager::finishSelectionChange(const QString& commandText) {
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager::finishSelectionChange called without an active change.";
        return;
    }
    m_selectionChangeActive = false;

    if (m_pendingChanges.isEmpty()) {
        return;
    }

    // --- TEMPORARY LOGIC: Apply changes directly and update m_selectedTiles ---
    // This section will be replaced by SelectionCommand integration in Step 2.
    // The actual calls to setSelected() will be done by the command's redo() method.
    // Here, we simulate it to test the logic for m_selectedTiles.
    qDebug() << "SelectionManager (Temporary): Simulating application of" << m_pendingChanges.count() << "pending changes for command:" << commandText;
    for (const auto& change : m_pendingChanges) {
        Tile* affectedTile = change.tile; // Tile context for the change

        // Simulate setting the selected state on the object
        switch (change.type) {
            case SelectionChange::TargetType::TILE:
                if (change.tile) {
                    qDebug() << "  Simulating: Tile" << change.tile << (change.currentState ? "setSelected(true)" : "setSelected(false)");
                    // change.tile->setSelected(change.currentState); // This will be in SelectionCommand
                }
                break;
            case SelectionChange::TargetType::ITEM:
                if (change.item) {
                     qDebug() << "  Simulating: Item" << change.item << "on Tile" << change.tile << (change.currentState ? "setSelected(true)" : "setSelected(false)");
                    // change.item->setSelected(change.currentState); // This will be in SelectionCommand
                }
                break;
            case SelectionChange::TargetType::CREATURE:
                if (change.creature) {
                    qDebug() << "  Simulating: Creature" << change.creature << "on Tile" << change.tile << (change.currentState ? "setSelected(true)" : "setSelected(false)");
                    // change.creature->setSelected(change.currentState); // This will be in SelectionCommand
                }
                break;
            case SelectionChange::TargetType::SPAWN:
                if (change.spawn) {
                    qDebug() << "  Simulating: Spawn" << change.spawn << "on Tile" << change.tile << (change.currentState ? "setSelected(true)" : "setSelected(false)");
                    // change.spawn->setSelected(change.currentState); // This will be in SelectionCommand
                }
                break;
        }

        // Update m_selectedTiles based on the simulated change
        if (affectedTile) {
            // To properly update m_selectedTiles, we need to know the *final* state of all elements on the tile
            // *after* all pending changes in this batch are conceptually applied.
            // This temporary direct update here is an approximation.
            // The SelectionCommand will have the definitive list of changes.

            // For now, a simplified update: if any change makes something selected on a tile, add it.
            // If a change deselects something, the check for removing from m_selectedTiles is more complex.
            bool tileShouldBeInSelectedSet = false;
            if (affectedTile->isSelected()) { // Check current state after simulated setSelected
                 tileShouldBeInSelectedSet = true;
            }
            // This needs Tile::getItems(), getCreature(), getSpawn() and Item::isSelected() etc.
            // for (Item* item : affectedTile->getItems()) { if (item->isSelected()) tileShouldBeInSelectedSet = true; break; }
            // if (affectedTile->getCreature() && affectedTile->getCreature()->isSelected()) tileShouldBeInSelectedSet = true;
            // if (affectedTile->getSpawn() && affectedTile->getSpawn()->isSelected()) tileShouldBeInSelectedSet = true;
            // A more accurate way for the temporary logic:
            // if (change.currentState) { // If this specific change results in selection
            //    m_selectedTiles.insert(affectedTile);
            // } else {
            //    // If this change is a deselection, we'd need to check if anything *else* on the tile is selected.
            //    // This is where affectedTile->hasSelectedElements() would be crucial.
            //    // if (affectedTile->hasSelectedElements()) { /* do nothing to m_selectedTiles yet */ }
            //    // else { m_selectedTiles.remove(affectedTile); }
            // }
            // Given this is temporary, let's assume for now:
             if (change.currentState) {
                 m_selectedTiles.insert(affectedTile);
             } else if (affectedTile->hasSelectedElements && !affectedTile->hasSelectedElements()) {
                 // ^ This assumes hasSelectedElements() reflects state *after* this current change would apply.
                 // This is tricky for temporary logic.
                 // A robust update to m_selectedTiles should happen *after* a command is redone/undone.
                 m_selectedTiles.remove(affectedTile);
             }
        }
    }
    // REMOVE THE TEMPORARY LOGIC FOR APPLYING CHANGES DIRECTLY HERE.
    // The SelectionCommand will now handle applying changes.

    QString cmdText = commandText;
    if (cmdText.isEmpty()) {
        // Generate a default command text based on what happened, if possible (complex)
        // For now, a generic one.
        if (m_pendingChanges.count() == 1) {
            const auto& ch = m_pendingChanges.first();
            cmdText = QString("Select %1").arg(
                ch.type == SelectionChange::TargetType::TILE ? "Tile" :
                ch.type == SelectionChange::TargetType::ITEM ? "Item" :
                ch.type == SelectionChange::TargetType::CREATURE ? "Creature" :
                ch.type == SelectionChange::TargetType::SPAWN ? "Spawn" : "Object"
            );
        } else {
            cmdText = "Select Objects";
        }
    }

    SelectionCommand* cmd = new SelectionCommand(this, m_map, m_pendingChanges, cmdText);
    m_undoStack->push(cmd);

    m_pendingChanges.clear(); // Important to clear after passing to command
}

void SelectionManager::recordTileSelectionChange(Tile* tile, bool select) {
    if (!tile) return;
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager: Modification attempted on Tile " << tile << " without active selection change. Call startSelectionChange() first.";
        return;
    }
    bool previousState = tile->isSelected(); // ASSUMES Tile::isSelected() exists
    if (previousState != select) {
        m_pendingChanges.append(SelectionChange(tile, previousState, select));
    }
}

void SelectionManager::recordItemSelectionChange(Tile* tile, Item* item, bool select) {
    if (!tile || !item) return;
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager: Modification attempted on Item " << item << " without active selection change.";
        return;
    }
    bool previousState = item->isSelected(); // ASSUMES Item::isSelected() exists
    if (previousState != select) {
        m_pendingChanges.append(SelectionChange(tile, item, previousState, select));
    }
}

void SelectionManager::recordCreatureSelectionChange(Tile* tile, Creature* creature, bool select) {
    if (!tile || !creature) return;
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager: Modification attempted on Creature " << creature << " without active selection change.";
        return;
    }
    bool previousState = creature->isSelected(); // ASSUMES Creature::isSelected() exists
    if (previousState != select) {
        m_pendingChanges.append(SelectionChange(tile, creature, previousState, select));
    }
}

void SelectionManager::recordSpawnSelectionChange(Tile* tile, Spawn* spawn, bool select) {
    if (!tile || !spawn) return;
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager: Modification attempted on Spawn " << spawn << " without active selection change.";
        return;
    }
    bool previousState = spawn->isSelected(); // ASSUMES Spawn::isSelected() exists
    if (previousState != select) {
        m_pendingChanges.append(SelectionChange(tile, spawn, previousState, select));
    }
}

void SelectionManager::addTile(Tile* tile) { recordTileSelectionChange(tile, true); }
void SelectionManager::removeTile(Tile* tile) { recordTileSelectionChange(tile, false); }
void SelectionManager::addItem(Tile* tile, Item* item) { recordItemSelectionChange(tile, item, true); }
void SelectionManager::removeItem(Tile* tile, Item* item) { recordItemSelectionChange(tile, item, false); }
void SelectionManager::addCreature(Tile* tile, Creature* creature) { recordCreatureSelectionChange(tile, creature, true); }
void SelectionManager::removeCreature(Tile* tile, Creature* creature) { recordCreatureSelectionChange(tile, creature, false); }
void SelectionManager::addSpawn(Tile* tile, Spawn* spawn) { recordSpawnSelectionChange(tile, spawn, true); }
void SelectionManager::removeSpawn(Tile* tile, Spawn* spawn) { recordSpawnSelectionChange(tile, spawn, false); }

void SelectionManager::toggleTileSelection(Tile* tile) {
    if (!tile) return;
    recordTileSelectionChange(tile, !tile->isSelected()); // ASSUMES Tile::isSelected()
}

void SelectionManager::toggleItemSelection(Tile* tile, Item* item) {
    if (!tile || !item) return;
    recordItemSelectionChange(tile, item, !item->isSelected()); // ASSUMES Item::isSelected()
}

void SelectionManager::toggleCreatureSelection(Tile* tile, Creature* creature) {
    if (!tile || !creature) return;
    recordCreatureSelectionChange(tile, creature, !creature->isSelected()); // ASSUMES Creature::isSelected()
}

void SelectionManager::toggleSpawnSelection(Tile* tile, Spawn* spawn) {
    if (!tile || !spawn) return;
    recordSpawnSelectionChange(tile, spawn, !spawn->isSelected()); // ASSUMES Spawn::isSelected()
}

void SelectionManager::clear() {
    if (!m_selectionChangeActive) {
        qWarning() << "SelectionManager::clear called without an active change. Starting one implicitly.";
        startSelectionChange();
    }

    // Create changes to deselect everything currently selected.
    // Iterate over a copy of m_selectedTiles or all tiles in map if necessary.
    // For now, this assumes m_selectedTiles gives a good starting point.
    // A truly robust clear might need to query the entire map if selection can be sparse.

    // Create a copy for iteration as m_pendingChanges might indirectly affect m_selectedTiles via temporary logic
    QSet<Tile*> currentSelectionSnapshot = m_selectedTiles;

    for (Tile* tile : currentSelectionSnapshot) {
        if (tile->isSelected()) {
            recordTileSelectionChange(tile, false);
        }
        // ASSUMPTION: Tile class provides methods like getItems(), getCreature(), getSpawn()
        // And Item, Creature, Spawn classes have isSelected().
        // Example for items:
        // for (Item* item : tile->getItems()) { // PSEUDOCODE: tile->getItems()
        //     if (item->isSelected()) {
        //         recordItemSelectionChange(tile, item, false);
        //     }
        // }
        // Similar for creatures and spawns:
        // Creature* creature = tile->getCreature(); // PSEUDOCODE
        // if (creature && creature->isSelected()) {
        //     recordCreatureSelectionChange(tile, creature, false);
        // }
        // Spawn* spawn = tile->getSpawn(); // PSEUDOCODE
        // if (spawn && spawn->isSelected()) {
        //     recordSpawnSelectionChange(tile, spawn, false);
        // }
    }
    // If items can be selected on tiles not in m_selectedTiles, this clear is insufficient.
    // The wxWidgets `Selection::clear()` iterates its own `tiles` set, which should contain all relevant tiles.
    // The logic for maintaining `m_selectedTiles` correctly is crucial.

    qDebug() << "SelectionManager: clear() has recorded pending changes to deselect relevant items/tiles.";
}

bool SelectionManager::isSelected(const Tile* tile) const {
    if (!tile) return false;
    return tile->isSelected(); // ASSUMES Tile::isSelected()
}

bool SelectionManager::isSelected(const Tile* tile, const Item* item) const {
    if (!tile || !item) return false;
    // Optionally, could add: Q_ASSERT(tile->getItems().contains(item));
    return item->isSelected(); // ASSUMES Item::isSelected()
}

bool SelectionManager::isSelected(const Tile* tile, const Creature* creature) const {
    if (!tile || !creature) return false;
    // Optionally, could add: Q_ASSERT(tile->getCreature() == creature);
    return creature->isSelected(); // ASSUMES Creature::isSelected()
}

bool SelectionManager::isSelected(const Tile* tile, const Spawn* spawn) const {
    if (!tile || !spawn) return false;
    // Optionally, could add: Q_ASSERT(tile->getSpawn() == spawn);
    return spawn->isSelected(); // ASSUMES Spawn::isSelected()
}

const QSet<Tile*>& SelectionManager::getSelectedTiles() const. {
    // m_selectedTiles should ideally be updated by the SelectionCommand after redo/undo
    // to accurately reflect tiles with selected content.
    // The temporary logic in finishSelectionChange is a placeholder for this.
    return m_selectedTiles;
}

bool SelectionManager::isSelectionChangeActive() const {
    return m_selectionChangeActive;
}

} // namespace RME
