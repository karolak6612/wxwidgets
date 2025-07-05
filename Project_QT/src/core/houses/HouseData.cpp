#include "HouseData.h"
#include "core/map/Map.h" // Fixed include path
#include "core/Tile.h"   // Fixed include path
#include <QTextStream>

namespace RME {
namespace core {
namespace houses {

// Note: HouseData is now a simple data structure as declared in header
// The implementation with m_id, m_name etc. was inconsistent with header
// Header declares: id, name, entryPoint, exits, townId, rent, sizeInSqms
// Implementation should match header declaration

// Constructor is already defined inline in header
// HouseData(uint32_t houseId, const QString& houseName, const Position& entry)

// This method is not needed - HouseData is a simple data structure
// Entry point manipulation should be handled by House class or Houses manager
// HouseData should only store data, not manipulate map state

// These methods are already defined inline in the header
// addExit() and removeExit() work with the 'exits' QList
// No need for separate tile position management in HouseData

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
    ss << name << " (ID:" << id << "; Rent: " << rent;
    if (isGuildhall) {
        ss << "; Guildhall";
    }
    ss << ")";
    return desc;
}

} // namespace houses
} // namespace core
} // namespace RME
