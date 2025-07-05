#pragma once

#include <QObject>
#include <QColor>
#include "core/Position.h"
#include "core/lighting/LightingTypes.h"

namespace RME {
namespace core {

/**
 * @brief Interface for the light calculator service
 * 
 * This service manages dynamic light sources and calculates lighting
 * information for tiles based on global ambient light, item lights,
 * and dynamic light sources.
 */
class ILightCalculatorService {
public:
    virtual ~ILightCalculatorService() = default;

    // Global lighting
    virtual void setGlobalLightColor(const QColor& color) = 0;
    virtual QColor getGlobalLightColor() const = 0;

    // Dynamic light management
    virtual void addDynamicLight(const lighting::LightSource& light) = 0;
    virtual void removeDynamicLight(const Position& position) = 0;
    virtual void clearDynamicLights() = 0;
    virtual const std::vector<lighting::LightSource>& getDynamicLights() const = 0;

    // Light calculation
    virtual lighting::TileLightInfo calculateLightForTile(const Position& tilePos) const = 0;
    
    // Batch calculation for visible area
    virtual void calculateLightForRegion(const Position& startPos, const Position& endPos, 
                                       std::vector<std::vector<lighting::TileLightInfo>>& lightMap) const = 0;

    // Settings
    virtual bool isLightingEnabled() const = 0;
    virtual void setLightingEnabled(bool enabled) = 0;
    
    virtual float getGlobalAmbientLevel() const = 0;
    virtual void setGlobalAmbientLevel(float level) = 0;
};

} // namespace core
} // namespace RME