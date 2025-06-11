/**
 * @file ChangeSetCommand.h
 * @brief Defines the ChangeSetCommand class for managing multiple tile changes.
 */

#ifndef RME_CHANGE_SET_COMMAND_H
#define RME_CHANGE_SET_COMMAND_H

#include "core/actions/AppUndoCommand.h"
#include "core/Position.h"
#include "core/Tile.h"      // For std::unique_ptr<Tile> and Tile::deepCopy()
#include <memory>           // For std::unique_ptr
#include <QList>            // For QList
#include <QPair>            // For QPair (used in constructor parameter)
#include <QString>          // For command text

// Forward declaration
namespace RME { namespace core { class Map; }}

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Represents a command that groups multiple tile changes into a single undo/redo operation.
 *
 * This command is useful for actions like brush strokes, fill operations, or pasting
 * a selection of tiles, where multiple individual tile modifications should be treated
 * as a single atomic change.
 */
class ChangeSetCommand : public AppUndoCommand {
public:
    /**
     * @struct TileChange
     * @brief Stores the state of a single tile before and after a modification.
     *
     * This struct holds the position of the tile, its state before the command
     * was executed (oldTileState), and the state it will have after the command
     * is executed or redone (newTileState).
     */
    struct TileChange {
        RME::core::Position pos; ///< The position of the tile on the map.
        std::unique_ptr<RME::core::Tile> oldTileState; ///< The state of the tile before the change. Nullptr if tile didn't exist.
        std::unique_ptr<RME::core::Tile> newTileState; ///< The state of the tile after the change. Nullptr if tile is to be removed.

        // Enable moving (std::unique_ptr members)
        TileChange() = default;
        TileChange(TileChange&& other) noexcept = default;
        TileChange& operator=(TileChange&& other) noexcept = default;

        // Explicitly delete copy constructor and assignment operator
        // because std::unique_ptr makes the class move-only by default.
        // Deep copying would need to be handled manually if required elsewhere.
        TileChange(const TileChange&) = delete;
        TileChange& operator=(const TileChange&) = delete;
    };

    /**
     * @brief Constructs a ChangeSetCommand.
     *
     * The constructor takes a list of initial changes, where each change is defined by a
     * position and a unique_ptr to the new tile state for that position. The current
     * state of each tile on the map is captured as the "old" state.
     *
     * @param map Pointer to the map instance the command will operate on.
     * @param initialChanges A list of pairs, each containing a Position and a unique_ptr
     *                       to a Tile representing the new state for that position.
     *                       A nullptr Tile unique_ptr signifies that the tile at that
     *                       position should be removed.
     * @param text The descriptive text for this command (e.g., "Brush stroke").
     * @param parent Parent command, if any.
     */
    ChangeSetCommand(RME::core::Map* map,
                     const QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>>& initialChanges,
                     const QString& text,
                     QUndoCommand* parent = nullptr);

    /**
     * @brief Destroys the ChangeSetCommand.
     *
     * Cleans up resources, primarily the stored TileChange unique_ptrs.
     */
    ~ChangeSetCommand() override;

    /**
     * @brief Reverts all tile changes performed by this command.
     *
     * Iterates through the stored changes in reverse order and restores each
     * tile to its `oldTileState`.
     */
    void undo() override;

    /**
     * @brief Applies all tile changes defined by this command.
     *
     * Iterates through the stored changes and sets each tile to its `newTileState`.
     */
    void redo() override;

    /**
     * @brief Retrieves a list of all map positions affected by this command.
     * @return QList<RME::core::Position> A list of all unique positions changed by this command.
     */
    QList<RME::core::Position> getAffectedPositions() const override;

    /**
     * @brief Returns a unique ID for the command type.
     * @return int The command ID. Default is -1 (no merging).
     * @see AppUndoCommand::id()
     */
    int id() const override; // Default: no merging

    /**
     * @brief Attempts to merge this command with a subsequent command.
     * @param other The command to merge with.
     * @return bool True if merging was successful, false otherwise. Default is false.
     * @see AppUndoCommand::mergeWith()
     */
    bool mergeWith(const QUndoCommand* other) override; // Default: no merging

    /**
     * @brief Estimates the memory cost of this command.
     *
     * Calculates the cost based on the size of the command object itself,
     * the approximate cost of the list structure holding changes, and the
     * sum of estimated memory usages of all old and new tile states stored.
     * @return int The estimated memory cost in bytes. Ensures cost is at least 1.
     * @see AppUndoCommand::cost()
     */
    int cost() const override;

private:
    QList<TileChange> m_changes; ///< List of individual tile changes.
    // QString m_text; // QUndoCommand::setText is used directly.
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_CHANGE_SET_COMMAND_H
