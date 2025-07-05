/**
 * @file ChangeSetCommand.cpp
 * @brief Implements the ChangeSetCommand class for managing multiple tile changes.
 */

#include "core/actions/ChangeSetCommand.h"
#include "core/Map.h"   // For Map methods like getTile, setTile, removeTile
#include "core/Tile.h"  // For Tile::deepCopy (though unique_ptr handles ownership)

// For Q_UNUSED
#include <QtGlobal>

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Constructs a ChangeSetCommand.
 *
 * Iterates through `initialChanges`. For each entry:
 * - Captures the current tile state from `m_map->getTile(pos)` and performs a `deepCopy()`
 *   to store it as `oldTileState`. If no tile exists, `oldTileState` remains `nullptr`.
 * - Moves the `newTileState` from the input pair into the `TileChange` struct.
 * - Adds the populated `TileChange` struct to the `m_changes` list.
 * Finally, sets the descriptive text for the command using `setText(text)`.
 *
 * @param map Pointer to the map instance the command will operate on.
 * @param initialChanges A list of pairs, each containing a Position and a unique_ptr
 *                       to a Tile representing the new state for that position.
 *                       A nullptr Tile unique_ptr signifies that the tile at that
 *                       position should be removed.
 * @param text The descriptive text for this command (e.g., "Brush stroke").
 * @param parent Parent command, if any.
 */
ChangeSetCommand::ChangeSetCommand(
    RME::core::Map* map,
    const QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>>& initialChanges,
    const QString& text,
    QUndoCommand* parent)
    : AppUndoCommand(map, parent) {

    // Reserve space for efficiency if QList supports it well for complex objects
    // m_changes.reserve(initialChanges.size());

    for (const auto& pair : initialChanges) {
        TileChange tc;
        tc.pos = pair.first;

        // Capture the old state
        const RME::core::Tile* currentTileOnMap = m_map->getTile(tc.pos);
        if (currentTileOnMap) {
            tc.oldTileState = currentTileOnMap->deepCopy();
        } else {
            tc.oldTileState = nullptr;
        }

        // Move the new state.
        // The const_cast is necessary because QList<QPair<...>> makes pairs const by default in range-based for.
        // We need to move from the unique_ptr. This is generally safe if initialChanges is an rvalue
        // or not used after this constructor. If initialChanges is lvalue and used later,
        // this would empty its unique_ptrs. Given typical command usage, this should be fine.
        // A more robust way might be to pass initialChanges by value if it's intended to be consumed.
        tc.newTileState = std::move(const_cast<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>&>(pair).second);

        m_changes.append(std::move(tc));
    }
    setText(text);
}

/**
 * @brief Destroys the ChangeSetCommand.
 *
 * The `std::unique_ptr` members within `m_changes` automatically handle memory deallocation
 * for the `Tile` objects they own.
 */
ChangeSetCommand::~ChangeSetCommand() {
    // QList<TileChange> m_changes will automatically clean up its elements.
    // TileChange struct uses std::unique_ptr which will delete managed Tile objects.
}

/**
 * @brief Reverts all tile changes performed by this command.
 *
 * Iterates through `m_changes` in reverse order. For each `TileChange`:
 * - If `tc.oldTileState` is not null, a deep copy is made and set on the map using `m_map->setTile()`.
 * - If `tc.oldTileState` is null (meaning the tile originally didn't exist), `m_map->removeTile()` is called.
 */
void ChangeSetCommand::undo() {
    if (!m_map) {
        return;
    }

    // Iterate in reverse to correctly restore states if changes overlap (e.g. tile move)
    // For independent tile changes, order doesn't strictly matter but reverse is safer.
    for (int i = m_changes.size() - 1; i >= 0; --i) {
        const TileChange& tc = m_changes.at(i);
        if (tc.oldTileState) {
            m_map->setTile(tc.pos, tc.oldTileState->deepCopy());
        } else {
            m_map->removeTile(tc.pos);
        }
    }
    // Consider emitting a signal that a larger area or multiple tiles changed.
    // The AppUndoCommand::getAffectedPositions() can be used by UndoManager/Map to refresh.
}

/**
 * @brief Applies all tile changes defined by this command.
 *
 * Iterates through `m_changes` in forward order. For each `TileChange`:
 * - If `tc.newTileState` is not null, a deep copy is made and set on the map using `m_map->setTile()`.
 * - If `tc.newTileState` is null (meaning the tile is intended to be removed), `m_map->removeTile()` is called.
 */
void ChangeSetCommand::redo() {
    if (!m_map) {
        return;
    }

    for (const auto& tc : m_changes) {
        if (tc.newTileState) {
            m_map->setTile(tc.pos, tc.newTileState->deepCopy());
        } else {
            m_map->removeTile(tc.pos);
        }
    }
    // Consider emitting a signal.
}

/**
 * @brief Retrieves a list of all unique map positions affected by this command.
 *
 * Iterates through `m_changes` and collects all `TileChange::pos` into a list.
 * @return QList<RME::core::Position> A list of affected positions.
 *         Duplicates might exist if the same position was added multiple times
 *         to initialChanges, though typically positions would be unique.
 */
QList<RME::core::Position> ChangeSetCommand::getAffectedPositions() const {
    QList<RME::core::Position> positions;
    // positions.reserve(m_changes.size()); // Optimization
    for (const auto& tc : m_changes) {
        positions.append(tc.pos);
    }
    // Optional: Remove duplicate positions if necessary, though for UI refresh,
    // refreshing a position multiple times is often harmless.
    // positions.removeDuplicates(); // If QList has this or use QSet.
    return positions;
}

/**
 * @brief Returns the command ID. Currently, ChangeSetCommand does not support merging.
 * @return int Always returns -1.
 */
int ChangeSetCommand::id() const {
    return -1; // No merging by default
}

/**
 * @brief Attempts to merge this command with another. ChangeSetCommand currently does not support merging.
 * @param other The command to potentially merge with.
 * @return bool Always returns false.
 */
bool ChangeSetCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // No merging by default
}

/**
 * @brief Estimates the memory cost of this command.
 *
 * Calculates the cost based on the size of the command object itself,
 * the approximate cost of the QList structure holding the changes,
 * and the sum of estimated memory usages of all old and new tile states stored.
 * Ensures the cost is at least 1.
 *
 * @return int The estimated memory cost in bytes.
 */
int ChangeSetCommand::cost() const {
    size_t calculatedCost = sizeof(ChangeSetCommand); // Base size of command object

    // Add approximate cost of the QList structure itself (overhead + capacity for pointers)
    calculatedCost += m_changes.capacity() * sizeof(TileChange); // Cost of the list's buffer for TileChange structs
                                                                 // Important: TileChange contains unique_ptrs to tile data, not direct Tile references
                                                                 // sizeof(m_changes) would give QList object overhead.

    for (const auto& tc : m_changes) {
        if (tc.oldTileState) {
            calculatedCost += tc.oldTileState->estimateMemoryUsage();
        }
        if (tc.newTileState) {
            calculatedCost += tc.newTileState->estimateMemoryUsage();
        }
        // Add cost of the TileChange struct itself if not implicitly covered by QList capacity.
        // However, QList<TileChange> stores TileChange objects directly, so sizeof(TileChange)
        // is accounted for in m_changes.capacity() * sizeof(TileChange) if list stores by value.
        // If QList stores pointers to TileChange, then it would be sizeof(TileChange*) and need
        // to add sizeof(TileChange) per element. QList stores objects by value.
    }

    // QUndoStack requires cost to be at least 1.
    return calculatedCost > 0 ? static_cast<int>(calculatedCost) : 1;
}

} // namespace actions
} // namespace core
} // namespace RME
