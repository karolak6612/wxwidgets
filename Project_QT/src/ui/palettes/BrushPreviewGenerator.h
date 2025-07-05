#pragma once

#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QCache>
#include <QMutex>
#include <QFuture>

namespace RME {
namespace core {
    class Brush;
    namespace assets {
        class AssetManager;
    }
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Generates and caches preview images for brushes
 * 
 * This class creates visual previews for different brush types,
 * caching the results for performance. Supports different preview
 * sizes and styles.
 */
class BrushPreviewGenerator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Preview styles for different use cases
     */
    enum PreviewStyle {
        IconStyle,      ///< Small icon for lists
        ThumbnailStyle, ///< Medium thumbnail for grids
        DetailStyle     ///< Large detailed preview
    };

    explicit BrushPreviewGenerator(QObject* parent = nullptr);
    ~BrushPreviewGenerator();

    // Asset manager for sprite access
    void setAssetManager(RME::core::assets::AssetManager* assetManager);

    // Preview generation
    QPixmap generatePreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style = IconStyle);
    QPixmap generatePreviewAsync(RME::core::Brush* brush, const QSize& size, PreviewStyle style = IconStyle);

    // Cache management
    void clearCache();
    void setCacheSize(int maxCost);
    int getCacheSize() const;

    // Preview configuration
    void setBackgroundColor(const QColor& color);
    QColor getBackgroundColor() const;

    void setGridEnabled(bool enabled);
    bool isGridEnabled() const;

public slots:
    void onPreviewGenerated(RME::core::Brush* brush, const QPixmap& preview);

signals:
    void previewReady(RME::core::Brush* brush, const QPixmap& preview);
    void previewGenerationFailed(RME::core::Brush* brush, const QString& error);

protected:
    // Preview generation methods
    QPixmap generateGroundBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateWallBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateCarpetBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateTableBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateDoodadBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateRawBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateCreatureBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateSpawnBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateWaypointBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateHouseBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateEraserBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);
    QPixmap generateDefaultBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style);

    // Helper methods
    QString generateCacheKey(RME::core::Brush* brush, const QSize& size, PreviewStyle style) const;
    void drawBackground(QPainter& painter, const QRect& rect, PreviewStyle style);
    void drawGrid(QPainter& painter, const QRect& rect, int gridSize = 8);
    void drawBorder(QPainter& painter, const QRect& rect, PreviewStyle style);
    void drawMaterialBorders(QPainter& painter, const BorderSetData& borderSet, const QRect& area, int tileSize, class SpriteManager* spriteManager);

private:
    // Dependencies
    RME::core::assets::AssetManager* m_assetManager = nullptr;

    // Cache
    QCache<QString, QPixmap> m_previewCache;
    mutable QMutex m_cacheMutex;

    // Configuration
    QColor m_backgroundColor = QColor(240, 240, 240);
    bool m_gridEnabled = false;
    int m_maxCacheSize = 100; // Maximum number of cached previews

    // Constants
    static constexpr int DEFAULT_ICON_SIZE = 32;
    static constexpr int DEFAULT_THUMBNAIL_SIZE = 64;
    static constexpr int DEFAULT_DETAIL_SIZE = 128;
};

} // namespace palettes
} // namespace ui
} // namespace RME