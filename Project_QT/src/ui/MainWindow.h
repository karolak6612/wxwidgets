#pragma once // Using pragma once for include guard

#include <QMainWindow>
#include <QMap>     // For storing actions
#include <QList>    // For QList<QAction*> m_recentFileActions

#include "widgets/MapView.h" // For RME::ui::widgets::MapView
#include "EditorInstanceWidget.h" // For EditorInstanceWidget

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IEditorStateService.h"
#include "core/services/IClientDataService.h"
#include "core/services/IWindowManagerService.h"
#include "core/services/IApplicationSettingsService.h"

// Forward declarations for RME classes
namespace RME {
namespace editor_logic {
    class EditorController;
}
namespace core {
namespace brush {
    class BrushIntegrationManager;
    class BrushStateService;
}
    class EditorStateService;
    class ClientDataService;
    class WindowManagerService;
    class ApplicationSettingsService;
    class ServiceContainer;
}
namespace ui {
    class DockManager;
}
}

// Forward declarations for Qt classes
class QAction;
class QMenu;
class QSettings;
class QCloseEvent;
class QXmlStreamReader; // For argument type in private method
class QMenuBar;         // For argument type in private method
class QTabWidget;       // For editor tab widget


namespace RME {
namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override; // Destructor should be implemented if m_settings is owned raw pointer

    void addRecentFile(const QString& filePath);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPlaceholderActionTriggered();
    
    // File menu actions
    void onNewMap();
    void onOpenMap();
    void onSaveMap();
    void onSaveMapAs();
    void onCloseMap();
    void onImportMap();
    void onExportMap();
    void onExportMinimap();
    void onRecentFile();
    void onExit();
    
    // Edit menu actions
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onSelectAll();
    void onClearSelection();
    void onDelete();
    void onPreferences();
    
    // View operations (FINAL-04)
    void onZoomIn();
    void onZoomOut();
    void onZoomNormal();
    void onFloorUp();
    void onFloorDown();
    void onSetFloor(int floor);
    void onToggleGrid();
    void onToggleCreatures();
    void onToggleSpawns();
    void onToggleHouses();
    void onToggleLights();
    void onToggleTooltips();
    
    // Tools operations (FINAL-04)
    void onMapProperties();
    void onFindItem();
    
    // Live collaboration operations (FINAL-05)
    void onHostServer();
    void onConnectToServer();
    void onDisconnectFromServer();
    
    // Help operations (FINAL-06)
    void onAbout();
    
    // Map menu actions
    void onBorderizeMap();
    void onRandomizeMap();
    void onClearInvalidHouseTiles();
    void onClearModifiedTileState();
    void onValidateGrounds();
    void onBorderizeSelection();
    void onRandomizeSelection();
    void onMoveSelection();
    void onResizeMap();
    void onMapProperties();
    
    // Search menu actions
    void onFindItem();
    void onFindCreature();
    void onSearchOnMap();
    void onSearchOnSelection();
    void onGoToPosition();
    
    // View menu actions
    void onZoomIn();
    void onZoomOut();
    void onZoomNormal();
    void onZoomFit();
    void onFloorUp();
    void onFloorDown();
    void onGoToFloor();
    void onShowGrid();
    void onShowCreatures();
    void onShowSpawns();
    void onShowHouses();
    void onShowWaypoints();
    void onShowItemPalette();
    void onShowCreaturePalette();
    void onShowHousePalette();
    void onShowWaypointPalette();
    void onShowPropertiesPanel();
    void onShowMinimap();
    
    // Tools menu actions
    void onSelectTool();
    void onBrushTool();
    void onHouseExitTool();
    void onWaypointTool();
    void onSpawnTool();
    
    // Help menu actions
    void onAbout();
    void onAboutQt();
    void onHelp();
    void onCheckUpdates(); // Placeholder for all menu actions initially
    
    // Brush & Material Editor actions
    void onBrushMaterialEditor();
    void onNewTileset();
    void onAddItemToTileset();
    void openRecentFile();             // Slot for dynamic recent file actions
    void updateMenus();                // Slot to update enabled/checked state of actions
    
    // Editor tab management
    void onActiveEditorTabChanged(int index);
    void onEditorTabCloseRequested(int index);
    void onEditorModificationChanged(bool modified);
    void onEditorDisplayNameChanged(const QString& name);

private:
    void createMenusFromXML(const QString& xmlFilePath);
    void parseMenuNode(QXmlStreamReader& xml, QMenu* parentMenu, QMenuBar* menuBarInstance); // Helper for XML parsing

    void loadWindowSettings();
    void saveWindowSettings();
    void updateRecentFilesMenu();
    void connectMapViewActions(); // Connect actions to MapView slots
    void connectBrushMaterialActions(); // Connect brush/material editor actions
    void connectEditActions(); // Connect edit menu actions (FINAL-03)
    void connectViewActions(); // Connect view menu actions (FINAL-04)
    void connectLiveActions(); // Connect live collaboration actions (FINAL-05)

    // QMenuBar* m_menuBar; // QMainWindow has one implicitly via menuBar()
    QStatusBar* m_statusBar = nullptr; // Initialized in constructor
    QSettings* m_settings = nullptr;   // Initialized in constructor, owned by this class

    QMap<QString, QAction*> m_actions; // Store actions by their XML "action" name for easy access
    QMenu* m_recentFilesMenu = nullptr;  // Pointer to the 'Recent Files' QMenu

    // MaxRecentFiles should be a static const int or similar
    // For simplicity, using a const int member, or it can be defined in .cpp
    static const int MaxRecentFiles = 10;
    QList<QAction*> m_recentFileActions; // To keep track of dynamically created recent file actions for easy clearing

    RME::ui::widgets::MapView* m_mapView = nullptr; // The MapView instance
    
    // Core integration
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::brush::BrushIntegrationManager* m_brushIntegrationManager = nullptr;
    
    // Service architecture
    RME::core::ServiceContainer* m_serviceContainer = nullptr;
    RME::core::brush::BrushStateService* m_brushStateService = nullptr;
    RME::core::EditorStateService* m_editorStateService = nullptr;
    RME::core::ClientDataService* m_clientDataService = nullptr;
    RME::core::WindowManagerService* m_windowManagerService = nullptr;
    RME::core::ApplicationSettingsService* m_applicationSettingsService = nullptr;
    
    // UI components
    class MainToolBar* m_mainToolBar = nullptr;
    class DockManager* m_dockManager = nullptr;
    
    // Live collaboration components (FINAL-05)
    RME::ui::widgets::LiveCollaborationPanel* m_liveCollaborationPanel = nullptr;
    RME::network::QtLiveClient* m_liveClient = nullptr;
    
    // Helper methods
    void createEditorController();
    void connectEditorController();
    void updateMenuStatesFromEditor();
    void updateWindowTitle();
    void createToolBar();
    void createDockManager();
    void createLiveCollaboration(); // Create live collaboration components (FINAL-05)
    
    // Service management
    void initializeServices();
    void connectServices();
    void cleanupServices();
    void testBasicServiceFunctionality();
};

} // namespace ui
} // namespace RME
