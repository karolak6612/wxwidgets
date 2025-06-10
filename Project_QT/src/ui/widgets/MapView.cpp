#include "ui/widgets/MapView.h" // Use path relative to src/ or as per CMake include_directories

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor> // Added for setCursor/unsetCursor
#include <QtGlobal> // Added for qFuzzyCompare
#include <QDebug> // For temporary logging if needed
#include <cmath>  // For std::pow, std::floor
#include <algorithm> // Added for std::max and std::min

namespace RME {
namespace ui {
namespace widgets {

MapView::MapView(QWidget *parent)
    : QOpenGLWidget(parent),
      m_currentFloor(7),      // Default surface floor
      m_zoomFactor(1.0),
      m_viewCenterMapCoords(1000.0, 1000.0), // Default center
      m_isPanning(false)
      // m_projectionMatrix is default initialized
{
    // Set focus policy to receive keyboard events
    setFocusPolicy(Qt::StrongFocus);
}

MapView::~MapView() {
    // Ensure OpenGL context is current if doing cleanup, but usually not needed for members
}

void MapView::initializeGL() {
    if (!initializeOpenGLFunctions()) {
        qWarning("MapView: Could not initialize OpenGL functions.");
        // Consider emitting a signal or handling this error more gracefully
        return;
    }
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST); // If using depth testing later
}

void MapView::resizeGL(int w, int h) {
    if (h == 0) h = 1; // Prevent division by zero
    glViewport(0, 0, w, h);
    updateProjectionMatrix(); // Update projection matrix whenever widget is resized
}

void MapView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

    // The m_projectionMatrix already incorporates view translation (pan) and scale (zoom).
    // QMatrix4x4 mvpMatrix = m_projectionMatrix;

    // Future drawing (RENDER-02) will use this matrix.
    // For example:
    // m_shaderProgram->bind();
    // m_shaderProgram->setUniformValue("mvp_matrix", mvpMatrix);
    // ... bind VAO, draw elements ...
    // m_shaderProgram->release();

    // For now, this method only clears the screen.
}

RME::core::Position MapView::screenToMapCoords(const QPoint& screenPos) const {
    if (width() <= 0 || height() <= 0 || m_zoomFactor <= 0 || TILE_PIXEL_SIZE <= 0) {
        // Return a default or invalid position if view parameters are not set
        return RME::core::Position(-1, -1, m_currentFloor);
    }

    // screenPixelsPerMapUnit is how many screen pixels one map unit (e.g., one tile width/height) currently occupies.
    qreal screenPixelsPerMapUnit = static_cast<qreal>(TILE_PIXEL_SIZE) * m_zoomFactor;

    // Distance from screen center to screenPos, in pixels
    qreal deltaScreenX = static_cast<qreal>(screenPos.x()) - static_cast<qreal>(width()) / 2.0;
    qreal deltaScreenY = static_cast<qreal>(screenPos.y()) - static_cast<qreal>(height()) / 2.0;

    // Convert delta from screen pixels to map units
    qreal deltaMapX = deltaScreenX / screenPixelsPerMapUnit;
    qreal deltaMapY = deltaScreenY / screenPixelsPerMapUnit;

    // Add delta in map units to the map coordinates of the view center
    qreal worldX = m_viewCenterMapCoords.x() + deltaMapX;
    qreal worldY = m_viewCenterMapCoords.y() + deltaMapY;

    return RME::core::Position(static_cast<int>(std::floor(worldX)),
                               static_cast<int>(std::floor(worldY)),
                               m_currentFloor);
}

QPoint MapView::mapCoordsToScreen(const RME::core::Position& mapPos) const {
    if (width() <= 0 || height() <= 0 || m_zoomFactor <= 0 || TILE_PIXEL_SIZE <= 0) {
        // Return an off-screen point if view parameters are not set
        return QPoint(-1, -1);
    }

    if (mapPos.z != m_currentFloor) {
        // Typically, you'd only want to convert coords for the current floor.
        // Or, if drawing other floors (e.g., dimmed), this might be different.
        // For now, return off-screen if not current floor.
        // This behavior might need adjustment based on rendering needs of other floors.
        // return QPoint(-10000, -10000); // Far off screen
    }

    // screenPixelsPerMapUnit is how many screen pixels one map unit (e.g., one tile width/height) currently occupies.
    qreal screenPixelsPerMapUnit = static_cast<qreal>(TILE_PIXEL_SIZE) * m_zoomFactor;

    // Delta from map center to mapPos, in map units
    qreal deltaMapX = static_cast<qreal>(mapPos.x) - m_viewCenterMapCoords.x();
    qreal deltaMapY = static_cast<qreal>(mapPos.y) - m_viewCenterMapCoords.y();

    // Convert delta from map units to screen pixels
    qreal deltaScreenX = deltaMapX * screenPixelsPerMapUnit;
    qreal deltaScreenY = deltaMapY * screenPixelsPerMapUnit;

    // Add to screen center
    qreal screenX = static_cast<qreal>(width()) / 2.0 + deltaScreenX;
    qreal screenY = static_cast<qreal>(height()) / 2.0 + deltaScreenY;

    return QPoint(qRound(screenX), qRound(screenY));
}

// Public Slots
void MapView::setCurrentFloor(int floor) {
    int newFloor = std::max(MIN_Z, std::min(floor, MAX_Z)); // Clamp floor
    if (newFloor != m_currentFloor) {
        m_currentFloor = newFloor;
        // No need to call updateProjectionMatrix() if only floor changes,
        // as the projection itself (left, right, bottom, top) doesn't depend on Z.
        // However, what is *drawn* will change, so a repaint is needed.
        update(); // Schedule repaint
        emit viewChanged();
        // emit currentFloorChanged(m_currentFloor); // If this specific signal exists
    }
}

void MapView::floorUp() {
    // In Tibia maps, lower Z is "higher" up (e.g. floor 7 is surface, floor 6 is +1)
    setCurrentFloor(m_currentFloor - 1);
}

void MapView::floorDown() {
    setCurrentFloor(m_currentFloor + 1);
}

void MapView::setViewCenter(const RME::core::Position& mapPos) {
    bool changed = false;
    QPointF newViewCenter(static_cast<qreal>(mapPos.x) + 0.5, static_cast<qreal>(mapPos.y) + 0.5); // Center on middle of tile

    if (newViewCenter != m_viewCenterMapCoords) {
        m_viewCenterMapCoords = newViewCenter;
        changed = true;
    }

    int newFloor = std::max(MIN_Z, std::min(mapPos.z, MAX_Z));
    if (newFloor != m_currentFloor) {
        m_currentFloor = newFloor;
        changed = true;
    }

    if (changed) {
        updateProjectionMatrix(); // Center change affects projection calculation
        update();
        emit viewChanged();
    }
}

void MapView::setZoom(qreal zoom) {
    qreal newZoomFactor = std::max(MIN_ZOOM, std::min(zoom, MAX_ZOOM));
    if (m_zoomFactor != newZoomFactor) { // Avoid qFuzzyCompare for direct assignment if not needed
        m_zoomFactor = newZoomFactor;

        // When zoom changes, the point under the mouse (or view center if mouse not involved)
        // should ideally remain fixed. However, this simple setZoom might just zoom on current center.
        // For zoom-to-cursor, wheelEvent logic is more elaborate.
        // This is a direct zoom setting, typically centers on current m_viewCenterMapCoords.
        updateProjectionMatrix();
        update();
        emit viewChanged();
    }
}

void MapView::updateProjectionMatrix() {
    if (width() <= 0 || height() <= 0 || m_zoomFactor <= 0) { // Prevent division by zero or invalid matrix
        m_projectionMatrix.setToIdentity(); // Default to identity if dimensions are invalid
        return;
    }

    m_projectionMatrix.setToIdentity();

    // TILE_PIXEL_SIZE is the base size of one tile in screen pixels when m_zoomFactor is 1.0.
    // screenPixelsPerMapUnit is how many screen pixels one map unit (e.g., one tile width/height) currently occupies.
    qreal screenPixelsPerMapUnit = TILE_PIXEL_SIZE * m_zoomFactor;

    // visibleMapUnitsX/Y is how many map units (tiles) are visible on screen horizontally/vertically.
    qreal visibleMapUnitsX = static_cast<qreal>(width()) / screenPixelsPerMapUnit;
    qreal visibleMapUnitsY = static_cast<qreal>(height()) / screenPixelsPerMapUnit;

    // Calculate the boundaries of the orthographic projection in map coordinates.
    // The view is centered on m_viewCenterMapCoords.
    qreal left = m_viewCenterMapCoords.x() - visibleMapUnitsX / 2.0;
    qreal right = m_viewCenterMapCoords.x() + visibleMapUnitsX / 2.0;
    // For ortho: bottom, top. If map Y increases downwards (like screen Y):
    // top map coord will be m_viewCenterMapCoords.y() - visibleMapUnitsY / 2.0
    // bottom map coord will be m_viewCenterMapCoords.y() + visibleMapUnitsY / 2.0
    // Qt's QMatrix4x4::ortho takes (left, right, bottom, top, near, far)
    qreal top_map_coord = m_viewCenterMapCoords.y() - visibleMapUnitsY / 2.0;
    qreal bottom_map_coord = m_viewCenterMapCoords.y() + visibleMapUnitsY / 2.0;

    m_projectionMatrix.ortho(left, right, bottom_map_coord, top_map_coord, -1.0f, 1.0f);
}

void MapView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor); // Change cursor to indicate panning
        event->accept();
    } else {
        // For other buttons, emit a click signal with map coordinates
        RME::core::Position mapPos = screenToMapCoords(event->pos());
        emit mapPositionClicked(mapPos, event->button(), event->modifiers());
        // event->ignore(); // Allow other widgets or parent to process if needed, or accept()
    }
}

void MapView::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPanning) {
        QPointF deltaPixels = event->pos() - m_lastPanMousePos;
        m_lastPanMousePos = event->pos();

        if (m_zoomFactor <= 0 || TILE_PIXEL_SIZE <=0) return; // Should not happen with guards
        qreal screenPixelsPerMapUnit = static_cast<qreal>(TILE_PIXEL_SIZE) * m_zoomFactor;

        // Convert pixel delta to map coordinate delta
        // Panning logic: if mouse moves right (deltaPixels.x() positive), view should show content to the left,
        // so viewCenterMapCoords.x should decrease.
        m_viewCenterMapCoords.rx() -= deltaPixels.x() / screenPixelsPerMapUnit;
        m_viewCenterMapCoords.ry() -= deltaPixels.y() / screenPixelsPerMapUnit;

        updateProjectionMatrix(); // Recalculate projection based on new center
        update();               // Schedule a repaint
        emit viewChanged();
        event->accept();
    } else {
        // Could emit mouseMove type signals with map coords if needed for hover effects etc.
        // event->ignore();
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        unsetCursor(); // Restore default cursor
        event->accept();
    } else {
        // event->ignore();
    }
}

void MapView::mouseDoubleClickEvent(QMouseEvent* event) {
    RME::core::Position mapPos = screenToMapCoords(event->pos());
    emit mapPositionDoubleClicked(mapPos, event->button());
    // event->accept(); // Typically double clicks are terminal for an event sequence
}

void MapView::wheelEvent(QWheelEvent* event) {
    QPointF screenPosGlobal = event->position(); // Mouse position relative to widget
    RME::core::Position mapPosUnderMouse = screenToMapCoords(screenPosGlobal.toPoint()); // toPoint for screenToMapCoords

    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15; // Standard way to get steps from wheel delta

    qreal oldZoomFactor = m_zoomFactor;
    qreal newZoomFactor = m_zoomFactor * std::pow(ZOOM_STEP_MULTIPLIER, numSteps);
    newZoomFactor = std::max(MIN_ZOOM, std::min(newZoomFactor, MAX_ZOOM)); // Clamp zoom

    if (qFuzzyCompare(m_zoomFactor, newZoomFactor)) {
        event->accept();
        return; // No change in zoom
    }

    m_zoomFactor = newZoomFactor;

    // Adjust m_viewCenterMapCoords to keep the point under the mouse cursor fixed
    // C_map_new_x = P_map_x - (S_mouse_x - ScreenWidth/2) / Z_new_pixels_per_map_unit
    qreal newScreenPixelsPerMapUnit = static_cast<qreal>(TILE_PIXEL_SIZE) * m_zoomFactor;
    if (newScreenPixelsPerMapUnit <= 0) { // Should be guarded by MIN_ZOOM but defensive check
        updateProjectionMatrix(); // Update with clamped zoom even if center logic fails
        update();
        emit viewChanged();
        event->accept();
        return;
    }

    m_viewCenterMapCoords.setX(mapPosUnderMouse.x - (screenPosGlobal.x() - static_cast<qreal>(width()) / 2.0) / newScreenPixelsPerMapUnit);
    m_viewCenterMapCoords.setY(mapPosUnderMouse.y - (screenPosGlobal.y() - static_cast<qreal>(height()) / 2.0) / newScreenPixelsPerMapUnit);

    updateProjectionMatrix();
    update(); // Schedule repaint
    emit viewChanged();
    event->accept();
}

void MapView::keyPressEvent(QKeyEvent* event) {
    bool viewChangedFlag = false;
    // Panning speed in map units (tiles) per key press
    const qreal panStep = 1.0; // Pan one tile at a time

    switch (event->key()) {
        case Qt::Key_Left:
            m_viewCenterMapCoords.rx() -= panStep;
            viewChangedFlag = true;
            break;
        case Qt::Key_Right:
            m_viewCenterMapCoords.rx() += panStep;
            viewChangedFlag = true;
            break;
        case Qt::Key_Up:
            m_viewCenterMapCoords.ry() -= panStep; // Screen up is typically lower Y in map if Y grows downwards
            viewChangedFlag = true;
            break;
        case Qt::Key_Down:
            m_viewCenterMapCoords.ry() += panStep; // Screen down is typically higher Y in map
            viewChangedFlag = true;
            break;
        case Qt::Key_PageUp:
            floorUp(); // floorUp() already handles update, signals
            // viewChangedFlag is handled by floorUp calling setCurrentFloor
            break;
        case Qt::Key_PageDown:
            floorDown(); // floorDown() already handles update, signals
            // viewChangedFlag is handled by floorDown calling setCurrentFloor
            break;
        // Example: Zoom with + / - keys
        // case Qt::Key_Plus:
        // case Qt::Key_Equal: // Equal is often unshifted +
        //     setZoom(m_zoomFactor * ZOOM_STEP_MULTIPLIER);
        //     break;
        // case Qt::Key_Minus:
        //     setZoom(m_zoomFactor / ZOOM_STEP_MULTIPLIER);
        //     break;
        default:
            QOpenGLWidget::keyPressEvent(event); // Pass to base class if not handled
            return;
    }

    if (viewChangedFlag) {
        updateProjectionMatrix();
        update(); // Schedule repaint
        emit viewChanged();
    }
    event->accept(); // Event was handled
}

} // namespace widgets
} // namespace ui
} // namespace RME
