#ifndef RME_BRUSH_ENUMS_H
#define RME_BRUSH_ENUMS_H

#include <cstdint> // For uint8_t, uint32_t

namespace RME {

// --- TILE NEIGHBOR BITMASK CONSTANTS ---
// Used for 8-neighbor analysis in brushes (Ground, Carpet, etc.)
// Bit order: 0=NW, 1=N, 2=NE, 3=W, 4=E, 5=SW, 6=S, 7=SE
static constexpr uint8_t TILE_NW = (1 << 0); // 0x01 North-West
static constexpr uint8_t TILE_N  = (1 << 1); // 0x02 North
static constexpr uint8_t TILE_NE = (1 << 2); // 0x04 North-East
static constexpr uint8_t TILE_W  = (1 << 3); // 0x08 West
static constexpr uint8_t TILE_E  = (1 << 4); // 0x10 East
static constexpr uint8_t TILE_SW = (1 << 5); // 0x20 South-West
static constexpr uint8_t TILE_S  = (1 << 6); // 0x40 South
static constexpr uint8_t TILE_SE = (1 << 7); // 0x80 South-East

// Helper function to check for a specific neighbor flag in a bitmask
inline constexpr bool hasNeighbor(uint8_t neighborBitmask, uint8_t directionFlag) {
    return (neighborBitmask & directionFlag) != 0;
}


enum class BrushShape {
    SQUARE,
    CIRCLE,
    // Other shapes can be added later if needed (e.g., Diamond)
};

// Other general brush-related enums can be added here later.
// For example, BrushFlags if there are common flags applicable to many brushes.
// enum class BrushFlag : quint32 {
//     NONE = 0,
//     SOME_COMMON_FLAG = 1 << 0,
// };
// Q_DECLARE_FLAGS(BrushFlags, BrushFlag)
// Q_DECLARE_OPERATORS_FOR_FLAGS(BrushFlags)

// Represents the type of border piece based on its orientation and form.
// These values are typically derived from analyzing 8 neighbors.
// The names are inspired by common RME border types.
enum class BorderType : uint8_t {
    NONE = 0,               // No border piece / or used as a default for CARPET_CENTER if not specified by s_carpet_types

    // Values used in wxWidgets for AutoBorder::tiles array indices (0-12)
    // These are the primary values expected to be produced by s_border_types / s_carpet_types.
    WX_NORTH_HORIZONTAL = 1,
    WX_EAST_HORIZONTAL = 2,
    WX_SOUTH_HORIZONTAL = 3,
    WX_WEST_HORIZONTAL = 4,
    WX_NORTHWEST_CORNER = 5,
    WX_NORTHEAST_CORNER = 6,
    WX_SOUTHWEST_CORNER = 7,
    WX_SOUTHEAST_CORNER = 8,
    WX_NORTHWEST_DIAGONAL = 9,
    WX_NORTHEAST_DIAGONAL = 10,
    WX_SOUTHWEST_DIAGONAL = 11,
    WX_SOUTHEAST_DIAGONAL = 12,

    CARPET_CENTER = 13,     // Explicit value for carpet center alignment, used by s_carpet_types

    // The more abstract conceptual types like NORTH_EDGE are removed to avoid redundancy,
    // as the WX_ values are what the lookup tables (s_border_types, s_carpet_types)
    // are expected to produce based on original RME logic.

    // Table Segment Types (start from 14)
    TABLE_ALONE = 14,
    TABLE_VERTICAL = 15,
    TABLE_HORIZONTAL = 16,
    TABLE_SOUTH_END = 17,    // Points South (connector is North)
    TABLE_EAST_END = 18,     // Points East (connector is West)
    TABLE_NORTH_END = 19,    // Points North (connector is South)
    TABLE_WEST_END = 20      // Points West (connector is East)
};

// Helper function to pack up to 4 BorderTypes into a uint32_t
// Each BorderType (uint8_t) takes one byte.
// Order: piece1 (LSB), piece2, piece3, piece4 (MSB)
inline uint32_t packBorderTypes(BorderType p1, BorderType p2 = BorderType::NONE, BorderType p3 = BorderType::NONE, BorderType p4 = BorderType::NONE) {
    return static_cast<uint32_t>(p1) |
           (static_cast<uint32_t>(p2) << 8) |
           (static_cast<uint32_t>(p3) << 16) |
           (static_cast<uint32_t>(p4) << 24);
}

// Helper function to unpack the Nth BorderType from a uint32_t
// n = 0 for LSB (piece1), up to n = 3 for MSB (piece4)
inline BorderType unpackBorderType(uint32_t packed_types, int n) {
    if (n < 0 || n > 3) return BorderType::NONE;
    return static_cast<BorderType>((packed_types >> (n * 8)) & 0xFF);
}


} // namespace RME

#endif // RME_BRUSH_ENUMS_H
