#ifndef RME_WAYPOINT_H
#define RME_WAYPOINT_H

#include <QString>
#include "core/Position.h" // Assuming Position.h is in core/

namespace RME {
namespace core {

struct Waypoint {
    QString name;
    RME::core::Position position;

    Waypoint() = default;

    Waypoint(QString waypointName, RME::core::Position waypointPos)
        : name(std::move(waypointName)), position(waypointPos) {}
};

} // namespace core
} // namespace RME

#endif // RME_WAYPOINT_H
