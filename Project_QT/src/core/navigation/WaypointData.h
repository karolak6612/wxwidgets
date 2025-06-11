#ifndef RME_NAVIGATION_WAYPOINTDATA_H
#define RME_NAVIGATION_WAYPOINTDATA_H

#include "core/Position.h" // For RME::core::Position
#include <QString>
#include <QSet>

namespace RME {
namespace core {
namespace navigation {

/**
 * @brief Stores data for a single waypoint on the map.
 * Includes its name, position, and connections to other waypoints.
 */
struct WaypointData {
    QString name;      ///< Unique name of the waypoint.
    RME::core::Position position; ///< Location of the waypoint on the map.
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
    WaypointData(const QString& wpName, const RME::core::Position& wpPos)
        : name(wpName), position(wpPos) {}

    // Copy constructor (defaulted)
    WaypointData(const WaypointData& other) = default;

    // Move constructor (defaulted)
    WaypointData(WaypointData&& other) noexcept = default;

    // Copy assignment operator (defaulted)
    WaypointData& operator=(const WaypointData& other) = default;

    // Move assignment operator (defaulted)
    WaypointData& operator=(WaypointData&& other) noexcept = default;

    /**
     * @brief Adds a connection to another waypoint by its name.
     * @param otherName The unique name of the waypoint to connect to.
     */
    void addConnection(const QString& otherName) {
        if (!otherName.isEmpty() && otherName != name) { // Prevent self-connection and empty names
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

    // Comparison operators (primarily for QMap key usage if WaypointData itself was a key, or for testing)
    bool operator==(const WaypointData& other) const {
        return name == other.name &&
               position == other.position &&
               connectedWaypointNames == other.connectedWaypointNames;
    }

    bool operator!=(const WaypointData& other) const {
        return !(*this == other);
    }
};

} // namespace navigation
} // namespace core
} // namespace RME

#endif // RME_NAVIGATION_WAYPOINTDATA_H
