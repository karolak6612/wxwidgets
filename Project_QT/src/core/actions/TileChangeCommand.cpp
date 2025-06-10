#include "core/actions/TileChangeCommand.h"
#include "core/Map.h"   // For Map methods like getTile, setTile
#include "core/Tile.h"  // For Tile::deepCopy()

// For setText for QUndoCommand
#include <QString>
// #include <QDebug> // For logging

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Constructs a TileChangeCommand.
 * @param map Pointer to the map on which the command operates.
 * @param pos The position of the tile to be changed.
 * @param newTileStateData A unique_ptr to the Tile object representing the new state.
 *                         This can be nullptr if the intention is to delete the tile.
 * @param parent Parent command, if any.
 */
TileChangeCommand::TileChangeCommand(
    RME::core::Map* map,
    const RME::core::Position& pos,
    std::unique_ptr<RME::core::Tile> newTileStateData, // This is the state TO BE APPLIED by redo()
    QUndoCommand* parent)
    : AppUndoCommand(map, parent), m_pos(pos), m_newTileStateData(std::move(newTileStateData)) {

    // Capture the state of the tile *before* this command is applied.
    // Assumes m_map is valid, which should be ensured by AppUndoCommand or usage context.
    const RME::core::Tile* currentTileOnMap = m_map->getTile(m_pos); // Assuming Map::getTile(Position)
    if (currentTileOnMap) {
        m_oldTileStateData = currentTileOnMap->deepCopy(); // Assuming Tile::deepCopy()
    } else {
        m_oldTileStateData = nullptr; // Represents that there was no tile (or an empty state)
    }

    // Set the command text for display in undo/redo views
    setText(QObject::tr("Change tile at (%1, %2, %3)")
                .arg(m_pos.x).arg(m_pos.y).arg(m_pos.z));
}

/**
 * @brief Destroys the TileChangeCommand.
 */
TileChangeCommand::~TileChangeCommand() {
    // std::unique_ptr members m_oldTileStateData and m_newTileStateData handle their own memory.
}

/**
 * @brief Reverts the tile to its state before the command was executed.
 *
 * This method restores the tile at `m_pos` to `m_oldTileStateData`.
 * If `m_oldTileStateData` is nullptr, the tile at `m_pos` is removed.
 */
void TileChangeCommand::undo() {
    if (!m_map) return;

    // Apply the m_oldTileStateData to the map.
    // Map::setTile should handle replacing existing tile or creating new if needed.
    // If m_oldTileStateData is nullptr, it means the tile should be removed or made empty.
    if (m_oldTileStateData) {
        // Create a copy to place on map, command retains its m_oldTileStateData
        std::unique_ptr<Tile> tileToPlace = m_oldTileStateData->deepCopy();
        m_map->setTile(m_pos, std::move(tileToPlace)); // Assuming Map::setTile takes ownership or copies
    } else {
        m_map->removeTile(m_pos); // Assuming Map::removeTile or Map::setTile(pos, nullptr)
    }

    // Placeholder: emit m_map->notifyTileChanged(m_pos); or similar
    // Or if AppUndoCommand has a signal: emit commandExecuted({m_pos});
    // qCDebug(someLoggingCategory) << "TileChangeCommand undone at" << m_pos.x << m_pos.y << m_pos.z;
}

/**
 * @brief Applies the tile change, setting the tile to its new state.
 *
 * This method sets the tile at `m_pos` to `m_newTileStateData`.
 * If `m_newTileStateData` is nullptr, the tile at `m_pos` is removed.
 */
void TileChangeCommand::redo() {
    if (!m_map) return;

    // Apply the m_newTileStateData to the map.
    if (m_newTileStateData) {
        std::unique_ptr<Tile> tileToPlace = m_newTileStateData->deepCopy();
        m_map->setTile(m_pos, std::move(tileToPlace));
    } else {
        // This means the action was to delete the tile at m_pos
        m_map->removeTile(m_pos);
    }

    // Placeholder: emit m_map->notifyTileChanged(m_pos); or similar
    // qCDebug(someLoggingCategory) << "TileChangeCommand redone at" << m_pos.x << m_pos.y << m_pos.z;
}

/**
 * @brief Returns the command ID.
 * @return int The static `CommandID` for `TileChangeCommand`.
 */
int TileChangeCommand::id() const {
    return CommandID;
}

/**
 * @brief Attempts to merge this command with another command.
 *
 * Merging is possible if `other` is also a `TileChangeCommand`, affects the
 * same tile position (`m_pos`), and they are consecutive. The `m_oldTileStateData`
 * of this command is preserved, and its `m_newTileStateData` is updated to
 * that of the `other` command. The text of the command is also updated.
 *
 * @param other The command to potentially merge with.
 * @return True if merging was successful, false otherwise.
 */
bool TileChangeCommand::mergeWith(const QUndoCommand* other) {
    const TileChangeCommand* otherCmd = dynamic_cast<const TileChangeCommand*>(other);
    if (!otherCmd) {
        return false;
    }

    // Check if the commands affect the same tile position.
    if (otherCmd->m_pos != this->m_pos) {
        return false;
    }

    // The "old" state of this command remains its original m_oldTileStateData.
    // The "new" state of this command becomes the "new" state of the other command.
    // We need to deep copy since otherCmd is const and we don't take ownership of its data.
    if (otherCmd->m_newTileStateData) {
        m_newTileStateData = otherCmd->m_newTileStateData->deepCopy();
    } else {
        m_newTileStateData = nullptr; // If the other command was to delete the tile.
    }

    // Update the command text to reflect the merged operation (optional, could be more descriptive)
    setText(otherCmd->text()); // Or generate a new text: "Modify tile at X,Y,Z (merged)"

    return true;
}

/**
 * @brief Retrieves a list of map positions affected by this command.
 * @return QList<RME::core::Position> A list containing only the position `m_pos`.
 */
QList<RME::core::Position> TileChangeCommand::getAffectedPositions() const {
    return {m_pos};
}

/**
 * @brief Estimates the memory cost of this command.
 *
 * Calculates the cost based on the size of the command object itself
 * and the estimated memory usage of the old and new tile states it stores.
 * Ensures the cost is at least 1.
 *
 * @return int The estimated memory cost in bytes.
 */
int TileChangeCommand::cost() const {
    size_t calculatedCost = sizeof(TileChangeCommand); // Base size of command object

    if (m_oldTileStateData) {
        calculatedCost += m_oldTileStateData->estimateMemoryUsage();
    }
    if (m_newTileStateData) {
        calculatedCost += m_newTileStateData->estimateMemoryUsage();
    }

    // QUndoStack requires cost to be at least 1.
    return calculatedCost > 0 ? static_cast<int>(calculatedCost) : 1;
}

} // namespace actions
} // namespace core
} // namespace RME
