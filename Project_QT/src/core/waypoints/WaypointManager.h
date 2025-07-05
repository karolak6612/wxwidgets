#ifndef RME_WAYPOINTMANAGER_H
#define RME_WAYPOINTMANAGER_H

#include "core/waypoints/Waypoint.h" // For RME::core::waypoints::Waypoint
#include "core/Position.h"           // For RME::core::Position
#include <QHash>                     // For QHash
#include <QList>                     // For QList
#include <QString>                   // For QString
#include <memory>                    // For std::unique_ptr

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile; // Used in implementation for tile counts
}
}

// Forward declaration for potential test class
class TestWaypointManager;

namespace RME {
namespace core {
namespace waypoints {

class WaypointManager {
    friend class ::TestWaypointManager; // Friend class for testing

public:
    explicit WaypointManager(RME::core::Map* map);
    // No explicit destructor needed if m_waypoints owns unique_ptrs

    // Adds a new waypoint. If a waypoint with the same (normalized) name exists, it's replaced.
    // Returns true if waypoint was successfully added/replaced, false otherwise (e.g., invalid position if map requires tiles).
    bool addWaypoint(const QString& name, const RME::core::Position& pos);

    // Retrieves a waypoint by its name (case-insensitive).
    // Returns nullptr if not found.
    Waypoint* getWaypointByName(const QString& name) const;

    // Retrieves all waypoints at a specific map position.
    QList<Waypoint*> getWaypointsAt(const RME::core::Position& pos) const;

    // Removes a waypoint by its name (case-insensitive).
    // Returns true if a waypoint was found and removed, false otherwise.
    bool removeWaypoint(const QString& name);

    // Returns a list of all managed waypoints.
    QList<Waypoint*> getAllWaypoints() const;

    // Removes all waypoints and updates tile counts accordingly.
    void clearAllWaypoints();

private:
    QString normalizeName(const QString& name) const;

    RME::core::Map* m_map; // Non-owning pointer to the map context
    // Stores waypoints, keyed by their normalized (lowercase) name for case-insensitive lookup.
    // std::unique_ptr ensures automatic memory management of Waypoint objects.
    QHash<QString, std::unique_ptr<Waypoint>> m_waypoints;

    // TODO (Optimization): For faster getWaypointsAt(pos):
    // QMultiHash<RME::core::Position, Waypoint*> m_positionalWaypointsCache;
    // This would store non-owning Waypoint* pointing to entries in m_waypoints.
    // Would need updates in addWaypoint, removeWaypoint, clearAllWaypoints.
};

} // namespace waypoints
} // namespace core
} // namespace RME

#endif // RME_WAYPOINTMANAGER_H
