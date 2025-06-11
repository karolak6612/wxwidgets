#ifndef RME_MAP_H
#define RME_MAP_H

#include "BaseMap.h"
// #include "MapElements.h" // For TownData, HouseData - Replaced by specific includes
#include "Project_QT/src/core/houses/HouseData.h" // Specific include for HouseData
#include "MapElements.h" // Assuming this is for TownData etc. and does NOT define HouseData
#include "core/navigation/WaypointData.h" // New WaypointData location

#include <QString>
#include <QList>
#include <QMap>

namespace RME {

struct MapVersionInfo {
    quint32 otbmVersion = 0;
    quint32 clientVersionID = 0;
    QString description;
};

class Map : public BaseMap {
public:
    Map(int mapWidth, int mapHeight, int mapFloors, RME::core::assets::AssetManager* assetManager);
    ~Map() override = default;

    // Map Metadata
    const QString& getDescription() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; setChanged(true); }

    const MapVersionInfo& getVersionInfo() const { return m_versionInfo; }
    void setVersionInfo(const MapVersionInfo& version) { m_versionInfo = version; setChanged(true); }
    void setOtbmVersion(quint32 otbmVer) { m_versionInfo.otbmVersion = otbmVer; setChanged(true); }
    void setClientVersionID(quint32 clientVerID) { m_versionInfo.clientVersionID = clientVerID; setChanged(true); }

    const QString& getHouseFile() const { return m_houseFile; }
    void setHouseFile(const QString& path) { m_houseFile = path; setChanged(true); }

    const QString& getSpawnFile() const { return m_spawnFile; }
    void setSpawnFile(const QString& path) { m_spawnFile = path; setChanged(true); }

    const QString& getWaypointFile() const { return m_waypointFile; }
    void setWaypointFile(const QString& path) { m_waypointFile = path; setChanged(true); }

    bool hasChanged() const { return m_changed; }
    void setChanged(bool c = true) { m_changed = c; }

    // --- Towns ---
    const QList<TownData>& getTowns() const { return m_towns; }
    TownData* getTown(quint32 townId);
    const TownData* getTown(quint32 townId) const;
    void addTown(const TownData& town);
    bool removeTown(quint32 townId);

    // --- Houses ---
    // const QMap<quint32, HouseData>& getHouses() const { return m_houses; } // Old
    // HouseData* getHouse(quint32 houseId); // Old
    // const HouseData* getHouse(quint32 houseId) const; // Old
    // void addHouse(const HouseData& house); // Old
    // bool removeHouse(quint32 houseId); // Old

    void addHouse(HouseData&& houseData); // Use rvalue reference for potential move
    HouseData* getHouse(uint32_t houseId);
    const HouseData* getHouse(uint32_t houseId) const;
    bool removeHouse(uint32_t houseId); // Returns true if house was found and removed
    const QMap<uint32_t, HouseData>& getHouses() const { return m_housesById; }
    QMap<uint32_t, HouseData>& getHouses() { return m_housesById; } // Non-const version

    uint32_t getUnusedHouseId();
    bool changeHouseId(uint32_t oldId, uint32_t newId); // Returns true on success

    // --- Waypoints ---
    const QMap<QString, RME::core::navigation::WaypointData>& getWaypoints() const { return m_waypoints; }
    RME::core::navigation::WaypointData* getWaypoint(const QString& name);
    const RME::core::navigation::WaypointData* getWaypoint(const QString& name) const;
    // Take by value to allow moving from temporary or copying, then move into map
    bool addWaypoint(RME::core::navigation::WaypointData waypointData);
    bool removeWaypoint(const QString& name);

    // Stubs for complex map-wide operations
    bool convertFormat(quint32 targetOtbmVersion, quint32 targetClientVersion) { setChanged(true); /*TODO*/ return false; }
    bool exportMinimap(const QString& filePath) { /*TODO*/ return false; }
    int cleanInvalidTiles() { setChanged(true); /*TODO*/ return 0; }
    // int cleanDuplicateItems(PropertyFlags properties); // PropertyFlags needs definition


private:
    QString m_description;
    MapVersionInfo m_versionInfo;

    QString m_houseFile;
    QString m_spawnFile;
    QString m_waypointFile;

    bool m_changed = false;

    QList<TownData> m_towns;
    // QMap<quint32, HouseData> m_houses; // Old
    QMap<uint32_t, HouseData> m_housesById; // New name and consistent type
    uint32_t m_maxHouseId = 0; // Tracks the highest ID ever used
    QMap<QString, RME::core::navigation::WaypointData> m_waypoints;
};

} // namespace RME

#endif // RME_MAP_H
