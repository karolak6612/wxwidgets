#ifndef RME_MAP_H
#define RME_MAP_H

#include "BaseMap.h"
#include "MapElements.h" // For TownData, HouseData, WaypointData
// #include "../../assets/ClientProfile.h" // Not directly storing ClientProfile object for now

#include <QString>
#include <QList>
#include <QMap>

namespace RME {

// Forward declare AssetManager if only needed for constructor pass-through to BaseMap
// class AssetManager; // Already included by BaseMap.h

// Structure to hold map version details (OTBM and associated client)
struct MapVersionInfo {
    quint32 otbmVersion = 0;    // e.g., 1, 2, 3, 4 (for OTBM format version)
    quint32 clientVersionID = 0;// e.g., 760, 1098 (references a ClientProfile ID or similar)
    QString description;      // e.g., "OTBM v3 - Tibia 10.98"
    // Potentially store pointer to the ClientProfile if needed often by Map logic
    // const ClientProfile* clientProfile = nullptr;
};


class Map : public BaseMap {
public:
    Map(int mapWidth, int mapHeight, int mapFloors, AssetManager* assetManager);
    ~Map() override = default;

    // Map Metadata
    const QString& getDescription() const { return description; }
    void setDescription(const QString& desc) { description = desc; setChanged(true); }

    const MapVersionInfo& getVersionInfo() const { return versionInfo; }
    void setVersionInfo(const MapVersionInfo& version) { versionInfo = version; setChanged(true); }
    // Individual setters for version components if needed
    void setOtbmVersion(quint32 otbmVer) { versionInfo.otbmVersion = otbmVer; setChanged(true); }
    void setClientVersionID(quint32 clientVerID) { versionInfo.clientVersionID = clientVerID; setChanged(true); }


    // Associated data file names (relative paths or just names)
    const QString& getHouseFile() const { return houseFile; }
    void setHouseFile(const QString& path) { houseFile = path; setChanged(true); }

    const QString& getSpawnFile() const { return spawnFile; }
    void setSpawnFile(const QString& path) { spawnFile = path; setChanged(true); }

    const QString& getWaypointFile() const { return waypointFile; } // If RME has separate waypoint file
    void setWaypointFile(const QString& path) { waypointFile = path; setChanged(true); }


    // Change tracking
    bool hasChanged() const { return changed; }
    void setChanged(bool c = true) { changed = c; } // Default to true when called

    // --- Towns ---
    const QList<TownData>& getTowns() const { return towns; }
    TownData* getTown(quint32 townId);
    const TownData* getTown(quint32 townId) const;
    void addTown(const TownData& town);
    bool removeTown(quint32 townId);

    // --- Houses ---
    const QMap<quint32, HouseData>& getHouses() const { return houses; } // Keyed by houseId
    HouseData* getHouse(quint32 houseId);
    const HouseData* getHouse(quint32 houseId) const;
    void addHouse(const HouseData& house);
    bool removeHouse(quint32 houseId);
    // void loadHouses(const QString& filePath); // Stub for later I/O task
    // void saveHouses(const QString& filePath); // Stub

    // --- Spawns ---
    // As decided, Map initially relies on Tile::spawn objects.
    // A dedicated SpawnManager or methods here to collect all spawns might come later.
    // void loadSpawns(const QString& filePath); // Stub
    // void saveSpawns(const QString& filePath); // Stub
    // QList<RME::Spawn*> getAllSpawns(); // Example for later

    // --- Waypoints ---
    const QMap<QString, WaypointData>& getWaypoints() const { return waypoints; } // Keyed by waypoint name
    WaypointData* getWaypoint(const QString& name);
    const WaypointData* getWaypoint(const QString& name) const;
    void addWaypoint(const WaypointData& waypoint);
    bool removeWaypoint(const QString& name);
    // void loadWaypoints(const QString& filePath); // Stub
    // void saveWaypoints(const QString& filePath); // Stub


    // Stubs for complex map-wide operations
    bool convertFormat(quint32 targetOtbmVersion, quint32 targetClientVersion) { setChanged(true); /*TODO*/ return false; }
    bool exportMinimap(const QString& filePath) { /*TODO*/ return false; }
    int cleanInvalidTiles() { setChanged(true); /*TODO*/ return 0; } // Returns count of cleaned items/tiles
    int cleanDuplicateItems(/* PropertyFlags properties */) { setChanged(true); /*TODO*/ return 0; }


private:
    QString description;
    MapVersionInfo versionInfo;

    QString houseFile;  // Filename for house data
    QString spawnFile;  // Filename for spawn data
    QString waypointFile; // Filename for waypoint data (if used)

    bool changed = false; // Has map been modified since last save/load?

    QList<TownData> towns;
    QMap<quint32, HouseData> houses; // Keyed by House ID
    // Spawns are on Tiles for now. Map might get a SpawnManager later.
    QMap<QString, WaypointData> waypoints; // Keyed by Waypoint name (must be unique)
};

} // namespace RME

#endif // RME_MAP_H
