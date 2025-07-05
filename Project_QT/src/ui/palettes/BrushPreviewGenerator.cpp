#include "BrushPreviewGenerator.h"
#include "core/brush/Brush.h"
#include "core/assets/AssetManager.h"
#include "core/sprites/SpriteManager.h"
#include <QPainter>
#include <QDebug>
#include <QtConcurrent>
#include <QRegExp>

namespace RME {
namespace ui {
namespace palettes {

BrushPreviewGenerator::BrushPreviewGenerator(QObject* parent)
    : QObject(parent)
    , m_previewCache(m_maxCacheSize)
{
    qDebug() << "BrushPreviewGenerator: Created";
}

BrushPreviewGenerator::~BrushPreviewGenerator()
{
    clearCache();
    qDebug() << "BrushPreviewGenerator: Destroyed";
}

void BrushPreviewGenerator::setAssetManager(RME::core::assets::AssetManager* assetManager)
{
    m_assetManager = assetManager;
    
    if (assetManager) {
        qDebug() << "BrushPreviewGenerator: AssetManager set";
    }
}

QPixmap BrushPreviewGenerator::generatePreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    if (!brush) {
        return QPixmap();
    }

    // Check cache first
    QString cacheKey = generateCacheKey(brush, size, style);
    
    QMutexLocker locker(&m_cacheMutex);
    QPixmap* cachedPreview = m_previewCache.object(cacheKey);
    if (cachedPreview) {
        return *cachedPreview;
    }
    locker.unlock();

    // Generate new preview
    QPixmap preview;
    QString brushType = brush->getType();

    try {
        if (brushType.contains("Ground")) {
            preview = generateGroundBrushPreview(brush, size, style);
        } else if (brushType.contains("Wall")) {
            preview = generateWallBrushPreview(brush, size, style);
        } else if (brushType.contains("Carpet")) {
            preview = generateCarpetBrushPreview(brush, size, style);
        } else if (brushType.contains("Table")) {
            preview = generateTableBrushPreview(brush, size, style);
        } else if (brushType.contains("Doodad")) {
            preview = generateDoodadBrushPreview(brush, size, style);
        } else if (brushType.contains("Raw")) {
            preview = generateRawBrushPreview(brush, size, style);
        } else if (brushType.contains("Creature")) {
            preview = generateCreatureBrushPreview(brush, size, style);
        } else if (brushType.contains("Spawn")) {
            preview = generateSpawnBrushPreview(brush, size, style);
        } else if (brushType.contains("Waypoint")) {
            preview = generateWaypointBrushPreview(brush, size, style);
        } else if (brushType.contains("House")) {
            preview = generateHouseBrushPreview(brush, size, style);
        } else if (brushType.contains("Eraser")) {
            preview = generateEraserBrushPreview(brush, size, style);
        } else {
            preview = generateDefaultBrushPreview(brush, size, style);
        }

        // Cache the result
        locker.relock();
        m_previewCache.insert(cacheKey, new QPixmap(preview));
        
    } catch (const std::exception& e) {
        qWarning() << "BrushPreviewGenerator: Failed to generate preview for" << brush->getName() << ":" << e.what();
        emit previewGenerationFailed(brush, QString::fromStdString(e.what()));
        return generateDefaultBrushPreview(brush, size, style);
    }

    return preview;
}

QPixmap BrushPreviewGenerator::generatePreviewAsync(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    // Check cache first for immediate return
    QString cacheKey = generateCacheKey(brush, size, style);
    
    QMutexLocker locker(&m_cacheMutex);
    QPixmap* cachedPreview = m_previewCache.object(cacheKey);
    if (cachedPreview) {
        return *cachedPreview;
    }
    locker.unlock();

    // Generate asynchronously
    QFuture<QPixmap> future = QtConcurrent::run([this, brush, size, style]() {
        return generatePreview(brush, size, style);
    });

    // For now, return a placeholder and emit signal when ready
    QPixmap placeholder = generateDefaultBrushPreview(brush, size, style);
    
    // Connect future to signal emission using QFutureWatcher
    QFutureWatcher<QPixmap>* watcher = new QFutureWatcher<QPixmap>(this);
    connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [this, watcher]() {
        emit previewReady(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(future);
    // This would require more complex async handling
    
    return placeholder;
}

void BrushPreviewGenerator::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_previewCache.clear();
    qDebug() << "BrushPreviewGenerator: Cache cleared";
}

void BrushPreviewGenerator::setCacheSize(int maxCost)
{
    QMutexLocker locker(&m_cacheMutex);
    m_maxCacheSize = maxCost;
    m_previewCache.setMaxCost(maxCost);
}

int BrushPreviewGenerator::getCacheSize() const
{
    return m_maxCacheSize;
}

void BrushPreviewGenerator::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        clearCache(); // Clear cache since background changed
    }
}

QColor BrushPreviewGenerator::getBackgroundColor() const
{
    return m_backgroundColor;
}

void BrushPreviewGenerator::setGridEnabled(bool enabled)
{
    if (m_gridEnabled != enabled) {
        m_gridEnabled = enabled;
        clearCache(); // Clear cache since grid setting changed
    }
}

bool BrushPreviewGenerator::isGridEnabled() const
{
    return m_gridEnabled;
}

void BrushPreviewGenerator::onPreviewGenerated(RME::core::Brush* brush, const QPixmap& preview)
{
    emit previewReady(brush, preview);
}

// Brush-specific preview generation methods

QPixmap BrushPreviewGenerator::generateGroundBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    if (!brush || !m_assetManager) {
        return generateDefaultBrushPreview(brush, size, style);
    }
    
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::SmoothPixmapTransformation);
    
    drawBackground(painter, preview.rect(), style);
    
    // Try to get material data from the ground brush
    auto* materialManager = m_assetManager->getMaterialManager();
    auto* spriteManager = m_assetManager->getSpriteManager();
    
    if (materialManager && spriteManager) {
        // Try to get material ID from brush name or properties
        QString brushName = brush->getName();
        uint32_t materialId = 0;
        
        // Extract material ID from brush name if available
        QRegExp rx("Material\\s*(\\d+)");
        if (rx.indexIn(brushName) != -1) {
            materialId = rx.cap(1).toUInt();
        }
        
        if (materialId > 0) {
            const auto& allMaterials = materialManager->getAllMaterials();
            auto it = allMaterials.find(materialId);
            if (it != allMaterials.end()) {
                const MaterialData& material = it.value();
                
                // Draw 3x3 grid of ground tiles using material data
                int tileSize = qMax(8, size.width() / 4);
                int offsetX = (size.width() - tileSize * 3) / 2;
                int offsetY = (size.height() - tileSize * 3) / 2;
                
                for (int y = 0; y < 3; ++y) {
                    for (int x = 0; x < 3; ++x) {
                        if (!material.groundItems.isEmpty()) {
                            // Select ground item based on position for variation
                            int itemIndex = (x + y * 3) % material.groundItems.size();
                            uint16_t itemId = material.groundItems[itemIndex].itemId;
                            
                            const SpriteData* spriteData = spriteManager->getSpriteData(itemId);
                            if (spriteData && !spriteData->frames.isEmpty()) {
                                const QImage& spriteImage = spriteData->frames.first().image;
                                if (!spriteImage.isNull()) {
                                    QPixmap sprite = QPixmap::fromImage(spriteImage);
                                    QRect tileRect(offsetX + x * tileSize, offsetY + y * tileSize, tileSize, tileSize);
                                    QPixmap scaledSprite = sprite.scaled(tileSize, tileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                                    
                                    // Center sprite in tile
                                    int spriteX = tileRect.x() + (tileSize - scaledSprite.width()) / 2;
                                    int spriteY = tileRect.y() + (tileSize - scaledSprite.height()) / 2;
                                    painter.drawPixmap(spriteX, spriteY, scaledSprite);
                                }
                            }
                        }
                    }
                }
                
                // Add border items if available
                if (material.hasBorders && !material.borderSets.isEmpty()) {
                    drawMaterialBorders(painter, material.borderSets.first(), 
                                      QRect(offsetX, offsetY, tileSize * 3, tileSize * 3), 
                                      tileSize, spriteManager);
                }
                
                if (m_gridEnabled) {
                    drawGrid(painter, preview.rect());
                }
                
                drawBorder(painter, preview.rect(), style);
                return preview;
            }
        }
    }
    
    // Fallback to generic ground pattern
    QColor groundColor(76, 175, 80); // Green
    painter.setBrush(groundColor);
    painter.setPen(QPen(groundColor.darker(120), 1));
    
    // Draw tiled pattern
    int tileSize = qMax(4, size.width() / 8);
    for (int x = 0; x < size.width(); x += tileSize) {
        for (int y = 0; y < size.height(); y += tileSize) {
            QRect tileRect(x, y, tileSize, tileSize);
            painter.drawRect(tileRect);
            
            // Add texture
            painter.setPen(QPen(groundColor.darker(140), 1));
            painter.drawLine(tileRect.topLeft(), tileRect.bottomRight());
        }
    }
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateWallBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    if (!brush || !m_assetManager) {
        return generateDefaultBrushPreview(brush, size, style);
    }
    
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::SmoothPixmapTransformation);
    
    drawBackground(painter, preview.rect(), style);
    
    // Try to get material data from the wall brush
    auto* materialManager = m_assetManager->getMaterialManager();
    auto* spriteManager = m_assetManager->getSpriteManager();
    
    if (materialManager && spriteManager) {
        // Try to get material ID from brush name or properties
        QString brushName = brush->getName();
        uint32_t materialId = 0;
        
        // Extract material ID from brush name if available
        QRegExp rx("Material\\s*(\\d+)");
        if (rx.indexIn(brushName) != -1) {
            materialId = rx.cap(1).toUInt();
        }
        
        if (materialId > 0) {
            const auto& allMaterials = materialManager->getAllMaterials();
            auto it = allMaterials.find(materialId);
            if (it != allMaterials.end()) {
                const MaterialData& material = it.value();
                
                if (material.hasWalls && !material.wallSets.isEmpty()) {
                    const WallSetData& wallSet = material.wallSets.first();
                    
                    // Draw wall pattern using actual wall sprites
                    int tileSize = qMax(8, size.width() / 4);
                    int offsetX = (size.width() - tileSize * 3) / 2;
                    int offsetY = (size.height() - tileSize * 3) / 2;
                    
                    // Draw a 3x3 wall pattern
                    for (int y = 0; y < 3; ++y) {
                        for (int x = 0; x < 3; ++x) {
                            // Determine wall type based on position
                            WallType wallType = WallType::Vertical; // Default
                            if (x == 1 && y == 1) wallType = WallType::Cross; // Center
                            else if (x == 0 && y == 0) wallType = WallType::TopLeft;
                            else if (x == 2 && y == 0) wallType = WallType::TopRight;
                            else if (x == 0 && y == 2) wallType = WallType::BottomLeft;
                            else if (x == 2 && y == 2) wallType = WallType::BottomRight;
                            else if (y == 0) wallType = WallType::Horizontal;
                            else if (y == 2) wallType = WallType::Horizontal;
                            else if (x == 0 || x == 2) wallType = WallType::Vertical;
                            
                            // Find wall piece for this type
                            uint16_t wallItemId = 0;
                            for (const auto& wallPart : wallSet.parts) {
                                if (wallPart.type == wallType && !wallPart.items.isEmpty()) {
                                    wallItemId = wallPart.items.first().itemId;
                                    break;
                                }
                            }
                            
                            if (wallItemId > 0) {
                                const SpriteData* spriteData = spriteManager->getSpriteData(wallItemId);
                                if (spriteData && !spriteData->frames.isEmpty()) {
                                    const QImage& spriteImage = spriteData->frames.first().image;
                                    if (!spriteImage.isNull()) {
                                        QPixmap sprite = QPixmap::fromImage(spriteImage);
                                        QRect tileRect(offsetX + x * tileSize, offsetY + y * tileSize, tileSize, tileSize);
                                        QPixmap scaledSprite = sprite.scaled(tileSize, tileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                                        
                                        // Center sprite in tile
                                        int spriteX = tileRect.x() + (tileSize - scaledSprite.width()) / 2;
                                        int spriteY = tileRect.y() + (tileSize - scaledSprite.height()) / 2;
                                        painter.drawPixmap(spriteX, spriteY, scaledSprite);
                                    }
                                }
                            }
                        }
                    }
                    
                    if (m_gridEnabled) {
                        drawGrid(painter, preview.rect());
                    }
                    
                    drawBorder(painter, preview.rect(), style);
                    return preview;
                }
            }
        }
    }
    
    // Fallback to generic wall pattern
    QColor wallColor(158, 158, 158); // Gray
    painter.setBrush(wallColor);
    painter.setPen(QPen(wallColor.darker(120), 1));
    
    // Draw brick pattern
    int brickHeight = qMax(3, size.height() / 6);
    int brickWidth = qMax(6, size.width() / 4);
    
    for (int y = 0; y < size.height(); y += brickHeight) {
        bool offset = (y / brickHeight) % 2 == 1;
        int startX = offset ? -brickWidth / 2 : 0;
        
        for (int x = startX; x < size.width(); x += brickWidth) {
            QRect brickRect(x, y, brickWidth, brickHeight);
            if (brickRect.intersects(preview.rect())) {
                painter.drawRect(brickRect.intersected(preview.rect()));
            }
        }
    }
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateCreatureBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw creature representation
    QColor creatureColor(33, 150, 243); // Blue
    painter.setBrush(creatureColor);
    painter.setPen(QPen(creatureColor.darker(120), 2));
    
    // Draw creature as circle with features
    QRect creatureRect = preview.rect().adjusted(size.width() / 6, size.height() / 6, 
                                                -size.width() / 6, -size.height() / 6);
    painter.drawEllipse(creatureRect);
    
    // Add simple features (eyes)
    painter.setBrush(Qt::white);
    painter.setPen(Qt::black);
    int eyeSize = qMax(2, size.width() / 12);
    QPoint leftEye(creatureRect.center().x() - eyeSize, creatureRect.center().y() - eyeSize / 2);
    QPoint rightEye(creatureRect.center().x() + eyeSize, creatureRect.center().y() - eyeSize / 2);
    painter.drawEllipse(leftEye, eyeSize / 2, eyeSize / 2);
    painter.drawEllipse(rightEye, eyeSize / 2, eyeSize / 2);
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateDefaultBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw generic brush icon
    QColor brushColor(158, 158, 158);
    painter.setBrush(brushColor);
    painter.setPen(QPen(brushColor.darker(120), 2));
    
    // Draw diamond shape
    QPolygon diamond;
    QRect drawRect = preview.rect().adjusted(4, 4, -4, -4);
    diamond << QPoint(drawRect.center().x(), drawRect.top())
            << QPoint(drawRect.right(), drawRect.center().y())
            << QPoint(drawRect.center().x(), drawRect.bottom())
            << QPoint(drawRect.left(), drawRect.center().y());
    painter.drawPolygon(diamond);
    
    // Add question mark
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(8, size.width() / 4), QFont::Bold));
    painter.drawText(preview.rect(), Qt::AlignCenter, "?");
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

// Placeholder implementations for other brush types
QPixmap BrushPreviewGenerator::generateCarpetBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw carpet pattern
    QColor carpetColor(156, 39, 176); // Purple
    painter.setBrush(carpetColor);
    painter.setPen(QPen(carpetColor.darker(120), 1));
    
    // Draw carpet tiles in a pattern
    int tileSize = qMax(3, size.width() / 6);
    for (int x = 0; x < size.width(); x += tileSize) {
        for (int y = 0; y < size.height(); y += tileSize) {
            QRect tileRect(x, y, tileSize, tileSize);
            painter.drawRect(tileRect);
            
            // Add carpet pattern (diamond in center)
            painter.setPen(QPen(carpetColor.lighter(150), 1));
            QPoint center = tileRect.center();
            int diamondSize = tileSize / 3;
            QPolygon diamond;
            diamond << QPoint(center.x(), center.y() - diamondSize)
                    << QPoint(center.x() + diamondSize, center.y())
                    << QPoint(center.x(), center.y() + diamondSize)
                    << QPoint(center.x() - diamondSize, center.y());
            painter.drawPolygon(diamond);
        }
    }
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateTableBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw table pattern
    QColor tableColor(121, 85, 72); // Brown
    painter.setBrush(tableColor);
    painter.setPen(QPen(tableColor.darker(120), 1));
    
    // Draw table surface
    QRect tableRect = preview.rect().adjusted(4, 4, -4, -4);
    painter.drawRect(tableRect);
    
    // Add wood grain pattern
    painter.setPen(QPen(tableColor.darker(140), 1));
    for (int i = 1; i < 4; ++i) {
        int y = tableRect.top() + (tableRect.height() * i) / 4;
        painter.drawLine(tableRect.left(), y, tableRect.right(), y);
    }
    
    // Draw table legs (simplified)
    painter.setBrush(tableColor.darker(130));
    int legSize = qMax(2, size.width() / 12);
    QRect topLeftLeg(tableRect.left(), tableRect.top(), legSize, legSize);
    QRect topRightLeg(tableRect.right() - legSize, tableRect.top(), legSize, legSize);
    QRect bottomLeftLeg(tableRect.left(), tableRect.bottom() - legSize, legSize, legSize);
    QRect bottomRightLeg(tableRect.right() - legSize, tableRect.bottom() - legSize, legSize, legSize);
    
    painter.drawRect(topLeftLeg);
    painter.drawRect(topRightLeg);
    painter.drawRect(bottomLeftLeg);
    painter.drawRect(bottomRightLeg);
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateDoodadBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw doodad items (decorative objects)
    QColor doodadColor(139, 195, 74); // Light green
    painter.setBrush(doodadColor);
    painter.setPen(QPen(doodadColor.darker(120), 1));
    
    // Draw multiple small decorative items scattered around
    int itemSize = qMax(3, size.width() / 8);
    
    // Item 1: Small circle (flower/decoration)
    QRect item1(size.width() / 4, size.height() / 4, itemSize, itemSize);
    painter.drawEllipse(item1);
    
    // Item 2: Small square (box/decoration)
    painter.setBrush(doodadColor.darker(110));
    QRect item2(size.width() * 3 / 4 - itemSize, size.height() / 4, itemSize, itemSize);
    painter.drawRect(item2);
    
    // Item 3: Small triangle (decoration)
    painter.setBrush(doodadColor.lighter(110));
    QPolygon triangle;
    QPoint triCenter(size.width() / 2, size.height() * 3 / 4);
    triangle << QPoint(triCenter.x(), triCenter.y() - itemSize / 2)
             << QPoint(triCenter.x() - itemSize / 2, triCenter.y() + itemSize / 2)
             << QPoint(triCenter.x() + itemSize / 2, triCenter.y() + itemSize / 2);
    painter.drawPolygon(triangle);
    
    // Add "D" for Doodad
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(6, size.width() / 8), QFont::Bold));
    painter.drawText(preview.rect(), Qt::AlignCenter, "D");
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateRawBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    if (!brush || !m_assetManager) {
        return generateDefaultBrushPreview(brush, size, style);
    }
    
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::SmoothPixmapTransformation);
    
    drawBackground(painter, preview.rect(), style);
    
    // Try to get the raw brush's item ID
    // This would need to be implemented in the RawBrush class
    uint32_t itemId = 0;
    
    // For now, try to extract item ID from brush name or properties
    QString brushName = brush->getName();
    if (brushName.contains("Raw") && brushName.contains("ID:")) {
        // Extract ID from name like "Raw Brush (ID: 123)"
        QRegExp rx("ID:\\s*(\\d+)");
        if (rx.indexIn(brushName) != -1) {
            itemId = rx.cap(1).toUInt();
        }
    }
    
    if (itemId > 0) {
        // Get sprite from AssetManager
        auto* spriteManager = m_assetManager->getSpriteManager();
        if (spriteManager) {
            const SpriteData* spriteData = spriteManager->getSpriteData(itemId);
            if (spriteData && !spriteData->frames.isEmpty()) {
                const QImage& spriteImage = spriteData->frames.first().image;
                if (!spriteImage.isNull()) {
                    // Scale sprite to fit preview
                    QPixmap sprite = QPixmap::fromImage(spriteImage);
                    QPixmap scaledSprite = sprite.scaled(size * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    
                    // Center the sprite in the preview
                    int x = (size.width() - scaledSprite.width()) / 2;
                    int y = (size.height() - scaledSprite.height()) / 2;
                    painter.drawPixmap(x, y, scaledSprite);
                    
                    if (m_gridEnabled) {
                        drawGrid(painter, preview.rect());
                    }
                    
                    drawBorder(painter, preview.rect(), style);
                    return preview;
                }
            }
        }
    }
    
    // Fallback to generic raw brush preview
    QColor rawColor(255, 193, 7); // Amber
    painter.setBrush(rawColor);
    painter.setPen(QPen(rawColor.darker(120), 2));
    
    // Draw item box
    QRect itemRect = preview.rect().adjusted(size.width() / 6, size.height() / 6, 
                                           -size.width() / 6, -size.height() / 6);
    painter.drawRect(itemRect);
    
    // Add "R" for Raw
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(8, size.width() / 3), QFont::Bold));
    painter.drawText(preview.rect(), Qt::AlignCenter, "R");
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateSpawnBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw spawn area circle
    QRect circleRect = preview.rect().adjusted(4, 4, -4, -4);
    QColor spawnColor(255, 100, 100); // Red
    painter.setPen(QPen(spawnColor, 2));
    painter.setBrush(QBrush(spawnColor.lighter(180)));
    painter.drawEllipse(circleRect);
    
    // Draw spawn icon in center
    QRect iconRect = QRect(circleRect.center() - QPoint(8, 8), QSize(16, 16));
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(spawnColor);
    painter.drawEllipse(iconRect);
    
    // Add "S" for Spawn
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(6, size.width() / 6), QFont::Bold));
    painter.drawText(iconRect, Qt::AlignCenter, "S");
    
    // Draw spawn radius indicators
    painter.setPen(QPen(spawnColor.darker(120), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    int radiusStep = qMax(4, size.width() / 8);
    for (int r = radiusStep; r < circleRect.width() / 2; r += radiusStep) {
        QRect radiusRect = QRect(circleRect.center() - QPoint(r, r), QSize(r * 2, r * 2));
        painter.drawEllipse(radiusRect);
    }
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateWaypointBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw waypoint flag
    QRect flagRect = preview.rect().adjusted(8, 4, -8, -4);
    QColor waypointColor(255, 152, 0); // Orange
    
    // Flag pole
    painter.setPen(QPen(QColor(139, 69, 19), 3)); // Brown
    int poleX = flagRect.left() + 4;
    painter.drawLine(poleX, flagRect.top(), poleX, flagRect.bottom());
    
    // Flag
    QPolygon flag;
    flag << QPoint(poleX, flagRect.top())
         << QPoint(flagRect.right() - 4, flagRect.top() + 8)
         << QPoint(poleX, flagRect.top() + 16);
    
    painter.setPen(QPen(waypointColor.darker(120), 1));
    painter.setBrush(QBrush(waypointColor));
    painter.drawPolygon(flag);
    
    // Add "W" for Waypoint
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(6, size.width() / 8), QFont::Bold));
    QRect textRect = flag.boundingRect();
    painter.drawText(textRect, Qt::AlignCenter, "W");
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateHouseBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw house outline
    QRect houseRect = preview.rect().adjusted(4, 4, -4, -4);
    QColor houseColor(100, 100, 255); // Blue
    painter.setPen(QPen(houseColor, 2));
    painter.setBrush(QBrush(houseColor.lighter(180)));
    painter.drawRect(houseRect);
    
    // Draw house icon (simple house shape)
    painter.setPen(QPen(houseColor.darker(120), 2));
    painter.setBrush(QBrush(houseColor));
    
    // House roof
    QPolygon roof;
    roof << QPoint(houseRect.center().x(), houseRect.top() + 4)
         << QPoint(houseRect.left() + 4, houseRect.center().y())
         << QPoint(houseRect.right() - 4, houseRect.center().y());
    painter.drawPolygon(roof);
    
    // House door
    QRect doorRect(houseRect.center().x() - 3, houseRect.center().y() + 2, 6, 8);
    painter.setBrush(QBrush(houseColor.darker(150)));
    painter.drawRect(doorRect);
    
    // Add "H" for House
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", qMax(6, size.width() / 8), QFont::Bold));
    painter.drawText(houseRect, Qt::AlignCenter, "H");
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

QPixmap BrushPreviewGenerator::generateEraserBrushPreview(RME::core::Brush* brush, const QSize& size, PreviewStyle style)
{
    QPixmap preview(size);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter, preview.rect(), style);
    
    // Draw eraser circle
    QRect eraserRect = preview.rect().adjusted(4, 4, -4, -4);
    QColor eraserColor(255, 100, 100); // Red
    painter.setPen(QPen(eraserColor, 2));
    painter.setBrush(QBrush(eraserColor.lighter(160)));
    painter.drawEllipse(eraserRect);
    
    // Draw X mark
    painter.setPen(QPen(Qt::white, 3));
    int margin = eraserRect.width() / 4;
    QRect xRect = eraserRect.adjusted(margin, margin, -margin, -margin);
    painter.drawLine(xRect.topLeft(), xRect.bottomRight());
    painter.drawLine(xRect.topRight(), xRect.bottomLeft());
    
    // Add eraser texture lines
    painter.setPen(QPen(eraserColor.darker(120), 1));
    for (int i = 1; i < 4; ++i) {
        int y = eraserRect.top() + (eraserRect.height() * i) / 4;
        painter.drawLine(eraserRect.left() + 2, y, eraserRect.right() - 2, y);
    }
    
    if (m_gridEnabled) {
        drawGrid(painter, preview.rect());
    }
    
    drawBorder(painter, preview.rect(), style);
    
    return preview;
}

// Helper methods

QString BrushPreviewGenerator::generateCacheKey(RME::core::Brush* brush, const QSize& size, PreviewStyle style) const
{
    return QString("%1_%2x%3_%4_%5_%6")
        .arg(reinterpret_cast<quintptr>(brush))
        .arg(size.width())
        .arg(size.height())
        .arg(static_cast<int>(style))
        .arg(m_backgroundColor.name())
        .arg(m_gridEnabled ? 1 : 0);
}

void BrushPreviewGenerator::drawBackground(QPainter& painter, const QRect& rect, PreviewStyle style)
{
    painter.fillRect(rect, m_backgroundColor);
}

void BrushPreviewGenerator::drawGrid(QPainter& painter, const QRect& rect, int gridSize)
{
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    
    for (int x = 0; x < rect.width(); x += gridSize) {
        painter.drawLine(x, 0, x, rect.height());
    }
    
    for (int y = 0; y < rect.height(); y += gridSize) {
        painter.drawLine(0, y, rect.width(), y);
    }
}

void BrushPreviewGenerator::drawBorder(QPainter& painter, const QRect& rect, PreviewStyle style)
{
    QColor borderColor;
    int borderWidth = 1;
    
    switch (style) {
        case IconStyle:
            borderColor = QColor(180, 180, 180);
            borderWidth = 1;
            break;
        case ThumbnailStyle:
            borderColor = QColor(160, 160, 160);
            borderWidth = 1;
            break;
        case DetailStyle:
            borderColor = QColor(140, 140, 140);
            borderWidth = 2;
            break;
    }
    
    painter.setPen(QPen(borderColor, borderWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect.adjusted(0, 0, -1, -1));
}

void BrushPreviewGenerator::drawMaterialBorders(QPainter& painter, const BorderSetData& borderSet, const QRect& area, int tileSize, SpriteManager* spriteManager)
{
    if (!spriteManager || borderSet.borders.isEmpty()) {
        return;
    }
    
    // Draw corner and edge borders around the area
    for (const auto& border : borderSet.borders) {
        const SpriteData* spriteData = spriteManager->getSpriteData(border.itemId);
        if (!spriteData || spriteData->frames.isEmpty()) {
            continue;
        }
        
        const QImage& spriteImage = spriteData->frames.first().image;
        if (spriteImage.isNull()) {
            continue;
        }
        
        QPixmap borderSprite = QPixmap::fromImage(spriteImage);
        QPixmap scaledBorder = borderSprite.scaled(tileSize, tileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        QRect borderRect;
        switch (border.position) {
            case BorderPosition::TopLeft:
                borderRect = QRect(area.left(), area.top(), tileSize, tileSize);
                break;
            case BorderPosition::Top:
                borderRect = QRect(area.left() + tileSize, area.top(), tileSize, tileSize);
                break;
            case BorderPosition::TopRight:
                borderRect = QRect(area.right() - tileSize, area.top(), tileSize, tileSize);
                break;
            case BorderPosition::Right:
                borderRect = QRect(area.right() - tileSize, area.top() + tileSize, tileSize, tileSize);
                break;
            case BorderPosition::BottomRight:
                borderRect = QRect(area.right() - tileSize, area.bottom() - tileSize, tileSize, tileSize);
                break;
            case BorderPosition::Bottom:
                borderRect = QRect(area.left() + tileSize, area.bottom() - tileSize, tileSize, tileSize);
                break;
            case BorderPosition::BottomLeft:
                borderRect = QRect(area.left(), area.bottom() - tileSize, tileSize, tileSize);
                break;
            case BorderPosition::Left:
                borderRect = QRect(area.left(), area.top() + tileSize, tileSize, tileSize);
                break;
            default:
                continue;
        }
        
        if (borderRect.isValid()) {
            // Center border sprite in rect
            int x = borderRect.x() + (tileSize - scaledBorder.width()) / 2;
            int y = borderRect.y() + (tileSize - scaledBorder.height()) / 2;
            painter.drawPixmap(x, y, scaledBorder);
        }
    }
}

} // namespace palettes
} // namespace ui
} // namespace RME