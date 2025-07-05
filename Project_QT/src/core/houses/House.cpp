#include "core/houses/House.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include <algorithm>   // For std::remove for QList, though QList::removeAll is better
#include <QDebug>      // For Q_ASSERT and qWarning

namespace RME {
namespace core {
namespace houses {

House::House(quint32 id, RME::core::Map* map)
    : m_id(id),
      m_name(),
      m_rent(0),
      m_townId(0),
      m_isGuildhall(false),
      m_exitPos(),
      m_tilePositions(),
      m_map(map)
{
    Q_ASSERT(m_map != nullptr);
    if (m_id == 0) {
        qWarning("House created with ID 0. This should be updated by Houses manager.");
    }
}

// Basic getters/setters are inlined in House.h

void House::addTilePosition(const RME::core::Position& pos) {
    if (!pos.isValid()) {
        qWarning("House::addTilePosition: Attempted to add invalid position.");
        return;
    }
    if (!m_tilePositions.contains(pos)) {
        m_tilePositions.append(pos);
    }
}

void House::removeTilePosition(const RME::core::Position& pos) {
    m_tilePositions.removeAll(pos);
}

bool House::hasTilePosition(const RME::core::Position& pos) const {
    return m_tilePositions.contains(pos);
}

void House::linkTile(RME::core::Tile* tile) {
    if (!tile || !m_map) {
        qWarning("House::linkTile: Tile or map pointer is null.");
        return;
    }
    // Ensure this tile is actually on the map this house belongs to (optional check)
    // if (m_map->getTile(tile->getPosition()) != tile) {
    //     qWarning("House::linkTile: Tile does not belong to the house's map context.");
    //     return;
    // }

    addTilePosition(tile->getPosition());
    tile->setHouseId(m_id);
    // PZ flag might be set by brush, not automatically by linkTile.
    // Example: tile->setIsProtectionZone(true);
    // Based on LOGIC-05, PZ is handled by HouseBrush, not directly here.
}

void House::unlinkTile(RME::core::Tile* tile) {
    if (!tile || !m_map) {
        qWarning("House::unlinkTile: Tile or map pointer is null.");
        return;
    }

    removeTilePosition(tile->getPosition()); // Remove from this house's list
    if (tile->getHouseId() == m_id) { // Only unlink if it actually belongs to this house
        tile->setHouseId(0);
        tile->setIsProtectionZone(false); // Typically PZ is removed when house is unlinked
                                          // And also when house exit flag is cleared from a tile
    }
    // If this tile was the house exit, clear its exit flag too
    if (m_exitPos == tile->getPosition() && tile->isHouseExit()) {
        tile->setIsHouseExit(false);
        // m_exitPos itself should be cleared by setExit(invalid_pos) if house no longer has this exit
    }
}

void House::setExit(const RME::core::Position& newExitPos) {
    if (!m_map) {
        qWarning("House::setExit: Map pointer is null.");
        return;
    }

    RME::core::Position oldExit = m_exitPos;

    if (oldExit == newExitPos) {
        return; // No change
    }

    // Clear the old exit flag if it was valid
    if (oldExit.isValid()) {
        Tile* oldExitTile = m_map->getTile(oldExit);
        if (oldExitTile) {
            // Check if it was this house's exit. For simplicity, if it's an exit and matches our old pos, assume it was.
            // A more robust system might involve the tile storing which house ID it's an exit for.
            if (oldExitTile->isHouseExit()) {
                oldExitTile->setIsHouseExit(false);
                // The command would notify this tile change
            }
        }
    }

    m_exitPos = newExitPos; // Update internal exit position

    // Set the new exit flag if the new position is valid
    if (m_exitPos.isValid()) {
        // Exits might be on tiles that don't exist yet.
        // getOrCreateTile would modify map state, which should be command's job.
        // Using getTile: if tile doesn't exist, flag isn't set on map, but m_exitPos is stored.
        Tile* newExitTile = m_map->getTile(m_exitPos);
        if (newExitTile) {
            newExitTile->setIsHouseExit(true);
            // The command would notify this tile change
        } else if (m_map->isPositionValid(m_exitPos)) {
            // Tile doesn't exist at valid map position.
            // HouseBrush/Tool should ensure tile exists and is suitable (e.g. ground) before setting exit.
            // qWarning() << "House::setExit: New exit tile at" << m_exitPos << "does not exist. Exit stored but not flagged on map.";
        }
    }
    // Actual map notifications (notifyTileChanged for old and new exit tiles)
    // are handled by the QUndoCommand that calls this method.
}

void House::cleanAllTileLinks() {
    if (!m_map) {
        qWarning("House::cleanAllTileLinks: Map pointer is null.");
        return;
    }

    QList<RME::core::Position> currentPositionsSnapshot = m_tilePositions;

    for (const RME::core::Position& pos : currentPositionsSnapshot) {
        Tile* tile = m_map->getTile(pos);
        if (tile) {
            if (tile->getHouseId() == m_id) {
                tile->setHouseId(0);
                tile->setIsProtectionZone(false);
                // Map notifications handled by the command wrapping this operation.
            }
        }
    }
    m_tilePositions.clear(); // Clear the list in the House object after processing map tiles.

    if (m_exitPos.isValid()) {
        Tile* exitTile = m_map->getTile(m_exitPos);
        if (exitTile) {
            if (exitTile->isHouseExit()) {
                // Similar to setExit, assume if it's flagged and m_exitPos matched, it's ours.
                exitTile->setIsHouseExit(false);
            }
        }
        m_exitPos = RME::core::Position(); // Clear internal exit position (marks as invalid)
    }
}

} // namespace houses
} // namespace core
} // namespace RME
