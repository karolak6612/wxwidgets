#ifndef RME_HOUSES_H
#define RME_HOUSES_H

#include <QHash>       // For QHash
#include <QList>       // For QList
#include <QString>     // For QString
#include <QtGlobal>    // For quint32
#include "HouseData.h" // Use HouseData as primary house class

// Forward declarations
namespace RME {
namespace core {
    class Map;
}
}

// Forward declaration for potential test class
class TestHouses;

namespace RME {
namespace core {
namespace houses {

class Houses {
    friend class ::TestHouses;
    friend class RME::core::Map; // Allow Map to potentially access/modify houses during load/save or complex ops

public:
    explicit Houses(RME::core::Map* map);
    ~Houses() = default;

    // Prevent copying, allow moving if needed
    Houses(const Houses&) = delete;
    Houses& operator=(const Houses&) = delete;
    Houses(Houses&&) = default;
    Houses& operator=(Houses&&) = default;

    // Creates a new HouseData with a unique ID (either desiredId if available, or next available).
    // Adds it to the manager and returns a pointer to the new HouseData.
    // Returns nullptr if a house with desiredId (and desiredId != 0) already exists.
    HouseData* createNewHouse(quint32 desiredId = 0);

    // Adds an existing HouseData object (e.g., from loading) to the manager.
    // Returns true if successfully added (ID not already taken), false otherwise.
    bool addExistingHouse(const HouseData& houseData);

    // Removes a house by its ID. This will also clean tile links on the map.
    // Returns true if the house was found and removed, false otherwise.
    bool removeHouse(quint32 houseId);

    // Retrieves a house by its ID. Returns nullptr if not found.
    HouseData* getHouse(quint32 houseId);
    const HouseData* getHouse(quint32 houseId) const;

    // Returns a list of all managed houses.
    QList<HouseData*> getAllHouses();
    QList<const HouseData*> getAllHouses() const;

    // Finds the next available unique house ID.
    quint32 getNextAvailableHouseID() const;

    // Changes the ID of an existing house.
    // Returns true if successful (old ID exists, new ID is not taken or is same as old), false otherwise.
    // This will also update tiles on the map to use the new house ID.
    bool changeHouseID(quint32 oldId, quint32 newId);

    // Removes all houses from the manager and ensures their tile links are cleaned.
    void clearAllHouses();

    // Gets the total count of houses managed.
    int getHouseCount() const { return m_housesById.size(); }
    
    // Tile management methods (moved from House class)
    void linkTileToHouse(quint32 houseId, const Position& tilePos);
    void unlinkTileFromHouse(quint32 houseId, const Position& tilePos);
    void setHouseExit(quint32 houseId, const Position& exitPos);
    
    // Door management methods (from original wxWidgets)
    quint8 getEmptyDoorID(quint32 houseId) const;
    Position getDoorPositionByID(quint32 houseId, quint8 doorId) const;
    
    // Additional utility methods
    int calculateHouseSizeInSqms(quint32 houseId) const;
    QList<Position> getHouseTilePositions(quint32 houseId) const;

private:
    RME::core::Map* m_map; // Non-owning pointer to the map context
    // Stores HouseData objects, keyed by their ID.
    QHash<quint32, HouseData> m_housesById;
};

} // namespace houses
} // namespace core
} // namespace RME

#endif // RME_HOUSES_H
