#ifndef RME_DOCK_MANAGER_H
#define RME_DOCK_MANAGER_H

#include <QObject>
#include <QHash>
#include <QMainWindow>
#include <QDockWidget>
#include <QAction>

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
namespace ui {
namespace palettes {
    class BasePalettePanel;
    class ItemPalettePanel;
    class CreaturePalettePanel;
    class HousePalettePanel;
    class WaypointPalettePanel;
    class PropertiesPanel;
    class MinimapPanel;
}
}
}

namespace RME {
namespace ui {

/**
 * @brief Manages dock panels for the main window
 * 
 * This class handles creation, management, and state persistence of all
 * dock panels in the application including palettes, properties panel,
 * and minimap.
 */
class DockManager : public QObject {
    Q_OBJECT

public:
    enum class DockPanelType {
        ItemPalette,
        CreaturePalette,
        HousePalette,
        WaypointPalette,
        Properties,
        Minimap
    };

    explicit DockManager(QMainWindow* mainWindow, QObject* parent = nullptr);
    ~DockManager() override = default;

    // Integration with editor system
    void setEditorController(RME::editor_logic::EditorController* controller);
    void setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager);

    // Dock panel management
    void createDockPanels();
    void showDockPanel(DockPanelType type, bool show = true);
    void hideDockPanel(DockPanelType type);
    void toggleDockPanel(DockPanelType type);
    bool isDockPanelVisible(DockPanelType type) const;

    // Dock panel access
    palettes::ItemPalettePanel* getItemPalette() const { return m_itemPalette; }
    palettes::CreaturePalettePanel* getCreaturePalette() const { return m_creaturePalette; }
    palettes::HousePalettePanel* getHousePalette() const { return m_housePalette; }
    palettes::WaypointPalettePanel* getWaypointPalette() const { return m_waypointPalette; }
    palettes::PropertiesPanel* getPropertiesPanel() const { return m_propertiesPanel; }
    palettes::MinimapPanel* getMinimapPanel() const { return m_minimapPanel; }

    // State management
    void saveDockLayout();
    void loadDockLayout();
    void resetDockLayout();

    // Menu integration
    void createDockMenuActions(QMenu* viewMenu);
    void updateDockMenuActions();

public slots:
    void onDockVisibilityChanged(bool visible);
    void onDockLocationChanged(Qt::DockWidgetArea area);
    void onDockFloatingChanged(bool floating);

signals:
    void dockPanelVisibilityChanged(DockPanelType type, bool visible);
    void dockLayoutChanged();

private:
    QMainWindow* m_mainWindow = nullptr;
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::brush::BrushIntegrationManager* m_brushManager = nullptr;

    // Dock panels
    palettes::ItemPalettePanel* m_itemPalette = nullptr;
    palettes::CreaturePalettePanel* m_creaturePalette = nullptr;
    palettes::HousePalettePanel* m_housePalette = nullptr;
    palettes::WaypointPalettePanel* m_waypointPalette = nullptr;
    palettes::PropertiesPanel* m_propertiesPanel = nullptr;
    palettes::MinimapPanel* m_minimapPanel = nullptr;

    // Menu actions
    QHash<DockPanelType, QAction*> m_dockActions;

    // Helper methods
    void createItemPalette();
    void createCreaturePalette();
    void createHousePalette();
    void createWaypointPalette();
    void createPropertiesPanel();
    void createMinimapPanel();

    void setupDockPanel(QDockWidget* dock, DockPanelType type, Qt::DockWidgetArea defaultArea);
    void connectDockSignals(QDockWidget* dock, DockPanelType type);

    QString getDockPanelName(DockPanelType type) const;
    QString getDockPanelTitle(DockPanelType type) const;
    Qt::DockWidgetArea getDefaultDockArea(DockPanelType type) const;
};

} // namespace ui
} // namespace RME

#endif // RME_DOCK_MANAGER_H