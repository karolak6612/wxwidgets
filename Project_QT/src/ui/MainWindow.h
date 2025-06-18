#pragma once // Using pragma once for include guard

#include <QMainWindow>
#include <QMap>     // For storing actions
#include <QList>    // For QList<QAction*> m_recentFileActions

#include "widgets/MapView.h" // For RME::ui::widgets::MapView
#include "EditorInstanceWidget.h" // For EditorInstanceWidget

// Forward declarations for RME classes
namespace RME {
namespace editor_logic {
    class EditorController;
}
namespace core {
namespace brush {
    class BrushIntegrationManager;
}
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

    // QMenuBar* m_menuBar; // QMainWindow has one implicitly via menuBar()
    QStatusBar* m_statusBar = nullptr; // Initialized in constructor
    QSettings* m_settings = nullptr;   // Initialized in constructor, owned by this class

    QMap<QString, QAction*> m_actions; // Store actions by their XML "action" name for easy access
    QMenu* m_recentFilesMenu = nullptr;  // Pointer to the 'Recent Files' QMenu

    // MaxRecentFiles should be a static const int or similar
    // For simplicity, using a const int member, or it can be defined in .cpp
    static const int MaxRecentFiles = 10;
    QList<QAction*> m_recentFileActions; // To keep track of dynamically created recent file actions for easy clearing

    // Editor tab management
    QTabWidget* m_editorTabWidget = nullptr;
    EditorInstanceWidget* m_currentEditorInstance = nullptr;
    
    // Core integration
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::brush::BrushIntegrationManager* m_brushIntegrationManager = nullptr;
    
    // UI components
    class MainToolBar* m_mainToolBar = nullptr;
    class DockManager* m_dockManager = nullptr;
    
    // Helper methods
    void createEditorController();
    void connectEditorController();
    void updateMenuStatesFromEditor();
    void createToolBar();
    void createDockManager();
    void setupEditorTabWidget();
    
    // Editor instance management
    EditorInstanceWidget* createNewEditorInstance(RME::core::Map* map, const QString& filePath);
    void addEditorTab(EditorInstanceWidget* editorInstance);
    void closeEditorTab(int index);
    EditorInstanceWidget* getEditorInstance(int index) const;
    EditorInstanceWidget* getCurrentEditorInstance() const;
    void updateWindowTitle();
    bool promptSaveChanges(EditorInstanceWidget* editorInstance);
};

} // namespace ui
} // namespace RME
