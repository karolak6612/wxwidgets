#pragma once // Using pragma once for include guard

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core> // Or your chosen version from RENDER-00, default 4.3
#include <QPointF>
#include <QMatrix4x4>
#include "core/Position.h" // Adjusted path for mapcore::Position
#include "../../editor_logic/EditorController.h" // Added
#include "../../core/settings/BrushSettings.h"   // Added
#include "../../core/map/Map.h"                  // Added for RENDER-02
#include "../../core/settings/AppSettings.h"    // Added for RENDER-02
#include "../../core/assets/AssetManager.h"      // Added for RENDER-02
#include "../../core/sprites/TextureManager.h"  // Added for RENDER-03
#include "../../core/lighting/LightCalculatorService.h"  // Added for RENDER-04
#include "../../core/lighting/LightRenderer.h"           // Added for RENDER-04

// Forward declarations for Qt event classes
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QTimer; // For potential animation/continuous update loop later
class QOpenGLShaderProgram; // For RENDER-02 shaders
class QOpenGLBuffer;         // For RENDER-02 VBO
class QOpenGLVertexArrayObject; // For RENDER-02 VAO

namespace RME {
namespace ui {
namespace widgets { // Assuming a namespace for UI widgets

class MapView : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);
    ~MapView() override;

    RME::core::Position screenToMapCoords(const QPoint& screenPos) const;
    QPoint mapCoordsToScreen(const RME::core::Position& mapPos) const;

    int getCurrentFloor() const { return m_currentFloor; }
    qreal getZoomFactor() const { return m_zoomFactor; }
    QPointF getViewCenterMapCoords() const { return m_viewCenterMapCoords; }

    void setEditorController(RME::editor_logic::EditorController* controller) { m_editorController = controller; }
    
    // RENDER-02: Dependency injection methods
    void setMap(RME::core::Map* map) { m_map = map; }
    void setAppSettings(RME::core::settings::AppSettings* settings) { m_appSettings = settings; }
    void setAssetManager(RME::core::assets::AssetManager* assetManager) { m_assetManager = assetManager; }
    
    // RENDER-03: Sprite rendering dependency
    void setTextureManager(RME::core::sprites::TextureManager* textureManager) { m_textureManager = textureManager; }
    
    // RENDER-04: Lighting system dependencies
    void setLightCalculatorService(RME::core::lighting::LightCalculatorService* lightService) { m_lightCalculatorService = lightService; }
    void setLightRenderer(RME::core::lighting::LightRenderer* lightRenderer) { m_lightRenderer = lightRenderer; }

public slots:
    void setCurrentFloor(int floor);
    void updateCurrentBrushSettings(const RME::core::BrushSettings& settings); // Added
    void floorUp();
    void floorDown();
    void setViewCenter(const RME::core::Position& mapPos); // Sets center and floor
    void setZoom(qreal zoom); // Sets zoom factor directly

signals:
    void viewChanged(); // Emitted after pan, zoom, or floor change that affects the view
    void mapPositionClicked(const RME::core::Position& mapPos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void mapPositionDoubleClicked(const RME::core::Position& mapPos, Qt::MouseButton button);
    // void currentFloorChanged(int newFloor); // More specific signal if needed

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override; // Added from boilerplate
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void updateProjectionMatrix();
    // void updateViewMatrix(); // Not using a separate view matrix for now, pan incorporated in projection logic
    
    // RENDER-02: Private methods
    bool initializeShaders();
    void cleanupShaders();
    void calculateVisibleRange(int& minMapX, int& maxMapX, int& minMapY, int& maxMapY, 
                              int& renderMinZ, int& renderMaxZ) const;
    QColor determineTileColor(const RME::core::Tile* tile) const;
    QColor getMinimapColorForItem(uint16_t itemId) const;
    float calculateFloorAlpha(int tileZ) const;
    void renderTiles();
    void renderGrid();
    void renderTileHighlights();
    
    // RENDER-03: Sprite rendering methods
    void renderSprites();
    void drawTexturedQuad(float x, float y, float width, float height, GLuint textureId);
    void drawStackedItems(const QPoint& screenPos, const RME::core::Tile* tile, float alpha);
    int getCurrentAnimationFrame(quint32 spriteId) const;
    
    // RENDER-04: Lighting rendering methods
    void renderLightingEffects();
    void updateDynamicLights();

    int m_currentFloor;
    qreal m_zoomFactor;
    QPointF m_viewCenterMapCoords; // Center of the view in map *tile* coordinates

    QPoint m_lastPanMousePos;
    bool m_isPanning;

    QMatrix4x4 m_projectionMatrix;

    // Added for bounding box selection
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::BrushSettings m_currentBrushSettings;
    bool m_isPerformingBoundingBoxSelection = false;
    QPoint m_dragStartScreenPoint;
    QPoint m_currentDragScreenPoint;
    
    // RENDER-02: Dependencies
    RME::core::Map* m_map = nullptr;
    RME::core::settings::AppSettings* m_appSettings = nullptr;
    RME::core::assets::AssetManager* m_assetManager = nullptr;
    
    // RENDER-03: Sprite rendering
    RME::core::sprites::TextureManager* m_textureManager = nullptr;
    
    // RENDER-04: Lighting system
    RME::core::lighting::LightCalculatorService* m_lightCalculatorService = nullptr;
    RME::core::lighting::LightRenderer* m_lightRenderer = nullptr;
    
    // Animation timing
    mutable QHash<quint32, int> m_spriteAnimationFrames; // Sprite ID -> current frame
    mutable qint64 m_lastAnimationUpdate = 0;
    
    // RENDER-02: Shader infrastructure
    QOpenGLShaderProgram* m_colorQuadShader = nullptr;
    QOpenGLBuffer* m_quadVBO = nullptr;
    QOpenGLVertexArrayObject* m_quadVAO = nullptr;

    // Constants from boilerplate
    // These should ideally be configurable or obtained from a settings manager eventually
    const int TILE_PIXEL_SIZE = 32;
    const int MAX_Z = 15;           // Max floor index (0-15 for typical Tibia maps)
    const int MIN_Z = 0;            // Min floor index
    const qreal MIN_ZOOM = 0.125;   // Min zoom factor
    const qreal MAX_ZOOM = 8.0;     // Max zoom factor
    const qreal ZOOM_STEP_MULTIPLIER = 1.12; // Multiplier for each zoom step
};

} // namespace widgets
} // namespace ui
} // namespace RME
