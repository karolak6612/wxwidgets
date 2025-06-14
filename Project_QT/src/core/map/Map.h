#ifndef RME_MAP_H
#define RME_MAP_H

#include "BaseMap.h"
// #include "MapElements.h" // For TownData, HouseData - Replaced by specific includes
#include "core/houses/HouseData.h" // Corrected include path
// #include "MapElements.h" // Assuming this is for TownData etc. and does NOT define HouseData - Removed as per CORE-110
#include "core/world/TownData.h" // New TownData location
#include "core/navigation/WaypointData.h" // New WaypointData location
#include "core/spawns/SpawnData.h" // Provides RME::SpawnData

#include <QString>
#include <QList>
#include <QMap>

namespace RME {
namespace core { // Define ClientVersionInfo within RME::core
    struct ClientVersionInfo {
        uint32_t major = 0;
        uint32_t minor = 0;
        uint32_t build = 0;

        ClientVersionInfo() = default;
        ClientVersionInfo(uint32_t maj, uint32_t min, uint32_t bld) : major(maj), minor(min), build(bld) {}

        bool isValid() const { return major != 0 || minor != 0 || build != 0; }
        void clear() { major = 0; minor = 0; build = 0; }

        bool operator==(const ClientVersionInfo& other) const {
            return major == other.major && minor == other.minor && build == other.build;
        }
        bool operator!=(const ClientVersionInfo& other) const {
            return !(*this == other);
        }
    };
} // namespace core

// Original RME namespace for Map class etc.
struct MapVersionInfo {
    quint32 otbmVersion = 0;
    quint32 clientVersionID = 0; // This might be legacy or different from the new major/minor/build
    QString description;
};

class Map : public BaseMap {
public:
    Map(int mapWidth, int mapHeight, int mapFloors, RME::core::assets::AssetManager* assetManager);
    ~Map() override = default;

    // Map Metadata
    const QString& getDescription() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; setChanged(true); }

    const MapVersionInfo& getVersionInfo() const { return m_versionInfo; } // OTBM version and legacy client ID
    void setVersionInfo(const MapVersionInfo& version) { m_versionInfo = version; setChanged(true); }
    void setOtbmVersion(quint32 otbmVer) { m_versionInfo.otbmVersion = otbmVer; setChanged(true); }
    void setClientVersionID(quint32 clientVerID) { m_versionInfo.clientVersionID = clientVerID; setChanged(true); } // Legacy client ID

    // New Client Version Info (major/minor/build)
    const RME::core::ClientVersionInfo& getClientVersionInfo() const;
    void setClientVersionInfo(const RME::core::ClientVersionInfo& versionInfo);

    const QString& getHouseFile() const { return m_houseFile; }
    void setHouseFile(const QString& path) { m_houseFile = path; setChanged(true); }

    const QString& getSpawnFile() const { return m_spawnFile; }
    void setSpawnFile(const QString& path) { m_spawnFile = path; setChanged(true); }

    const QString& getWaypointFile() const { return m_waypointFile; }
    void setWaypointFile(const QString& path) { m_waypointFile = path; setChanged(true); }

    bool hasChanged() const { return m_changed; }
    void setChanged(bool c = true) { m_changed = c; }

    // --- Towns ---
    // const QList<TownData>& getTowns() const { return m_towns; } // OLD - RME::TownData is now in core/world/
    // TownData* getTown(quint32 townId); // OLD
    // const TownData* getTown(quint32 townId) const; // OLD
    // void addTown(const TownData& town); // OLD
    // bool removeTown(quint32 townId); // OLD
    bool addTown(RME::core::world::TownData&& townData);
    RME::core::world::TownData* getTown(uint32_t townId);
    const RME::core::world::TownData* getTown(uint32_t townId) const;
    bool removeTown(uint32_t townId);
    const QMap<uint32_t, RME::core::world::TownData>& getTownsById() const { return m_townsById; } // Kept for existing compatibility
    const QMap<uint32_t, RME::core::world::TownData>& getTowns() const { return m_townsById; } // Added as per subtask
    void clearTowns(); // Added as per subtask
    uint32_t getUnusedTownId() const;

    // --- Houses ---
    // const QMap<quint32, HouseData>& getHouses() const { return m_houses; } // Old
    // HouseData* getHouse(quint32 houseId); // Old
    // const HouseData* getHouse(quint32 houseId) const; // Old
    // void addHouse(const HouseData& house); // Old
    // bool removeHouse(quint32 houseId); // Old

    bool addHouse(RME::core::houses::HouseData&& houseData);
    RME::core::houses::HouseData* getHouse(uint32_t houseId);
    const RME::core::houses::HouseData* getHouse(uint32_t houseId) const;
    bool removeHouse(uint32_t houseId); // Returns true if house was found and removed
    const QMap<uint32_t, RME::core::houses::HouseData>& getHouses() const;
    QMap<uint32_t, RME::core::houses::HouseData>& getHouses(); // Non-const version, updated for consistency
    void clearHouses();
    uint32_t getNextFreeHouseId() const; // Renamed from getUnusedHouseId and made const
    bool changeHouseId(uint32_t oldId, uint32_t newId); // Returns true on success

    // --- Waypoints ---
    const QMap<QString, RME::core::navigation::WaypointData>& getWaypoints() const { return m_waypoints; }
    RME::core::navigation::WaypointData* getWaypoint(const QString& name);
    const RME::core::navigation::WaypointData* getWaypoint(const QString& name) const;
    // Take by value to allow moving from temporary or copying, then move into map
    bool addWaypoint(RME::core::navigation::WaypointData waypointData);
    bool removeWaypoint(const QString& name);
    void clearWaypoints(); // Useful for new map or closing map

    // --- Spawns ---
    void addSpawn(RME::SpawnData&& spawnData);
    QList<RME::SpawnData>& getSpawns(); // Non-const
    const QList<RME::SpawnData>& getSpawns() const; // Const
    bool removeSpawn(const RME::SpawnData& spawnData); // Returns true if found and removed

    // --- Advanced Queries / Tile Property Queries ---
    /**
     * @brief Counts how many spawn areas overlap a given position.
     * Considers the spawn's center, radius, and that the Z-level matches.
     */
    int getSpawnOverlapCount(const Position& pos) const;

    /**
     * @brief Gets the town whose temple is at the given position.
     * @return Pointer to TownData if found, nullptr otherwise.
     */
    TownData* getTownByTempleLocation(const Position& pos);
    const TownData* getTownByTempleLocation(const Position& pos) const;

    /**
     * @brief Gets a list of houses that have an exit at the given position.
     * @return QList of pointers to HouseData. List is empty if no houses have an exit there.
     */
    QList<HouseData*> getHousesWithExitAt(const Position& pos);
    QList<const HouseData*> getHousesWithExitAt(const Position& pos) const; // Const version

    /**
     * @brief Checks if a given position is a valid location for a house exit.
     * A valid location typically must exist, have ground, not be part of an existing house,
     * and not be blocking (i.e., be walkable).
     * @param pos The position to check.
     * @return True if the location is valid for a house exit, false otherwise.
     */
    bool isValidHouseExitLocation(const Position& pos) const;

    // Tile change notification
    void notifyTileChanged(const Position& pos);

    // Stubs for complex map-wide operations
    bool convertFormat(quint32 targetOtbmVersion, quint32 targetClientVersion) { setChanged(true); /*TODO*/ return false; }
    bool exportMinimap(const QString& filePath) { /*TODO*/ return false; }
    int cleanInvalidTiles() { setChanged(true); /*TODO*/ return 0; }
    // int cleanDuplicateItems(PropertyFlags properties); // PropertyFlags needs definition


private:
    QString m_description;
    MapVersionInfo m_versionInfo; // Stores OTBM version and potentially a single legacy client ID
    RME::core::ClientVersionInfo m_clientVersionInfo; // Stores detailed client major/minor/build

    QString m_houseFile;
    QString m_spawnFile;
    QString m_waypointFile;

    bool m_changed = false;

    // QList<TownData> m_towns; // OLD - Replaced by m_townsById
    QMap<uint32_t, RME::core::world::TownData> m_townsById; // Explicitly namespaced
    uint32_t m_maxTownId = 0; // New

    // QMap<quint32, HouseData> m_houses; // Old
    QMap<uint32_t, RME::core::houses::HouseData> m_housesById; // Explicitly namespaced
    uint32_t m_maxHouseId = 0; // Tracks the highest ID ever used
    QMap<QString, RME::core::navigation::WaypointData> m_waypoints;
    QList<RME::SpawnData> m_spawns; // Changed to RME::SpawnData
};

} // namespace RME

#endif // RME_MAP_H
