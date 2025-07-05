#ifndef RME_MAIN_TOOLBAR_H
#define RME_MAIN_TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

// Forward declarations
namespace RME {
namespace editor_logic {
    class EditorController;
}
namespace core {
namespace brush {
    class BrushIntegrationManager;
}
}
}

namespace RME {
namespace ui {

/**
 * @brief Main toolbar for the RME application
 * 
 * This toolbar provides quick access to commonly used tools and operations
 * including file operations, editing tools, view controls, and brush selection.
 */
class MainToolBar : public QToolBar {
    Q_OBJECT

public:
    explicit MainToolBar(QWidget* parent = nullptr);
    ~MainToolBar() override = default;

    // Integration with editor controller
    void setEditorController(RME::editor_logic::EditorController* controller);
    void setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager);

    // State updates
    void updateToolStates();
    void updateZoomLevel(int zoomLevel);
    void updateFloorLevel(int floor);

public slots:
    void onMapStateChanged();
    void onSelectionChanged();
    void onToolModeChanged(int toolMode);

private slots:
    // File operations
    void onNewMapClicked();
    void onOpenMapClicked();
    void onSaveMapClicked();
    
    // Edit operations
    void onUndoClicked();
    void onRedoClicked();
    void onCutClicked();
    void onCopyClicked();
    void onPasteClicked();
    
    // Tool selection
    void onSelectToolClicked();
    void onBrushToolClicked();
    void onHouseExitToolClicked();
    void onWaypointToolClicked();
    
    // View controls
    void onZoomInClicked();
    void onZoomOutClicked();
    void onZoomNormalClicked();
    void onFloorUpClicked();
    void onFloorDownClicked();
    void onFloorChanged(int floor);
    
    // Map operations
    void onBorderizeMapClicked();
    void onRandomizeMapClicked();
    void onValidateGroundsClicked();

signals:
    void newMapRequested();
    void openMapRequested();
    void saveMapRequested();
    void toolModeChangeRequested(int toolMode);
    void zoomChangeRequested(int zoomLevel);
    void floorChangeRequested(int floor);

private:
    // Core integration
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::brush::BrushIntegrationManager* m_brushManager = nullptr;
    
    // File operations section
    QAction* m_newAction = nullptr;
    QAction* m_openAction = nullptr;
    QAction* m_saveAction = nullptr;
    
    // Edit operations section
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;
    QAction* m_cutAction = nullptr;
    QAction* m_copyAction = nullptr;
    QAction* m_pasteAction = nullptr;
    
    // Tool selection section
    QActionGroup* m_toolGroup = nullptr;
    QAction* m_selectToolAction = nullptr;
    QAction* m_brushToolAction = nullptr;
    QAction* m_houseExitToolAction = nullptr;
    QAction* m_waypointToolAction = nullptr;
    
    // View controls section
    QAction* m_zoomInAction = nullptr;
    QAction* m_zoomOutAction = nullptr;
    QAction* m_zoomNormalAction = nullptr;
    QSpinBox* m_floorSpinBox = nullptr;
    QLabel* m_floorLabel = nullptr;
    
    // Map operations section
    QAction* m_borderizeMapAction = nullptr;
    QAction* m_randomizeMapAction = nullptr;
    QAction* m_validateGroundsAction = nullptr;
    
    // Status indicators
    QLabel* m_statusLabel = nullptr;
    
    // Helper methods
    void createActions();
    void createToolGroups();
    void setupLayout();
    void connectSignals();
    QAction* createAction(const QString& text, const QString& iconName, const QString& tooltip);
    void addSeparatorWithSpacing();
};

} // namespace ui
} // namespace RME

#endif // RME_MAIN_TOOLBAR_H