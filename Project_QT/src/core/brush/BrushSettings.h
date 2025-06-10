#ifndef RME_BRUSH_SETTINGS_H
#define RME_BRUSH_SETTINGS_H

#include "BrushEnums.h" // For BrushShape
#include <QString>

namespace RME {

struct BrushSettings {
    // Common brush parameters
    BrushShape shape = BrushShape::SQUARE; // Default shape
    int size = 1;                          // Default size (e.g., 1x1 for square, radius 1 for circle)
    int variation = 0;                     // Default variation (e.g., for items with multiple looks, or terrain patterns)
    bool isEraseMode = false;              // Default to drawing mode

    // Name of the currently active brush (e.g., "GroundBrush", "ItemBrush:Wood")
    // This helps identify which specific brush these settings apply to,
    // or could be used by BrushManager to fetch the active brush.
    // Alternatively, BrushManager could hold activeBrush* and BrushSettings separately.
    // For now, including it as per YAML.
    QString activeBrushName;

    // Add other common settings that might apply across many brush types if identified later
    // For example, a generic 'flags' field using BrushFlags from BrushEnums.h
    // BrushFlags flags = BrushFlag::NONE;

    // Default constructor is fine.
    // Consider adding an equality operator if useful for detecting changes.
    bool operator==(const BrushSettings& other) const {
        return shape == other.shape &&
               size == other.size &&
               variation == other.variation &&
               isEraseMode == other.isEraseMode &&
               activeBrushName == other.activeBrushName;
    }

    bool operator!=(const BrushSettings& other) const {
        return !(*this == other);
    }
};

} // namespace RME

#endif // RME_BRUSH_SETTINGS_H
