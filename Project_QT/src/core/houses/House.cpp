#include "core/houses/House.h"
#include "core/map/Map.h" // For RME::core::Map, if needed for validation or context
#include "core/Tile.h"    // For RME::core::Tile, for future methods like linkTile
#include <algorithm>   // For std::remove
#include <QDebug>      // For Q_ASSERT and qWarning

namespace RME {
namespace core {
namespace houses {

House::House(quint32 id, RME::core::Map* map)
    : m_id(id),
      m_name(), // Default QString is empty
      m_rent(0),
      m_townId(0),
      m_isGuildhall(false),
      m_exitPos(), // Default Position is invalid
      m_tilePositions(),
      m_map(map)
{
    Q_ASSERT(m_map != nullptr); // A house must belong to a map context
    // If id is 0, it might indicate an issue or a temporary state before a real ID is assigned.
    // The Houses manager should typically assign a valid, non-zero ID.
    if (m_id == 0) {
        qWarning("House created with ID 0. This might need to be updated by Houses manager.");
    }
}

// --- Getters implemented in header as they are simple ---
// getId, getName, getRent, getTownId, isGuildhall, getExitPos, getMap, getTilePositions

// --- Setters implemented in header as they are simple ---
// setName, setRent, setTownId, setIsGuildhall, setExitPosInternal

// --- Tile List Management ---
void House::addTilePosition(const RME::core::Position& pos) {
    if (!pos.isValid()) { // Assuming Position::isValid() exists
        qWarning("House::addTilePosition: Attempted to add invalid position.");
        return;
    }
    if (!m_tilePositions.contains(pos)) {
        m_tilePositions.append(pos);
    }
}

void House::removeTilePosition(const RME::core::Position& pos) {
    m_tilePositions.removeAll(pos); // QList::removeAll removes all occurrences
}

bool House::hasTilePosition(const RME::core::Position& pos) const {
    return m_tilePositions.contains(pos);
}

// clearTilePositions() is simple enough for header: m_tilePositions.clear();
// getTileCount() is simple enough for header: return m_tilePositions.size();

// --- Tile Interaction Logic (Placeholders/Stubs for now as per plan) ---
void House::linkTile(RME::core::Tile* tile) {
    if (!tile || !m_map) return;
    // Basic link: add position, set house ID on tile
    // Full logic might involve undo/redo commands if called directly outside HouseBrush.
    // For now, HouseBrush will handle command creation.
    addTilePosition(tile->getPosition());
    tile->setHouseId(m_id);
    // qCDebug(LogHouse) << "House" << m_id << "linked to tile" << tile->getPosition();
}

void House::unlinkTile(RME::core::Tile* tile) {
    if (!tile || !m_map) return;
    // Basic unlink: remove position, clear house ID on tile
    removeTilePosition(tile->getPosition());
    if (tile->getHouseId() == m_id) { // Only unlink if it actually belongs to this house
        tile->setHouseId(0);
    }
    // qCDebug(LogHouse) << "House" << m_id << "unlinked from tile" << tile->getPosition();
}

void House::setExit(const RME::core::Position& newExitPos) {
    if (!m_map) return;

    RME::core::Position oldExitPos = m_exitPos;
    m_exitPos = newExitPos; // Store new exit position internally

    // Update tile flags on the map
    if (oldExitPos.isValid()) {
        Tile* oldExitTile = m_map->getTile(oldExitPos);
        if (oldExitTile && oldExitTile->isHouseExit()) { // Check if it was actually this house's exit
            // Heuristic: if this tile is part of THIS house, it's unlikely to be an exit for THIS house.
            // Exits are usually outside. But if it IS flagged, clear it.
            // More robustly, Tile should store which house ID it's an exit FOR if multiple houses share exits.
            // For now, simple clear if it was an exit.
            oldExitTile->setIsHouseExit(false);
            // m_map->notifyTileChanged(oldExitPos); // Done by command
        }
    }

    if (m_exitPos.isValid()) {
        Tile* newExitTile = m_map->getOrCreateTile(m_exitPos); // Exits might be on non-house tiles, ensure tile exists
        if (newExitTile) {
            newExitTile->setIsHouseExit(true);
            // m_map->notifyTileChanged(m_exitPos); // Done by command
        }
    }
    // This whole operation should be wrapped in a QUndoCommand by the caller (e.g. a UI action or tool)
    // qCDebug(LogHouse) << "House" << m_id << "exit set to" << newExitPos;
}

void House::cleanAllTileLinks() {
    if (!m_map) return;
    // Create a copy of positions because unlinkTile might modify m_tilePositions if called directly,
    // though current unlinkTile(Tile*) doesn't modify this House's list, only the tile's ID.
    // This method is about ensuring all tiles on map that think they belong to this house are updated.
    QList<RME::core::Position> currentPositions = m_tilePositions;
    m_tilePositions.clear(); // Clear internal list first

    for (const RME::core::Position& pos : currentPositions) {
        Tile* tile = m_map->getTile(pos);
        if (tile && tile->getHouseId() == m_id) { // Double check it's still this house's tile
            tile->setHouseId(0);
            tile->setIsProtectionZone(false); // Also clear PZ when house is unlinked
            // m_map->notifyTileChanged(pos); // Done by command or caller
        }
    }

    // Also clear the exit flag if this house had one
    if (m_exitPos.isValid()) {
        Tile* exitTile = m_map->getTile(m_exitPos);
        if (exitTile && exitTile->isHouseExit()) { // Check if it was this house's exit (heuristic)
            // This needs a way for Tile to know *which* house's exit it is if shared exits are possible.
            // For now, if it's an exit and this house is being cleaned, assume it's this house's exit.
            exitTile->setIsHouseExit(false);
            // m_map->notifyTileChanged(m_exitPos);
        }
        m_exitPos = RME::core::Position(); // Clear internal exit position
    }
    // qCDebug(LogHouse) << "House" << m_id << "cleaned all tile links.";
}

} // namespace houses
} // namespace core
} // namespace RME
