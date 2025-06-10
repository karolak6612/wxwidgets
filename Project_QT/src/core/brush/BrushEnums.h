#ifndef RME_BRUSH_ENUMS_H
#define RME_BRUSH_ENUMS_H

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

} // namespace RME

#endif // RME_BRUSH_ENUMS_H
