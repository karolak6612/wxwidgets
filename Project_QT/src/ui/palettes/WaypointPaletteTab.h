#ifndef RME_WAYPOINT_PALETTE_TAB_H
#define RME_WAYPOINT_PALETTE_TAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>

// Forward declarations
namespace RME {
namespace core {
    namespace waypoints { 
        class WaypointManager; 
        class Waypoint;
    }
    namespace brush { class BrushStateManager; }
    namespace editor { class EditorControllerInterface; }
    class Position;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Waypoint palette tab for the main palette system
 * 
 * Provides UI for managing waypoints including adding, removing, renaming,
 * and selecting waypoints for navigation and brush operations.
 */
class WaypointPaletteTab : public QWidget {
    Q_OBJECT

public:
    explicit WaypointPaletteTab(QWidget* parent = nullptr);
    ~WaypointPaletteTab() override = default;

    // Integration with core systems
    void setWaypointManager(RME::core::waypoints::WaypointManager* waypointManager);
    void setBrushStateManager(RME::core::brush::BrushStateManager* brushManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);

    // Public interface
    void refreshContent();
    void reloadWaypoints();
    QString getSelectedWaypointName() const;

public slots:
    void onWaypointSelectionChanged();
    void onWaypointItemChanged(QListWidgetItem* item);
    void onAddWaypoint();
    void onRemoveWaypoint();

signals:
    void waypointSelected(const QString& waypointName);
    void waypointActivated(const QString& waypointName);
    void navigateToWaypoint(const RME::core::Position& position);

private:
    void setupUI();
    void connectSignals();
    void updateWaypointList();
    void updateBrushState();
    void selectWaypointInList(const QString& waypointName);
    
    // Helper methods
    QString generateUniqueWaypointName() const;
    bool validateWaypointName(const QString& name, const QString& originalName = QString()) const;
    QList<QString> getSelectedWaypointNames() const;

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QListWidget* m_waypointList = nullptr;
    QHBoxLayout* m_buttonLayout = nullptr;
    QPushButton* m_addWaypointButton = nullptr;
    QPushButton* m_removeWaypointButton = nullptr;

    // Core system integration
    RME::core::waypoints::WaypointManager* m_waypointManager = nullptr;
    RME::core::brush::BrushStateManager* m_brushStateManager = nullptr;
    RME::core::editor::EditorControllerInterface* m_editorController = nullptr;

    // State
    bool m_updatingUI = false;
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_WAYPOINT_PALETTE_TAB_H