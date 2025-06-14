#ifndef RME_HOUSES_H
#define RME_HOUSES_H

#include <QHash>       // For QHash
#include <QList>       // For QList
#include <QString>     // For QString (potentially in House names, though not directly used by Houses.h)
#include <memory>      // For std::unique_ptr
#include <QtGlobal>    // For quint32

// Forward declarations
namespace RME {
namespace core {
    class Map;
    namespace houses {
        class House;
    }
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
    // Destructor will be default, relying on unique_ptr to manage House objects
    ~Houses() = default;

    // Prevent copying, allow moving if needed (though Houses manager usually has one instance per map)
    Houses(const Houses&) = delete;
    Houses& operator=(const Houses&) = delete;
    Houses(Houses&&) = default;
    Houses& operator=(Houses&&) = default;

    // Creates a new House object with a unique ID (either desiredId if available, or next available).
    // Adds it to the manager and returns a raw pointer to the new House.
    // Returns nullptr if a house with desiredId (and desiredId != 0) already exists.
    House* createNewHouse(quint32 desiredId = 0);

    // Adds an existing House object (e.g., from loading) to the manager.
    // Takes ownership of the House via std::unique_ptr.
    // Returns true if successfully added (ID not already taken), false otherwise.
    // If false, the passed unique_ptr is not consumed and caller retains ownership.
    bool addExistingHouse(std::unique_ptr<House> house);

    // Removes a house by its ID. This will also trigger house->cleanAllTileLinks().
    // Returns true if the house was found and removed, false otherwise.
    bool removeHouse(quint32 houseId);

    // Retrieves a house by its ID. Returns nullptr if not found.
    House* getHouse(quint32 houseId) const;

    // Returns a list of all managed houses (as raw pointers).
    QList<House*> getAllHouses() const;

    // Finds the next available unique house ID.
    quint32 getNextAvailableHouseID() const;

    // Changes the ID of an existing house.
    // Returns true if successful (old ID exists, new ID is not taken or is same as old), false otherwise.
    // This will also update the house object's internal ID.
    bool changeHouseID(quint32 oldId, quint32 newId);

    // Removes all houses from the manager and ensures their tile links are cleaned.
    void clearAllHouses();

    // Gets the total count of houses managed.
    int getHouseCount() const { return m_housesById.size(); }

private:
    RME::core::Map* m_map; // Non-owning pointer to the map context
    // Stores House objects, keyed by their ID.
    // std::unique_ptr ensures automatic memory management of House objects.
    QHash<quint32, std::unique_ptr<House>> m_housesById;
};

} // namespace houses
} // namespace core
} // namespace RME

#endif // RME_HOUSES_H
