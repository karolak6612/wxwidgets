#include "core/houses/Houses.h"
#include "core/houses/House.h"
#include "core/map/Map.h" // For RME::core::Map, though mainly for House constructor

#include <QDebug>    // For qWarning, Q_ASSERT
#include <QtGlobal>  // For quint32
#include <algorithm> // For std::max for getNextAvailableHouseID

namespace RME {
namespace core {
namespace houses {

Houses::Houses(RME::core::Map* map)
    : m_map(map) {
    Q_ASSERT(m_map); // Houses manager needs a valid map context
}

House* Houses::createNewHouse(quint32 desiredId) {
    quint32 newId = desiredId;
    if (newId == 0 || m_housesById.contains(newId)) {
        newId = getNextAvailableHouseID();
    }

    if (newId == 0) { // getNextAvailableHouseID could return 0 on failure
        qWarning("Houses::createNewHouse: Could not find or assign a unique valid ID for the new house.");
        return nullptr;
    }

    // This check is slightly redundant if getNextAvailableHouseID guarantees a non-taken ID,
    // but good as a safeguard if desiredId was non-zero and taken, and getNext... also failed.
    if (m_housesById.contains(newId)) {
        qWarning("Houses::createNewHouse: ID %u is already taken, even after trying to find next available.").arg(newId);
        return nullptr;
    }

    auto newHouse = std::make_unique<House>(newId, m_map);
    House* rawPtr = newHouse.get();
    m_housesById.insert(newId, std::move(newHouse));
    return rawPtr;
}

bool Houses::addExistingHouse(std::unique_ptr<House> house) {
    if (!house) {
        qWarning("Houses::addExistingHouse: Attempted to add a null house.");
        return false;
    }
    quint32 houseId = house->getId();
    if (houseId == 0) {
        qWarning("Houses::addExistingHouse: House with ID 0 cannot be added.");
        // Optionally, assign a new ID here if that's desired behavior
        // quint32 newId = getNextAvailableHouseID();
        // if (newId == 0) return false; // Could not get a valid ID
        // house->setId(newId); // Assumes House has setId and Houses is a friend
        // houseId = newId;
        return false; // For now, reject ID 0
    }
    if (m_housesById.contains(houseId)) {
        qWarning("Houses::addExistingHouse: House with ID %u already exists.").arg(houseId);
        return false; // ID collision, caller retains ownership of 'house'
    }

    if (house->getMap() != m_map) {
        qWarning("Houses::addExistingHouse: House (ID %u) has different map context. This is not directly handled by addExistingHouse.").arg(houseId);
        // Consider if house->setMap(m_map) should be called here if House has such a setter.
        // For now, the house's internal map pointer is set at its construction.
    }

    m_housesById.insert(houseId, std::move(house));
    return true;
}

bool Houses::removeHouse(quint32 houseId) {
    auto it = m_housesById.find(houseId);
    if (it == m_housesById.end()) {
        return false; // House not found
    }

    House* houseToRemove = it.value().get();
    if (houseToRemove) {
        houseToRemove->cleanAllTileLinks(); // Unlink from tiles on the map
    }

    m_housesById.erase(it); // This deletes the House object via unique_ptr
    return true;
}

House* Houses::getHouse(quint32 houseId) const {
    auto it = m_housesById.constFind(houseId);
    if (it != m_housesById.constEnd()) {
        return it.value().get();
    }
    return nullptr;
}

QList<House*> Houses::getAllHouses() const {
    QList<House*> result;
    result.reserve(m_housesById.size());
    for (auto it = m_housesById.constBegin(); it != m_housesById.constEnd(); ++it) {
        result.append(it.value().get());
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

    std::unique_ptr<House> house_ptr = std::move(itOld.value());
    m_housesById.erase(itOld);

    house_ptr->setId(newId); // House::setId is private, friend class Houses can call it.

    m_housesById.insert(newId, std::move(house_ptr));

    // Caller (e.g., a command) is responsible for updating tiles on the map.
    // This manager only handles the house list.
    // Example of what the command would do:
    // House* house = getHouse(newId);
    // if (house && m_map) {
    //     for (const Position& tilePos : house->getTilePositions()) {
    //         Tile* tile = m_map->getTile(tilePos);
    //         if (tile && tile->getHouseId() == oldId) {
    //             tile->setHouseId(newId);
    //             // m_map->notifyTileChanged(tilePos); // This should be handled by the command
    //         }
    //     }
    // }
    return true;
}

void Houses::clearAllHouses() {
    if (m_map) {
        for (auto it = m_housesById.begin(); it != m_housesById.end(); ++it) {
            House* house = it.value().get();
            if (house) {
                house->cleanAllTileLinks();
            }
        }
    }
    m_housesById.clear();
}

} // namespace houses
} // namespace core
} // namespace RME
