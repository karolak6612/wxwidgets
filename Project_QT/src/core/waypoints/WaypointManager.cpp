#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include "core/map/Map.h" // For RME::core::Map
#include "core/Tile.h"    // For RME::core::Tile

#include <QDebug> // For qWarning if needed

namespace RME {
namespace core {
namespace waypoints {

WaypointManager::WaypointManager(RME::core::Map* map)
    : m_map(map) {
    Q_ASSERT(m_map); // WaypointManager requires a valid map context
}

QString WaypointManager::normalizeName(const QString& name) const {
    return name.trimmed().toLower();
}

bool WaypointManager::addWaypoint(const QString& name, const RME::core::Position& pos) {
    if (name.trimmed().isEmpty()) {
        qWarning("WaypointManager::addWaypoint: Waypoint name cannot be empty or just whitespace.");
        return false;
    }
    QString normalizedName = normalizeName(name);

    // If a waypoint with the same normalized name exists, remove it first.
    // This ensures tile counts are correctly decremented for the old one before adding the new one.
    if (m_waypoints.contains(normalizedName)) {
        // removeWaypoint will handle tile count decrement and erasing from m_waypoints hash.
        // We pass the original name used for the existing waypoint if we had it,
        // but since we key by normalized, we just use the passed name (or normalized one).
        // This is fine as removeWaypoint also normalizes.
        if (!removeWaypoint(normalizedName)) {
            // This case should ideally not happen if contains() is true, but as a safeguard:
            qWarning("WaypointManager::addWaypoint: Failed to remove existing waypoint '%s' before replacing.").arg(normalizedName);
            // Continue to try adding, might overwrite if removeWaypoint had an issue not related to finding.
        }
    }

    auto newWp = std::make_unique<Waypoint>(name, pos); // Store original name in Waypoint object
    Waypoint* wpRawPtr = newWp.get(); // Get raw pointer for potential use before move

    if (m_map && pos.isValid()) { // Assuming Position::isValid() checks for non-negative coords etc.
                                 // Or some other agreed upon validity check for positions.
        Tile* tile = m_map->getTile(pos);
        // According to YAML for LOGIC-04, for add: "get/create 'Tile' and call 'tile->increaseWaypointCount()'"
        // For now, if tile doesn't exist, we won't create it for a waypoint, just store the waypoint.
        // The user should place waypoints on existing map areas.
        // If getOrCreateTile was used: tile = m_map->getOrCreateTile(pos);
        if (tile) {
            tile->increaseWaypointCount();
        } else {
            // Optional: If waypoints can only be on existing tiles, return false or warn.
            // qWarning("WaypointManager::addWaypoint: Tile at position %s does not exist.", qUtf8Printable(pos.toString()));
            // For now, we allow waypoints at positions without pre-existing tiles,
            // but they won't affect tile waypoint counts until a tile is created there.
            // This behavior might need review based on how map interaction is designed.
        }
    }

    m_waypoints.insert(normalizedName, std::move(newWp));

    // TODO: Update m_positionalWaypointsCache if implemented
    // if (pos.isValid()) m_positionalWaypointsCache.insert(pos, wpRawPtr);

    return true;
}

bool WaypointManager::addWaypoint(std::unique_ptr<Waypoint> waypoint) {
    if (!waypoint) {
        qWarning() << "WaypointManager::addWaypoint: Waypoint pointer is null.";
        return false;
    }
    
    return addWaypoint(waypoint->name, waypoint->position);
}

Waypoint* WaypointManager::getWaypointByName(const QString& name) const {
    QString normalizedName = normalizeName(name);
    auto it = m_waypoints.constFind(normalizedName);
    if (it != m_waypoints.constEnd()) {
        return it.value().get(); // .get() from std::unique_ptr
    }
    return nullptr;
}

QList<Waypoint*> WaypointManager::getWaypointsAt(const RME::core::Position& pos) const {
    QList<Waypoint*> result;
    // TODO: If m_positionalWaypointsCache is implemented, use it for efficiency.
    // For now, iterate all waypoints:
    for (auto it = m_waypoints.constBegin(); it != m_waypoints.constEnd(); ++it) {
        if (it.value() && it.value()->m_position == pos) {
            result.append(it.value().get());
        }
    }
    return result;
}

bool WaypointManager::removeWaypoint(const QString& name) {
    QString normalizedName = normalizeName(name);
    auto it = m_waypoints.find(normalizedName);

    if (it == m_waypoints.end()) {
        return false; // Waypoint not found
    }

    Waypoint* wpToRemove = it.value().get(); // Get raw pointer before unique_ptr is invalidated
    RME::core::Position pos = wpToRemove->m_position;

    // Erase from main hash first. This will delete the Waypoint object via unique_ptr.
    m_waypoints.erase(it);

    // Now update tile count and positional cache (if any)
    if (m_map && pos.isValid()) {
        Tile* tile = m_map->getTile(pos);
        if (tile) {
            tile->decreaseWaypointCount();
        }
    }

    // TODO: Update m_positionalWaypointsCache if implemented
    // if (pos.isValid()) {
    //    // This is tricky if multiple waypoints could share a pos and we stored Waypoint*
    //    // QMultiHash allows multiple identical keys, so remove(key, value) is needed.
    //    m_positionalWaypointsCache.remove(pos, wpToRemove);
    // }

    return true;
}

bool WaypointManager::updateWaypointPosition(const QString& name, const RME::core::Position& newPos) {
    QString normalizedName = normalizeName(name);
    auto it = m_waypoints.find(normalizedName);
    
    if (it == m_waypoints.end()) {
        return false; // Waypoint not found
    }
    
    Waypoint* waypoint = it.value().get();
    RME::core::Position oldPos = waypoint->position;
    
    // Update tile counts if map is available
    if (m_map) {
        // Decrease count at old position
        if (oldPos.isValid()) {
            Tile* oldTile = m_map->getTile(oldPos);
            if (oldTile) {
                oldTile->decreaseWaypointCount();
            }
        }
        
        // Increase count at new position
        if (newPos.isValid()) {
            Tile* newTile = m_map->getTile(newPos);
            if (newTile) {
                newTile->increaseWaypointCount();
            }
        }
    }
    
    // Update waypoint position
    waypoint->position = newPos;
    
    // TODO: Update m_positionalWaypointsCache if implemented
    
    return true;
}

QList<Waypoint*> WaypointManager::getAllWaypoints() const {
    QList<Waypoint*> result;
    result.reserve(m_waypoints.size());
    for (auto it = m_waypoints.constBegin(); it != m_waypoints.constEnd(); ++it) {
        result.append(it.value().get());
    }
    return result;
}

void WaypointManager::clearAllWaypoints() {
    if (m_map) {
        for (auto it = m_waypoints.constBegin(); it != m_waypoints.constEnd(); ++it) {
            const Waypoint* wp = it.value().get();
            if (wp && wp->m_position.isValid()) {
                Tile* tile = m_map->getTile(wp->m_position);
                if (tile) {
                    // If a tile could have multiple waypoints from this manager (not possible with unique names),
                    // this loop would call decrease multiple times. But since names are unique,
                    // each waypoint is distinct. So, one decrease per waypoint is correct.
                    tile->decreaseWaypointCount();
                }
            }
        }
    }
    m_waypoints.clear(); // Destroys all unique_ptrs and their Waypoint objects

    // TODO: Update m_positionalWaypointsCache if implemented
    // m_positionalWaypointsCache.clear();
}

} // namespace waypoints
} // namespace core
} // namespace RME
