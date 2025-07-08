#include "SelectionManager.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Creature.h"
#include "core/spawns/Spawn.h"
#include "core/map/Map.h"
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
        qDebug() << "SelectionManager::finishSelectionChange: No pending changes to process.";
        return;
    }

    // Generate command text if not provided
    QString cmdText = commandText;
    if (cmdText.isEmpty()) {
        if (m_pendingChanges.count() == 1) {
            const auto& ch = m_pendingChanges.first();
            QString action = ch.currentState ? "Select" : "Deselect";
            QString objectType = 
                ch.type == SelectionChange::TargetType::TILE ? "Tile" :
                ch.type == SelectionChange::TargetType::ITEM ? "Item" :
                ch.type == SelectionChange::TargetType::CREATURE ? "Creature" :
                ch.type == SelectionChange::TargetType::SPAWN ? "Spawn" : "Object";
            cmdText = QString("%1 %2").arg(action, objectType);
        } else {
            // Count selections vs deselections
            int selections = 0, deselections = 0;
            for (const auto& change : m_pendingChanges) {
                if (change.currentState) selections++;
                else deselections++;
            }
            
            if (selections > 0 && deselections == 0) {
                cmdText = QString("Select %1 Objects").arg(selections);
            } else if (deselections > 0 && selections == 0) {
                cmdText = QString("Deselect %1 Objects").arg(deselections);
            } else {
                cmdText = QString("Modify Selection (%1 selected, %2 deselected)").arg(selections).arg(deselections);
            }
        }
    }

    // Create and execute the selection command
    SelectionCommand* cmd = new SelectionCommand(this, m_map, m_pendingChanges, cmdText);
    
    if (m_undoStack) {
        m_undoStack->push(cmd);
        qDebug() << "SelectionManager: Created SelectionCommand with" << m_pendingChanges.count() << "changes:" << cmdText;
    } else {
        qWarning() << "SelectionManager::finishSelectionChange: No undo stack available, executing command directly";
        cmd->redo(); // Execute immediately if no undo stack
        delete cmd; // Clean up since we can't add to undo stack
    }

    m_pendingChanges.clear(); // Important to clear after passing to command
}

void SelectionManager::recordTileSelectionChange(RME::Tile* tile, bool select) {
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

void SelectionManager::recordItemSelectionChange(RME::Tile* tile, RME::Item* item, bool select) {
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

void SelectionManager::recordCreatureSelectionChange(RME::Tile* tile, RME::Creature* creature, bool select) {
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

void SelectionManager::recordSpawnSelectionChange(RME::Tile* tile, RME::core::spawns::Spawn* spawn, bool select) {
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

void SelectionManager::addTile(RME::Tile* tile) { recordTileSelectionChange(tile, true); }
void SelectionManager::removeTile(RME::Tile* tile) { recordTileSelectionChange(tile, false); }
void SelectionManager::addItem(RME::Tile* tile, RME::Item* item) { recordItemSelectionChange(tile, item, true); }
void SelectionManager::removeItem(RME::Tile* tile, RME::Item* item) { recordItemSelectionChange(tile, item, false); }
void SelectionManager::addCreature(RME::Tile* tile, RME::Creature* creature) { recordCreatureSelectionChange(tile, creature, true); }
void SelectionManager::removeCreature(RME::Tile* tile, RME::Creature* creature) { recordCreatureSelectionChange(tile, creature, false); }
void SelectionManager::addSpawn(RME::Tile* tile, RME::core::spawns::Spawn* spawn) { recordSpawnSelectionChange(tile, spawn, true); }
void SelectionManager::removeSpawn(RME::Tile* tile, RME::core::spawns::Spawn* spawn) { recordSpawnSelectionChange(tile, spawn, false); }

void SelectionManager::toggleTileSelection(RME::Tile* tile) {
    if (!tile) return;
    recordTileSelectionChange(tile, !tile->hasStateFlag(RME::TileStateFlag::SELECTED)); // Use proper method
}

void SelectionManager::toggleItemSelection(RME::Tile* tile, RME::Item* item) {
    if (!tile || !item) return;
    recordItemSelectionChange(tile, item, !item->isSelected()); // ASSUMES Item::isSelected()
}

void SelectionManager::toggleCreatureSelection(RME::Tile* tile, RME::Creature* creature) {
    if (!tile || !creature) return;
    recordCreatureSelectionChange(tile, creature, !creature->isSelected()); // ASSUMES Creature::isSelected()
}

void SelectionManager::toggleSpawnSelection(RME::Tile* tile, RME::core::spawns::Spawn* spawn) {
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
    QSet<RME::Tile*> currentSelectionSnapshot = m_selectedTiles;

    for (RME::Tile* tile : currentSelectionSnapshot) {
        if (tile->hasStateFlag(RME::TileStateFlag::SELECTED)) {
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

bool SelectionManager::isSelected(const RME::Tile* tile) const {
    if (!tile) return false;
    return tile->hasStateFlag(RME::TileStateFlag::SELECTED); // Use proper method
}

bool SelectionManager::isSelected(const RME::Tile* tile, const RME::Item* item) const {
    if (!tile || !item) return false;
    // For now, if the tile is selected, all its items are considered selected
    // In a full implementation, we'd track individual item selection states
    return isSelected(tile);
}

bool SelectionManager::isSelected(const RME::Tile* tile, const RME::core::creatures::Creature* creature) const {
    if (!tile || !creature) return false;
    // Check if creature has its own selection state
    return creature->isSelected();
}

bool SelectionManager::isSelected(const RME::Tile* tile, const RME::core::spawns::Spawn* spawn) const {
    if (!tile || !spawn) return false;
    // For now, if the tile is selected and has spawn data, the spawn is considered selected
    // In a full implementation, we'd track individual spawn selection states
    return isSelected(tile) && tile->isSpawnTile();
}

// Convenience methods for clipboard integration
bool SelectionManager::isItemSelected(const RME::Tile* tile, const RME::Item* item) const {
    return isSelected(tile, item);
}

bool SelectionManager::isCreatureSelected(const RME::Tile* tile, const RME::core::creatures::Creature* creature) const {
    return isSelected(tile, creature);
}

bool SelectionManager::isSpawnSelected(const RME::Tile* tile, const RME::core::spawns::Spawn* spawn) const {
    return isSelected(tile, spawn);
}

const QSet<RME::Tile*>& SelectionManager::getSelectedTiles() const {
    // m_selectedTiles should ideally be updated by the SelectionCommand after redo/undo
    // to accurately reflect tiles with selected content.
    // The temporary logic in finishSelectionChange is a placeholder for this.
    return m_selectedTiles;
}

bool SelectionManager::isSelectionChangeActive() const {
    return m_selectionChangeActive;
}

// --- Internal methods for direct state manipulation ---

void SelectionManager::clearSelectionInternal() {
    // Create a copy for safe iteration if setSelected might trigger signals
    // that could modify m_selectedTiles indirectly (though less likely here).
    QSet<RME::core::Tile*> tilesToClear = m_selectedTiles;
    for (RME::core::Tile* tile : tilesToClear) {
        if (tile) {
            tile->setSelected(false); // Assumes this deselects tile and its contents
        }
    }
    m_selectedTiles.clear();
    emit selectionChanged();
}

void SelectionManager::addTilesToSelectionInternal(const QList<RME::Tile*>& tilesToSelect) {
    bool changed = false;
    for (RME::Tile* tile : tilesToSelect) {
        if (tile) {
            if (!m_selectedTiles.contains(tile)) { // Add only if not already present
                m_selectedTiles.insert(tile);
                changed = true;
            }
            tile->addStateFlag(RME::TileStateFlag::SELECTED); // Use proper method
        }
    }
    if (changed) {
        emit selectionChanged();
    }
}

void SelectionManager::removeTilesFromSelectionInternal(const QList<RME::Tile*>& tilesToDeselect) {
    bool changed = false;
    for (RME::Tile* tile : tilesToDeselect) {
        if (tile) {
            if (m_selectedTiles.contains(tile)) {
                m_selectedTiles.remove(tile);
                changed = true;
            }
            tile->removeStateFlag(RME::TileStateFlag::SELECTED); // Use proper method
        }
    }
    if (changed) {
        emit selectionChanged();
    }
}

void SelectionManager::setSelectedTilesInternal(const QList<RME::Tile*>& tilesToSelect) {
    // Simpler version: clear all then add. This will emit selectionChanged twice.
    // clearSelectionInternal(); // Emits selectionChanged
    // addTilesToSelectionInternal(tilesToSelect); // Emits selectionChanged if anything was added

    // More optimized version to emit signal once if possible:
    QSet<RME::Tile*> newSelectionSet;
    for (RME::Tile* tile : tilesToSelect) {
        if (tile) {
            newSelectionSet.insert(tile);
        }
    }

    bool changed = false;
    // Deselect tiles that are in current selection but not in new one
    QList<RME::Tile*> toDeselect;
    for (RME::Tile* oldTile : m_selectedTiles) {
        if (oldTile && !newSelectionSet.contains(oldTile)) {
            toDeselect.append(oldTile);
        }
    }
    for (RME::Tile* tile : toDeselect) {
        tile->removeStateFlag(RME::TileStateFlag::SELECTED);
        m_selectedTiles.remove(tile);
        changed = true;
    }

    // Select tiles that are in new selection but not in old one (or ensure they are selected)
    for (RME::Tile* newTile : newSelectionSet) {
        if (newTile) { // newTile is already confirmed not null from loop above
            if (!m_selectedTiles.contains(newTile)) {
                 m_selectedTiles.insert(newTile);
                 changed = true;
            }
            newTile->addStateFlag(RME::TileStateFlag::SELECTED); // Use proper method
        }
    }

    if (changed) {
        emit selectionChanged();
    }
}

QList<RME::Tile*> SelectionManager::getCurrentSelectedTilesList() const {
    QList<RME::Tile*> list;
    // QSet to QList conversion
    list.reserve(m_selectedTiles.size());
    for(RME::Tile* tile : m_selectedTiles) {
        list.append(tile);
    }
    // Optionally sort for consistent order if needed by callers
    // std::sort(list.begin(), list.end(), [](Tile* a, Tile* b){ /* some criteria */ });
    return list;
}

// Position-based selection methods for easier integration
QList<RME::core::Position> SelectionManager::getSelectedPositions() const {
    QList<RME::core::Position> positions;
    positions.reserve(m_selectedTiles.size());
    
    for (RME::Tile* tile : m_selectedTiles) {
        if (tile) {
            positions.append(tile->getPosition());
        }
    }
    
    return positions;
}

void SelectionManager::setSelectedPositions(const QList<RME::core::Position>& positions) {
    if (!m_map) {
        qWarning() << "SelectionManager::setSelectedPositions: No map available";
        return;
    }
    
    // Start a selection change session
    startSelectionChange();
    
    // Clear current selection
    for (RME::Tile* tile : m_selectedTiles) {
        if (tile && tile->hasStateFlag(RME::TileStateFlag::SELECTED)) {
            recordTileSelectionChange(tile, false);
        }
    }
    
    // Add new positions to selection
    for (const RME::core::Position& pos : positions) {
        RME::Tile* tile = m_map->getTile(pos);
        if (tile && !tile->hasStateFlag(RME::TileStateFlag::SELECTED)) {
            recordTileSelectionChange(tile, true);
        }
    }
    
    // Finish the selection change
    finishSelectionChange("Set Selection");
}

void SelectionManager::clearSelection() {
    if (m_selectedTiles.isEmpty()) {
        return; // Nothing to clear
    }
    
    // Start a selection change session
    startSelectionChange();
    
    // Record deselection for all selected tiles
    for (RME::Tile* tile : m_selectedTiles) {
        if (tile && tile->hasStateFlag(RME::TileStateFlag::SELECTED)) {
            recordTileSelectionChange(tile, false);
        }
    }
    
    // Finish the selection change
    finishSelectionChange("Clear Selection");
}

bool SelectionManager::hasSelection() const {
    return !m_selectedTiles.isEmpty();
}

} // namespace RME

// #include "SelectionManager.moc" // Removed - Q_OBJECT is in header
