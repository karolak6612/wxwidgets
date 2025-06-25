#ifndef RME_EDITOR_STATE_SERVICE_H
#define RME_EDITOR_STATE_SERVICE_H

#include <QObject>
#include <QPoint>
#include "core/Position.h"
#include "services/IEditorStateService.h"

namespace RME {
namespace core {

class Map;
class EditorController;

namespace editor {

/**
 * @brief Service that provides information about the current editor state
 * 
 * This service is responsible for tracking and providing information about
 * the current state of the editor, such as the current map, floor, position,
 * zoom level, etc. It also emits signals when these values change.
 */
class EditorStateService : public IEditorStateService
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new Editor State Service
     * 
     * @param parent Parent QObject
     */
    explicit EditorStateService(QObject* parent = nullptr);
    
    // IEditorStateService implementation
    void setEditorMode(EditorMode mode) override;
    EditorMode getEditorMode() const override;
    
    void setCurrentFloor(int floor) override;
    int getCurrentFloor() const override;
    
    void setActiveEditorSession(EditorController* editor) override;
    EditorController* getActiveEditorSession() const override;
    
    void setZoomLevel(float zoom) override;
    float getZoomLevel() const override;
    
    void setViewPosition(const QPoint& position) override;
    QPoint getViewPosition() const override;
    
    void setShowGrid(bool show) override;
    bool getShowGrid() const override;
    
    void setShowCreatures(bool show) override;
    bool getShowCreatures() const override;
    
    void setShowSpawns(bool show) override;
    bool getShowSpawns() const override;
    
    void setShowHouses(bool show) override;
    bool getShowHouses() const override;
    
    /**
     * @brief Get the current map
     * 
     * @return Map* Pointer to the current map, or nullptr if no map is loaded
     */
    Map* getCurrentMap() const;
    
    /**
     * @brief Get the current floor
     * 
     * @return int Current floor index
     */
    int getCurrentFloor() const;
    
    /**
     * @brief Get the current position
     * 
     * @return Position Current position
     */
    Position getCurrentPosition() const;
    
    /**
     * @brief Get the current zoom level
     * 
     * @return double Current zoom level
     */
    double getCurrentZoom() const;
    
    /**
     * @brief Get the current view center
     * 
     * @return Position Center position of the current view
     */
    Position getViewCenter() const;
    
    /**
     * @brief Get the current view rectangle in map coordinates
     * 
     * @return QRectF View rectangle in map coordinates
     */
    QRectF getViewRect() const;
    
    /**
     * @brief Check if a map is currently loaded
     * 
     * @return true If a map is loaded
     * @return false If no map is loaded
     */
    bool hasMap() const;

public slots:
    /**
     * @brief Set the current map
     * 
     * @param map Pointer to the new map
     */
    void setCurrentMap(Map* map);
    
    /**
     * @brief Set the current floor
     * 
     * @param floor New floor index
     */
    void setCurrentFloor(int floor);
    
    /**
     * @brief Set the current position
     * 
     * @param position New position
     */
    void setCurrentPosition(const Position& position);
    
    /**
     * @brief Set the current zoom level
     * 
     * @param zoom New zoom level
     */
    void setCurrentZoom(double zoom);
    
    /**
     * @brief Set the view center
     * 
     * @param center New center position
     */
    void setViewCenter(const Position& center);
    
    /**
     * @brief Set the view rectangle in map coordinates
     * 
     * @param rect New view rectangle
     */
    void setViewRect(const QRectF& rect);

signals:
    /**
     * @brief Signal emitted when the current map changes
     * 
     * @param map Pointer to the new map
     */
    void mapChanged(Map* map);
    
    /**
     * @brief Signal emitted when the current floor changes
     * 
     * @param floor New floor index
     */
    void currentFloorChanged(int floor);
    
    /**
     * @brief Signal emitted when the current position changes
     * 
     * @param position New position
     */
    void currentPositionChanged(const Position& position);
    
    /**
     * @brief Signal emitted when the current zoom level changes
     * 
     * @param zoom New zoom level
     */
    void currentZoomChanged(double zoom);
    
    /**
     * @brief Signal emitted when the view center changes
     * 
     * @param center New center position
     */
    void viewCenterChanged(const Position& center);
    
    /**
     * @brief Signal emitted when the view rectangle changes
     * 
     * @param rect New view rectangle
     */
    void viewRectChanged(const QRectF& rect);
    
    /**
     * @brief Signal emitted when the view changes (center and zoom)
     * 
     * @param center New center position
     * @param zoom New zoom level
     */
    void viewChanged(const Position& center, double zoom);

private:
    Map* m_currentMap = nullptr;
    int m_currentFloor = 7; // Default floor
    Position m_currentPosition;
    double m_currentZoom = 1.0;
    Position m_viewCenter;
    QRectF m_viewRect;
    
    // Additional state for IEditorStateService
    EditorMode m_editorMode = EditorMode::Drawing;
    EditorController* m_activeEditorSession = nullptr;
    QPoint m_viewPosition;
    bool m_showGrid = true;
    bool m_showCreatures = true;
    bool m_showSpawns = true;
    bool m_showHouses = true;
};

} // namespace editor
} // namespace core
} // namespace RME

#endif // RME_EDITOR_STATE_SERVICE_H