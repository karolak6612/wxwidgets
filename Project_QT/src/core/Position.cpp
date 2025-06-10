#include "Position.h"
#include <stdexcept> // For potential exceptions, though not used in current isValid

// If QDataStream or QDebug support is added:
// #include <QDataStream>
// #include <QDebug>

namespace RME {

Position::Position(int x_val, int y_val, int z_val) : x(x_val), y(y_val), z(z_val) {}

bool Position::operator==(const Position& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}

// Lexicographical comparison: z, then y, then x
bool Position::operator<(const Position& other) const {
    if (z != other.z) return z < other.z;
    if (y != other.y) return y < other.y;
    return x < other.x;
}

Position Position::operator+(const Position& other) const {
    return Position(x + other.x, y + other.y, z + other.z);
}

Position Position::operator-(const Position& other) const {
    return Position(x - other.x, y - other.y, z - other.z);
}

Position& Position::operator+=(const Position& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Position& Position::operator-=(const Position& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

bool Position::isValid() const {
    // Note: Original RME might have different boundaries or use x/y >= 0.
    // Assuming x, y are non-negative. Z is floor level.
    return x >= 0 && x < MAP_MAX_WIDTH &&
           y >= 0 && y < MAP_MAX_HEIGHT &&
           z >= MAP_MIN_FLOOR && z <= MAP_MAX_FLOOR;
}

Position Position::translated(int dx, int dy, int dz) const {
    return Position(x + dx, y + dy, z + dz);
}

// Example for QDebug integration (if QDebug is included)
// QDebug operator<<(QDebug dbg, const Position& pos) {
//     dbg.nospace() << "Position(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
//     return dbg;
// }

// Example for QDataStream (if QDataStream is included)
// QDataStream& operator<<(QDataStream& out, const Position& pos) {
//     out << qint32(pos.x) << qint32(pos.y) << qint32(pos.z);
//     return out;
// }
// QDataStream& operator>>(QDataStream& in, Position& pos) {
//     qint32 x, y, z;
//     in >> x >> y >> z;
//     pos.x = x;
//     pos.y = y;
//     pos.z = z;
//     return in;
// }

} // namespace RME

// Specialization of std::hash for Position
namespace std {
    size_t hash<RME::Position>::operator()(const RME::Position& pos) const noexcept {
        // A common way to combine hashes
        size_t h1 = std::hash<int>{}(pos.x);
        size_t h2 = std::hash<int>{}(pos.y);
        size_t h3 = std::hash<int>{}(pos.z);
        // Simple combination, can be improved if collisions are an issue
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
}
