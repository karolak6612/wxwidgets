#include "Map.h"
#include <algorithm> // For std::find_if, std::remove_if
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

// --- Houses ---
HouseData* Map::getHouse(quint32 houseId) {
    auto it = m_houses.find(houseId);
    if (it != m_houses.end()) {
        return &it.value();
    }
    return nullptr;
}

const HouseData* Map::getHouse(quint32 houseId) const {
     auto it = m_houses.constFind(houseId);
    if (it != m_houses.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

void Map::addHouse(const HouseData& house) {
    m_houses.insert(house.houseId, house);
    setChanged(true);
}

bool Map::removeHouse(quint32 houseId) {
    if (m_houses.remove(houseId) > 0) {
        setChanged(true);
        return true;
    }
    return false;
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

} // namespace RME
