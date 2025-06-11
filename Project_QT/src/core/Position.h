#ifndef RME_POSITION_H
#define RME_POSITION_H

#include "map_constants.h" // For validation
#include <functional> // For std::hash
#include <QtGlobal> // For qHash and uint

// Forward declaration for ostream/istream operators if needed outside class
// class QDataStream; // Example for Qt's serialization

namespace RME {

struct Position {
    int x = 0;
    int y = 0;
    int z = 0; // Floor

    Position() = default;
    Position(int x, int y, int z);

    // Comparison operators
    bool operator==(const Position& other) const;
    bool operator!=(const Position& other) const;
    bool operator<(const Position& other) const; // For use in std::map keys, etc.

    // Arithmetic operators
    Position operator+(const Position& other) const;
    Position operator-(const Position& other) const;
    Position& operator+=(const Position& other);
    Position& operator-=(const Position& other);

    // Check if position is valid within map boundaries
    bool isValid() const;

    // Add a method to get neighboring positions for convenience
    Position translated(int dx, int dy, int dz = 0) const;

    // Consider adding a QHashable-like function if used with QHash
    // friend uint qHash(const Position& key, uint seed = 0);
};

// Non-member functions if preferred for some operations
// Position operator+(const Position& p1, const Position& p2);
// Position operator-(const Position& p1, const Position& p2);

// If Qt's QDebug is to be used for logging Positions
// class QDebug;
// QDebug operator<<(QDebug dbg, const Position& pos);

// qHash overload for RME::Position, enabling its use as a key in QHash, QMap, QSet, etc.
inline uint qHash(const Position& key, uint seed = 0) {
    // A common way to combine hashes for struct members
    uint h1 = qHash(key.x, seed);
    uint h2 = qHash(key.y, seed);
    uint h3 = qHash(key.z, seed);
    // Simple XOR combination, can be made more sophisticated if needed
    return h1 ^ (h2 << 1) ^ (h3 << 2);
    // Alternative from example:
    // return qHash(key.x, seed) ^ qHash(key.y, seed << 1) ^ qHash(key.z, seed << 2);
    // Or a more robust combination to better distribute hash values:
    // return ((h1 << 16) | (h1 >> 16)) ^ h2 ^ ((h3 << 8) | (h3 >> 8)) ;
}

} // namespace RME

// Specialization of std::hash for Position to allow use in std::unordered_map
namespace std {
    template <>
    struct hash<RME::Position> {
        size_t operator()(const RME::Position& pos) const noexcept;
    };
}

#endif // RME_POSITION_H
