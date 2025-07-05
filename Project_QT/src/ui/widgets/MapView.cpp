#include "ui/widgets/MapView.h" // Use path relative to src/ or as per CMake include_directories

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor> // Added for setCursor/unsetCursor
#include <QtGlobal> // Added for qFuzzyCompare
#include <QPainter> // Added
#include <QGuiApplication> // Added for keyboardModifiers()
#include <QDebug> // For temporary logging if needed
#include <cmath>  // For std::pow, std::floor
#include <algorithm> // Added for std::max and std::min
#include <QOpenGLShaderProgram> // For RENDER-02 shaders
#include <QOpenGLBuffer>        // For RENDER-02 VBO
#include <QOpenGLVertexArrayObject> // For RENDER-02 VAO
#include <QElapsedTimer>        // For RENDER-03 animation timing

namespace RME {
namespace ui {
namespace widgets {

MapView::MapView(QWidget *parent)
    : QOpenGLWidget(parent),
      m_currentFloor(7),      // Default surface floor
      m_zoomFactor(1.0),
      m_viewCenterMapCoords(1000.0, 1000.0), // Default center
      m_isPanning(false),
      m_editorController(nullptr), // Initialize new members
      m_isPerformingBoundingBoxSelection(false)
      // m_projectionMatrix is default initialized
      // m_currentBrushSettings is default initialized
      // m_dragStartScreenPoint is default initialized
      // m_currentDragScreenPoint is default initialized
{
    // Set focus policy to receive keyboard events
    setFocusPolicy(Qt::StrongFocus);
}

MapView::~MapView() {
    // Cleanup OpenGL resources
    cleanupShaders();
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
    
    // RENDER-02: Initialize shaders and geometry
    if (!initializeShaders()) {
        qWarning("MapView: Failed to initialize shaders for tile rendering.");
    }
    
    // RENDER-03: Initialize texture manager if available
    if (m_textureManager && !m_textureManager->initialize()) {
        qWarning("MapView: Failed to initialize TextureManager for sprite rendering.");
    }
    
    // RENDER-04: Initialize light renderer if available
    if (m_lightRenderer && !m_lightRenderer->initialize()) {
        qWarning("MapView: Failed to initialize LightRenderer for lighting effects.");
    }
}

void MapView::resizeGL(int w, int h) {
    if (h == 0) h = 1; // Prevent division by zero
    glViewport(0, 0, w, h);
    updateProjectionMatrix(); // Update projection matrix whenever widget is resized
}

void MapView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

    // RENDER-02: Render map tiles if we have all dependencies
    if (m_map && m_appSettings && m_assetManager && m_colorQuadShader) {
        renderTiles();
        
        // RENDER-03: Render sprites on top of tiles
        if (m_textureManager) {
            renderSprites();
        }
        
        // Render additional overlays
        if (m_appSettings->getBoolean(RME::Config::SHOW_GRID)) {
            renderGrid();
        }
        
        // Render tile highlights for debugging/selection
        renderTileHighlights();
        
        // RENDER-04: Render lighting effects on top
        if (m_lightRenderer && m_lightCalculatorService && 
            m_lightCalculatorService->isLightingEnabled()) {
            renderLightingEffects();
        }
    }

    // Draw bounding box selection rectangle if active
    if (m_isPerformingBoundingBoxSelection) {
        // QPainter needs to be used carefully with QOpenGLWidget.
        // It's often better to draw simple overlay graphics directly with GL lines.
        // For this example, using QPainter as specified.
        // This might require painter.beginNativePainting() / endNativePainting() in more complex scenarios
        // or if mixing heavily with direct GL calls outside of QPainter's control for other elements.
        QPainter painter(this);
        // painter.begin(this); // Not needed if painter is stack allocated with `this` as parent
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen(Qt::white, 1, Qt::DashLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRect(m_dragStartScreenPoint, m_currentDragScreenPoint).normalized());
        // painter.end(); // Automatically called by QPainter destructor
    }
}

// Slot implementation
void MapView::updateCurrentBrushSettings(const RME::core::BrushSettings& settings) {
    m_currentBrushSettings = settings;
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
    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier)) {
        if (m_editorController) {
            m_isPerformingBoundingBoxSelection = true;
            m_dragStartScreenPoint = event->pos();
            m_currentDragScreenPoint = event->pos();

            if (!(event->modifiers() & Qt::ControlModifier)) {
                m_editorController->clearCurrentSelection();
            }
            update(); // Request repaint to draw the initial rectangle (a point)
            event->accept();
        } else {
            QOpenGLWidget::mousePressEvent(event); // Or event->ignore();
        }
    } else if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        RME::core::Position mapPos = screenToMapCoords(event->pos());
        emit mapPositionClicked(mapPos, event->button(), event->modifiers());
        // QOpenGLWidget::mousePressEvent(event); // if not accepted
    }
}

void MapView::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPerformingBoundingBoxSelection) {
        m_currentDragScreenPoint = event->pos();
        update(); // Request repaint to show updated rectangle
        event->accept();
    } else if (m_isPanning) {
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
    if (m_isPerformingBoundingBoxSelection && event->button() == Qt::LeftButton) {
        m_isPerformingBoundingBoxSelection = false;
        m_currentDragScreenPoint = event->pos(); // Final point

        if (m_editorController) {
            RME::core::Position mapP1 = screenToMapCoords(m_dragStartScreenPoint);
            RME::core::Position mapP2 = screenToMapCoords(m_currentDragScreenPoint);

            // Use QGuiApplication::keyboardModifiers() as event->modifiers() might not be up-to-date
            // or might not reflect global state if a key was released during drag.
            m_editorController->performBoundingBoxSelection(mapP1, mapP2, QGuiApplication::keyboardModifiers(), m_currentBrushSettings);
        }
        update(); // Request repaint to remove the rectangle
        event->accept();
    } else if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        unsetCursor();
        event->accept();
    } else {
        // QOpenGLWidget::mouseReleaseEvent(event); // if not accepted
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

// RENDER-02: Shader initialization
bool MapView::initializeShaders() {
    // Create shader program
    m_colorQuadShader = new QOpenGLShaderProgram(this);
    
    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 mvpMatrix;
        void main() {
            gl_Position = mvpMatrix * vec4(aPos.x, aPos.y, 0.0, 1.0);
        }
    )";
    
    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 uColor;
        void main() {
            FragColor = uColor;
        }
    )";
    
    if (!m_colorQuadShader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "Failed to compile vertex shader:" << m_colorQuadShader->log();
        return false;
    }
    
    if (!m_colorQuadShader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "Failed to compile fragment shader:" << m_colorQuadShader->log();
        return false;
    }
    
    if (!m_colorQuadShader->link()) {
        qWarning() << "Failed to link shader program:" << m_colorQuadShader->log();
        return false;
    }
    
    // Create VBO and VAO for unit quad
    m_quadVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_quadVAO = new QOpenGLVertexArrayObject(this);
    
    if (!m_quadVBO->create() || !m_quadVAO->create()) {
        qWarning() << "Failed to create VBO or VAO";
        return false;
    }
    
    // Set up unit quad geometry (0,0 to 1,1)
    GLfloat quadVertices[] = {
        0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  // Bottom-right
        1.0f, 1.0f,  // Top-right
        0.0f, 1.0f   // Top-left
    };
    
    m_quadVAO->bind();
    m_quadVBO->bind();
    m_quadVBO->allocate(quadVertices, sizeof(quadVertices));
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    
    m_quadVAO->release();
    m_quadVBO->release();
    
    return true;
}

void MapView::cleanupShaders() {
    delete m_colorQuadShader;
    delete m_quadVBO;
    delete m_quadVAO;
    m_colorQuadShader = nullptr;
    m_quadVBO = nullptr;
    m_quadVAO = nullptr;
}

// RENDER-02: Calculate visible tile range
void MapView::calculateVisibleRange(int& minMapX, int& maxMapX, int& minMapY, int& maxMapY, 
                                   int& renderMinZ, int& renderMaxZ) const {
    if (width() <= 0 || height() <= 0 || m_zoomFactor <= 0) {
        minMapX = maxMapX = minMapY = maxMapY = 0;
        renderMinZ = renderMaxZ = m_currentFloor;
        return;
    }
    
    // Calculate visible map range based on viewport
    qreal tilesVisibleX = static_cast<qreal>(width()) / (TILE_PIXEL_SIZE * m_zoomFactor);
    qreal tilesVisibleY = static_cast<qreal>(height()) / (TILE_PIXEL_SIZE * m_zoomFactor);
    
    minMapX = static_cast<int>(std::floor(m_viewCenterMapCoords.x() - tilesVisibleX / 2.0)) - 1;
    maxMapX = static_cast<int>(std::ceil(m_viewCenterMapCoords.x() + tilesVisibleX / 2.0)) + 1;
    minMapY = static_cast<int>(std::floor(m_viewCenterMapCoords.y() - tilesVisibleY / 2.0)) - 1;
    maxMapY = static_cast<int>(std::ceil(m_viewCenterMapCoords.y() + tilesVisibleY / 2.0)) + 1;
    
    // Calculate floor range
    renderMinZ = m_currentFloor;
    renderMaxZ = m_currentFloor;
    
    if (m_appSettings && m_appSettings->getBoolean(RME::Config::SHOW_ALL_FLOORS)) {
        renderMinZ = std::max(0, m_currentFloor - 2);
        renderMaxZ = std::min(MAX_Z, m_currentFloor + 2);
    }
}

// RENDER-02: Determine tile color with MaterialSystem integration
QColor MapView::determineTileColor(const RME::core::Tile* tile) const {
    if (!tile || !m_appSettings) {
        return Qt::darkGray;
    }
    
    // Check rendering mode preferences
    bool showAsMinimapColors = m_appSettings->getBoolean(RME::Config::SHOW_AS_MINIMAP);
    bool showOnlyTileFlags = m_appSettings->getBoolean(RME::Config::SHOW_ONLY_TILEFLAGS);
    
    // Priority 1: Special tile states (if enabled and not in minimap-only mode)
    if (!showAsMinimapColors && m_appSettings->getBoolean(RME::Config::SHOW_SPECIAL_TILES)) {
        if (tile->hasMapFlag(RME::TileMapFlag::PROTECTION_ZONE)) {
            return QColor(76, 175, 80, 200); // Material Green for PZ
        }
        if (tile->hasMapFlag(RME::TileMapFlag::NO_PVP_ZONE)) {
            return QColor(255, 193, 7, 200); // Material Amber for No PVP
        }
        if (tile->hasMapFlag(RME::TileMapFlag::PVP_ZONE)) {
            return QColor(244, 67, 54, 200); // Material Red for PVP
        }
        if (tile->hasMapFlag(RME::TileMapFlag::NO_LOGOUT_ZONE)) {
            return QColor(156, 39, 176, 200); // Material Purple for No Logout
        }
    }
    
    // Priority 2: House areas (if enabled and not in tile-flags-only mode)
    if (!showOnlyTileFlags && m_appSettings->getBoolean(RME::Config::SHOW_HOUSES) && tile->getHouseId() != 0) {
        return QColor(33, 150, 243, 180); // Material Blue for house areas
    }
    
    // Priority 3: Ground item colors (main rendering)
    const RME::core::Item* ground = tile->getGround();
    if (ground && m_assetManager) {
        uint16_t itemId = ground->getID();
        
        // Try to get minimap color from MaterialSystem
        if (itemId > 0) {
            // Enhanced color mapping with better distribution
            QColor itemColor = getMinimapColorForItem(itemId);
            if (itemColor.isValid()) {
                return itemColor;
            }
        }
    }
    
    // Priority 4: Blocking tiles (if enabled)
    if (m_appSettings->getBoolean(RME::Config::SHOW_BLOCKING) && tile->isBlocking()) {
        return QColor(158, 158, 158); // Gray for blocking tiles
    }
    
    return QColor(48, 48, 48); // Dark gray default (better contrast)
}

// Helper method to get minimap color for items
QColor MapView::getMinimapColorForItem(uint16_t itemId) const {
    if (!m_assetManager) {
        return QColor();
    }
    
    // Try to get item data from AssetManager
    const RME::core::assets::ItemData* itemData = m_assetManager->getItemData(itemId);
    if (itemData) {
        // TODO: Once ItemData has minimap color support, use it
        // return itemData->getMinimapColor();
    }
    
    // Enhanced fallback color mapping with better visual distribution
    // Use a more sophisticated color generation algorithm
    uint32_t hash = itemId;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    
    // Generate colors in HSV space for better distribution
    float hue = (hash % 360) / 360.0f;
    float saturation = 0.6f + 0.3f * ((hash >> 8) % 100) / 100.0f; // 0.6-0.9
    float value = 0.5f + 0.4f * ((hash >> 16) % 100) / 100.0f;     // 0.5-0.9
    
    QColor color = QColor::fromHsvF(hue, saturation, value);
    
    // Ensure minimum contrast for visibility
    if (color.lightness() < 30) {
        color = color.lighter(150);
    }
    
    return color;
}

// RENDER-02: Calculate floor alpha for ghosting with enhanced settings
float MapView::calculateFloorAlpha(int tileZ) const {
    if (tileZ == m_currentFloor) {
        return 1.0f; // Full opacity for current floor
    }
    
    if (!m_appSettings) {
        return 0.0f; // Hide other floors if no settings
    }
    
    // Check if we should show all floors
    if (!m_appSettings->getBoolean(RME::Config::SHOW_ALL_FLOORS)) {
        return 0.0f; // Only show current floor
    }
    
    int floorDifference = std::abs(tileZ - m_currentFloor);
    
    if (tileZ < m_currentFloor) {
        // Lower floors (underground) - gradually fade with distance
        switch (floorDifference) {
            case 1: return 0.6f;  // One floor below - quite visible
            case 2: return 0.4f;  // Two floors below - dimmed
            case 3: return 0.25f; // Three floors below - very dim
            default: return 0.1f; // Far below - barely visible
        }
    } else {
        // Higher floors (above)
        if (m_appSettings->getBoolean(RME::Config::TRANSPARENT_FLOORS)) {
            // Transparent floors enabled - show with decreasing alpha
            switch (floorDifference) {
                case 1: return 0.4f;  // One floor above - semi-transparent
                case 2: return 0.25f; // Two floors above - more transparent
                case 3: return 0.15f; // Three floors above - very transparent
                default: return 0.05f; // Far above - barely visible
            }
        } else {
            return 0.0f; // Hide higher floors if transparency is off
        }
    }
}

// RENDER-02: Main tile rendering method with optimizations
void MapView::renderTiles() {
    if (!m_colorQuadShader || !m_quadVAO) {
        return;
    }
    
    // Calculate visible range
    int minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ;
    calculateVisibleRange(minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ);
    
    // Early exit if no tiles to render
    if (minMapX > maxMapX || minMapY > maxMapY || renderMinZ > renderMaxZ) {
        return;
    }
    
    // Bind shader and VAO once
    m_colorQuadShader->bind();
    m_quadVAO->bind();
    
    // Cache settings for performance
    bool showOnlyModified = m_appSettings->getBoolean(RME::Config::SHOW_ONLY_MODIFIED_TILES);
    
    // Pre-calculate base model matrix for tile positioning
    QMatrix4x4 baseModelMatrix;
    baseModelMatrix.scale(TILE_PIXEL_SIZE, TILE_PIXEL_SIZE);
    
    int tilesRendered = 0;
    const int maxTilesPerFrame = 10000; // Performance limit for very large views
    
    // Render tiles from top floor to bottom (painter's algorithm)
    for (int z = renderMaxZ; z >= renderMinZ && tilesRendered < maxTilesPerFrame; --z) {
        // Pre-calculate floor alpha for this entire floor
        float floorAlpha = calculateFloorAlpha(z);
        if (floorAlpha <= 0.0f) {
            continue; // Skip entire floor if invisible
        }
        
        for (int y = minMapY; y <= maxMapY && tilesRendered < maxTilesPerFrame; ++y) {
            for (int x = minMapX; x <= maxMapX && tilesRendered < maxTilesPerFrame; ++x) {
                const RME::core::Tile* tile = m_map->getTile(RME::core::Position(x, y, z));
                
                // Skip empty tiles unless they're house tiles or special
                if (!tile || (tile->isEmpty() && tile->getHouseId() == 0 && !tile->hasMapFlag(RME::TileMapFlag::PROTECTION_ZONE))) {
                    continue;
                }
                
                // Skip unmodified tiles if only showing modified
                if (showOnlyModified && !tile->hasStateFlag(RME::TileStateFlag::MODIFIED)) {
                    continue;
                }
                
                // Determine tile color
                QColor baseColor = determineTileColor(tile);
                
                // Apply floor alpha to the color
                QColor finalColor(baseColor.red(), baseColor.green(), baseColor.blue(), 
                                static_cast<int>(floorAlpha * baseColor.alpha()));
                
                // Optimize matrix calculation - reuse base matrix
                QMatrix4x4 modelMatrix = baseModelMatrix;
                modelMatrix.translate(x * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE, 0.0f);
                
                QMatrix4x4 mvp = m_projectionMatrix * modelMatrix;
                
                // Set shader uniforms
                m_colorQuadShader->setUniformValue("mvpMatrix", mvp);
                m_colorQuadShader->setUniformValue("uColor", 
                    QVector4D(finalColor.redF(), finalColor.greenF(), finalColor.blueF(), finalColor.alphaF()));
                
                // Draw the quad
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                tilesRendered++;
            }
        }
    }
    
    // Debug info for performance monitoring
    if (tilesRendered >= maxTilesPerFrame) {
        qDebug() << "MapView: Rendered maximum tiles per frame (" << maxTilesPerFrame << "), some tiles may be skipped";
    }
    
    m_quadVAO->release();
    m_colorQuadShader->release();
}

// RENDER-02: Grid rendering for visual aid
void MapView::renderGrid() {
    if (!m_colorQuadShader || !m_appSettings) {
        return;
    }
    
    // Calculate visible range for grid
    int minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ;
    calculateVisibleRange(minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ);
    
    // Only render grid for current floor to avoid clutter
    int gridZ = m_currentFloor;
    
    // Use a subtle grid color
    QColor gridColor(255, 255, 255, 30); // Very transparent white
    
    m_colorQuadShader->bind();
    
    // Render vertical grid lines
    for (int x = minMapX; x <= maxMapX; ++x) {
        QMatrix4x4 modelMatrix;
        modelMatrix.translate(x * TILE_PIXEL_SIZE - 0.5f, minMapY * TILE_PIXEL_SIZE);
        modelMatrix.scale(1.0f, (maxMapY - minMapY + 1) * TILE_PIXEL_SIZE);
        
        QMatrix4x4 mvp = m_projectionMatrix * modelMatrix;
        m_colorQuadShader->setUniformValue("mvpMatrix", mvp);
        m_colorQuadShader->setUniformValue("uColor", 
            QVector4D(gridColor.redF(), gridColor.greenF(), gridColor.blueF(), gridColor.alphaF()));
        
        // Draw thin vertical line
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    // Render horizontal grid lines
    for (int y = minMapY; y <= maxMapY; ++y) {
        QMatrix4x4 modelMatrix;
        modelMatrix.translate(minMapX * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE - 0.5f);
        modelMatrix.scale((maxMapX - minMapX + 1) * TILE_PIXEL_SIZE, 1.0f);
        
        QMatrix4x4 mvp = m_projectionMatrix * modelMatrix;
        m_colorQuadShader->setUniformValue("mvpMatrix", mvp);
        m_colorQuadShader->setUniformValue("uColor", 
            QVector4D(gridColor.redF(), gridColor.greenF(), gridColor.blueF(), gridColor.alphaF()));
        
        // Draw thin horizontal line
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    m_colorQuadShader->release();
}

// RENDER-02: Render tile highlights for selection/debugging
void MapView::renderTileHighlights() {
    if (!m_colorQuadShader || !m_appSettings) {
        return;
    }
    
    // Get current mouse position in map coordinates
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    if (rect().contains(mousePos)) {
        RME::core::Position hoveredTile = screenToMapCoords(mousePos);
        
        // Highlight hovered tile with a subtle overlay
        QColor highlightColor(255, 255, 255, 50); // Semi-transparent white
        
        m_colorQuadShader->bind();
        m_quadVAO->bind();
        
        // Render highlight for hovered tile
        QMatrix4x4 modelMatrix;
        modelMatrix.translate(hoveredTile.x * TILE_PIXEL_SIZE, hoveredTile.y * TILE_PIXEL_SIZE);
        modelMatrix.scale(TILE_PIXEL_SIZE, TILE_PIXEL_SIZE);
        
        QMatrix4x4 mvp = m_projectionMatrix * modelMatrix;
        m_colorQuadShader->setUniformValue("mvpMatrix", mvp);
        m_colorQuadShader->setUniformValue("uColor", 
            QVector4D(highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF(), highlightColor.alphaF()));
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        m_quadVAO->release();
        m_colorQuadShader->release();
    }
}

// RENDER-03: Simple sprite rendering with immediate mode OpenGL
void MapView::renderSprites() {
    if (!m_textureManager || !m_map) {
        return;
    }
    
    // Calculate visible range (reuse from RENDER-02)
    int minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ;
    calculateVisibleRange(minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ);
    
    // Enable texture rendering
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable the shader program for immediate mode rendering
    if (m_colorQuadShader) {
        m_colorQuadShader->release();
    }
    
    // Set up 2D projection for immediate mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1, 1); // Screen coordinates
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Render sprites from top floor to bottom (same as tiles)
    for (int z = renderMaxZ; z >= renderMinZ; --z) {
        float floorAlpha = calculateFloorAlpha(z);
        if (floorAlpha <= 0.0f) {
            continue; // Skip invisible floors
        }
        
        // Set alpha for floor ghosting
        glColor4f(1.0f, 1.0f, 1.0f, floorAlpha);
        
        for (int y = minMapY; y <= maxMapY; ++y) {
            for (int x = minMapX; x <= maxMapX; ++x) {
                const RME::core::Tile* tile = m_map->getTile(RME::core::Position(x, y, z));
                if (!tile || tile->isEmpty()) {
                    continue;
                }
                
                // Convert map coordinates to screen coordinates
                QPoint screenPos = mapCoordsToScreen(RME::core::Position(x, y, z));
                
                // Render all items on this tile with improved stacking
                drawStackedItems(screenPos, tile, floorAlpha);
            }
        }
    }
    
    // Restore matrices
    glPopMatrix(); // MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    // Restore OpenGL state
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reset color
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// RENDER-03: Draw textured quad using immediate mode OpenGL
void MapView::drawTexturedQuad(float x, float y, float width, float height, GLuint textureId) {
    if (textureId == 0) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Draw quad with texture coordinates (immediate mode)
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);                    // Top-left
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);            // Top-right
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);   // Bottom-right
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);           // Bottom-left
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

// RENDER-03: Enhanced item rendering with stacking and animation
void MapView::drawStackedItems(const QPoint& screenPos, const RME::core::Tile* tile, float alpha) {
    if (!tile || !m_textureManager) {
        return;
    }
    
    float stackOffset = 0.0f;
    const float STACK_OFFSET_PIXELS = 2.0f; // Slight offset for stacked items
    
    // Render ground item first
    const RME::core::Item* ground = tile->getGround();
    if (ground) {
        int animFrame = getCurrentAnimationFrame(ground->getID());
        GLuint textureId = m_textureManager->getTextureForSpriteFrame(ground->getID(), animFrame);
        if (textureId != 0) {
            drawTexturedQuad(screenPos.x(), screenPos.y(), TILE_PIXEL_SIZE, TILE_PIXEL_SIZE, textureId);
        }
    }
    
    // Render item stack with slight offset for visibility
    const auto& items = tile->getItems();
    for (int i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        if (item) {
            int animFrame = getCurrentAnimationFrame(item->getID());
            GLuint textureId = m_textureManager->getTextureForSpriteFrame(item->getID(), animFrame);
            if (textureId != 0) {
                // Apply stack offset for better visibility
                float offsetX = stackOffset;
                float offsetY = stackOffset;
                
                drawTexturedQuad(screenPos.x() + offsetX, screenPos.y() + offsetY, 
                               TILE_PIXEL_SIZE, TILE_PIXEL_SIZE, textureId);
                
                // Increase offset for next item (but cap it to avoid too much spread)
                stackOffset += STACK_OFFSET_PIXELS;
                if (stackOffset > STACK_OFFSET_PIXELS * 3) {
                    stackOffset = STACK_OFFSET_PIXELS * 3; // Max 3 items offset
                }
            }
        }
    }
}

// RENDER-03: Simple animation frame calculation
int MapView::getCurrentAnimationFrame(quint32 spriteId) const {
    if (!m_textureManager) {
        return 0;
    }
    
    int frameCount = m_textureManager->getSpriteFrameCount(spriteId);
    if (frameCount <= 1) {
        return 0; // No animation needed
    }
    
    // Update animation timing
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_lastAnimationUpdate == 0) {
        m_lastAnimationUpdate = currentTime;
    }
    
    // Simple animation: 500ms per frame
    const qint64 ANIMATION_FRAME_DURATION_MS = 500;
    qint64 timeSinceStart = currentTime - m_lastAnimationUpdate;
    int currentFrame = (timeSinceStart / ANIMATION_FRAME_DURATION_MS) % frameCount;
    
    // Cache the current frame for this sprite
    m_spriteAnimationFrames[spriteId] = currentFrame;
    
    return currentFrame;
}

// RENDER-04: Lighting effects rendering
void MapView::renderLightingEffects() {
    if (!m_lightRenderer || !m_lightCalculatorService || !m_map) {
        return;
    }
    
    // Calculate visible range for lighting
    int minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ;
    calculateVisibleRange(minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ);
    
    // Only render lighting for current floor to avoid complexity
    RME::core::Position startPos(minMapX, minMapY, m_currentFloor);
    RME::core::Position endPos(maxMapX, maxMapY, m_currentFloor);
    
    // Calculate scroll offset for screen positioning
    int scrollX = static_cast<int>((m_viewCenterMapCoords.x() - width() / (2.0 * TILE_PIXEL_SIZE * m_zoomFactor)) * TILE_PIXEL_SIZE);
    int scrollY = static_cast<int>((m_viewCenterMapCoords.y() - height() / (2.0 * TILE_PIXEL_SIZE * m_zoomFactor)) * TILE_PIXEL_SIZE);
    
    // Update dynamic lights from items on the map
    updateDynamicLights();
    
    // Render lighting overlay
    m_lightRenderer->renderLighting(startPos, endPos, scrollX, scrollY, false);
}

void MapView::updateDynamicLights() {
    if (!m_lightCalculatorService || !m_map) {
        return;
    }
    
    // Clear existing dynamic lights
    m_lightCalculatorService->clearDynamicLights();
    
    // Calculate visible range
    int minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ;
    calculateVisibleRange(minMapX, maxMapX, minMapY, maxMapY, renderMinZ, renderMaxZ);
    
    // Add lights from items in visible area
    for (int z = renderMinZ; z <= renderMaxZ; ++z) {
        for (int y = minMapY; y <= maxMapY; ++y) {
            for (int x = minMapX; x <= maxMapX; ++x) {
                const RME::core::Tile* tile = m_map->getTile(RME::core::Position(x, y, z));
                if (!tile) {
                    continue;
                }
                
                // Check ground item for light
                const RME::core::Item* ground = tile->getGround();
                if (ground && ground->hasLight()) {
                    RME::core::Position lightPos(x, y, z);
                    QColor lightColor = QColor::fromHsv((ground->getLightColor() * 137) % 360, 128, 255);
                    uint8_t intensity = ground->getLightIntensity();
                    
                    RME::core::lighting::LightSource lightSource(lightPos, lightColor, intensity);
                    m_lightCalculatorService->addDynamicLight(lightSource);
                }
                
                // Check other items for light
                const auto& items = tile->getItems();
                for (const auto& item : items) {
                    if (item && item->hasLight()) {
                        RME::core::Position lightPos(x, y, z);
                        QColor lightColor = QColor::fromHsv((item->getLightColor() * 137) % 360, 128, 255);
                        uint8_t intensity = item->getLightIntensity();
                        
                        RME::core::lighting::LightSource lightSource(lightPos, lightColor, intensity);
                        m_lightCalculatorService->addDynamicLight(lightSource);
                    }
                }
            }
        }
    }
}

} // namespace widgets
} // namespace ui
} // namespace RME
