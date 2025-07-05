#include "EditorStateService.h"
#include "core/map/Map.h"
#include "editor_logic/EditorController.h"

namespace RME {
namespace core {
namespace editor {

EditorStateService::EditorStateService(QObject* parent)
    : IEditorStateService(parent)
    , m_currentMap(nullptr)
    , m_currentFloor(7)
    , m_currentPosition(0, 0, 7)
    , m_currentZoom(1.0)
    , m_viewCenter(0, 0, 7)
    , m_viewRect(0, 0, 0, 0)
{
}

Map* EditorStateService::getCurrentMap() const
{
    return m_currentMap;
}

int EditorStateService::getCurrentFloor() const
{
    return m_currentFloor;
}

Position EditorStateService::getCurrentPosition() const
{
    return m_currentPosition;
}

double EditorStateService::getCurrentZoom() const
{
    return m_currentZoom;
}

Position EditorStateService::getViewCenter() const
{
    return m_viewCenter;
}

QRectF EditorStateService::getViewRect() const
{
    return m_viewRect;
}

bool EditorStateService::hasMap() const
{
    return m_currentMap != nullptr;
}

void EditorStateService::setCurrentMap(Map* map)
{
    if (m_currentMap != map) {
        m_currentMap = map;
        emit mapChanged(map);
    }
}

void EditorStateService::setCurrentFloor(int floor)
{
    if (m_currentFloor != floor) {
        m_currentFloor = floor;
        
        // Update position and view center to match the new floor
        m_currentPosition.z = floor;
        m_viewCenter.z = floor;
        
        emit currentFloorChanged(floor);
        emit currentPositionChanged(m_currentPosition);
        emit viewCenterChanged(m_viewCenter);
    }
}

void EditorStateService::setCurrentPosition(const Position& position)
{
    if (m_currentPosition != position) {
        m_currentPosition = position;
        
        // If the floor changed, update the current floor
        if (m_currentFloor != position.z) {
            m_currentFloor = position.z;
            emit currentFloorChanged(m_currentFloor);
        }
        
        emit currentPositionChanged(position);
    }
}

void EditorStateService::setCurrentZoom(double zoom)
{
    if (m_currentZoom != zoom) {
        m_currentZoom = zoom;
        emit currentZoomChanged(zoom);
        emit viewChanged(m_viewCenter, zoom);
    }
}

void EditorStateService::setViewCenter(const Position& center)
{
    if (m_viewCenter != center) {
        m_viewCenter = center;
        
        // If the floor changed, update the current floor
        if (m_currentFloor != center.z) {
            m_currentFloor = center.z;
            emit currentFloorChanged(m_currentFloor);
        }
        
        emit viewCenterChanged(center);
        emit viewChanged(center, m_currentZoom);
    }
}

void EditorStateService::setViewRect(const QRectF& rect)
{
    if (m_viewRect != rect) {
        m_viewRect = rect;
        emit viewRectChanged(rect);
    }
}

// IEditorStateService implementation
void EditorStateService::setEditorMode(EditorMode mode)
{
    if (m_editorMode != mode) {
        m_editorMode = mode;
        emit editorModeChanged(mode);
    }
}

IEditorStateService::EditorMode EditorStateService::getEditorMode() const
{
    return m_editorMode;
}

void EditorStateService::setActiveEditorSession(EditorController* editor)
{
    if (m_activeEditorSession != editor) {
        m_activeEditorSession = editor;
        emit activeEditorChanged(editor);
    }
}

EditorController* EditorStateService::getActiveEditorSession() const
{
    return m_activeEditorSession;
}

void EditorStateService::setZoomLevel(float zoom)
{
    setCurrentZoom(static_cast<double>(zoom));
    emit zoomLevelChanged(zoom);
}

float EditorStateService::getZoomLevel() const
{
    return static_cast<float>(getCurrentZoom());
}

void EditorStateService::setViewPosition(const QPoint& position)
{
    if (m_viewPosition != position) {
        m_viewPosition = position;
        emit viewPositionChanged(position);
    }
}

QPoint EditorStateService::getViewPosition() const
{
    return m_viewPosition;
}

void EditorStateService::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        emit showGridChanged(show);
    }
}

bool EditorStateService::getShowGrid() const
{
    return m_showGrid;
}

void EditorStateService::setShowCreatures(bool show)
{
    if (m_showCreatures != show) {
        m_showCreatures = show;
        emit showCreaturesChanged(show);
    }
}

bool EditorStateService::getShowCreatures() const
{
    return m_showCreatures;
}

void EditorStateService::setShowSpawns(bool show)
{
    if (m_showSpawns != show) {
        m_showSpawns = show;
        emit showSpawnsChanged(show);
    }
}

bool EditorStateService::getShowSpawns() const
{
    return m_showSpawns;
}

void EditorStateService::setShowHouses(bool show)
{
    if (m_showHouses != show) {
        m_showHouses = show;
        emit showHousesChanged(show);
    }
}

bool EditorStateService::getShowHouses() const
{
    return m_showHouses;
}

} // namespace editor
} // namespace core
} // namespace RME