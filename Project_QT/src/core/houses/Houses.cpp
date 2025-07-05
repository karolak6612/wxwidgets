#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/map/Map.h" // For RME::core::Map and tile access
#include "core/Tile.h"   // For tile manipulation
#include "core/items/DoorItem.h" // For door detection and management

#include <QDebug>    // For qWarning, Q_ASSERT
#include <QtGlobal>  // For quint32
#include <algorithm> // For std::max for getNextAvailableHouseID
#include <QSet>      // For QSet<quint8> in door ID management

namespace RME {
namespace core {
namespace houses {

Houses::Houses(RME::core::Map* map)
    : m_map(map) {
    Q_ASSERT(m_map); // Houses manager needs a valid map context
}

HouseData* Houses::createNewHouse(quint32 desiredId) {
    quint32 newId = desiredId;
    if (newId == 0 || m_housesById.contains(newId)) {
        newId = getNextAvailableHouseID();
    }

    if (newId == 0) { // getNextAvailableHouseID could return 0 on failure
        qWarning("Houses::createNewHouse: Could not find or assign a unique valid ID for the new house.");
        return nullptr;
    }

    if (m_housesById.contains(newId)) {
        qWarning("Houses::createNewHouse: ID %u is already taken, even after trying to find next available.").arg(newId);
        return nullptr;
    }

    HouseData newHouse;
    newHouse.id = newId;
    newHouse.name = QString("House %1").arg(newId); // Default name
    
    m_housesById.insert(newId, newHouse);
    return &m_housesById[newId];
}

bool Houses::addExistingHouse(const HouseData& houseData) {
    if (houseData.id == 0) {
        qWarning("Houses::addExistingHouse: House with ID 0 cannot be added.");
        return false;
    }
    if (m_housesById.contains(houseData.id)) {
        qWarning("Houses::addExistingHouse: House with ID %u already exists.").arg(houseData.id);
        return false;
    }

    m_housesById.insert(houseData.id, houseData);
    return true;
}

bool Houses::removeHouse(quint32 houseId) {
    auto it = m_housesById.find(houseId);
    if (it == m_housesById.end()) {
        return false; // House not found
    }

    // Clean all tile links for this house
    if (m_map) {
        const HouseData& house = it.value();
        // Clear house ID from all tiles that belong to this house
        // Note: This is a simplified approach - in a full implementation,
        // we'd need to track which tiles belong to each house
        for (int x = 0; x < m_map->getWidth(); ++x) {
            for (int y = 0; y < m_map->getHeight(); ++y) {
                for (int z = 0; z < m_map->getFloors(); ++z) {
                    Position pos(x, y, z);
                    Tile* tile = m_map->getTile(pos);
                    if (tile && tile->getHouseId() == houseId) {
                        tile->setHouseId(0);
                        tile->setIsProtectionZone(false);
                        if (tile->isHouseExit()) {
                            tile->setIsHouseExit(false);
                        }
                    }
                }
            }
        }
    }

    m_housesById.erase(it);
    return true;
}

HouseData* Houses::getHouse(quint32 houseId) {
    auto it = m_housesById.find(houseId);
    if (it != m_housesById.end()) {
        return &it.value();
    }
    return nullptr;
}

const HouseData* Houses::getHouse(quint32 houseId) const {
    auto it = m_housesById.constFind(houseId);
    if (it != m_housesById.constEnd()) {
        return &it.value();
    }
    return nullptr;
}

QList<HouseData*> Houses::getAllHouses() {
    QList<HouseData*> result;
    result.reserve(m_housesById.size());
    for (auto it = m_housesById.begin(); it != m_housesById.end(); ++it) {
        result.append(&it.value());
    }
    return result;
}

QList<const HouseData*> Houses::getAllHouses() const {
    QList<const HouseData*> result;
    result.reserve(m_housesById.size());
    for (auto it = m_housesById.constBegin(); it != m_housesById.constEnd(); ++it) {
        result.append(&it.value());
    }
    return result;
}

quint32 Houses::getNextAvailableHouseID() const {
    if (m_housesById.isEmpty()) {
        return 1; // Start IDs from 1
    }

    quint32 maxId = 0;
    // Find the maximum existing ID.
    // QHash::keys() returns a QList, iterating this might be slow if N is huge.
    // A more performant way for very large N might be to keep track of maxId separately.
    // For typical numbers of houses, this is fine.
    for (quint32 id : m_housesById.keys()) {
        if (id > maxId) {
            maxId = id;
        }
    }

    quint32 nextId = maxId + 1;
    // Handle potential wrap-around or if maxId was near max quint32 value.
    // Also ensure we don't return 0 if maxId was max_quint32.
    if (nextId == 0 || nextId < maxId /*wrapped*/) {
        // Fallback: search for the first gap starting from 1.
        nextId = 1;
        while (m_housesById.contains(nextId)) {
            nextId++;
            if (nextId == 0) { // Exhausted all quint32 values, extremely unlikely.
                qWarning("Houses::getNextAvailableHouseID: No available house IDs found (exhausted quint32 range).");
                return 0; // Indicate failure: no ID available
            }
        }
    }
    return nextId;
}

bool Houses::changeHouseID(quint32 oldId, quint32 newId) {
    if (oldId == newId) return true; // No change needed

    auto itOld = m_housesById.find(oldId);
    if (itOld == m_housesById.end()) {
        qWarning("Houses::changeHouseID: House with old ID %u not found.").arg(oldId);
        return false;
    }

    if (newId == 0) {
        qWarning("Houses::changeHouseID: New house ID cannot be 0.");
        return false;
    }

    if (m_housesById.contains(newId)) {
        qWarning("Houses::changeHouseID: New house ID %u is already taken.").arg(newId);
        return false;
    }

    HouseData houseData = itOld.value();
    m_housesById.erase(itOld);

    houseData.id = newId; // Update the house ID

    m_housesById.insert(newId, houseData);

    // Update tiles on the map to use the new house ID
    if (m_map) {
        for (int x = 0; x < m_map->getWidth(); ++x) {
            for (int y = 0; y < m_map->getHeight(); ++y) {
                for (int z = 0; z < m_map->getFloors(); ++z) {
                    Position pos(x, y, z);
                    Tile* tile = m_map->getTile(pos);
                    if (tile && tile->getHouseId() == oldId) {
                        tile->setHouseId(newId);
                        m_map->notifyTileChanged(pos);
                    }
                }
            }
        }
    }
    return true;
}

void Houses::clearAllHouses() {
    if (m_map) {
        // Clear all house-related tile data
        for (int x = 0; x < m_map->getWidth(); ++x) {
            for (int y = 0; y < m_map->getHeight(); ++y) {
                for (int z = 0; z < m_map->getFloors(); ++z) {
                    Position pos(x, y, z);
                    Tile* tile = m_map->getTile(pos);
                    if (tile && tile->getHouseId() > 0) {
                        tile->setHouseId(0);
                        tile->setIsProtectionZone(false);
                        if (tile->isHouseExit()) {
                            tile->setIsHouseExit(false);
                        }
                    }
                }
            }
        }
    }
    m_housesById.clear();
}

// Tile management methods (moved from House class)
void Houses::linkTileToHouse(quint32 houseId, const Position& tilePos) {
    if (!m_map) {
        qWarning("Houses::linkTileToHouse: Map pointer is null.");
        return;
    }
    
    Tile* tile = m_map->getTile(tilePos);
    if (!tile) {
        qWarning("Houses::linkTileToHouse: Tile at position %s does not exist.").arg(tilePos.toString());
        return;
    }
    
    HouseData* house = getHouse(houseId);
    if (!house) {
        qWarning("Houses::linkTileToHouse: House with ID %u does not exist.").arg(houseId);
        return;
    }
    
    tile->setHouseId(houseId);
    // Note: Protection zone setting should be handled by brush logic, not automatically here
    m_map->notifyTileChanged(tilePos);
}

void Houses::unlinkTileFromHouse(quint32 houseId, const Position& tilePos) {
    if (!m_map) {
        qWarning("Houses::unlinkTileFromHouse: Map pointer is null.");
        return;
    }
    
    Tile* tile = m_map->getTile(tilePos);
    if (!tile) {
        return; // Tile doesn't exist, nothing to unlink
    }
    
    if (tile->getHouseId() == houseId) {
        tile->setHouseId(0);
        tile->setIsProtectionZone(false);
        if (tile->isHouseExit()) {
            tile->setIsHouseExit(false);
        }
        m_map->notifyTileChanged(tilePos);
    }
}

void Houses::setHouseExit(quint32 houseId, const Position& exitPos) {
    if (!m_map) {
        qWarning("Houses::setHouseExit: Map pointer is null.");
        return;
    }
    
    HouseData* house = getHouse(houseId);
    if (!house) {
        qWarning("Houses::setHouseExit: House with ID %u does not exist.").arg(houseId);
        return;
    }
    
    Position oldExit = house->entryPoint;
    
    // Clear old exit flag if it was valid
    if (oldExit.isValid()) {
        Tile* oldExitTile = m_map->getTile(oldExit);
        if (oldExitTile && oldExitTile->isHouseExit()) {
            oldExitTile->setIsHouseExit(false);
            m_map->notifyTileChanged(oldExit);
        }
    }
    
    // Update house entry point
    house->entryPoint = exitPos;
    
    // Set new exit flag if the new position is valid
    if (exitPos.isValid()) {
        Tile* newExitTile = m_map->getTile(exitPos);
        if (newExitTile) {
            newExitTile->setIsHouseExit(true);
            m_map->notifyTileChanged(exitPos);
        }
    }
}

// Door management methods (from original wxWidgets)
quint8 Houses::getEmptyDoorID(quint32 houseId) const {
    if (!m_map) {
        return 1; // Default if no map context
    }
    
    const HouseData* house = getHouse(houseId);
    if (!house) {
        return 1; // Default if house doesn't exist
    }
    
    // Collect all door IDs used by this house
    QSet<quint8> usedIds;
    
    // Iterate through all tiles on the map to find doors belonging to this house
    for (int x = 0; x < m_map->getWidth(); ++x) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int z = 0; z < m_map->getFloors(); ++z) {
                Position pos(x, y, z);
                const Tile* tile = m_map->getTile(pos);
                if (tile && tile->getHouseId() == houseId) {
                    // Check all items on this tile for doors
                    const auto& items = tile->getItems();
                    for (const auto& item : items) {
                        if (item) {
                            // Check if this item is a door and get its door ID
                            const DoorItem* door = dynamic_cast<const DoorItem*>(item.get());
                            if (door && door->getDoorId() > 0) {
                                usedIds.insert(door->getDoorId());
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Find the first unused door ID (1-255)
    for (quint8 id = 1; id < 255; ++id) {
        if (!usedIds.contains(id)) {
            return id;
        }
    }
    
    return 255; // Fallback if all IDs are somehow used
}

Position Houses::getDoorPositionByID(quint32 houseId, quint8 doorId) const {
    if (!m_map) {
        return Position(); // Invalid position if no map context
    }
    
    const HouseData* house = getHouse(houseId);
    if (!house) {
        return Position(); // Invalid position if house doesn't exist
    }
    
    // Iterate through all tiles on the map to find the door with the specified ID
    for (int x = 0; x < m_map->getWidth(); ++x) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int z = 0; z < m_map->getFloors(); ++z) {
                Position pos(x, y, z);
                const Tile* tile = m_map->getTile(pos);
                if (tile && tile->getHouseId() == houseId) {
                    // Check all items on this tile for doors
                    const auto& items = tile->getItems();
                    for (const auto& item : items) {
                        if (item) {
                            // Check if this item is a door with the specified ID
                            const DoorItem* door = dynamic_cast<const DoorItem*>(item.get());
                            if (door && door->getDoorId() == doorId) {
                                return pos; // Found the door at this position
                            }
                        }
                    }
                }
            }
        }
    }
    
    return Position(); // Door not found
}

// Additional utility methods
int Houses::calculateHouseSizeInSqms(quint32 houseId) const {
    if (!m_map) {
        return 0;
    }
    
    const HouseData* house = getHouse(houseId);
    if (!house) {
        return 0;
    }
    
    int walkableCount = 0;
    
    // Count walkable tiles belonging to this house
    for (int x = 0; x < m_map->getWidth(); ++x) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int z = 0; z < m_map->getFloors(); ++z) {
                Position pos(x, y, z);
                const Tile* tile = m_map->getTile(pos);
                if (tile && tile->getHouseId() == houseId) {
                    // Check if tile is walkable (not blocking)
                    if (!tile->isBlocking()) {
                        walkableCount++;
                    }
                }
            }
        }
    }
    
    return walkableCount;
}

QList<Position> Houses::getHouseTilePositions(quint32 houseId) const {
    QList<Position> positions;
    
    if (!m_map) {
        return positions;
    }
    
    const HouseData* house = getHouse(houseId);
    if (!house) {
        return positions;
    }
    
    // Collect all tile positions belonging to this house
    for (int x = 0; x < m_map->getWidth(); ++x) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int z = 0; z < m_map->getFloors(); ++z) {
                Position pos(x, y, z);
                const Tile* tile = m_map->getTile(pos);
                if (tile && tile->getHouseId() == houseId) {
                    positions.append(pos);
                }
            }
        }
    }
    
    return positions;
}

} // namespace houses
} // namespace core
} // namespace RME
