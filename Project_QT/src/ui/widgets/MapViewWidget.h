#ifndef RME_MAP_VIEW_WIDGET_H
#define RME_MAP_VIEW_WIDGET_H

#include <QWidget>
#include <QPoint>

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IEditorStateService.h"
#include "core/services/IClientDataService.h"
#include "core/services/IApplicationSettingsService.h"

// Forward declarations
class QMenu;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QContextMenuEvent;

namespace RME {
namespace core {
    class Map;
    class Position;
    namespace settings { class AppSettings; }
}
namespace editor_logic {
    class EditorController;
}
namespace ui {
namespace widgets {
    class MapView;
}
}
}

// Forward declarations removed - using service interfaces now

namespace RME {
namespace ui {
namespace widgets {

/**
 * @brief Interactive Map View Widget
 * 
 * This widget hosts the OpenGL rendering canvas and handles all user input
 * for interacting with the map: panning, zooming, drawing, and selection.
 * It serves as the primary user interface for map editing operations.
 */
class MapViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapViewWidget(
        RME::core::IBrushStateService* brushStateService,
        RME::core::IEditorStateService* editorStateService,
        RME::core::IClientDataService* clientDataService,
        RME::core::IApplicationSettingsService* settingsService,
        QWidget* parent = nullptr
    );
    ~MapViewWidget() override = default;

    // Core component access
    MapView* getMapView() const { return m_mapView; }
    
    // Integration methods
    void setMap(RME::core::Map* map);
    void setEditorController(RME::editor_logic::EditorController* controller);
    void setAppSettings(RME::core::settings::AppSettings* settings);

    // View state access
    int getCurrentFloor() const;
    void setCurrentFloor(int floor);
    float getZoomLevel() const;
    void setZoomLevel(float zoom);
    RME::core::Position getCurrentPosition() const;
    void centerOnPosition(const RME::core::Position& position);

    // Coordinate conversion
    RME::core::Position screenToMapCoords(const QPoint& screenPos);

public slots:
    // Map interaction slots
    void onMapPositionClicked(const RME::core::Position& mapPos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void onViewChanged();
    
    // Context menu actions
    void onCutSelection();
    void onCopySelection();
    void onPasteSelection();
    void onDeleteSelection();
    void onTileProperties();
    void onItemProperties();

signals:
    void floorChanged(int floor);
    void zoomChanged(float zoom);
    void positionChanged(const RME::core::Position& position);

protected:
    // Event handlers for user interaction
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setupUI();
    void createContextMenu();
    void connectServices();
    void updateViewSettings();

    // Core components
    MapView* m_mapView;
    RME::editor_logic::EditorController* m_editorController;
    
    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IEditorStateService* m_editorStateService;
    RME::core::IClientDataService* m_clientDataService;
    RME::core::IApplicationSettingsService* m_settingsService;

    // Interaction state
    bool m_isPanning;
    bool m_isSelecting;
    bool m_isDrawing;
    QPoint m_lastMousePos;
    RME::core::Position m_selectionStartMapPos;

    // UI components
    QMenu* m_contextMenu;

} // namespace widgets
} // namespace ui
} // namespace RME

#endif // RME_MAP_VIEW_WIDGET_H