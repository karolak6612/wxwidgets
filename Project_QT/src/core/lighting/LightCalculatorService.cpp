#include "LightCalculatorService.h"
#include "core/map/Map.h"
#include "core/map/Tile.h"
#include "core/Item.h"
#include "core/settings/AppSettings.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemData.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace RME {
namespace core {
namespace lighting {

LightCalculatorService::LightCalculatorService(QObject* parent)
    : QObject(parent)
    , m_globalLightColor(50, 50, 50)  // Dark gray default
    , m_globalAmbientLevel(0.2f)      // 20% ambient light
    , m_lightingEnabled(true)
{
}

LightCalculatorService::~LightCalculatorService() = default;

void LightCalculatorService::setMap(RME::core::Map* map)
{
    if (m_map != map) {
        m_map = map;
        emit onMapChanged();
    }
}

void LightCalculatorService::setAppSettings(RME::core::settings::AppSettings* settings)
{
    if (m_appSettings != settings) {
        m_appSettings = settings;
        
        // Load settings
        if (m_appSettings) {
            // Load lighting settings from AppSettings
            m_lightingEnabled = m_appSettings->getBool("lighting/enabled", true);
            m_globalAmbientLevel = m_appSettings->getFloat("lighting/ambientLevel", 0.2f);
            
            // Load global light color
            QColor defaultColor(50, 50, 50);
            QString colorString = m_appSettings->getString("lighting/globalColor", defaultColor.name());
            QColor loadedColor(colorString);
            if (loadedColor.isValid()) {
                m_globalLightColor = loadedColor;
            }
            
            qDebug() << "LightCalculatorService: Loaded lighting settings - enabled:" << m_lightingEnabled 
                     << "ambient:" << m_globalAmbientLevel << "color:" << m_globalLightColor.name();
        }
    }
}

void LightCalculatorService::setAssetManager(RME::core::assets::AssetManager* assetManager)
{
    m_assetManager = assetManager;
}

void LightCalculatorService::setGlobalLightColor(const QColor& color)
{
    m_globalLightColor = color;
}

void LightCalculatorService::addDynamicLight(const LightSource& light)
{
    // Check if light already exists at this position
    auto it = std::find_if(m_dynamicLights.begin(), m_dynamicLights.end(),
        [&light](const LightSource& existing) {
            return existing.position == light.position;
        });
    
    if (it != m_dynamicLights.end()) {
        // Update existing light (use maximum intensity)
        if (light.intensity > it->intensity) {
            *it = light;
        }
    } else {
        // Add new light
        m_dynamicLights.push_back(light);
    }
}

void LightCalculatorService::removeDynamicLight(const Position& position)
{
    m_dynamicLights.erase(
        std::remove_if(m_dynamicLights.begin(), m_dynamicLights.end(),
            [&position](const LightSource& light) {
                return light.position == position;
            }),
        m_dynamicLights.end());
}

void LightCalculatorService::clearDynamicLights()
{
    m_dynamicLights.clear();
}

TileLightInfo LightCalculatorService::calculateLightForTile(const Position& tilePos) const
{
    if (!m_lightingEnabled || !m_map) {
        return TileLightInfo(QColor(255, 255, 255), 1.0f); // Full bright if lighting disabled
    }

    // Start with ambient light
    TileLightInfo tileLight = calculateAmbientLight(tilePos);

    // Add contribution from items on this tile
    const Tile* tile = m_map->getTile(tilePos);
    if (tile) {
        // Check ground item for light
        const Item* ground = tile->getGround();
        if (ground && ground->hasLight()) {
            QColor itemLightColor = getLightColorFromItemId(ground->getID());
            float intensity = ground->getLightIntensity() * LIGHT_FALLOFF_FACTOR;
            addLightContribution(tileLight, itemLightColor, intensity);
        }

        // Check other items for light
        const auto& items = tile->getItems();
        for (const auto& item : items) {
            if (item && item->hasLight()) {
                QColor itemLightColor = getLightColorFromItemId(item->getID());
                float intensity = item->getLightIntensity() * LIGHT_FALLOFF_FACTOR;
                addLightContribution(tileLight, itemLightColor, intensity);
            }
        }
    }

    // Add contribution from dynamic lights
    for (const auto& light : m_dynamicLights) {
        float intensity = calculateLightIntensity(tilePos, light);
        if (intensity > MIN_LIGHT_INTENSITY) {
            addLightContribution(tileLight, light.color, intensity);
        }
    }

    return tileLight;
}

void LightCalculatorService::calculateLightForRegion(const Position& startPos, const Position& endPos, 
                                                   std::vector<std::vector<TileLightInfo>>& lightMap) const
{
    int width = endPos.x - startPos.x + 1;
    int height = endPos.y - startPos.y + 1;
    
    lightMap.resize(height);
    for (auto& row : lightMap) {
        row.resize(width);
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Position tilePos(startPos.x + x, startPos.y + y, startPos.z);
            lightMap[y][x] = calculateLightForTile(tilePos);
        }
    }
}

bool LightCalculatorService::isLightingEnabled() const
{
    return m_lightingEnabled;
}

void LightCalculatorService::setLightingEnabled(bool enabled)
{
    if (m_lightingEnabled != enabled) {
        m_lightingEnabled = enabled;
        emit onSettingsChanged();
    }
}

float LightCalculatorService::getGlobalAmbientLevel() const
{
    return m_globalAmbientLevel;
}

void LightCalculatorService::setGlobalAmbientLevel(float level)
{
    m_globalAmbientLevel = std::clamp(level, 0.0f, 1.0f);
}

void LightCalculatorService::onMapChanged()
{
    // Clear dynamic lights when map changes
    clearDynamicLights();
}

void LightCalculatorService::onSettingsChanged()
{
    // Reload settings if needed
    if (m_appSettings) {
        // Reload lighting settings
        bool oldEnabled = m_lightingEnabled;
        float oldAmbient = m_globalAmbientLevel;
        QColor oldColor = m_globalLightColor;
        
        m_lightingEnabled = m_appSettings->getBool("lighting/enabled", true);
        m_globalAmbientLevel = m_appSettings->getFloat("lighting/ambientLevel", 0.2f);
        
        // Reload global light color
        QColor defaultColor(50, 50, 50);
        QString colorString = m_appSettings->getString("lighting/globalColor", defaultColor.name());
        QColor loadedColor(colorString);
        if (loadedColor.isValid()) {
            m_globalLightColor = loadedColor;
        }
        
        // Check if settings actually changed
        bool settingsChanged = (oldEnabled != m_lightingEnabled) ||
                              (qAbs(oldAmbient - m_globalAmbientLevel) > 0.001f) ||
                              (oldColor != m_globalLightColor);
        
        if (settingsChanged) {
            qDebug() << "LightCalculatorService: Settings changed - enabled:" << m_lightingEnabled 
                     << "ambient:" << m_globalAmbientLevel << "color:" << m_globalLightColor.name();
            
            // Emit signal to notify that lighting needs to be recalculated
            emit lightingSettingsChanged();
        }
    }
}

float LightCalculatorService::calculateLightIntensity(const Position& tilePos, const LightSource& light) const
{
    // Calculate distance between tile and light source
    int dx = tilePos.x - light.position.x;
    int dy = tilePos.y - light.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Check if within light radius
    if (distance > light.intensity) {
        return 0.0f;
    }
    
    // Calculate intensity based on distance
    float intensity = (-distance + light.intensity) * LIGHT_FALLOFF_FACTOR;
    
    if (intensity < MIN_LIGHT_INTENSITY) {
        return 0.0f;
    }
    
    return std::min(intensity, 1.0f);
}

QColor LightCalculatorService::getLightColorFromItemId(uint16_t itemId) const
{
    if (!m_assetManager) {
        return QColor(255, 255, 255); // White default
    }

    // Try to get item data
    const auto* itemData = m_assetManager->getItemData(itemId);
    if (itemData && itemData->hasLight()) {
        // Get actual light color from item data when available
        if (itemData->lightColor.isValid()) {
            return itemData->lightColor;
        }
        
        // Fallback: use predefined colors for common light sources
        return getPredefinedLightColor(itemId);
    }

    return QColor(255, 255, 255); // White default
}

QColor LightCalculatorService::getPredefinedLightColor(uint16_t itemId) const
{
    // Predefined colors for common light sources
    // These are based on typical Tibia light source items
    
    // Torches - warm orange/yellow
    if (itemId >= 1479 && itemId <= 1482) {
        return QColor(255, 200, 100); // Warm orange
    }
    
    // Candles - soft yellow
    if (itemId >= 2041 && itemId <= 2044) {
        return QColor(255, 220, 150); // Soft yellow
    }
    
    // Lamps - bright white-yellow
    if (itemId >= 2051 && itemId <= 2054) {
        return QColor(255, 255, 200); // Bright white-yellow
    }
    
    // Magical lights - blue/purple
    if (itemId >= 2162 && itemId <= 2165) {
        return QColor(150, 150, 255); // Magical blue
    }
    
    // Fire fields - red/orange
    if (itemId >= 1487 && itemId <= 1490) {
        return QColor(255, 100, 50); // Fire red-orange
    }
    
    // Energy fields - electric blue
    if (itemId >= 1491 && itemId <= 1494) {
        return QColor(100, 200, 255); // Electric blue
    }
    
    // Poison fields - green
    if (itemId >= 1495 && itemId <= 1498) {
        return QColor(100, 255, 100); // Poison green
    }
    
    // Use a deterministic color based on item ID for unknown items
    int hue = (itemId * 137) % 360;
    return QColor::fromHsv(hue, 128, 255);
}

TileLightInfo LightCalculatorService::calculateAmbientLight(const Position& tilePos) const
{
    // Global ambient light
    QColor ambientColor = m_globalLightColor;
    float ambientLevel = m_globalAmbientLevel;
    
    // Add time-of-day variations, underground modifications, etc.
    
    // Underground modifications (floors 8+ are underground)
    if (tilePos.z >= 8) {
        float undergroundFactor = calculateUndergroundLightFactor(tilePos.z);
        ambientLevel *= undergroundFactor;
        
        // Make underground light slightly more blue/cold
        ambientColor = QColor(
            static_cast<int>(ambientColor.red() * 0.8f),
            static_cast<int>(ambientColor.green() * 0.9f),
            static_cast<int>(ambientColor.blue() * 1.1f)
        );
    }
    
    // Time-of-day variations (if enabled in settings)
    if (m_appSettings && m_appSettings->getBool("lighting/enableTimeOfDay", false)) {
        float timeOfDayFactor = calculateTimeOfDayFactor();
        ambientLevel *= timeOfDayFactor;
        
        // Adjust color temperature based on time of day
        QColor timeColor = getTimeOfDayColor();
        ambientColor = blendColors(ambientColor, timeColor, 0.3f);
    }
    
    // Weather effects (if enabled)
    if (m_appSettings && m_appSettings->getBool("lighting/enableWeather", false)) {
        float weatherFactor = calculateWeatherLightFactor();
        ambientLevel *= weatherFactor;
    }
    
    // Clamp ambient level
    ambientLevel = std::clamp(ambientLevel, 0.0f, 1.0f);
    
    return TileLightInfo(ambientColor, ambientLevel);
}

float LightCalculatorService::calculateUndergroundLightFactor(int floor) const
{
    // Underground floors have progressively less ambient light
    if (floor < 8) {
        return 1.0f; // Surface level
    }
    
    // Each underground level reduces ambient light
    float reduction = (floor - 7) * 0.15f; // 15% reduction per level
    float factor = 1.0f - reduction;
    
    // Minimum 5% ambient light even in deepest levels
    return std::max(0.05f, factor);
}

float LightCalculatorService::calculateTimeOfDayFactor() const
{
    if (!m_appSettings) {
        return 1.0f;
    }
    
    // Get current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    float timeOfDay = m_appSettings->getFloat("lighting/timeOfDay", 0.5f);
    
    // Convert to 24-hour format
    float hour = timeOfDay * 24.0f;
    
    // Calculate lighting factor based on time
    if (hour >= 6.0f && hour <= 18.0f) {
        // Daytime: smooth curve from dawn to noon to dusk
        float dayProgress = (hour - 6.0f) / 12.0f; // 0.0 to 1.0
        float curve = 1.0f - std::abs(dayProgress - 0.5f) * 2.0f; // Peak at 0.5 (noon)
        return 0.3f + curve * 0.7f; // Range from 0.3 to 1.0
    } else {
        // Nighttime: low light
        return 0.1f;
    }
}

QColor LightCalculatorService::getTimeOfDayColor() const
{
    if (!m_appSettings) {
        return QColor(255, 255, 255);
    }
    
    float timeOfDay = m_appSettings->getFloat("lighting/timeOfDay", 0.5f);
    float hour = timeOfDay * 24.0f;
    
    if (hour >= 5.0f && hour <= 7.0f) {
        // Dawn - warm orange
        return QColor(255, 200, 150);
    } else if (hour >= 7.0f && hour <= 17.0f) {
        // Day - neutral white
        return QColor(255, 255, 255);
    } else if (hour >= 17.0f && hour <= 19.0f) {
        // Dusk - warm red-orange
        return QColor(255, 180, 120);
    } else {
        // Night - cool blue
        return QColor(150, 180, 255);
    }
}

float LightCalculatorService::calculateWeatherLightFactor() const
{
    if (!m_appSettings) {
        return 1.0f;
    }
    
    // Get weather type from settings
    QString weather = m_appSettings->getString("lighting/weather", "clear");
    
    if (weather == "rain") {
        return 0.7f; // 30% reduction for rain
    } else if (weather == "storm") {
        return 0.5f; // 50% reduction for storms
    } else if (weather == "fog") {
        return 0.6f; // 40% reduction for fog
    } else if (weather == "snow") {
        return 0.8f; // 20% reduction for snow
    }
    
    return 1.0f; // Clear weather
}

QColor LightCalculatorService::blendColors(const QColor& color1, const QColor& color2, float factor) const
{
    factor = std::clamp(factor, 0.0f, 1.0f);
    
    int r = static_cast<int>(color1.red() * (1.0f - factor) + color2.red() * factor);
    int g = static_cast<int>(color1.green() * (1.0f - factor) + color2.green() * factor);
    int b = static_cast<int>(color1.blue() * (1.0f - factor) + color2.blue() * factor);
    
    return QColor(r, g, b);
}

void LightCalculatorService::addLightContribution(TileLightInfo& tileLight, const QColor& lightColor, float intensity) const
{
    if (intensity <= MIN_LIGHT_INTENSITY) {
        return;
    }

    // Additive light blending
    int newRed = std::min(255, tileLight.lightColor.red() + static_cast<int>(lightColor.red() * intensity));
    int newGreen = std::min(255, tileLight.lightColor.green() + static_cast<int>(lightColor.green() * intensity));
    int newBlue = std::min(255, tileLight.lightColor.blue() + static_cast<int>(lightColor.blue() * intensity));
    
    tileLight.lightColor = QColor(newRed, newGreen, newBlue);
    tileLight.lightLevel = std::min(1.0f, tileLight.lightLevel + intensity);
}

} // namespace lighting
} // namespace core
} // namespace RME