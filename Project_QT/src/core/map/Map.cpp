#include "Map.h"
#include <algorithm> // For std::find_if, std::remove_if

namespace RME {

Map::Map(int mapWidth, int mapHeight, int mapFloors, AssetManager* assetManager)
    : BaseMap(mapWidth, mapHeight, mapFloors, assetManager), changed(false) {
    // Initialize default map description or version info if desired
    description = "New RME Map";
    versionInfo.otbmVersion = 4; // Default to a common modern OTBM version
    versionInfo.clientVersionID = 0; // Needs to be set based on loaded assets or user choice
    versionInfo.description = "OTBM v4 / Unknown Client"; // Default description
}

// --- Towns ---
TownData* Map::getTown(quint32 townId) {
    for (TownData& town : towns) {
        if (town.id == townId) {
            return &town;
        }
    }
    return nullptr;
}

const TownData* Map::getTown(quint32 townId) const {
    // C++11 range-based for loop makes this cleaner
    for (const TownData& town : towns) {
        if (town.id == townId) {
            return &town;
        }
    }
    return nullptr;
}

void Map::addTown(const TownData& town) {
    // Check for existing ID before adding to prevent duplicates?
    // Or assume IDs are managed correctly by caller/loader.
    // For now, simple append.
    towns.append(town);
    setChanged(true);
}

bool Map::removeTown(quint32 townId) {
    // QList::removeIf (Qt 5.12+) or std::remove_if + erase
    int initialSize = towns.size();
    towns.erase(std::remove_if(towns.begin(), towns.end(),
                               [townId](const TownData& t){ return t.id == townId; }),
                towns.end());
    if (towns.size() != initialSize) {
        setChanged(true);
        return true;
    }
    return false;
}

// --- Houses ---
HouseData* Map::getHouse(quint32 houseId) {
    auto it = houses.find(houseId);
    if (it != houses.end()) {
        return &it.value();
    }
    return nullptr;
}

const HouseData* Map::getHouse(quint32 houseId) const {
    auto it = houses.constFind(houseId);
    if (it != houses.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

void Map::addHouse(const HouseData& house) {
    houses.insert(house.houseId, house);
    setChanged(true);
}

bool Map::removeHouse(quint32 houseId) {
    if (houses.remove(houseId) > 0) {
        setChanged(true);
        return true;
    }
    return false;
}

// --- Waypoints ---
WaypointData* Map::getWaypoint(const QString& name) {
    auto it = waypoints.find(name);
    if (it != waypoints.end()) {
        return &it.value();
    }
    return nullptr;
}

const WaypointData* Map::getWaypoint(const QString& name) const {
     auto it = waypoints.constFind(name);
    if (it != waypoints.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

void Map::addWaypoint(const WaypointData& waypoint) {
    waypoints.insert(waypoint.name, waypoint);
    setChanged(true);
}

bool Map::removeWaypoint(const QString& name) {
    if (waypoints.remove(name) > 0) {
        setChanged(true);
        return true;
    }
    return false;
}

} // namespace RME
