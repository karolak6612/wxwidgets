#include "MinimapViewWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>

namespace RME {
namespace ui {
namespace widgets {

MinimapViewWidget::MinimapViewWidget(RME::core::editor::EditorStateService* editorState, 
                                   RME::core::assets::ItemDatabase* itemDatabase, 
                                   QWidget* parent)
    : QWidget(parent)
    , m_editorStateService(editorState)
    , m_itemDatabase(itemDatabase)
    , m_currentMap(nullptr)
    , m_currentFloor(7)
    , m_needsFullRedraw(true)
    , m_isDragging(false)
{
    // Set widget attributes
    setMinimumSize(200, 200);
    setFocusPolicy(Qt::StrongFocus);
    
    // Enable mouse tracking for drag operations
    setMouseTracking(true);
    
    // Set background color
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_backgroundColor);
    setPalette(pal);
    
    // Initialize pixmap buffer
    m_minimapPixmap = QPixmap(size());
    m_minimapPixmap.fill(m_backgroundColor);
    
    // Connect to editor state signals
    if (m_editorStateService) {
        connect(m_editorStateService, &RME::core::editor::EditorStateService::mapChanged, 
                this, &MinimapViewWidget::onMapChanged);
        connect(m_editorStateService, &RME::core::editor::EditorStateService::currentFloorChanged, 
                this, &MinimapViewWidget::onCurrentFloorChanged);
        connect(m_editorStateService, &RME::core::editor::EditorStateService::viewRectChanged,
                this, &MinimapViewWidget::setMainMapViewRect);
        connect(m_editorStateService, &RME::core::editor::EditorStateService::viewChanged,
                this, &MinimapViewWidget::onMainViewChanged);
        
        // Initialize with current map if available
        m_currentMap = m_editorStateService->getCurrentMap();
        m_currentFloor = m_editorStateService->getCurrentFloor();
        
        // Initialize view rectangle
        m_mainMapViewRect = m_editorStateService->getViewRect();
    }
}

void MinimapViewWidget::setMainMapViewRect(const QRectF& viewRect)
{
    m_mainMapViewRect = viewRect;
    update(); // Request repaint but don't regenerate the entire minimap
}

void MinimapViewWidget::onMapChanged(RME::core::Map* currentMap)
{
    m_currentMap = currentMap;
    m_needsFullRedraw = true;
    update();
}

void MinimapViewWidget::onCurrentFloorChanged(int newFloor)
{
    m_currentFloor = newFloor;
    m_needsFullRedraw = true;
    update();
}

void MinimapViewWidget::onMainViewChanged(const RME::core::Position& center, double zoom)
{
    // Calculate the view rectangle based on the center position and zoom level
    // This is a placeholder implementation
    
    // Get the widget size
    int width = size().width();
    int height = size().height();
    
    // Calculate the visible area in map coordinates
    // The zoom factor determines how many tiles are visible
    double visibleTilesX = width / zoom;
    double visibleTilesY = height / zoom;
    
    // Calculate the top-left corner of the view rectangle
    double left = center.x() - visibleTilesX / 2;
    double top = center.y() - visibleTilesY / 2;
    
    // Create the view rectangle
    m_mainMapViewRect = QRectF(left, top, visibleTilesX, visibleTilesY);
    
    // Request a repaint but don't regenerate the entire minimap
    update();
}

void MinimapViewWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    
    // If we need a full redraw, regenerate the minimap
    if (m_needsFullRedraw) {
        renderMinimap();
    }
    
    // Draw the minimap pixmap
    painter.drawPixmap(0, 0, m_minimapPixmap);
    
    // Draw the viewport rectangle if valid
    if (!m_mainMapViewRect.isEmpty() && m_currentMap) {
        painter.setPen(QPen(m_viewportRectColor, 2));
        painter.setBrush(Qt::NoBrush);
        
        // Convert map coordinates to widget coordinates
        double left = (m_mainMapViewRect.left() - m_mapOffsetX) * m_mapToWidgetScale;
        double top = (m_mainMapViewRect.top() - m_mapOffsetY) * m_mapToWidgetScale;
        double width = m_mainMapViewRect.width() * m_mapToWidgetScale;
        double height = m_mainMapViewRect.height() * m_mapToWidgetScale;
        
        QRectF viewportRect(left, top, width, height);
        
        // Ensure the rectangle is within the widget bounds
        viewportRect = viewportRect.intersected(QRectF(0, 0, this->width(), this->height()));
        
        painter.drawRect(viewportRect);
    }
}

void MinimapViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        m_isDragging = true;
        
        // Request navigation to the clicked position
        RME::core::Position mapPos = widgetToMapCoords(event->pos());
        emit navigationRequested(mapPos);
    }
}

void MinimapViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        // Request navigation to the dragged position
        RME::core::Position mapPos = widgetToMapCoords(event->pos());
        emit navigationRequested(mapPos);
    }
}

void MinimapViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}

void MinimapViewWidget::resizeEvent(QResizeEvent* event)
{
    // Resize the pixmap buffer to match the new widget size
    m_minimapPixmap = QPixmap(event->size());
    m_minimapPixmap.fill(m_backgroundColor);
    
    // Request a full redraw with the new size
    m_needsFullRedraw = true;
    
    QWidget::resizeEvent(event);
}

void MinimapViewWidget::renderMinimap()
{
    // Clear the flag first to avoid re-entry
    m_needsFullRedraw = false;
    
    // Check if we have a valid map
    if (!m_currentMap) {
        // Just clear the pixmap if no map is available
        m_minimapPixmap.fill(m_backgroundColor);
        return;
    }
    
    // Get the widget size
    int width = m_minimapPixmap.width();
    int height = m_minimapPixmap.height();
    
    // Clear the pixmap with the background color
    m_minimapPixmap.fill(m_backgroundColor);
    
    // Create a painter for the pixmap
    QPainter painter(&m_minimapPixmap);
    
    // Get actual map dimensions from the map
    int mapWidth = m_currentMap->getWidth();
    int mapHeight = m_currentMap->getHeight();
    
    // Calculate scaling factor to fit the map in the widget
    double scaleX = static_cast<double>(width) / mapWidth;
    double scaleY = static_cast<double>(height) / mapHeight;
    double scale = std::min(scaleX, scaleY);
    
    // Store the scale factor for coordinate conversions
    m_mapToWidgetScale = scale;
    
    // Calculate the visible area of the map
    int visibleMapWidth = static_cast<int>(width / scale);
    int visibleMapHeight = static_cast<int>(height / scale);
    
    // Calculate the starting position (center the map in the widget)
    int startX = std::max(0, (mapWidth - visibleMapWidth) / 2);
    int startY = std::max(0, (mapHeight - visibleMapHeight) / 2);
    
    // Store the map offset for coordinate conversions
    m_mapOffsetX = startX;
    m_mapOffsetY = startY;
    
    // Iterate through the visible tiles and draw them
    for (int y = 0; y < visibleMapHeight && (startY + y) < mapHeight; ++y) {
        for (int x = 0; x < visibleMapWidth && (startX + x) < mapWidth; ++x) {
            int mapX = startX + x;
            int mapY = startY + y;
            
            // Get the tile at this position
            RME::core::Position tilePos(mapX, mapY, m_currentFloor);
            RME::core::Tile* tile = m_currentMap->getTile(tilePos);
            
            // Calculate the pixel position in the widget
            int pixelX = static_cast<int>(x * scale);
            int pixelY = static_cast<int>(y * scale);
            int pixelSize = std::max(1, static_cast<int>(scale));
            
            // Get the color for this tile
            QColor color;
            if (tile) {
                color = getTileMinimapColor(tile);
            } else {
                color = m_backgroundColor;
            }
            
            // Draw the tile as a rectangle
            painter.fillRect(pixelX, pixelY, pixelSize, pixelSize, color);
        }
    }
    
    // Draw a border around the minimap
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawRect(0, 0, width - 1, height - 1);
}

RME::core::Position MinimapViewWidget::widgetToMapCoords(const QPoint& widgetPos)
{
    // Check if we have a valid map
    if (!m_currentMap) {
        return RME::core::Position(0, 0, m_currentFloor);
    }
    
    // Use the stored scale and offset values from renderMinimap
    int mapX = static_cast<int>(widgetPos.x() / m_mapToWidgetScale) + m_mapOffsetX;
    int mapY = static_cast<int>(widgetPos.y() / m_mapToWidgetScale) + m_mapOffsetY;
    
    // Clamp to valid map coordinates
    mapX = std::max(0, std::min(mapX, m_currentMap->getWidth() - 1));
    mapY = std::max(0, std::min(mapY, m_currentMap->getHeight() - 1));
    
    return RME::core::Position(mapX, mapY, m_currentFloor);
}

QColor MinimapViewWidget::getTileMinimapColor(const RME::core::Tile* tile) const
{
    if (!tile) {
        return m_backgroundColor;
    }
    
    // Check for ground item first
    uint16_t groundId = tile->getGroundId();
    if (groundId != 0) {
        return getItemMinimapColor(groundId);
    }
    
    // If no ground, check for other items
    const auto& items = tile->getItems();
    if (!items.empty()) {
        // Try to find a suitable item to represent the tile
        for (const auto& item : items) {
            if (item) {
                uint16_t itemId = item->getID();
                // Skip items that shouldn't be visible on the minimap
                if (itemId != 0) {
                    return getItemMinimapColor(itemId);
                }
            }
        }
    }
    
    // Check if this is a special tile (spawn, waypoint, etc.)
    if (tile->hasSpawn()) {
        return QColor(255, 0, 0); // Red for spawns
    }
    
    if (tile->hasWaypoint()) {
        return QColor(0, 0, 255); // Blue for waypoints
    }
    
    return m_defaultTileColor;
}

QColor MinimapViewWidget::getItemMinimapColor(uint16_t itemId) const
{
    if (itemId == 0) {
        return m_backgroundColor;
    }
    
    // Use the item database to get the actual item color if available
    if (m_itemDatabase) {
        auto itemData = m_itemDatabase->getItemData(itemId);
        if (itemData) {
            // Use the minimap color if defined
            if (itemData->minimapColor.isValid()) {
                return itemData->minimapColor;
            }
            
            // Otherwise use the lookup color if defined
            if (itemData->lookupColor.isValid()) {
                return itemData->lookupColor;
            }
        }
    }
    
    // Fallback to a generated color based on the item ID
    int r = (itemId * 7) % 255;
    int g = (itemId * 11) % 255;
    int b = (itemId * 13) % 255;
    
    return QColor(r, g, b);
}

} // namespace widgets
} // namespace ui
} // namespace RME