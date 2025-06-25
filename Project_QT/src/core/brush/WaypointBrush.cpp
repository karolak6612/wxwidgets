#include "core/brush/WaypointBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"

#include <QDebug> // For qWarning, qDebug

// Placeholder for a waypoint brush sprite ID, replace with actual if available
const int EDITOR_SPRITE_WAYPOINT_BRUSH_LOOK_ID = 0; // Or some defined constant

namespace RME {
namespace core {

WaypointBrush::WaypointBrush()
    : m_currentWaypointName() // Default to no waypoint selected
{
}

void WaypointBrush::setCurrentWaypoint(const QString& waypointName) {
    m_currentWaypointName = waypointName;
}

QString WaypointBrush::getCurrentWaypoint() const {
    return m_currentWaypointName;
}

QString WaypointBrush::getName() const {
    if (m_currentWaypointName.isEmpty()) {
        return QStringLiteral("Waypoint Brush (No Waypoint Selected)");
    }
    return QStringLiteral("Waypoint Brush (%1)").arg(m_currentWaypointName);
}

int WaypointBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    return EDITOR_SPRITE_WAYPOINT_BRUSH_LOOK_ID;
}

bool WaypointBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    
    // A waypoint name must be selected/entered to place a waypoint
    if (m_currentWaypointName.isEmpty()) {
        return false;
    }
    
    // Get tile at position - waypoints can be placed on any existing tile
    const Tile* tile = map->getTile(pos);
    if (!tile) {
        return false; // Tile must exist
    }
    
    return true;
}

void WaypointBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    
    if (!controller || !controller->getMap() || !controller->getWaypointManager()) {
        qWarning() << "WaypointBrush::apply: Controller, Map, or WaypointManager is null.";
        return;
    }
    
    if (m_currentWaypointName.isEmpty()) {
        qWarning() << "WaypointBrush::apply: No waypoint name selected for placement.";
        return;
    }
    
    Map* map = controller->getMap();
    
    // Re-check canApply with live map
    if (!canApply(map, pos, settings)) {
        qDebug() << "WaypointBrush::apply: Preconditions not met at" << pos.toString();
        return;
    }
    
    // Check if waypoint already exists at this exact position
    RME::core::waypoints::WaypointManager* waypointManager = controller->getWaypointManager();
    RME::core::waypoints::Waypoint* existingWaypoint = waypointManager->getWaypointByName(m_currentWaypointName);
    
    if (existingWaypoint && existingWaypoint->getPosition() == pos) {
        qDebug() << "WaypointBrush::apply: Waypoint" << m_currentWaypointName << "is already at position" << pos.toString();
        return;
    }
    
    // Use EditorController's placeOrMoveWaypoint method which handles undo/redo
    controller->placeOrMoveWaypoint(m_currentWaypointName, pos);
    
    if (existingWaypoint) {
        qDebug() << "WaypointBrush::apply: Moved waypoint" << m_currentWaypointName << "to position" << pos.toString();
    } else {
        qDebug() << "WaypointBrush::apply: Placed new waypoint" << m_currentWaypointName << "at position" << pos.toString();
    }
}

// Legacy compatibility methods - not used for waypoint placement
void WaypointBrush::draw(map::Map* /*map*/, Tile* /*tile*/, const BrushSettings* /*settings*/) {
    // Never called - waypoint placement is handled via apply() method
    Q_ASSERT(false && "WaypointBrush::draw should not be called - use apply() instead");
}

void WaypointBrush::undraw(map::Map* /*map*/, Tile* /*tile*/, const BrushSettings* /*settings*/) {
    // Never called - waypoint removal is handled via EditorController methods
    Q_ASSERT(false && "WaypointBrush::undraw should not be called - use EditorController::removeWaypoint() instead");
}

} // namespace core
} // namespace RME