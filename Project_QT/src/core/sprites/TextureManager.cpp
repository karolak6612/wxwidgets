#include "TextureManager.h"
#include "SpriteManager.h"
#include "SpriteData.h"
#include <QDebug>
#include <QOpenGLContext>

namespace RME {
namespace core {
namespace sprites {

TextureManager::TextureManager(QObject* parent)
    : QObject(parent)
{
}

TextureManager::~TextureManager()
{
    clearCache();
}

bool TextureManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    // Check if we have an active OpenGL context
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning() << "TextureManager: No active OpenGL context for initialization";
        return false;
    }

    // Initialize OpenGL functions
    if (!initializeOpenGLFunctions()) {
        qWarning() << "TextureManager: Failed to initialize OpenGL functions";
        return false;
    }

    m_initialized = true;
    qDebug() << "TextureManager: Initialized successfully";
    return true;
}

void TextureManager::setSpriteManager(SpriteManager* spriteManager)
{
    if (m_spriteManager != spriteManager) {
        // Clear cache when sprite manager changes
        clearCache();
        m_spriteManager = spriteManager;
    }
}

GLuint TextureManager::getTextureForSprite(quint32 spriteId)
{
    if (!m_initialized || !m_spriteManager) {
        return 0;
    }

    // Check if texture is already cached
    auto it = m_spriteTextures.find(spriteId);
    if (it != m_spriteTextures.end()) {
        return it.value();
    }

    // Get sprite data from SpriteManager
    const SpriteData* spriteData = m_spriteManager->getSpriteData(spriteId);
    if (!spriteData) {
        qWarning() << "TextureManager: Sprite" << spriteId << "not found in SpriteManager";
        return 0;
    }

    // Get the first frame image (for now, animation support can be added later)
    if (spriteData->frames.isEmpty()) {
        qWarning() << "TextureManager: Sprite" << spriteId << "has no frames";
        return 0;
    }

    const QImage& spriteImage = spriteData->frames.first().image;
    if (spriteImage.isNull()) {
        qWarning() << "TextureManager: Sprite" << spriteId << "has null image";
        return 0;
    }

    // Create OpenGL texture
    GLuint textureId = createTextureFromQImage(spriteImage);
    if (textureId != 0) {
        // Cache the texture
        m_spriteTextures[spriteId] = textureId;
        qDebug() << "TextureManager: Created texture" << textureId << "for sprite" << spriteId;
    }

    return textureId;
}

GLuint TextureManager::createTextureFromQImage(const QImage& image)
{
    if (!m_initialized || image.isNull()) {
        return 0;
    }

    // Convert QImage to OpenGL-compatible format
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    
    // Generate OpenGL texture
    GLuint textureId;
    glGenTextures(1, &textureId);
    
    if (textureId == 0) {
        qWarning() << "TextureManager: Failed to generate OpenGL texture";
        return 0;
    }

    // Bind and configure texture
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Upload image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.constBits());
    
    // Set texture parameters for pixel-perfect 2D rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "TextureManager: OpenGL error creating texture:" << error;
        glDeleteTextures(1, &textureId);
        return 0;
    }

    return textureId;
}

GLuint TextureManager::getTextureForSpriteFrame(quint32 spriteId, int frameIndex)
{
    if (!m_initialized || !m_spriteManager) {
        return 0;
    }

    // Check if frame texture is already cached
    QPair<quint32, int> frameKey(spriteId, frameIndex);
    auto it = m_frameTextures.find(frameKey);
    if (it != m_frameTextures.end()) {
        return it.value();
    }

    // Get sprite data from SpriteManager
    const SpriteData* spriteData = m_spriteManager->getSpriteData(spriteId);
    if (!spriteData) {
        qWarning() << "TextureManager: Sprite" << spriteId << "not found in SpriteManager";
        return 0;
    }

    // Check if frame index is valid
    if (frameIndex < 0 || frameIndex >= spriteData->frames.size()) {
        qWarning() << "TextureManager: Invalid frame index" << frameIndex << "for sprite" << spriteId 
                   << "(has" << spriteData->frames.size() << "frames)";
        return 0;
    }

    const QImage& frameImage = spriteData->frames[frameIndex].image;
    if (frameImage.isNull()) {
        qWarning() << "TextureManager: Sprite" << spriteId << "frame" << frameIndex << "has null image";
        return 0;
    }

    // Create OpenGL texture for this frame
    GLuint textureId = createTextureFromQImage(frameImage);
    if (textureId != 0) {
        // Cache the frame texture
        m_frameTextures[frameKey] = textureId;
        qDebug() << "TextureManager: Created texture" << textureId << "for sprite" << spriteId << "frame" << frameIndex;
    }

    return textureId;
}

int TextureManager::getSpriteFrameCount(quint32 spriteId) const
{
    if (!m_spriteManager) {
        return 0;
    }

    const SpriteData* spriteData = m_spriteManager->getSpriteData(spriteId);
    if (!spriteData) {
        return 0;
    }

    return spriteData->frames.size();
}

void TextureManager::clearCache()
{
    if (!m_initialized) {
        return;
    }

    // Delete all cached textures
    for (auto it = m_spriteTextures.begin(); it != m_spriteTextures.end(); ++it) {
        deleteTexture(it.value());
    }
    
    // Delete all cached frame textures
    for (auto it = m_frameTextures.begin(); it != m_frameTextures.end(); ++it) {
        deleteTexture(it.value());
    }
    
    m_spriteTextures.clear();
    m_frameTextures.clear();
    qDebug() << "TextureManager: Cleared texture cache";
}

void TextureManager::deleteTexture(GLuint textureId)
{
    if (textureId != 0 && m_initialized) {
        glDeleteTextures(1, &textureId);
    }
}

} // namespace sprites
} // namespace core
} // namespace RME