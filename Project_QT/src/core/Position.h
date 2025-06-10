#ifndef RME_POSITION_H
#define RME_POSITION_H

#include "map_constants.h" // For validation
#include <functional> // For std::hash

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

} // namespace RME

// Specialization of std::hash for Position to allow use in std::unordered_map
namespace std {
    template <>
    struct hash<RME::Position> {
        size_t operator()(const RME::Position& pos) const noexcept;
    };
}

#endif // RME_POSITION_H
