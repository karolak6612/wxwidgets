#include "Map.h"
#include "Project_QT/src/core/Tile.h" // Needed for removeHouse to update tiles
#include "core/world/TownData.h" // For RME::TownData
#include <algorithm> // For std::max, std::find_if, std::remove_if
#include <QDebug> // For qWarning, qCritical
#include "core/spawns/SpawnData.h"
// No need to include AssetManager.h directly if BaseMap handles it and it's not used otherwise

namespace RME {

Map::Map(int mapWidth, int mapHeight, int mapFloors, RME::core::assets::AssetManager* assetManager)
    : BaseMap(mapWidth, mapHeight, mapFloors, assetManager), m_changed(false) {
    m_description = "New RME Map";
    m_versionInfo.otbmVersion = 4;
    m_versionInfo.clientVersionID = 0;
    m_versionInfo.description = "OTBM v4 / Unknown Client";
}

// --- Towns ---
/* OLD IMPLEMENTATIONS
TownData* Map::getTown(quint32 townId) {
    for (TownData& town : m_towns) {
        if (town.id == townId) {
            return &town;
        }
    }
    return nullptr;
}

const TownData* Map::getTown(quint32 townId) const {
    for (const TownData& town : m_towns) {
        if (town.id == townId) {
            return &town;
        }
    }
    return nullptr;
}

void Map::addTown(const TownData& town) {
    m_towns.append(town);
    setChanged(true);
}

bool Map::removeTown(quint32 townId) {
    int initialSize = m_towns.size();
    m_towns.erase(std::remove_if(m_towns.begin(), m_towns.end(),
                               [townId](const TownData& t){ return t.id == townId; }),
                m_towns.end());
    if (m_towns.size() != initialSize) {
        setChanged(true);
        return true;
    }
    return false;
}
*/

bool Map::addTown(RME::TownData&& townData) {
    uint32_t townId = townData.getId();
    if (townId == 0) {
        qWarning("Map::addTown: Town ID 0 is invalid.");
        return false;
    }
    if (m_townsById.contains(townId)) {
        qWarning("Map::addTown: Town ID %u already exists.", townId);
        return false; // Or overwrite and return true, but false for "already exists" is common
    }
    m_townsById.insert(townId, std::move(townData));
    m_maxTownId = std::max(m_maxTownId, townId);
    setChanged(true);
    return true;
}

RME::TownData* Map::getTown(uint32_t townId) {
    auto it = m_townsById.find(townId);
    if (it != m_townsById.end()) {
        return &it.value();
    }
    return nullptr;
}

const RME::TownData* Map::getTown(uint32_t townId) const {
    auto it = m_townsById.constFind(townId);
    if (it != m_townsById.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

bool Map::removeTown(uint32_t townId) {
    if (m_townsById.remove(townId) > 0) {
        setChanged(true);
        // Update m_maxTownId if the removed ID was the max
        if (townId == m_maxTownId) {
            m_maxTownId = 0;
            for (uint32_t id : m_townsById.keys()) {
                if (id > m_maxTownId) {
                    m_maxTownId = id;
                }
            }
        }
        return true;
    }
    return false;
}

uint32_t Map::getUnusedTownId() const {
    uint32_t currentId = m_maxTownId + 1;
    if (currentId == 0) currentId = 1; // Handle overflow or initial state (0 is invalid)

    while (m_townsById.contains(currentId)) {
        currentId++;
        if (currentId == 0) { // Wrapped around, very unlikely in practice
            qCritical("Map::getUnusedTownId: Could not find an unused town ID, wrapped around.");
            return 0; // Indicate error
        }
    }
    return currentId;
}

// --- Houses ---
// Implementations for the new/modified house management methods

void Map::addHouse(HouseData&& houseData) {
    uint32_t houseId = houseData.getId();
    if (houseId == 0) {
        // Optionally assign a new ID if 0 is considered invalid or placeholder
        // For now, assume ID is pre-assigned and valid.
        // qWarning("Map::addHouse: Attempted to add house with ID 0. This might be an error.");
        // return; // Or assign new ID
    }

    if (m_housesById.contains(houseId)) {
        qWarning("Map::addHouse: House ID %u already exists. Overwriting.", houseId);
    }

    m_housesById.insert(houseId, std::move(houseData));
    m_maxHouseId = std::max(m_maxHouseId, houseId);
    setChanged(true);
}

HouseData* Map::getHouse(uint32_t houseId) {
    auto it = m_housesById.find(houseId);
    if (it != m_housesById.end()) {
        return &it.value();
    }
    return nullptr;
}

const HouseData* Map::getHouse(uint32_t houseId) const {
    auto it = m_housesById.constFind(houseId);
    if (it != m_housesById.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

bool Map::removeHouse(uint32_t houseId) {
    auto it = m_housesById.find(houseId);
    if (it == m_housesById.end()) {
        return false; // House not found
    }

    const HouseData& houseToRemove = it.value();

    for (const Position& tilePos : houseToRemove.getTilePositions()) {
        Tile* tile = getTile(tilePos);
        if (tile && tile->getHouseId() == houseId) {
            tile->setHouseId(0);
            markTileDirty(tilePos); // Mark tile dirty for rendering updates
        }
    }

    for (const Position& exitPos : houseToRemove.getExits()) {
        Tile* tile = getTile(exitPos);
        if (tile) {
            // Complex logic for shared exits would go here.
            // For now, assume no specific exit flag on tile or it's handled elsewhere/later.
        }
    }

    m_housesById.erase(it);
    setChanged(true);

    if (houseId == m_maxHouseId) {
        m_maxHouseId = 0;
        for (uint32_t id : m_housesById.keys()) {
            if (id > m_maxHouseId) {
                m_maxHouseId = id;
            }
        }
    }
    return true;
}

uint32_t Map::getUnusedHouseId() {
    uint32_t currentId = m_maxHouseId + 1;
    if (currentId == 0) currentId = 1; // Handle overflow or initial state

    while (m_housesById.contains(currentId)) {
        currentId++;
        if (currentId == 0) {
            qCritical("Map::getUnusedHouseId: Could not find an unused house ID, wrapped around.");
            return 0;
        }
    }
    return currentId;
}

bool Map::changeHouseId(uint32_t oldId, uint32_t newId) {
    if (oldId == newId) return true;
    if (newId == 0) {
        qWarning("Map::changeHouseId: Cannot change house ID to 0.");
        return false;
    }
    if (m_housesById.contains(newId)) {
        qWarning("Map::changeHouseId: New house ID %u is already taken.", newId);
        return false;
    }

    auto it = m_housesById.find(oldId);
    if (it == m_housesById.end()) {
        qWarning("Map::changeHouseId: Old house ID %u not found.", oldId);
        return false;
    }

    HouseData houseData = std::move(it.value());
    m_housesById.erase(it);

    houseData.setId(newId);

    for (const Position& tilePos : houseData.getTilePositions()) {
        Tile* tile = getTile(tilePos);
        if (tile && tile->getHouseId() == oldId) {
            tile->setHouseId(newId);
            markTileDirty(tilePos);
        }
    }

    m_housesById.insert(newId, std::move(houseData));
    setChanged(true);

    m_maxHouseId = 0;
    for (uint32_t id : m_housesById.keys()) {
        if (id > m_maxHouseId) {
            m_maxHouseId = id;
        }
    }
    // Or more simply:
    // m_maxHouseId = std::max(m_maxHouseId, newId);
    // And handle m_maxHouseId update in removeHouse more efficiently.
    // The current full rescan in removeHouse and changeHouseId is safest if keys can be arbitrary.
    return true;
}

// --- Waypoints ---
RME::core::navigation::WaypointData* Map::getWaypoint(const QString& name) {
    auto it = m_waypoints.find(name);
    if (it != m_waypoints.end()) {
        return &it.value();
    }
    return nullptr;
}

const RME::core::navigation::WaypointData* Map::getWaypoint(const QString& name) const {
     auto it = m_waypoints.constFind(name);
    if (it != m_waypoints.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

// Taking by value to allow caller to move or copy.
// Then std::move into the map.
bool Map::addWaypoint(RME::core::navigation::WaypointData waypointData) {
    if (waypointData.name.isEmpty() || m_waypoints.contains(waypointData.name)) {
        return false; // Prevent adding empty name or duplicate name
    }
    m_waypoints.insert(waypointData.name, std::move(waypointData));
    setChanged(true);
    return true;
}

bool Map::removeWaypoint(const QString& name) {
    if (m_waypoints.remove(name) > 0) {
        setChanged(true);
        return true;
    }
    return false;
}

// --- Spawns ---
void Map::addSpawn(RME::SpawnData&& spawnData) {
    m_spawns.append(std::move(spawnData));
    setChanged(true);
}

QList<RME::SpawnData>& Map::getSpawns() {
    return m_spawns;
}

const QList<RME::SpawnData>& Map::getSpawns() const {
    return m_spawns;
}

bool Map::removeSpawn(const RME::SpawnData& spawnData) {
    // QList::removeOne requires the type to have operator==
    bool removed = m_spawns.removeOne(spawnData);
    if (removed) {
        setChanged(true);
    }
    return removed;
}

// --- Advanced Queries / Tile Property Queries ---
int Map::getSpawnOverlapCount(const Position& pos) const {
    int count = 0;
    for (const SpawnData& spawn : m_spawns) {
        // Check Z-level first
        if (spawn.getCenter().z != pos.z) {
            continue;
        }
        // Simple circular distance check in XY plane
        // (dx*dx + dy*dy) <= radius*radius
        int dx = pos.x - spawn.getCenter().x;
        int dy = pos.y - spawn.getCenter().y;
        if ((dx * dx + dy * dy) <= (spawn.getRadius() * spawn.getRadius())) {
            count++;
        }
    }
    return count;
}

TownData* Map::getTownByTempleLocation(const Position& pos) {
    for (auto it = m_townsById.begin(); it != m_townsById.end(); ++it) {
        if (it.value().getTemplePosition() == pos) {
            return &it.value();
        }
    }
    return nullptr;
}

const TownData* Map::getTownByTempleLocation(const Position& pos) const {
    for (auto it = m_townsById.constBegin(); it != m_townsById.constEnd(); ++it) {
        if (it.value().getTemplePosition() == pos) {
            return &it.value();
        }
    }
    return nullptr;
}

QList<HouseData*> Map::getHousesWithExitAt(const Position& pos) {
    QList<HouseData*> result;
    for (auto it = m_housesById.begin(); it != m_housesById.end(); ++it) {
        // HouseData::getExits() returns QList<Position>
        if (it.value().getExits().contains(pos)) {
            result.append(&it.value());
        }
    }
    return result;
}

QList<const HouseData*> Map::getHousesWithExitAt(const Position& pos) const {
    QList<const HouseData*> result;
    for (auto it = m_housesById.constBegin(); it != m_housesById.constEnd(); ++it) {
        if (it.value().getExits().contains(pos)) {
            result.append(&it.value());
        }
    }
    return result;
}

void Map::notifyTileChanged(const Position& pos) {
    // TODO: Add any internal map state updates if needed (e.g., dirty flags for minimap regions)
    setChanged(true); // Mark map as changed
    // Since Map is not a QObject, we cannot emit Qt signals directly.
    // If direct UI updates are needed, an observer pattern or callback list
    // would be implemented here, or EditorController could manage this.
    // For now, just marking the map as dirty is the primary effect.
    // qInfo() << "Map tile changed at:" << pos.x << pos.y << pos.z; // Optional debug log
}

} // namespace RME
