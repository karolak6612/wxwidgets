#ifndef RME_BRUSH_INTEGRATION_MANAGER_H
#define RME_BRUSH_INTEGRATION_MANAGER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <memory>

// Forward declarations
namespace RME {
namespace core {
    namespace brush { 
        class Brush; 
        class HouseBrush;
        class HouseExitBrush;
        class WaypointBrush;
    }
    namespace editor { class EditorControllerInterface; }
    namespace houses { class HouseData; }
    namespace waypoints { class Waypoint; }
}
}

namespace RME {
namespace core {
namespace brush {

/**
 * @brief Manages brush integration with UI components and tool modes
 * 
 * This class handles the integration between brushes, UI palette selections,
 * and the EditorController tool modes. It provides a centralized way to
 * manage brush activation and configuration based on UI interactions.
 */
class BrushIntegrationManager : public QObject {
    Q_OBJECT

public:
    explicit BrushIntegrationManager(RME::core::editor::EditorControllerInterface* editorController,
                                   QObject* parent = nullptr);
    ~BrushIntegrationManager() override = default;

    // Brush activation methods
    void activateHouseBrush(quint32 houseId);
    void activateHouseExitTool(quint32 houseId);
    void activateWaypointTool(const QString& waypointName);
    void activateRegularBrush(RME::core::brush::Brush* brush);

    // Current state queries
    bool isHouseBrushActive() const;
    bool isHouseExitToolActive() const;
    bool isWaypointToolActive() const;
    quint32 getCurrentHouseId() const;
    QString getCurrentWaypointName() const;

    // Brush instance management
    RME::core::brush::HouseBrush* getHouseBrush();
    RME::core::brush::HouseExitBrush* getHouseExitBrush();
    RME::core::brush::WaypointBrush* getWaypointBrush();

public slots:
    // Slots for UI integration
    void onHouseSelected(quint32 houseId);
    void onHouseDeselected();
    void onWaypointSelected(const QString& waypointName);
    void onWaypointDeselected();
    void onBrushModeRequested();

signals:
    // Signals for UI feedback
    void brushActivated(RME::core::brush::Brush* brush);
    void toolModeChanged(int toolMode); // EditorController::ToolMode as int
    void houseToolConfigured(quint32 houseId);
    void waypointToolConfigured(const QString& waypointName);

private:
    RME::core::editor::EditorControllerInterface* m_editorController;
    
    // Brush instances
    std::unique_ptr<RME::core::brush::HouseBrush> m_houseBrush;
    std::unique_ptr<RME::core::brush::HouseExitBrush> m_houseExitBrush;
    std::unique_ptr<RME::core::brush::WaypointBrush> m_waypointBrush;
    
    // Current state
    quint32 m_currentHouseId = 0;
    QString m_currentWaypointName;
    
    // Helper methods
    void initializeBrushes();
    void updateEditorControllerToolMode();
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_BRUSH_INTEGRATION_MANAGER_H