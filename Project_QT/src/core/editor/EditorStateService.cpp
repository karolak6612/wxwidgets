#include "EditorStateService.h"
#include "core/map/Map.h"

namespace RME {
namespace core {
namespace editor {

EditorStateService::EditorStateService(QObject* parent)
    : QObject(parent)
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

} // namespace editor
} // namespace core
} // namespace RME