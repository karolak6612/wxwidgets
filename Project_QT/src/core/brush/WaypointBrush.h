#ifndef RME_WAYPOINT_BRUSH_H
#define RME_WAYPOINT_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position
#include <QString>
#include <QtGlobal>    // For quint32

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace editor { class EditorControllerInterface; }
    namespace waypoints { class Waypoint; }
}
}

// Forward declaration for potential test class
class TestWaypointBrush;

namespace RME {
namespace core {
namespace brush {

class WaypointBrush : public RME::core::Brush {
    friend class ::TestWaypointBrush; // Friend class for testing

public:
    WaypointBrush();
    ~WaypointBrush() override = default;

    void setCurrentWaypoint(const QString& waypointName);
    QString getCurrentWaypoint() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    bool hasMaterial() const override { return false; } // WaypointBrush doesn't use materials
    bool canDrag() const override { return false; } // Waypoints are placed individually
    bool canSmear() const override { return false; } // Waypoints are placed individually
    bool oneSizeFitsAll() const override { return true; } // Size doesn't matter for waypoints

    // Legacy compatibility methods (not used for waypoint placement)
    void draw(map::Map* map, Tile* tile, const BrushSettings* settings) override;
    void undraw(map::Map* map, Tile* tile, const BrushSettings* settings = nullptr) override;

private:
    QString m_currentWaypointName; // Name of the waypoint to place/move
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_WAYPOINT_BRUSH_H