#ifndef RME_MINIMAP_VIEW_WIDGET_H
#define RME_MINIMAP_VIEW_WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QRectF>
#include <QColor>

// Forward declarations
class QMouseEvent;
class QWheelEvent;
class QResizeEvent;
class QPaintEvent;

namespace RME {
namespace core {
    class Map;
    class Position;
    class Tile;
    namespace assets {
        class ItemDatabase;
    }
}
namespace ui {
namespace widgets {

// Forward declaration for placeholder service (until REFACTOR-01)
class EditorStateService;

/**
 * @brief Minimap View Widget
 * 
 * This widget displays a small, zoomed-out overview of the current map floor
 * and allows navigation by clicking or dragging on the minimap.
 */
class MinimapViewWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct a new Minimap View Widget
     * 
     * @param editorState Service to get current map and floor
     * @param itemDatabase Database to get item colors
     * @param parent Parent widget
     */
    MinimapViewWidget(EditorStateService* editorState, 
                     RME::core::assets::ItemDatabase* itemDatabase, 
                     QWidget* parent = nullptr);
    
    /**
     * @brief Set the main map view's visible rectangle
     * 
     * @param viewRect Rectangle in map coordinates
     */
    void setMainMapViewRect(const QRectF& viewRect);

public slots:
    /**
     * @brief Handle map change
     * 
     * @param currentMap Pointer to the new map
     */
    void onMapChanged(RME::core::Map* currentMap);
    
    /**
     * @brief Handle floor change
     * 
     * @param newFloor New floor index
     */
    void onCurrentFloorChanged(int newFloor);
    
    /**
     * @brief Handle main view change
     * 
     * @param center New center position
     * @param zoom New zoom level
     */
    void onMainViewChanged(const RME::core::Position& center, double zoom);

signals:
    /**
     * @brief Signal emitted when user requests navigation to a position
     * 
     * @param mapCenter Position to navigate to
     */
    void navigationRequested(const RME::core::Position& mapCenter);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief Render the minimap to the pixmap buffer
     */
    void renderMinimap();
    
    /**
     * @brief Convert widget coordinates to map coordinates
     * 
     * @param widgetPos Position in widget coordinates
     * @return RME::core::Position Position in map coordinates
     */
    RME::core::Position widgetToMapCoords(const QPoint& widgetPos);
    
    /**
     * @brief Get color for a tile on the minimap
     * 
     * @param tile Tile to get color for
     * @return QColor Color to use for the tile
     */
    QColor getTileMinimapColor(const RME::core::Tile* tile) const;
    
    /**
     * @brief Get color for an item on the minimap
     * 
     * @param itemId Item ID
     * @return QColor Color to use for the item
     */
    QColor getItemMinimapColor(uint16_t itemId) const;

    // Services
    EditorStateService* m_editorStateService;
    RME::core::assets::ItemDatabase* m_itemDatabase;
    
    // Map data
    RME::core::Map* m_currentMap = nullptr;
    int m_currentFloor = 7; // Default floor
    QRectF m_mainMapViewRect; // Viewport of main map in map coordinates
    
    // Rendering
    QPixmap m_minimapPixmap; // Offscreen buffer for the minimap rendering
    bool m_needsFullRedraw = true;
    
    // Interaction state
    QPoint m_dragStartPos;
    bool m_isDragging = false;
    
    // Constants
    const QColor m_backgroundColor = QColor(0, 0, 0); // Black background
    const QColor m_viewportRectColor = QColor(255, 255, 255, 128); // Semi-transparent white
    const QColor m_defaultTileColor = QColor(128, 128, 128); // Gray for unknown tiles
};

} // namespace widgets
} // namespace ui
} // namespace RME

#endif // RME_MINIMAP_VIEW_WIDGET_H