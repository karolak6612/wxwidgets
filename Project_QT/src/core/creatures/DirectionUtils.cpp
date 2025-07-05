#include "core/creatures/Outfit.h"
#include <QString>

namespace RME {
namespace core {
namespace creatures {

QString DirectionUtils::directionToName(Direction dir) {
    switch (dir) {
        case Direction::NORTH:
            return QStringLiteral("North");
        case Direction::EAST:
            return QStringLiteral("East");
        case Direction::SOUTH:
            return QStringLiteral("South");
        case Direction::WEST:
            return QStringLiteral("West");
        default:
            return QStringLiteral("Unknown");
    }
}

Direction DirectionUtils::nameToDirection(const QString& name) {
    QString lowerName = name.toLower();
    if (lowerName == "north") {
        return Direction::NORTH;
    }
    if (lowerName == "east") {
        return Direction::EAST;
    }
    if (lowerName == "south") {
        return Direction::SOUTH;
    }
    if (lowerName == "west") {
        return Direction::WEST;
    }
    return Direction::SOUTH; // Default fallback
}

QString DirectionUtils::directionToString(Direction dir) {
    return directionToName(dir);
}

Direction DirectionUtils::stringToDirection(const QString& str) {
    return nameToDirection(str);
}

} // namespace creatures
} // namespace core
} // namespace RME