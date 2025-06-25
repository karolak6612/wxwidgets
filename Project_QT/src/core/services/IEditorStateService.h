#ifndef RME_IEDITORSTATESERVICE_H
#define RME_IEDITORSTATESERVICE_H

#include <QObject>
#include <QPoint>

namespace RME {
namespace core {

class EditorController;

/**
 * @brief Interface for editor state management service
 * 
 * This interface defines the contract for managing editor state including
 * editor mode, floor, zoom level, view position, and active editor session.
 */
class IEditorStateService : public QObject {
    Q_OBJECT

public:
    enum class EditorMode {
        Drawing,
        Selection,
        Pasting,
        Filling
    };

    virtual ~IEditorStateService() = default;

    // Editor mode
    virtual void setEditorMode(EditorMode mode) = 0;
    virtual EditorMode getEditorMode() const = 0;
    
    // Floor management
    virtual void setCurrentFloor(int floor) = 0;
    virtual int getCurrentFloor() const = 0;
    
    // Editor session
    virtual void setActiveEditorSession(EditorController* editor) = 0;
    virtual EditorController* getActiveEditorSession() const = 0;
    
    // Zoom level
    virtual void setZoomLevel(float zoom) = 0;
    virtual float getZoomLevel() const = 0;
    
    // View position
    virtual void setViewPosition(const QPoint& position) = 0;
    virtual QPoint getViewPosition() const = 0;
    
    // View state
    virtual void setShowGrid(bool show) = 0;
    virtual bool getShowGrid() const = 0;
    
    virtual void setShowCreatures(bool show) = 0;
    virtual bool getShowCreatures() const = 0;
    
    virtual void setShowSpawns(bool show) = 0;
    virtual bool getShowSpawns() const = 0;
    
    virtual void setShowHouses(bool show) = 0;
    virtual bool getShowHouses() const = 0;

signals:
    void editorModeChanged(EditorMode mode);
    void currentFloorChanged(int floor);
    void activeEditorChanged(EditorController* editor);
    void zoomLevelChanged(float zoom);
    void viewPositionChanged(const QPoint& position);
    void showGridChanged(bool show);
    void showCreaturesChanged(bool show);
    void showSpawnsChanged(bool show);
    void showHousesChanged(bool show);
};

} // namespace core
} // namespace RME

#endif // RME_IEDITORSTATESERVICE_H