#ifndef RME_BRUSH_SETTINGS_H
#define RME_BRUSH_SETTINGS_H

#include "core/brush/BrushShape.h"
#include <QString>

namespace RME {
namespace core {

struct BrushSettings {
    BrushShape shape = BrushShape::Square;
    int size = 1;
    int variation = 0;
    bool isEraseMode = false;
    QString activeBrushName;
    int activeFloor = 7; // Add floor tracking, default to surface level
    
    // Add getter/setter methods for floor
    int getActiveZ() const { return activeFloor; }
    void setActiveZ(int floor) { activeFloor = floor; }
    
    // Comparison operator for command merging
    bool operator==(const BrushSettings& other) const {
        return shape == other.shape && 
               size == other.size && 
               variation == other.variation && 
               isEraseMode == other.isEraseMode && 
               activeBrushName == other.activeBrushName &&
               activeFloor == other.activeFloor;
    }
    
    bool operator!=(const BrushSettings& other) const {
        return !(*this == other);
    }
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_SETTINGS_H
