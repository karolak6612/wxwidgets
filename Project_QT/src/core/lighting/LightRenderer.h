#pragma once

#include "LightingTypes.h"
#include <QObject>
#include <QOpenGLFunctions>
#include <vector>
#include <memory>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;

namespace RME {
namespace core {
    class Position;
    namespace settings { class AppSettings; }
}
}

namespace RME {
namespace core {
namespace lighting {

class LightCalculatorService;

/**
 * @brief Renders lighting effects using OpenGL
 * 
 * This class handles the OpenGL rendering of lighting effects calculated
 * by the LightCalculatorService. It creates a lighting overlay that can
 * be blended with the map rendering.
 */
class LightRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit LightRenderer(QObject* parent = nullptr);
    ~LightRenderer();

    // Initialization
    bool initialize();
    void cleanup();

    // Dependencies
    void setLightCalculatorService(LightCalculatorService* service);
    void setAppSettings(RME::core::settings::AppSettings* settings);

    // Rendering
    void renderLighting(const Position& startPos, const Position& endPos, 
                       int scrollX, int scrollY, bool fogMode = false);

    // Settings
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

private:
    // OpenGL setup
    bool initializeShaders();
    bool initializeBuffers();
    void cleanupOpenGL();

    // Rendering helpers
    void updateLightTexture(const Position& startPos, const Position& endPos);
    void renderLightOverlay(const Position& startPos, const Position& endPos, 
                           int scrollX, int scrollY);
    void renderFogOverlay(const Position& startPos, const Position& endPos, 
                         int scrollX, int scrollY);

    // Dependencies
    LightCalculatorService* m_lightCalculatorService = nullptr;
    RME::core::settings::AppSettings* m_appSettings = nullptr;

    // OpenGL resources
    QOpenGLShaderProgram* m_lightShader = nullptr;
    QOpenGLBuffer* m_quadVBO = nullptr;
    QOpenGLVertexArrayObject* m_quadVAO = nullptr;
    GLuint m_lightTexture = 0;

    // Rendering state
    bool m_initialized = false;
    bool m_enabled = true;
    std::vector<uint8_t> m_lightBuffer;
    int m_textureWidth = 0;
    int m_textureHeight = 0;

    // Constants
    static constexpr int TILE_SIZE = 32;
    static constexpr int PIXEL_FORMAT_RGBA = 4;
};

} // namespace lighting
} // namespace core
} // namespace RME