#ifndef RME_WAYPOINT_H
#define RME_WAYPOINT_H

#include "core/Position.h" // For RME::core::Position
#include <QString>

namespace RME {
namespace core {
namespace waypoints {

struct Waypoint {
    QString m_name;
    RME::core::Position m_position;

    // Constructor
    Waypoint(const QString& name, const RME::core::Position& pos)
        : m_name(name), m_position(pos) {}

    // Default constructor needed for some container operations if not providing args
    Waypoint() = default;

    // Optional: Equality operator for easier testing or searching if needed
    bool operator==(const Waypoint& other) const {
        return m_name == other.m_name && m_position == other.m_position;
    }
};

} // namespace waypoints
} // namespace core
} // namespace RME

#endif // RME_WAYPOINT_H
