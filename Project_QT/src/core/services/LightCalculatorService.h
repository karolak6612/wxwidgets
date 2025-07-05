#ifndef RME_LIGHTCALCULATORSERVICE_H
#define RME_LIGHTCALCULATORSERVICE_H

#include "ILightCalculatorService.h"
#include <vector>
#include <QMap>

namespace RME {
namespace core {

class Map;
class Position;

/**
 * @brief Concrete implementation of ILightCalculatorService
 * 
 * This class provides lighting calculations for map rendering,
 * including light propagation, shadows, and ambient lighting.
 */
class LightCalculatorService : public QObject, public ILightCalculatorService {
    Q_OBJECT

public:
    explicit LightCalculatorService(QObject* parent = nullptr);
    ~LightCalculatorService() override;

    // Global lighting
    void setGlobalLightColor(const QColor& color) override;
    QColor getGlobalLightColor() const override;

    // Dynamic light management
    void addDynamicLight(const lighting::LightSource& light) override;
    void removeDynamicLight(const Position& position) override;
    void clearDynamicLights() override;
    const std::vector<lighting::LightSource>& getDynamicLights() const override;

    // Light calculation
    lighting::TileLightInfo calculateLightForTile(const Position& tilePos) const override;
    
    // Batch calculation for visible area
    void calculateLightForRegion(const Position& startPos, const Position& endPos, 
                               std::vector<std::vector<lighting::TileLightInfo>>& lightMap) const override;

    // Settings
    bool isLightingEnabled() const override;
    void setLightingEnabled(bool enabled) override;
    
    float getGlobalAmbientLevel() const override;
    void setGlobalAmbientLevel(float level) override;

private:
    QColor convertLightColorToRGB(uint8_t colorIndex) const;
    QColor blendColors(const QColor& base, const QColor& overlay, float alpha) const;
    
    std::vector<lighting::LightSource> m_dynamicLights;
    QColor m_globalLightColor;
    bool m_lightingEnabled;
    float m_globalAmbientLevel;
};

} // namespace core
} // namespace RME

#endif // RME_LIGHTCALCULATORSERVICE_H