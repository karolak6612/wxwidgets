#ifndef RME_MAP_ELEMENTS_H
#define RME_MAP_ELEMENTS_H

#include "../Position.h" // For RME::Position
#include <QString>
#include <QList> // For list of creature names in SpawnData (though SpawnData itself is deferred)
#include <QRect> // For HouseData exit scrollbars/area (commented out for now)
#include <QSet>  // For QSet in WaypointData

namespace RME {

// --- TownData ---
struct TownData {
    quint32 id = 0;
    QString name;
    Position templePosition; // Central position or temple location

    TownData() = default;
    TownData(quint32 townId, const QString& townName, const Position& townPos)
        : id(townId), name(townName), templePosition(townPos) {}
};

// --- HouseData ---
// Forward declaration if House class will be more complex later
// class House;

// Basic data for a house, more details (door positions, tile lists)
// would be part of a full House class/manager.
struct HouseData {
    quint32 houseId = 0;        // Unique ID of the house
    QString name;             // Name of the house
    Position entryPosition;   // Main entry point (tile in front of door, or door itself)
    quint32 townId = 0;         // Town this house belongs to
    quint32 size = 0;           // Number of tiles in the house
    quint32 rent = 0;           // Rent price
    // QRect exitScrollbars;    // Original RME had this for house exit editor (commented for now)

    // More complex data like list of door IDs, tile coordinates, owner, etc.
    // will be handled by a dedicated House system/class later.

    HouseData() = default;
};


// --- SpawnData ---
// As per task description:
// For now, Map will interact with RME::Spawn objects on Tiles (from "core/Spawn.h").
// A specific MapElements::SpawnData struct or a list like MapSpawnLocation is deferred
// until a clear need arises for the Map class to store spawn information separately
// from the Tile-based RME::Spawn instances.

// --- WaypointData ---
/**
 * @brief Stores data for a single waypoint on the map.
 * Includes its name, position, and connections to other waypoints.
 */
struct WaypointData {
    QString name;      ///< Unique name of the waypoint.
    Position position; ///< Location of the waypoint on the map.
    QSet<QString> connectedWaypointNames; ///< Set of names of waypoints connected to this one.

    /**
     * @brief Default constructor.
     */
    WaypointData() = default;

    /**
     * @brief Constructs a WaypointData with a name and position.
     * @param wpName The name of the waypoint.
     * @param wpPos The position of the waypoint.
     */
    WaypointData(const QString& wpName, const Position& wpPos)
        : name(wpName), position(wpPos) {}

    /**
     * @brief Adds a connection to another waypoint by its name.
     * @param otherName The unique name of the waypoint to connect to.
     */
    void addConnection(const QString& otherName) {
        if (!otherName.isEmpty() && otherName != name) { // Prevent self-connection
            connectedWaypointNames.insert(otherName);
        }
    }

    /**
     * @brief Removes a connection to another waypoint by its name.
     * @param otherName The unique name of the waypoint to disconnect from.
     */
    void removeConnection(const QString& otherName) {
        connectedWaypointNames.remove(otherName);
    }

    /**
     * @brief Checks if this waypoint is connected to another waypoint by its name.
     * @param otherName The unique name of the other waypoint.
     * @return True if a connection exists, false otherwise.
     */
    bool isConnectedTo(const QString& otherName) const {
        return connectedWaypointNames.contains(otherName);
    }

    /**
     * @brief Gets the set of names of all waypoints connected to this one.
     * @return A const reference to the QSet of connected waypoint names.
     */
    const QSet<QString>& getConnections() const {
        return connectedWaypointNames;
    }
};


// --- Other potential map-wide elements ---
// E.g., Map Signs, if they are stored globally by Map
/*
struct MapSignData {
    Position position;
    QString text;
};
*/

} // namespace RME

#endif // RME_MAP_ELEMENTS_H
