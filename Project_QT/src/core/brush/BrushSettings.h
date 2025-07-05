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
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_SETTINGS_H
