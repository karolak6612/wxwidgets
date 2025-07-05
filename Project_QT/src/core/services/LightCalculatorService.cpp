#include "LightCalculatorService.h"
#include "core/map/Map.h"
#include "core/Position.h"
#include "core/lighting/LightingTypes.h"
#include <QDebug>
#include <cmath>

namespace RME {
namespace core {

LightCalculatorService::LightCalculatorService(QObject* parent)
    : QObject(parent)
    , m_globalLightColor(255, 255, 255)
    , m_lightingEnabled(true)
    , m_globalAmbientLevel(0.2f)
{
    qDebug() << "LightCalculatorService: Initialized with default lighting settings";
}

LightCalculatorService::~LightCalculatorService()
{
    clearDynamicLights();
}

void LightCalculatorService::setGlobalLightColor(const QColor& color)
{
    if (m_globalLightColor != color) {
        m_globalLightColor = color;
        qDebug() << "LightCalculatorService: Global light color changed to" << color;
    }
}

QColor LightCalculatorService::getGlobalLightColor() const
{
    return m_globalLightColor;
}

void LightCalculatorService::addDynamicLight(const lighting::LightSource& light)
{
    // Remove existing light at the same position
    removeDynamicLight(light.position);
    
    m_dynamicLights.push_back(light);
    qDebug() << "LightCalculatorService: Added dynamic light at" 
             << light.position.x << light.position.y << light.position.z
             << "with level" << static_cast<int>(light.level);
}

void LightCalculatorService::removeDynamicLight(const Position& position)
{
    auto it = std::find_if(m_dynamicLights.begin(), m_dynamicLights.end(),
        [&position](const lighting::LightSource& light) {
            return light.position == position;
        });
    
    if (it != m_dynamicLights.end()) {
        m_dynamicLights.erase(it);
        qDebug() << "LightCalculatorService: Removed dynamic light at" 
                 << position.x << position.y << position.z;
    }
}

void LightCalculatorService::clearDynamicLights()
{
    if (!m_dynamicLights.empty()) {
        m_dynamicLights.clear();
        qDebug() << "LightCalculatorService: Cleared all dynamic lights";
    }
}

const std::vector<lighting::LightSource>& LightCalculatorService::getDynamicLights() const
{
    return m_dynamicLights;
}

lighting::TileLightInfo LightCalculatorService::calculateLightForTile(const Position& tilePos) const
{
    lighting::TileLightInfo lightInfo;
    
    if (!m_lightingEnabled) {
        // Return full brightness when lighting is disabled
        lightInfo.totalLight = 255;
        lightInfo.lightColor = QColor(255, 255, 255);
        lightInfo.ambientContribution = 255;
        lightInfo.dynamicContribution = 0;
        return lightInfo;
    }
    
    // Start with ambient light
    float ambientLight = m_globalAmbientLevel * 255.0f;
    lightInfo.ambientContribution = static_cast<uint8_t>(qMin(255.0f, ambientLight));
    
    // Calculate dynamic light contributions
    float dynamicLight = 0.0f;
    QColor combinedColor = m_globalLightColor;
    
    for (const auto& light : m_dynamicLights) {
        // Calculate distance
        float dx = static_cast<float>(tilePos.x - light.position.x);
        float dy = static_cast<float>(tilePos.y - light.position.y);
        float dz = static_cast<float>(tilePos.z - light.position.z);
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        // Calculate light falloff (inverse square law with minimum)
        float falloff = 1.0f / (1.0f + distance * distance * 0.1f);
        float contribution = static_cast<float>(light.level) * falloff;
        
        if (contribution > 1.0f) { // Only consider significant contributions
            dynamicLight += contribution;
            
            // Blend light colors (simplified)
            if (light.color != 215) { // 215 is default white
                // Convert color index to RGB (simplified)
                QColor lightColor = convertLightColorToRGB(light.color);
                combinedColor = blendColors(combinedColor, lightColor, contribution / 255.0f);
            }
        }
    }
    
    lightInfo.dynamicContribution = static_cast<uint8_t>(qMin(255.0f, dynamicLight));
    
    // Combine ambient and dynamic light
    float totalLight = ambientLight + dynamicLight;
    lightInfo.totalLight = static_cast<uint8_t>(qMin(255.0f, totalLight));
    lightInfo.lightColor = combinedColor;
    
    return lightInfo;
}

void LightCalculatorService::calculateLightForRegion(const Position& startPos, const Position& endPos, 
                                                   std::vector<std::vector<lighting::TileLightInfo>>& lightMap) const
{
    int width = endPos.x - startPos.x + 1;
    int height = endPos.y - startPos.y + 1;
    
    // Resize the light map
    lightMap.resize(height);
    for (auto& row : lightMap) {
        row.resize(width);
    }
    
    // Calculate lighting for each tile in the region
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
        qDebug() << "LightCalculatorService: Lighting" << (enabled ? "enabled" : "disabled");
    }
}

float LightCalculatorService::getGlobalAmbientLevel() const
{
    return m_globalAmbientLevel;
}

void LightCalculatorService::setGlobalAmbientLevel(float level)
{
    float clampedLevel = qMax(0.0f, qMin(level, 1.0f));
    
    if (qAbs(m_globalAmbientLevel - clampedLevel) > 0.001f) {
        m_globalAmbientLevel = clampedLevel;
        qDebug() << "LightCalculatorService: Global ambient level changed to" << clampedLevel;
    }
}

QColor LightCalculatorService::convertLightColorToRGB(uint8_t colorIndex) const
{
    // Simplified color conversion - in a real implementation this would
    // use the actual Tibia color palette
    switch (colorIndex) {
        case 206: return QColor(255, 255, 0);   // Yellow
        case 207: return QColor(255, 128, 0);   // Orange  
        case 208: return QColor(255, 0, 0);     // Red
        case 209: return QColor(128, 0, 255);   // Purple
        case 210: return QColor(0, 0, 255);     // Blue
        case 211: return QColor(0, 255, 255);   // Cyan
        case 212: return QColor(0, 255, 0);     // Green
        case 213: return QColor(255, 255, 255); // White
        case 214: return QColor(128, 128, 128); // Gray
        case 215: return QColor(255, 255, 255); // Default white
        default:  return QColor(255, 255, 255); // Default to white
    }
}

QColor LightCalculatorService::blendColors(const QColor& base, const QColor& overlay, float alpha) const
{
    float invAlpha = 1.0f - alpha;
    
    int r = static_cast<int>(base.red() * invAlpha + overlay.red() * alpha);
    int g = static_cast<int>(base.green() * invAlpha + overlay.green() * alpha);
    int b = static_cast<int>(base.blue() * invAlpha + overlay.blue() * alpha);
    
    return QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
}

} // namespace core
} // namespace RME