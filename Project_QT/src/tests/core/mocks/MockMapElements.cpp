#include "MockMapElements.h"

// Implementation of any non-inline mock methods, if any.
// For now, most mock methods are inline in the header.

// If RME::Position hash/equals are not global or inline in Position.h,
// their definitions could go here.
// For example, if they were declared static in MockMapElements.h or similar:
//
// #include "Project_QT/src/core/Position.h"
// bool operator==(const RME::Position& p1, const RME::Position& p2) {
//    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
// }
// uint qHash(const RME::Position& key, uint seed) {
//    return qHash(static_cast<quint16>(key.x), seed) ^
//           qHash(static_cast<quint16>(key.y), seed << 16) ^
//           qHash(static_cast<quint8>(key.z), seed << 24);
// }
// It's assumed these are handled by Position.h from CORE-01.
// If QMap<RME::Position, ...> or QSet<RME::Position> are used, these must be available.
// Position.cpp or Position.h is the best place for them.
// This file remains empty if all mock implementations are in the header.
