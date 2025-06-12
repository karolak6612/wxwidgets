#ifndef RME_BRUSH_ENUMS_H
#define RME_BRUSH_ENUMS_H

#include <cstdint> // For uint8_t

namespace RME {

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
    NONE = 0,               // No border piece

    // Cardinal Edges (Outer Edges)
    NORTH_EDGE = 1,         // Edge facing North
    EAST_EDGE = 2,          // Edge facing East
    SOUTH_EDGE = 3,         // Edge facing South
    WEST_EDGE = 4,          // Edge facing West

    // Corners (Outer Corners)
    NORTH_WEST_CORNER = 5,  // Outer corner NW
    NORTH_EAST_CORNER = 6,  // Outer corner NE
    SOUTH_EAST_CORNER = 7,  // Outer corner SE
    SOUTH_WEST_CORNER = 8,  // Outer corner SW

    // Inner Corners (Concave) - names might vary based on convention
    INNER_NORTH_WEST = 9,   // Inner corner, open to NW
    INNER_NORTH_EAST = 10,  // Inner corner, open to NE
    INNER_SOUTH_EAST = 11,  // Inner corner, open to SE
    INNER_SOUTH_WEST = 12,  // Inner corner, open to SW

    // Diagonals (less common for direct matching, often composed)
    // These might not be directly output by s_border_types but are conceptual
    DIAGONAL_NW_SE = 13,    // Diagonal from NW to SE
    DIAGONAL_NE_SW = 14,    // Diagonal from NE to SW

    // Special cases like U-shapes or T-shapes could be added if needed,
    // but the 8-neighbor lookup usually resolves to simpler edge/corner pieces.
    // For example, a U-shape might be a combination of two corners and an edge.

    // Values used in wxWidgets for AutoBorder::tiles array indices
    // These are more specific and map directly to array indices in wx AutoBorder.
    // We might use these directly if s_border_types is ported literally.
    // Or, s_border_types could output the more abstract types above.
    // For now, defining the wx-like indices for potential direct porting of s_border_types logic:
    // WX_BORDER_NONE = 0, // Already have NONE
    WX_NORTH_HORIZONTAL = 1, // Note: reusing values from above for simplicity if they match.
                               // If wx values are different, define them separately.
                               // For this example, assuming generic NORTH_EDGE (1) can map to WX_NORTH_HORIZONTAL (1)
    WX_EAST_HORIZONTAL = 2,
    WX_SOUTH_HORIZONTAL = 3,
    WX_WEST_HORIZONTAL = 4,
    WX_NORTHWEST_CORNER = 5,
    WX_NORTHEAST_CORNER = 6,
    WX_SOUTHWEST_CORNER = 7, // Note: wx order might be different, CSE then CSW. Check actual wx values.
    WX_SOUTHEAST_CORNER = 8, // For this example, using values that match the earlier corners.
    WX_NORTHWEST_DIAGONAL = 9, // These were for diagonal ground patterns, maps to INNER_NORTH_WEST here conceptually
    WX_NORTHEAST_DIAGONAL = 10, // maps to INNER_NORTH_EAST
    WX_SOUTHWEST_DIAGONAL = 11, // maps to INNER_SOUTH_EAST
    WX_SOUTHEAST_DIAGONAL = 12, // maps to INNER_SOUTH_WEST
    // Max value used by wx for array indexing was typically 12.
    // The MaterialBorderRule.align string will be the primary matching mechanism
    // against Material rules in Qt6. The BorderType calculated from s_border_types
    // will need to be translated into an appropriate 'align' string.
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
