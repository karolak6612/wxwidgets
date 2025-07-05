#ifndef RME_TILE_CHANGE_COMMAND_H
#define RME_TILE_CHANGE_COMMAND_H

#include "core/actions/AppUndoCommand.h"
#include "core/actions/CommandIds.h"
#include "core/Position.h"
#include "core/Tile.h" // For std::unique_ptr<Tile>
#include <memory>      // For std::unique_ptr
#include <QList>       // For getAffectedPositions

// Forward declaration
// namespace RME { namespace core { class Map; class Tile; }} // Already included Tile

namespace RME {
namespace core {
namespace actions {

// Command ID for merging (optional)
// enum { TileChangeCommandId = 1 }; // Example

/**
 * @brief Represents a command that changes the state of a single tile on the map.
 *
 * This command stores the state of a tile before and after a change, allowing
 * for undo and redo operations. It supports merging consecutive changes to the
 * same tile.
 */
class TileChangeCommand : public AppUndoCommand {
public:
    /**
     * @brief Unique identifier for TileChangeCommand, used for merging.
     */
    constexpr static int CommandID = toInt(CommandId::TileChange);

    /**
     * @brief Constructs a TileChangeCommand.
     *
     * The constructor captures the current state of the tile at the given position
     * to store as the "old" state. The `newTileStateData` represents the state
     * the tile will have after this command is executed (redone).
     *
     * @param map Pointer to the map on which the command operates.
     * @param pos The position of the tile to be changed.
     * @param newTileStateData A unique_ptr to the Tile object representing the new state.
     *                         This can be nullptr if the intention is to delete the tile.
     * @param parent Parent command, if any.
     */
    TileChangeCommand(RME::core::Map* map,
                      const RME::core::Position& pos,
                      std::unique_ptr<RME::core::Tile> newTileStateData, // Can be nullptr if deleting tile
                      QUndoCommand* parent = nullptr);
    /**
     * @brief Destroys the TileChangeCommand.
     */
    ~TileChangeCommand() override;

    /**
     * @brief Reverts the tile to its state before the command was executed.
     */
    void undo() override;
    /**
     * @brief Applies the tile change, setting the tile to its new state.
     */
    void redo() override;

    /**
     * @brief Returns the command ID, used for merging.
     * @return int The static CommandID for TileChangeCommand.
     * @see AppUndoCommand::id()
     */
    int id() const override; // For merging
    /**
     * @brief Attempts to merge this command with a subsequent command.
     *
     * Merging is possible if the `other` command is also a TileChangeCommand,
     * affects the same tile position, and occurs consecutively. The merged
     * command will retain its original "old" state and adopt the "new" state
     * of the `other` command.
     * @param other The command to merge with.
     * @return bool True if merging was successful, false otherwise.
     * @see AppUndoCommand::mergeWith()
     */
    bool mergeWith(const QUndoCommand* other) override; // For merging

    /**
     * @brief Retrieves the map position affected by this command.
     * @return QList<RME::core::Position> A list containing the position of the changed tile.
     * @see AppUndoCommand::getAffectedPositions()
     */
    QList<RME::core::Position> getAffectedPositions() const override;

    /**
     * @brief Estimates the memory cost of this command.
     *
     * Calculates the cost based on the size of the command object itself
     * and the estimated memory usage of the old and new tile states it stores.
     * @return int The estimated memory cost in bytes. Ensures cost is at least 1.
     * @see AppUndoCommand::cost()
     */
    int cost() const override;

private:
    RME::core::Position m_pos;
    std::unique_ptr<RME::core::Tile> m_oldTileStateData; // State of tile at m_pos BEFORE change
    std::unique_ptr<RME::core::Tile> m_newTileStateData; // State of tile at m_pos AFTER change
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_TILE_CHANGE_COMMAND_H
