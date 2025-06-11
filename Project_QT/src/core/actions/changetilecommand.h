#ifndef CHANGETILECOMMAND_H
#define CHANGETILECOMMAND_H

#include "actions/appundocommand.h"
#include "Position.h"
#include "Tile.h"
#include <memory>     // For std::unique_ptr
#include <QList> // For QList<Position> return type

class Map; // Forward declaration

/**
 * @brief Command for changing a single tile on the map.
 *
 * ChangeTileCommand handles the modification of a tile at a specific position.
 * It stores the state of the tile before and after the change to support
 * undo and redo operations. It also supports merging with subsequent
 * ChangeTileCommands on the same tile if they occur within a configured time window.
 */
class ChangeTileCommand : public AppUndoCommand
{
public:
    /**
     * @brief Constructs a ChangeTileCommand.
     * @param map Pointer to the Map object this command will operate on.
     * @param pos The map Position (x, y, z) of the tile to be changed.
     * @param new_tile_data A unique_ptr to a Tile object representing the new state of the tile.
     *                      The command takes ownership of this pointer. If nullptr, the tile at pos will be cleared.
     * @param parent Optional parent QUndoCommand.
     */
    ChangeTileCommand(Map* map, const Position& pos, std::unique_ptr<Tile> new_tile_data, QUndoCommand *parent = nullptr);

    /**
     * @brief Destructor. Cleans up owned Tile data.
     */
    ~ChangeTileCommand() override;

    /**
     * @brief Reverts the tile change on the map to its previous state.
     */
    void undo() override;

    /**
     * @brief Re-applies the tile change on the map.
     * On first execution, it captures the original state of the tile.
     * On subsequent executions (after an undo), it applies the new state.
     */
    void redo() override;

    /**
     * @brief Returns a unique ID for ChangeTileCommand type.
     * This ID is used by QUndoStack to identify commands that might be mergeable.
     * @return Integer ID for this command type (e.g., 1001).
     */
    int id() const override;

    /**
     * @brief Attempts to merge this command with a subsequent command.
     * Merging occurs if grouping is enabled, the other command is also a ChangeTileCommand
     * for the same position, and it occurs within the configured stacking delay.
     * @param other Pointer to the other QUndoCommand to potentially merge with.
     * @return True if merging was successful, false otherwise.
     */
    bool mergeWith(const QUndoCommand *other) override;

    /**
     * @brief Retrieves the map position affected by this command.
     * @return A QList containing the single Position object that this command modified.
     */
    QList<Position> getChangedPositions() const override;

    /**
     * @brief Enables or disables the grouping (merging) of actions.
     * @param enabled True to enable merging, false to disable.
     */
    static void setGroupActions(bool enabled);

    /**
     * @brief Sets the maximum time delay (in milliseconds) for merging consecutive commands.
     * @param ms The stacking delay in milliseconds.
     */
    static void setStackingDelay(int ms);

private:
    Position m_position; ///< The position of the tile being changed.
    std::unique_ptr<Tile> m_new_tile_data; ///< The new state of the tile (after redo).
    std::unique_ptr<Tile> m_old_tile_data; ///< The original state of the tile (before redo, for undo).
    bool m_first_execution; ///< Flag to manage capturing m_old_tile_data on the first redo.

    // Static members for merging configuration
    static bool s_group_actions_enabled; ///< Global flag to enable/disable merging for ChangeTileCommands.
    static int s_stacking_delay_ms;   ///< Global stacking delay for merging ChangeTileCommands.
};

#endif // CHANGETILECOMMAND_H
