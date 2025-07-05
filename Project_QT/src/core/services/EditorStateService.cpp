#include "EditorStateService.h"
#include "editor_logic/EditorController.h"
#include <QDebug>

namespace RME {
namespace core {

EditorStateService::EditorStateService(QObject* parent)
    : IEditorStateService(parent)
    , m_editorMode(EditorMode::Drawing)
    , m_currentFloor(7) // Ground floor
    , m_activeEditorSession(nullptr)
    , m_zoomLevel(1.0f)
    , m_viewPosition(0, 0)
    , m_showGrid(false)
    , m_showCreatures(true)
    , m_showSpawns(true)
    , m_showHouses(true)
{
    qDebug() << "EditorStateService: Initialized with default settings";
}

EditorStateService::~EditorStateService()
{
    // Note: We don't own the EditorController, so we don't delete it
}

void EditorStateService::setEditorMode(EditorMode mode)
{
    if (m_editorMode != mode) {
        m_editorMode = mode;
        emit editorModeChanged(mode);
        qDebug() << "EditorStateService: Editor mode changed to" << static_cast<int>(mode);
    }
}

IEditorStateService::EditorMode EditorStateService::getEditorMode() const
{
    return m_editorMode;
}

void EditorStateService::setCurrentFloor(int floor)
{
    // Clamp floor to valid range (0-15)
    int clampedFloor = qMax(0, qMin(floor, 15));
    
    if (m_currentFloor != clampedFloor) {
        m_currentFloor = clampedFloor;
        emit currentFloorChanged(clampedFloor);
        qDebug() << "EditorStateService: Current floor changed to" << clampedFloor;
    }
}

int EditorStateService::getCurrentFloor() const
{
    return m_currentFloor;
}

void EditorStateService::setActiveEditorSession(EditorController* editor)
{
    if (m_activeEditorSession != editor) {
        m_activeEditorSession = editor;
        emit activeEditorChanged(editor);
        qDebug() << "EditorStateService: Active editor session changed to" 
                 << (editor ? "valid editor" : "null");
    }
}

EditorController* EditorStateService::getActiveEditorSession() const
{
    return m_activeEditorSession;
}

void EditorStateService::setZoomLevel(float zoom)
{
    // Clamp zoom to reasonable bounds
    float clampedZoom = qMax(0.1f, qMin(zoom, 10.0f));
    
    if (qAbs(m_zoomLevel - clampedZoom) > 0.001f) {
        m_zoomLevel = clampedZoom;
        emit zoomLevelChanged(clampedZoom);
        qDebug() << "EditorStateService: Zoom level changed to" << clampedZoom;
    }
}

float EditorStateService::getZoomLevel() const
{
    return m_zoomLevel;
}

void EditorStateService::setViewPosition(const QPoint& position)
{
    if (m_viewPosition != position) {
        m_viewPosition = position;
        emit viewPositionChanged(position);
        qDebug() << "EditorStateService: View position changed to" << position;
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
        qDebug() << "EditorStateService: Show grid" << (show ? "enabled" : "disabled");
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
        qDebug() << "EditorStateService: Show creatures" << (show ? "enabled" : "disabled");
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
        qDebug() << "EditorStateService: Show spawns" << (show ? "enabled" : "disabled");
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
        qDebug() << "EditorStateService: Show houses" << (show ? "enabled" : "disabled");
    }
}

bool EditorStateService::getShowHouses() const
{
    return m_showHouses;
}

} // namespace core
} // namespace RME