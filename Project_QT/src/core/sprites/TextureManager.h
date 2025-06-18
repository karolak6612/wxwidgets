#pragma once

#include <QObject>
#include <QHash>
#include <QOpenGLFunctions>
#include <QImage>

namespace RME {
namespace core {
namespace sprites {

class SpriteManager;

/**
 * @brief Simple texture manager for 2D sprite rendering
 * 
 * Converts SpriteManager QImages to OpenGL textures for immediate mode rendering.
 * Uses simple texture caching for performance.
 */
class TextureManager : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit TextureManager(QObject* parent = nullptr);
    ~TextureManager();

    /**
     * @brief Initialize OpenGL functions and set up texture management
     * Must be called with active OpenGL context
     */
    bool initialize();

    /**
     * @brief Set the sprite manager to use for sprite data
     */
    void setSpriteManager(SpriteManager* spriteManager);

    /**
     * @brief Get OpenGL texture ID for a sprite
     * Creates texture if not cached, returns 0 if sprite not found
     */
    GLuint getTextureForSprite(quint32 spriteId);

    /**
     * @brief Get OpenGL texture ID for a specific sprite frame
     * Supports animation by allowing frame selection
     */
    GLuint getTextureForSpriteFrame(quint32 spriteId, int frameIndex = 0);

    /**
     * @brief Create OpenGL texture from QImage
     * Returns 0 if creation fails
     */
    GLuint createTextureFromQImage(const QImage& image);

    /**
     * @brief Clear all cached textures
     */
    void clearCache();

    /**
     * @brief Get number of cached textures
     */
    int getCachedTextureCount() const { return m_spriteTextures.size(); }

    /**
     * @brief Get number of frames for a sprite (for animation)
     */
    int getSpriteFrameCount(quint32 spriteId) const;

private:
    /**
     * @brief Delete OpenGL texture and remove from cache
     */
    void deleteTexture(GLuint textureId);

    SpriteManager* m_spriteManager = nullptr;
    QHash<quint32, GLuint> m_spriteTextures; // Sprite ID -> OpenGL texture ID (first frame)
    QHash<QPair<quint32, int>, GLuint> m_frameTextures; // (Sprite ID, Frame) -> OpenGL texture ID
    bool m_initialized = false;
};

} // namespace sprites
} // namespace core
} // namespace RME