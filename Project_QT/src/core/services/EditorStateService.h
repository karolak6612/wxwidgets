#ifndef RME_EDITORSTATESERVICE_H
#define RME_EDITORSTATESERVICE_H

#include "IEditorStateService.h"

namespace RME {
namespace core {

class EditorController;

/**
 * @brief Concrete implementation of IEditorStateService
 * 
 * This class manages the state of the editor including current mode,
 * floor, zoom level, view position, and active editor session.
 */
class EditorStateService : public IEditorStateService {
    Q_OBJECT

public:
    explicit EditorStateService(QObject* parent = nullptr);
    ~EditorStateService() override;

    // Editor mode
    void setEditorMode(EditorMode mode) override;
    EditorMode getEditorMode() const override;
    
    // Floor management
    void setCurrentFloor(int floor) override;
    int getCurrentFloor() const override;
    
    // Editor session
    void setActiveEditorSession(EditorController* editor) override;
    EditorController* getActiveEditorSession() const override;
    
    // Zoom level
    void setZoomLevel(float zoom) override;
    float getZoomLevel() const override;
    
    // View position
    void setViewPosition(const QPoint& position) override;
    QPoint getViewPosition() const override;
    
    // View state
    void setShowGrid(bool show) override;
    bool getShowGrid() const override;
    
    void setShowCreatures(bool show) override;
    bool getShowCreatures() const override;
    
    void setShowSpawns(bool show) override;
    bool getShowSpawns() const override;
    
    void setShowHouses(bool show) override;
    bool getShowHouses() const override;

private:
    EditorMode m_editorMode;
    int m_currentFloor;
    EditorController* m_activeEditorSession;
    float m_zoomLevel;
    QPoint m_viewPosition;
    bool m_showGrid;
    bool m_showCreatures;
    bool m_showSpawns;
    bool m_showHouses;
};

} // namespace core
} // namespace RME

#endif // RME_EDITORSTATESERVICE_H