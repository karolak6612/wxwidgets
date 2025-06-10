#ifndef RME_WAYPOINT_MANAGER_H
#define RME_WAYPOINT_MANAGER_H

#include <QHash>
#include <QString>
#include <memory> // For std::unique_ptr
#include <QList>  // For returning lists of waypoints

// Forward declaration for Map
namespace RME { namespace core { class Map; }}

// Own types
#include "core/waypoints/Waypoint.h" // Defines RME::core::Waypoint
#include "core/Position.h"         // Defines RME::core::Position

namespace RME {
namespace core {

class WaypointManager {
public:
    explicit WaypointManager(RME::core::Map* map);
    ~WaypointManager() = default; // std::unique_ptr will manage Waypoint objects

    // Adds a waypoint. Takes ownership. Returns true on success.
    // Replaces existing waypoint with the same normalized name.
    bool addWaypoint(std::unique_ptr<Waypoint> waypoint);

    // Retrieves a waypoint by its name (case-insensitive). Returns nullptr if not found.
    Waypoint* getWaypoint(const QString& name) const;

    // Retrieves all waypoints at a specific position.
    QList<Waypoint*> getWaypointsAt(const Position& pos) const;

    // Removes a waypoint by its name (case-insensitive). Returns true if removed.
    bool removeWaypoint(const QString& name);

    // Gets a list of all waypoints.
    QList<Waypoint*> getAllWaypoints() const;

    // Typedef for iterator for convenience
    typedef QHash<QString, std::unique_ptr<Waypoint>>::const_iterator const_iterator;

    // Iterators to loop through waypoints (read-only)
    const_iterator begin() const { return m_waypoints.constBegin(); }
    const_iterator end() const { return m_waypoints.constEnd(); }
    const_iterator constBegin() const { return m_waypoints.constBegin(); }
    const_iterator constEnd() const { return m_waypoints.constEnd(); }

private:
    // Helper to normalize names for case-insensitive storage/lookup.
    QString normalizeName(const QString& name) const;

    RME::core::Map* m_map = nullptr; // Non-owning pointer to the map
    QHash<QString, std::unique_ptr<Waypoint>> m_waypoints; // Owns the Waypoint objects
};

} // namespace core
} // namespace RME

#endif // RME_WAYPOINT_MANAGER_H
