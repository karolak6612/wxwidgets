#include "Map.h"
#include "Project_QT/src/core/Tile.h" // Needed for removeHouse to update tiles
#include "core/world/TownData.h" // For RME::TownData
#include <algorithm> // For std::max, std::find_if, std::remove_if
#include <QDebug> // For qWarning, qCritical
#include "core/spawns/SpawnData.h"
// No need to include AssetManager.h directly if BaseMap handles it and it's not used otherwise

namespace RME {

Map::Map(int mapWidth, int mapHeight, int mapFloors, RME::core::assets::AssetManager* assetManager)
    : BaseMap(mapWidth, mapHeight, mapFloors, assetManager), m_changed(false), m_maxTownId(0), m_maxHouseId(0) {
    m_description = "New RME Map";
    m_versionInfo.otbmVersion = 4;
    m_versionInfo.clientVersionID = 0;
    m_versionInfo.description = "OTBM v4 / Unknown Client";
    // m_clientVersionInfo is default constructed (major=0, minor=0, build=0)
}

const RME::core::ClientVersionInfo& Map::getClientVersionInfo() const {
    return m_clientVersionInfo;
}

void Map::setClientVersionInfo(const RME::core::ClientVersionInfo& versionInfo) {
    if (m_clientVersionInfo != versionInfo) {
        m_clientVersionInfo = versionInfo;
        setChanged(true);
    }
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

bool Map::addTown(RME::core::world::TownData&& townData) {
    if (townData.name.isEmpty() || townData.id == 0) { // Basic validation
        qWarning("Map::addTown: Town name cannot be empty and ID cannot be 0.");
        return false;
    }
    // QMap::insert will overwrite if key exists.
    // If we want to distinguish add vs update, check contains first.
    // bool existing = m_townsById.contains(townData.id);
    m_townsById.insert(townData.id, std::move(townData));
    m_maxTownId = std::max(m_maxTownId, townData.id); // Ensure m_maxTownId is updated
    setChanged(true);
    return true; // existing ? indicates_update : indicates_add;
                 // For now, always true on success.
}

RME::core::world::TownData* Map::getTown(uint32_t townId) {
    auto it = m_townsById.find(townId);
    if (it != m_townsById.end()) {
        return &it.value();
    }
    return nullptr;
}

const RME::core::world::TownData* Map::getTown(uint32_t townId) const {
    auto it = m_townsById.constFind(townId);
    if (it != m_townsById.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

bool Map::removeTown(uint32_t townId) {
    if (m_townsById.remove(townId) > 0) {
        setChanged(true);
        // Additionally, might need to update houses that reference this townId.
        // This could be a simple loop through m_housesById or a more complex notification system.
        // For now, just removing the town definition.
        // for (auto& housePair : m_housesById) { // Assuming m_housesById exists
        //     if (housePair.second.townId == townId) {
        //         housePair.second.townId = 0; // Reset townId for affected houses
        //     }
        // }
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

bool Map::addHouse(RME::core::houses::HouseData&& houseData) {
    if (houseData.name.isEmpty() || houseData.id == 0) {
        qWarning("Map::addHouse: House name cannot be empty and ID cannot be 0.");
        return false;
    }
    m_housesById.insert(houseData.id, std::move(houseData));
    if (houseData.id > m_maxHouseId) {
        m_maxHouseId = houseData.id;
    }
    setChanged(true);
    return true;
}

RME::core::houses::HouseData* Map::getHouse(uint32_t houseId) {
    auto it = m_housesById.find(houseId);
    if (it != m_housesById.end()) {
        return &it.value();
    }
    return nullptr;
}

const RME::core::houses::HouseData* Map::getHouse(uint32_t houseId) const {
    auto it = m_housesById.constFind(houseId);
    if (it != m_housesById.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

QMap<uint32_t, RME::core::houses::HouseData>& Map::getHouses() {
    // Note: Caller should call setChanged(true) if they modify the map via this reference,
    // or this method should set m_changed = true unconditionally if direct map access implies modification.
    // For now, direct access does not automatically set m_changed.
    return m_housesById;
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
            // markTileDirty(tilePos); // Mark tile dirty for rendering updates
            // TODO: Re-integrate tile dirty notification for UI updates (e.g., via signals or EditorController).
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

// Old getUnusedHouseId() removed. The new getNextFreeHouseId() is below and is preferred.

void Map::clearHouses() {
    if (!m_housesById.isEmpty()) {
        m_housesById.clear();
        m_maxHouseId = 0; // Reset max ID
        setChanged(true);
        // TODO: Iterate all tiles and set tile->setHouseID(0) for all tiles
        // that belonged to any of the cleared houses. This is complex as it requires
        // knowing which tiles belonged to which house if not clearing all house attributes
        // from all tiles universally. For now, this just clears the house definitions.
    }
}

uint32_t Map::getNextFreeHouseId() const {
    // If m_maxHouseId is 0 (e.g. new map or all houses deleted carefully recalculating maxId to 0),
    // this will return 1. If m_housesById is empty, m_maxHouseId should be 0.
    return m_maxHouseId + 1;
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
            // markTileDirty(tilePos);
            // TODO: Re-integrate tile dirty notification for UI updates (e.g., via signals or EditorController).
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

void Map::clearTowns() {
    if (!m_townsById.isEmpty()) {
        m_townsById.clear();
        setChanged(true);
        // Similar to removeTown, potentially update all houses to have townId = 0
        // for (auto& housePair : m_housesById) {
        //     housePair.second.townId = 0;
        // }
    }
}

void Map::clearWaypoints() {
    if (!m_waypoints.isEmpty()) {
        m_waypoints.clear();
        setChanged(true);
    }
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

bool Map::isValidHouseExitLocation(const Position& pos) const {
    if (!isPositionValid(pos)) {
        qDebug("Map::isValidHouseExitLocation: Position %s is outside map bounds.", qUtf8Printable(pos.toString()));
        return false;
    }

    const Tile* tile = getTile(pos);

    if (!tile) {
        qDebug("Map::isValidHouseExitLocation: No tile at %s", qUtf8Printable(pos.toString()));
        return false; // Tile must exist
    }

    if (!tile->getGround()) {
        qDebug("Map::isValidHouseExitLocation: Tile at %s has no ground.", qUtf8Printable(pos.toString()));
        return false; // Tile must have ground
    }

    if (tile->getHouseId() != 0) {
        qDebug("Map::isValidHouseExitLocation: Tile at %s is already part of a house (ID: %u).",
                qUtf8Printable(pos.toString()), tile->getHouseId());
        return false; // Tile must not be a house tile itself
    }

    if (tile->isBlocking()) {
        // isBlocking considers items and ground.
        // For an exit, we might also want to ensure no creatures block it,
        // but isBlocking() usually covers impassable items/ground.
        qDebug("Map::isValidHouseExitLocation: Tile at %s is blocking.", qUtf8Printable(pos.toString()));
        return false; // Tile must not be blocking
    }

    // Add any other game-specific rules for exits if necessary.
    // For example, some servers might restrict exits from being too close to other exits,
    // or require specific surrounding tiles. For now, these are the core RME checks.

    return true; // All checks passed
}

} // namespace RME
