#include "core/waypoints/WaypointManager.h"
#include "core/Map.h"   // Full definition for RME::core::Map
#include "core/Tile.h"  // Full definition for RME::core::Tile
#include <utility>      // For std::move

// It's good practice to include Qt headers specifically if their types are directly used in .cpp
// #include <QDebug> // For potential logging

namespace RME {
namespace core {

WaypointManager::WaypointManager(RME::core::Map* map) : m_map(map) {
    // Q_ASSERT(map); // Or other assertion/error handling if map is null
}

QString WaypointManager::normalizeName(const QString& name) const {
    return name.trimmed().toLower();
}

bool WaypointManager::addWaypoint(std::unique_ptr<Waypoint> waypoint) {
    if (!waypoint) {
        return false;
    }

    QString normalizedNewName = normalizeName(waypoint->name);
    if (normalizedNewName.isEmpty()) {
        // qWarning() << "Waypoint name cannot be empty or just whitespace.";
        return false;
    }

    // If a waypoint with the same normalized name exists, remove it first.
    // This also handles decrementing waypoint count on the old tile.
    if (m_waypoints.contains(normalizedNewName)) {
        removeWaypoint(normalizedNewName); // Use the public removeWaypoint for consistent logic
    }

    // If the new waypoint has a valid position, update the tile's waypoint count.
    if (m_map && waypoint->position.isValid()) {
        Tile* tile = m_map->getTile(waypoint->position); // getTile should ideally create if not present
        if (tile) {
            tile->increaseWaypointCount();
        } else {
            // This case might indicate an issue: position is valid but no tile could be obtained/created.
            // Depending on Map::getTile behavior for non-existent but valid positions.
            // qWarning() << "Could not get/create tile for waypoint at" << waypoint->position;
            // Decide if this should prevent adding the waypoint or just add it without tile association.
            // For now, assume if tile is nullptr, we can't update count but still add waypoint.
        }
    }

    m_waypoints.insert(normalizedNewName, std::move(waypoint));
    return true;
}

Waypoint* WaypointManager::getWaypoint(const QString& name) const {
    QString normalized = normalizeName(name);
    if (m_waypoints.contains(normalized)) {
        return m_waypoints.value(normalized).get();
    }
    return nullptr;
}

QList<Waypoint*> WaypointManager::getWaypointsAt(const Position& pos) const {
    QList<Waypoint*> foundWaypoints;
    if (!pos.isValid()) {
        return foundWaypoints;
    }
    for (const auto& wp_ptr : m_waypoints.values()) {
        if (wp_ptr && wp_ptr->position == pos) {
            foundWaypoints.append(wp_ptr.get());
        }
    }
    return foundWaypoints;
}

bool WaypointManager::removeWaypoint(const QString& name) {
    QString normalized = normalizeName(name);
    auto it = m_waypoints.find(normalized);
    if (it == m_waypoints.end()) {
        return false; // Not found
    }

    Waypoint* waypointToRemove = it.value().get();

    if (m_map && waypointToRemove && waypointToRemove->position.isValid()) {
        Tile* tile = m_map->getTile(waypointToRemove->position);
        if (tile) {
            tile->decreaseWaypointCount();
        } else {
            // qWarning() << "Could not get tile to decrease waypoint count for waypoint at" << waypointToRemove->position;
        }
    }

    m_waypoints.erase(it); // Erase using iterator
    return true;
}

QList<Waypoint*> WaypointManager::getAllWaypoints() const {
    QList<Waypoint*> allWaypoints;
    allWaypoints.reserve(m_waypoints.size());
    for (const auto& wp_ptr : m_waypoints.values()) {
        allWaypoints.append(wp_ptr.get());
    }
    return allWaypoints;
}

// Iterators are defined inline in the header.

} // namespace core
} // namespace RME
