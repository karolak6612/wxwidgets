#pragma once

#include "LightingTypes.h"
#include "core/services/ILightCalculatorService.h"
#include <QObject>
#include <QColor>
#include <vector>
#include <memory>

namespace RME {
namespace core {
    class Map;
    class Position;
    class Tile;
    namespace settings { class AppSettings; }
    namespace assets { class AssetManager; }
}
}

namespace RME {
namespace core {
namespace lighting {

/**
 * @brief Service for calculating lighting effects on the map
 * 
 * This service manages dynamic light sources and calculates lighting
 * information for tiles based on global ambient light, item lights,
 * and dynamic light sources.
 */
class LightCalculatorService : public QObject, public RME::core::ILightCalculatorService
{
    Q_OBJECT

public:
    explicit LightCalculatorService(QObject* parent = nullptr);
    ~LightCalculatorService();

    // Dependencies
    void setMap(RME::core::Map* map);
    void setAppSettings(RME::core::settings::AppSettings* settings);
    void setAssetManager(RME::core::assets::AssetManager* assetManager);

    // Global lighting
    void setGlobalLightColor(const QColor& color);
    QColor getGlobalLightColor() const { return m_globalLightColor; }

    // Dynamic light management
    void addDynamicLight(const LightSource& light);
    void removeDynamicLight(const Position& position);
    void clearDynamicLights();
    const std::vector<LightSource>& getDynamicLights() const { return m_dynamicLights; }

    // Light calculation
    TileLightInfo calculateLightForTile(const Position& tilePos) const;
    
    // Batch calculation for visible area
    void calculateLightForRegion(const Position& startPos, const Position& endPos, 
                                std::vector<std::vector<TileLightInfo>>& lightMap) const;

    // Settings
    bool isLightingEnabled() const;
    void setLightingEnabled(bool enabled);
    
    float getGlobalAmbientLevel() const;
    void setGlobalAmbientLevel(float level);

signals:
    void lightingSettingsChanged();

public slots:
    void onMapChanged();
    void onSettingsChanged();

private:
    // Helper methods
    float calculateLightIntensity(const Position& tilePos, const LightSource& light) const;
    QColor getLightColorFromItemId(uint16_t itemId) const;
    QColor getPredefinedLightColor(uint16_t itemId) const;
    TileLightInfo calculateAmbientLight(const Position& tilePos) const;
    void addLightContribution(TileLightInfo& tileLight, const QColor& lightColor, float intensity) const;
    
    // Environmental lighting helpers
    float calculateUndergroundLightFactor(int floor) const;
    float calculateTimeOfDayFactor() const;
    QColor getTimeOfDayColor() const;
    float calculateWeatherLightFactor() const;
    QColor blendColors(const QColor& color1, const QColor& color2, float factor) const;

    // Dependencies
    RME::core::Map* m_map = nullptr;
    RME::core::settings::AppSettings* m_appSettings = nullptr;
    RME::core::assets::AssetManager* m_assetManager = nullptr;

    // Lighting state
    QColor m_globalLightColor;
    float m_globalAmbientLevel;
    bool m_lightingEnabled;
    std::vector<LightSource> m_dynamicLights;

    // Constants
    static constexpr float LIGHT_FALLOFF_FACTOR = 0.2f;
    static constexpr float MIN_LIGHT_INTENSITY = 0.01f;
    static constexpr uint8_t DEFAULT_AMBIENT_ALPHA = 140;
};

} // namespace lighting
} // namespace core
} // namespace RME