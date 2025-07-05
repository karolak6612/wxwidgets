#pragma once

#include <QObject>
#include <QHash>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QImage>
#include <QRect>
#include "LightingTypes.h"

namespace RME {
namespace core {
    class Map;
    class Position;
    namespace settings { class AppSettings; }
}

namespace core {
namespace lighting {

/**
 * @brief Calculates and manages lighting effects for the map
 * 
 * This class is responsible for:
 * - Managing dynamic light sources
 * - Calculating light propagation
 * - Generating light textures for rendering
 * - Applying ambient light
 */
class LightCalculator : public QObject, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit LightCalculator(QObject* parent = nullptr);
    ~LightCalculator();

    /**
     * @brief Initialize OpenGL functions
     * Must be called with active OpenGL context
     */
    bool initialize();

    /**
     * @brief Set the map to calculate lighting for
     */
    void setMap(RME::core::Map* map);

    /**
     * @brief Set the application settings
     */
    void setAppSettings(RME::core::settings::AppSettings* settings);

    /**
     * @brief Set global ambient light color
     */
    void setGlobalLightColor(const QColor& color);

    /**
     * @brief Add a dynamic light source
     */
    void addLight(const Position& position, uint8_t color, uint8_t intensity);

    /**
     * @brief Add a light source from sprite light data
     */
    void addLight(int mapX, int mapY, int mapZ, uint8_t color, uint8_t intensity);

    /**
     * @brief Clear all dynamic lights
     */
    void clear();

    /**
     * @brief Calculate lighting for a visible map region
     * 
     * @param visibleRect Rectangle of visible map tiles
     * @param scrollX Scroll X offset in pixels
     * @param scrollY Scroll Y offset in pixels
     * @param enableFog Whether to apply fog effect
     */
    void calculateLighting(const QRect& visibleRect, int scrollX, int scrollY, bool enableFog);

    /**
     * @brief Draw the calculated lighting
     * Must be called after calculateLighting
     */
    void draw();

    /**
     * @brief Get light information for a specific tile
     */
    TileLightInfo getLightForTile(const Position& position) const;

private:
    /**
     * @brief Create OpenGL texture for lighting
     */
    void createGLTexture();

    /**
     * @brief Clean up OpenGL resources
     */
    void cleanupGLResources();

    /**
     * @brief Calculate light intensity at a position from a light source
     */
    float calculateIntensity(int mapX, int mapY, const LightSource& light) const;

    /**
     * @brief Convert 8-bit color to RGB
     */
    QColor colorFromEightBit(uint8_t color) const;

    // Map and settings
    RME::core::Map* m_map = nullptr;
    RME::core::settings::AppSettings* m_settings = nullptr;

    // Light sources
    std::vector<LightSource> m_lights;
    QColor m_globalLightColor;

    // OpenGL resources
    GLuint m_lightTexture = 0;
    std::vector<uint8_t> m_lightBuffer;

    // Rendering state
    QRect m_lastVisibleRect;
    int m_lastScrollX = 0;
    int m_lastScrollY = 0;
    bool m_fogEnabled = false;
    bool m_initialized = false;

    // Constants
    static constexpr int PIXEL_FORMAT_RGBA = 4;
    static constexpr int TILE_SIZE = 32;
};

} // namespace lighting
} // namespace core
} // namespace RME