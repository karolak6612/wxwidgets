#include "HouseData.h"
#include "Project_QT/src/core/map/Map.h" // For calculateSizeSqms (needs Tile definition too)
#include "Project_QT/src/core/Tile.h"   // For calculateSizeSqms
#include <QTextStream>

namespace RME {

HouseData::HouseData() :
    m_id(0),
    m_townId(0),
    m_rent(0),
    m_sizeInSqms(0),
    m_isGuildhall(false)
{
}

HouseData::HouseData(uint32_t houseId, const QString& houseName) :
    m_id(houseId),
    m_name(houseName),
    m_townId(0),
    m_rent(0),
    m_sizeInSqms(0),
    m_isGuildhall(false)
{
}

void HouseData::setEntryPoint(const Position& newEntryPoint, Map* map) {
    if (!map) {
        // qWarning("HouseData::setEntryPoint called with null Map context. Tile flags will not be updated.");
        m_entryPoint = newEntryPoint; // Update internal position anyway
        return;
    }

    Position oldEntryPoint = m_entryPoint;

    // If new entry point is same as old, nothing to do for flags or position.
    if (oldEntryPoint == newEntryPoint) {
        return;
    }

    // Clear flag on old entry point tile
    if (oldEntryPoint.isValid()) {
        Tile* oldTile = map->getTile(oldEntryPoint);
        if (oldTile) {
            oldTile->setIsHouseExit(false);
            map->notifyTileChanged(oldEntryPoint);
        } else {
            // qWarning("HouseData::setEntryPoint: Could not find old entry point tile at (%d,%d,%d) to clear exit flag.",
            //          oldEntryPoint.x, oldEntryPoint.y, oldEntryPoint.z);
        }
    }

    // Update the entry point
    m_entryPoint = newEntryPoint;

    // Set flag on new entry point tile
    if (m_entryPoint.isValid()) {
        bool created = false;
        Tile* newTile = map->getOrCreateTile(m_entryPoint, created); // Ensure tile exists
        if (newTile) {
            newTile->setIsHouseExit(true);
            map->notifyTileChanged(m_entryPoint);
        } else {
            // qWarning("HouseData::setEntryPoint: Could not get/create new entry point tile at (%d,%d,%d) to set exit flag.",
            //          m_entryPoint.x, m_entryPoint.y, m_entryPoint.z);
        }
    }
    // The HouseData object itself might be considered "changed" for saving purposes
    // if it's managed by a system that tracks changes.
}

// --- Exits Management ---
void HouseData::addExit(const Position& pos) {
    if (!m_exits.contains(pos)) {
        m_exits.append(pos);
    }
}

bool HouseData::removeExit(const Position& pos) {
    return m_exits.removeOne(pos);
}

// --- Tile Positions Management ---
void HouseData::addTilePosition(const Position& pos) {
    m_tiles.insert(pos);
}

bool HouseData::removeTilePosition(const Position& pos) {
    return m_tiles.remove(pos);
}

bool HouseData::containsTile(const Position& pos) const {
    return m_tiles.contains(pos);
}

// --- Utility Methods ---
/*
// Requires full Map and Tile definitions, and Tile::isBlocking() or similar
int HouseData::calculateSizeSqms(const Map& map) const {
    int count = 0;
    for (const Position& pos : m_tiles) {
        const Tile* tile = map.getTile(pos);
        // Assuming Tile::isWalkable() or !Tile::isBlocking()
        // For example, if Tile::isBlocking() is not available or means something else:
        // A common way is to check if there's no "blocking" ground item,
        // or if the ground itself is walkable. This depends on game mechanics.
        // Simplified: just count tiles for now if detailed logic isn't available.
        if (tile) { // Further checks needed here based on actual tile properties
            count++;
        }
    }
    return count;
}
*/

QString HouseData::getDescription() const {
    QString desc;
    QTextStream ss(&desc);
    ss << m_name << " (ID:" << m_id << "; Rent: " << m_rent;
    if (m_isGuildhall) {
        ss << "; Guildhall";
    }
    ss << ")";
    return desc;
}

} // namespace RME
