#include "MapViewWidget.h"
#include "MapView.h"
#include "../../editor_logic/EditorController.h"
#include "../../core/settings/AppSettings.h"
#include "../../core/Position.h"
#include "../../core/brush/BrushSettings.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

namespace RME {
namespace ui {
namespace widgets {

// Placeholder service classes removed - using real services now

MapViewWidget::MapViewWidget(
    RME::core::IBrushStateService* brushStateService,
    RME::core::IEditorStateService* editorStateService,
    RME::core::IClientDataService* clientDataService,
    RME::core::IApplicationSettingsService* settingsService,
    QWidget* parent
) : QWidget(parent)
    , m_mapView(nullptr)
    , m_editorController(nullptr)
    , m_brushStateService(brushStateService)
    , m_editorStateService(editorStateService)
    , m_clientDataService(clientDataService)
    , m_settingsService(settingsService)
    , m_isPanning(false)
    , m_isSelecting(false)
    , m_isDrawing(false)
    , m_contextMenu(nullptr)
{
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_editorStateService);
    Q_ASSERT(m_clientDataService);
    Q_ASSERT(m_settingsService);
    
    setupUI();
    createContextMenu();
    connectServices();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void MapViewWidget::setupUI()
{
    // Create main layout
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Create and add the MapView
    m_mapView = new MapView(this);
    layout->addWidget(m_mapView);
    
    // Connect MapView signals
    connect(m_mapView, &MapView::mapPositionClicked, 
            this, &MapViewWidget::onMapPositionClicked);
    connect(m_mapView, &MapView::viewChanged,
            this, &MapViewWidget::onViewChanged);
}

void MapViewWidget::createContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    // Create actions
    auto* cutAction = m_contextMenu->addAction("Cut");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &MapViewWidget::onCutSelection);
    
    auto* copyAction = m_contextMenu->addAction("Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MapViewWidget::onCopySelection);
    
    auto* pasteAction = m_contextMenu->addAction("Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MapViewWidget::onPasteSelection);
    
    m_contextMenu->addSeparator();
    
    auto* deleteAction = m_contextMenu->addAction("Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MapViewWidget::onDeleteSelection);
    
    m_contextMenu->addSeparator();
    
    auto* propertiesAction = m_contextMenu->addAction("Tile Properties...");
    connect(propertiesAction, &QAction::triggered, this, &MapViewWidget::onTileProperties);
    
    auto* itemPropertiesAction = m_contextMenu->addAction("Item Properties...");
    connect(itemPropertiesAction, &QAction::triggered, this, &MapViewWidget::onItemProperties);
}

void MapViewWidget::connectServices()
{
    // Connect to brush state changes
    connect(m_brushStateService, &RME::core::IBrushStateService::brushSizeChanged,
            this, [this](int size) {
                qDebug() << "MapViewWidget: Brush size changed to" << size;
                // Update UI if needed
            });
    
    connect(m_brushStateService, &RME::core::IBrushStateService::brushShapeChanged,
            this, [this](BrushShape shape) {
                qDebug() << "MapViewWidget: Brush shape changed to" << static_cast<int>(shape);
                // Update UI if needed
            });
    
    // Connect to editor state changes
    connect(m_editorStateService, &RME::core::IEditorStateService::currentFloorChanged,
            this, [this](int floor) {
                qDebug() << "MapViewWidget: Floor changed to" << floor;
                if (m_mapView) {
                    m_mapView->setCurrentFloor(floor);
                }
                emit floorChanged(floor);
            });
    
    connect(m_editorStateService, &RME::core::IEditorStateService::zoomLevelChanged,
            this, [this](float zoom) {
                qDebug() << "MapViewWidget: Zoom changed to" << zoom;
                if (m_mapView) {
                    m_mapView->setZoomLevel(zoom);
                }
                emit zoomChanged(zoom);
            });
    
    // Connect to settings changes
    connect(m_settingsService, &RME::core::IApplicationSettingsService::viewSettingsChanged,
            this, [this]() {
                qDebug() << "MapViewWidget: View settings changed";
                // Update view based on new settings
                updateViewSettings();
            });
}

void MapViewWidget::updateViewSettings()
{
    if (!m_mapView || !m_settingsService) {
        return;
    }
    
    // Update view based on current settings
    // This would update grid visibility, creature visibility, etc.
    qDebug() << "MapViewWidget: Updating view settings";
}

void MapViewWidget::setEditorController(RME::editor_logic::EditorController* controller)
{
    m_editorController = controller;
    if (m_mapView) {
        m_mapView->setEditorController(controller);
    }
}

void MapViewWidget::setAppSettings(RME::core::settings::AppSettings* settings)
{
    m_appSettings = settings;
    if (m_mapView) {
        m_mapView->setAppSettings(settings);
    }
}

void MapViewWidget::setMap(RME::core::Map* map)
{
    if (m_mapView) {
        m_mapView->setMap(map);
    }
}

RME::core::Position MapViewWidget::screenToMapCoords(const QPoint& screenPos)
{
    if (m_mapView) {
        return m_mapView->screenToMapCoords(screenPos);
    }
    return RME::core::Position(0, 0, 0);
}

void MapViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_editorController || !m_editorStateService || !m_brushStateService) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    m_lastMousePos = event->pos();
    RME::core::Position mapPos = screenToMapCoords(event->pos());
    
    auto currentMode = m_editorStateService->getCurrentMode();
    
    if (event->button() == Qt::MiddleButton || 
        (event->button() == Qt::LeftButton && QApplication::keyboardModifiers() & Qt::AltModifier)) {
        // Start panning
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->button() == Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            // Start selection
            m_isSelecting = true;
            m_selectionStartMapPos = mapPos;
        } else if (currentMode == EditorStateService::Mode::Brush) {
            // Start drawing
            m_isDrawing = true;
            auto brushSettings = m_brushStateService->getCurrentBrushSettings();
            m_editorController->handleMapClick(mapPos, event->button(), 
                                             event->modifiers(), brushSettings);
        }
    }
    
    QWidget::mousePressEvent(event);
}

void MapViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_editorController || !m_editorStateService || !m_brushStateService) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    RME::core::Position mapPos = screenToMapCoords(event->pos());
    
    if (m_isPanning) {
        // Handle panning
        QPoint delta = event->pos() - m_lastMousePos;
        if (m_mapView) {
            // Convert screen delta to map coordinate delta and pan
            auto currentCenter = m_mapView->getViewCenterMapCoords();
            float zoomFactor = m_mapView->getZoomFactor();
            
            // Invert delta for natural panning feel
            float mapDeltaX = -delta.x() / (32.0f * zoomFactor);
            float mapDeltaY = -delta.y() / (32.0f * zoomFactor);
            
            QPointF newCenter(currentCenter.x() + mapDeltaX, currentCenter.y() + mapDeltaY);
            RME::core::Position newCenterPos(static_cast<int>(newCenter.x()), 
                                           static_cast<int>(newCenter.y()), 
                                           m_mapView->getCurrentFloor());
            m_mapView->setViewCenter(newCenterPos);
        }
        m_lastMousePos = event->pos();
    }
    else if (m_isDrawing && (event->buttons() & Qt::LeftButton)) {
        // Handle drawing
        auto brushSettings = m_brushStateService->getCurrentBrushSettings();
        QList<RME::core::Position> positions;
        positions.append(mapPos);
        m_editorController->handleMapDrag(positions, Qt::LeftButton, 
                                        event->modifiers(), brushSettings);
    }
    else if (m_isSelecting) {
        // Update selection marquee - this would be handled by the rendering system
        // For now, just store the current position
        update(); // Trigger repaint for selection rectangle
    }
    
    // Update status with current map coordinates
    emit positionChanged(mapPos);
    
    QWidget::mouseMoveEvent(event);
}

void MapViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_editorController || !m_editorStateService || !m_brushStateService) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    
    RME::core::Position mapPos = screenToMapCoords(event->pos());
    
    if (m_isPanning && event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
    else if (m_isDrawing && event->button() == Qt::LeftButton) {
        auto brushSettings = m_brushStateService->getCurrentBrushSettings();
        m_editorController->handleMapRelease(mapPos, event->button(), 
                                           event->modifiers(), brushSettings);
        m_isDrawing = false;
    }
    else if (m_isSelecting && event->button() == Qt::LeftButton) {
        // Finalize bounding box selection
        auto brushSettings = m_brushStateService->getCurrentBrushSettings();
        m_editorController->performBoundingBoxSelection(m_selectionStartMapPos, mapPos, 
                                                      event->modifiers(), brushSettings);
        m_isSelecting = false;
        update(); // Clear selection rectangle
    }
    
    QWidget::mouseReleaseEvent(event);
}

void MapViewWidget::wheelEvent(QWheelEvent* event)
{
    if (m_mapView) {
        // Forward wheel events to MapView for zoom handling
        QApplication::sendEvent(m_mapView, event);
    } else {
        QWidget::wheelEvent(event);
    }
}

void MapViewWidget::keyPressEvent(QKeyEvent* event)
{
    if (!m_editorStateService) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_PageUp:
            m_editorStateService->changeFloor(1);
            if (m_mapView) {
                m_mapView->setCurrentFloor(m_editorStateService->getCurrentFloor());
            }
            event->accept();
            break;
            
        case Qt::Key_PageDown:
            m_editorStateService->changeFloor(-1);
            if (m_mapView) {
                m_mapView->setCurrentFloor(m_editorStateService->getCurrentFloor());
            }
            event->accept();
            break;
            
        case Qt::Key_Space:
            // Toggle to panning mode while space is held
            if (!event->isAutoRepeat()) {
                m_editorStateService->setCurrentMode(EditorStateService::Mode::Panning);
                setCursor(Qt::OpenHandCursor);
            }
            event->accept();
            break;
            
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void MapViewWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (!m_editorStateService) {
        QWidget::keyReleaseEvent(event);
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_Space:
            // Return to brush mode when space is released
            if (!event->isAutoRepeat()) {
                m_editorStateService->setCurrentMode(EditorStateService::Mode::Brush);
                setCursor(Qt::ArrowCursor);
            }
            event->accept();
            break;
            
        default:
            QWidget::keyReleaseEvent(event);
            break;
    }
}

void MapViewWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_contextMenu) {
        QWidget::contextMenuEvent(event);
        return;
    }
    
    RME::core::Position mapPos = screenToMapCoords(event->pos());
    
    // Update context menu based on current context
    // This would typically check what's at the clicked position
    // and enable/disable actions accordingly
    
    m_contextMenu->popup(event->globalPos());
    event->accept();
}

// Slot implementations
void MapViewWidget::onMapPositionClicked(const RME::core::Position& mapPos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    // This is called from MapView - we can add additional logic here if needed
    emit positionChanged(mapPos);
}

void MapViewWidget::onViewChanged()
{
    // Called when the view changes (pan, zoom, floor change)
    if (m_mapView) {
        emit floorChanged(m_mapView->getCurrentFloor());
        emit zoomChanged(m_mapView->getZoomFactor());
    }
}

void MapViewWidget::onCutSelection()
{
    if (m_editorController) {
        m_editorController->cutSelection();
    }
}

void MapViewWidget::onCopySelection()
{
    if (m_editorController) {
        m_editorController->copySelection();
    }
}

void MapViewWidget::onPasteSelection()
{
    if (m_editorController && m_mapView) {
        // Paste at current view center
        auto center = m_mapView->getViewCenterMapCoords();
        RME::core::Position pastePos(static_cast<int>(center.x()), 
                                   static_cast<int>(center.y()), 
                                   m_mapView->getCurrentFloor());
        m_editorController->pasteAtPosition(pastePos);
    }
}

void MapViewWidget::onDeleteSelection()
{
    if (m_editorController) {
        m_editorController->deleteSelection();
    }
}

void MapViewWidget::onTileProperties()
{
    // This would open a tile properties dialog
    // Implementation depends on the dialog system
    qDebug() << "Tile Properties requested";
}

void MapViewWidget::onItemProperties()
{
    // This would open an item properties dialog
    // Implementation depends on the dialog system
    qDebug() << "Item Properties requested";
}

// Public interface methods
int MapViewWidget::getCurrentFloor() const
{
    return m_mapView ? m_mapView->getCurrentFloor() : 0;
}

void MapViewWidget::setCurrentFloor(int floor)
{
    if (m_mapView) {
        m_mapView->setCurrentFloor(floor);
    }
    if (m_editorStateService) {
        m_editorStateService->setCurrentFloor(floor);
    }
}

float MapViewWidget::getZoomLevel() const
{
    return m_mapView ? m_mapView->getZoomFactor() : 1.0f;
}

void MapViewWidget::setZoomLevel(float zoom)
{
    if (m_mapView) {
        m_mapView->setZoom(zoom);
    }
}

RME::core::Position MapViewWidget::getCurrentPosition() const
{
    if (m_mapView) {
        auto center = m_mapView->getViewCenterMapCoords();
        return RME::core::Position(static_cast<int>(center.x()), 
                                 static_cast<int>(center.y()), 
                                 m_mapView->getCurrentFloor());
    }
    return RME::core::Position(0, 0, 0);
}

void MapViewWidget::centerOnPosition(const RME::core::Position& position)
{
    if (m_mapView) {
        m_mapView->setViewCenter(position);
    }
}

} // namespace widgets
} // namespace ui
} // namespace RME

// #include "MapViewWidget.moc" // Removed - Q_OBJECT is in header