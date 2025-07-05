#include "LightRenderer.h"
#include "LightCalculatorService.h"
#include "core/Position.h"
#include "core/settings/AppSettings.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <algorithm>

namespace RME {
namespace core {
namespace lighting {

LightRenderer::LightRenderer(QObject* parent)
    : QObject(parent)
{
}

LightRenderer::~LightRenderer()
{
    cleanup();
}

bool LightRenderer::initialize()
{
    if (m_initialized) {
        return true;
    }

    if (!initializeOpenGLFunctions()) {
        qWarning() << "LightRenderer: Failed to initialize OpenGL functions";
        return false;
    }

    if (!initializeShaders()) {
        qWarning() << "LightRenderer: Failed to initialize shaders";
        return false;
    }

    if (!initializeBuffers()) {
        qWarning() << "LightRenderer: Failed to initialize buffers";
        return false;
    }

    // Create light texture
    glGenTextures(1, &m_lightTexture);
    if (m_lightTexture == 0) {
        qWarning() << "LightRenderer: Failed to create light texture";
        return false;
    }

    m_initialized = true;
    qDebug() << "LightRenderer: Initialized successfully";
    return true;
}

void LightRenderer::cleanup()
{
    if (m_initialized) {
        cleanupOpenGL();
        m_initialized = false;
    }
}

void LightRenderer::setLightCalculatorService(LightCalculatorService* service)
{
    m_lightCalculatorService = service;
}

void LightRenderer::setAppSettings(RME::core::settings::AppSettings* settings)
{
    m_appSettings = settings;
}

void LightRenderer::renderLighting(const Position& startPos, const Position& endPos, 
                                  int scrollX, int scrollY, bool fogMode)
{
    if (!m_initialized || !m_enabled || !m_lightCalculatorService) {
        return;
    }

    // Check if lighting is enabled in settings
    if (m_appSettings && !m_lightCalculatorService->isLightingEnabled()) {
        return;
    }

    // Update light texture with current lighting data
    updateLightTexture(startPos, endPos);

    // Render the lighting overlay
    renderLightOverlay(startPos, endPos, scrollX, scrollY);

    // Render fog overlay if requested
    if (fogMode) {
        renderFogOverlay(startPos, endPos, scrollX, scrollY);
    }
}

bool LightRenderer::initializeShaders()
{
    m_lightShader = new QOpenGLShaderProgram(this);

    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 mvpMatrix;
        
        out vec2 texCoord;
        
        void main() {
            gl_Position = mvpMatrix * vec4(aPos, 0.0, 1.0);
            texCoord = aTexCoord;
        }
    )";

    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 texCoord;
        out vec4 FragColor;
        
        uniform sampler2D lightTexture;
        uniform float globalAlpha;
        
        void main() {
            vec4 lightColor = texture(lightTexture, texCoord);
            FragColor = vec4(lightColor.rgb, lightColor.a * globalAlpha);
        }
    )";

    if (!m_lightShader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "LightRenderer: Failed to compile vertex shader:" << m_lightShader->log();
        return false;
    }

    if (!m_lightShader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "LightRenderer: Failed to compile fragment shader:" << m_lightShader->log();
        return false;
    }

    if (!m_lightShader->link()) {
        qWarning() << "LightRenderer: Failed to link shader program:" << m_lightShader->log();
        return false;
    }

    return true;
}

bool LightRenderer::initializeBuffers()
{
    // Create VAO
    m_quadVAO = new QOpenGLVertexArrayObject(this);
    if (!m_quadVAO->create()) {
        qWarning() << "LightRenderer: Failed to create VAO";
        return false;
    }

    // Create VBO for quad vertices (position + texture coordinates)
    m_quadVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if (!m_quadVBO->create()) {
        qWarning() << "LightRenderer: Failed to create VBO";
        return false;
    }

    // Quad vertices with texture coordinates
    float quadVertices[] = {
        // Position   // TexCoord
        0.0f, 0.0f,   0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,   1.0f, 0.0f,  // Bottom-right
        1.0f, 1.0f,   1.0f, 1.0f,  // Top-right
        0.0f, 1.0f,   0.0f, 1.0f   // Top-left
    };

    m_quadVAO->bind();
    m_quadVBO->bind();
    m_quadVBO->allocate(quadVertices, sizeof(quadVertices));

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m_quadVBO->release();
    m_quadVAO->release();

    return true;
}

void LightRenderer::cleanupOpenGL()
{
    delete m_lightShader;
    delete m_quadVBO;
    delete m_quadVAO;
    
    if (m_lightTexture != 0) {
        glDeleteTextures(1, &m_lightTexture);
        m_lightTexture = 0;
    }

    m_lightShader = nullptr;
    m_quadVBO = nullptr;
    m_quadVAO = nullptr;
}

void LightRenderer::updateLightTexture(const Position& startPos, const Position& endPos)
{
    int width = endPos.x - startPos.x + 1;
    int height = endPos.y - startPos.y + 1;

    if (width <= 0 || height <= 0) {
        return;
    }

    // Resize buffer if needed
    size_t bufferSize = width * height * PIXEL_FORMAT_RGBA;
    if (m_lightBuffer.size() != bufferSize) {
        m_lightBuffer.resize(bufferSize);
    }

    // Calculate lighting for the region
    std::vector<std::vector<TileLightInfo>> lightMap;
    m_lightCalculatorService->calculateLightForRegion(startPos, endPos, lightMap);

    // Fill the buffer with lighting data
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * PIXEL_FORMAT_RGBA;
            const TileLightInfo& lightInfo = lightMap[y][x];

            m_lightBuffer[index] = lightInfo.lightColor.red();
            m_lightBuffer[index + 1] = lightInfo.lightColor.green();
            m_lightBuffer[index + 2] = lightInfo.lightColor.blue();
            m_lightBuffer[index + 3] = static_cast<uint8_t>(lightInfo.lightLevel * 255);
        }
    }

    // Update OpenGL texture
    glBindTexture(GL_TEXTURE_2D, m_lightTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_lightBuffer.data());

    m_textureWidth = width;
    m_textureHeight = height;
}

void LightRenderer::renderLightOverlay(const Position& startPos, const Position& endPos, 
                                      int scrollX, int scrollY)
{
    if (!m_lightShader || !m_quadVAO || m_lightTexture == 0) {
        return;
    }

    // Calculate screen position and size
    int drawX = startPos.x * TILE_SIZE - scrollX;
    int drawY = startPos.y * TILE_SIZE - scrollY;
    int drawWidth = (endPos.x - startPos.x + 1) * TILE_SIZE;
    int drawHeight = (endPos.y - startPos.y + 1) * TILE_SIZE;

    // Set up blending for lighting overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);

    // Bind shader and set uniforms
    m_lightShader->bind();
    
    // Create transformation matrix
    QMatrix4x4 modelMatrix;
    modelMatrix.translate(drawX, drawY);
    modelMatrix.scale(drawWidth, drawHeight);
    
    // Create orthographic projection matrix for 2D rendering
    QMatrix4x4 projectionMatrix;
    
    // Get current viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int viewportWidth = viewport[2];
    int viewportHeight = viewport[3];
    
    // Set up orthographic projection (0,0 at top-left, like screen coordinates)
    projectionMatrix.ortho(0.0f, static_cast<float>(viewportWidth), 
                          static_cast<float>(viewportHeight), 0.0f, 
                          -1.0f, 1.0f);
    
    QMatrix4x4 mvp = projectionMatrix * modelMatrix;
    m_lightShader->setUniformValue("mvpMatrix", mvp);
    m_lightShader->setUniformValue("globalAlpha", 1.0f);

    // Bind texture and render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_lightTexture);
    m_lightShader->setUniformValue("lightTexture", 0);

    m_quadVAO->bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_quadVAO->release();

    m_lightShader->release();

    // Restore normal blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void LightRenderer::renderFogOverlay(const Position& startPos, const Position& endPos, 
                                    int scrollX, int scrollY)
{
    // Calculate screen position and size
    int drawX = startPos.x * TILE_SIZE - scrollX;
    int drawY = startPos.y * TILE_SIZE - scrollY;
    int drawWidth = (endPos.x - startPos.x + 1) * TILE_SIZE;
    int drawHeight = (endPos.y - startPos.y + 1) * TILE_SIZE;

    // Render simple fog overlay using immediate mode
    glDisable(GL_TEXTURE_2D);
    glColor4ub(10, 10, 10, 80); // Dark semi-transparent overlay

    glBegin(GL_QUADS);
    glVertex2f(drawX, drawY);
    glVertex2f(drawX + drawWidth, drawY);
    glVertex2f(drawX + drawWidth, drawY + drawHeight);
    glVertex2f(drawX, drawY + drawHeight);
    glEnd();

    glColor4ub(255, 255, 255, 255); // Reset color
}

} // namespace lighting
} // namespace core
} // namespace RME