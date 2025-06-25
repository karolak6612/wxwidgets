#pragma once

#include <QColor>
#include <vector>
#include "core/Position.h"

namespace RME {
namespace core {
namespace lighting {

// Maximum light intensity (radius)
constexpr int MAX_LIGHT_INTENSITY = 15;

/**
 * @brief Represents a light source in the map
 */
struct LightSource {
    Position position;  // Position of the light source
    QColor color;       // Light color
    uint8_t intensity;  // Light intensity (radius)
    
    LightSource() : intensity(0) {}
    
    LightSource(const Position& pos, const QColor& lightColor, uint8_t lightIntensity) 
        : position(pos), color(lightColor), intensity(lightIntensity) {}
};

/**
 * @brief Stores lighting information for a single tile
 */
struct TileLightInfo {
    QColor lightColor = QColor(0, 0, 0);  // Accumulated light color
    float lightLevel = 0.0f;              // Light intensity (0.0 - 1.0+)
    
    TileLightInfo() = default;
    
    TileLightInfo(const QColor& color, float level) 
        : lightColor(color), lightLevel(level) {}
};

} // namespace lighting
} // namespace core
} // namespace RME