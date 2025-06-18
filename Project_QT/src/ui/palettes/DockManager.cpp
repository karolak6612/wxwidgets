#include "ui/palettes/DockManager.h"
#include "ui/palettes/ItemPalettePanel.h"
#include "ui/palettes/CreaturePalettePanel.h"
#include "ui/palettes/HousePalettePanel.h"
#include "ui/palettes/WaypointPalettePanel.h"
#include "ui/palettes/PropertiesPanel.h"
#include "ui/palettes/MinimapPanel.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QDebug>

namespace RME {
namespace ui {

DockManager::DockManager(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
    Q_ASSERT(m_mainWindow);
}

void DockManager::setEditorController(RME::editor_logic::EditorController* controller) {
    m_editorController = controller;
    
    // Update all existing dock panels
    if (m_itemPalette) {
        m_itemPalette->setEditorController(controller);
    }
    if (m_creaturePalette) {
        m_creaturePalette->setEditorController(controller);
    }
    if (m_housePalette) {
        m_housePalette->setEditorController(controller);
    }
    if (m_waypointPalette) {
        m_waypointPalette->setEditorController(controller);
    }
    if (m_propertiesPanel) {
        m_propertiesPanel->setEditorController(controller);
    }
    if (m_minimapPanel) {
        m_minimapPanel->setEditorController(controller);
    }
}

void DockManager::setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager) {
    m_brushManager = manager;
    
    // Update all existing dock panels
    if (m_itemPalette) {
        m_itemPalette->setBrushIntegrationManager(manager);
    }
    if (m_creaturePalette) {
        m_creaturePalette->setBrushIntegrationManager(manager);
    }
    if (m_housePalette) {
        m_housePalette->setBrushIntegrationManager(manager);
    }
    if (m_waypointPalette) {
        m_waypointPalette->setBrushIntegrationManager(manager);
    }
    if (m_propertiesPanel) {
        m_propertiesPanel->setBrushIntegrationManager(manager);
    }
    if (m_minimapPanel) {
        m_minimapPanel->setBrushIntegrationManager(manager);
    }
}

void DockManager::createDockPanels() {
    createItemPalette();
    createCreaturePalette();
    createHousePalette();
    createWaypointPalette();
    createPropertiesPanel();
    createMinimapPanel();
    
    qDebug() << "DockManager::createDockPanels: Created all dock panels";
}

void DockManager::createItemPalette() {
    if (m_itemPalette) {
        return;
    }
    
    m_itemPalette = new palettes::ItemPalettePanel();
    m_itemPalette->setEditorController(m_editorController);
    m_itemPalette->setBrushIntegrationManager(m_brushManager);
    
    setupDockPanel(m_itemPalette, DockPanelType::ItemPalette, Qt::LeftDockWidgetArea);
    connectDockSignals(m_itemPalette, DockPanelType::ItemPalette);
}

void DockManager::createCreaturePalette() {
    if (m_creaturePalette) {
        return;
    }
    
    m_creaturePalette = new palettes::CreaturePalettePanel();
    m_creaturePalette->setEditorController(m_editorController);
    m_creaturePalette->setBrushIntegrationManager(m_brushManager);
    
    setupDockPanel(m_creaturePalette, DockPanelType::CreaturePalette, Qt::LeftDockWidgetArea);
    connectDockSignals(m_creaturePalette, DockPanelType::CreaturePalette);
}

void DockManager::createHousePalette() {
    if (m_housePalette) {
        return;
    }
    
    // TODO: Create HousePalettePanel when implemented
    // m_housePalette = new palettes::HousePalettePanel();
    // m_housePalette->setEditorController(m_editorController);
    // m_housePalette->setBrushIntegrationManager(m_brushManager);
    // setupDockPanel(m_housePalette, DockPanelType::HousePalette, Qt::LeftDockWidgetArea);
    // connectDockSignals(m_housePalette, DockPanelType::HousePalette);
}

void DockManager::createWaypointPalette() {
    if (m_waypointPalette) {
        return;
    }
    
    // TODO: Create WaypointPalettePanel when implemented
    // m_waypointPalette = new palettes::WaypointPalettePanel();
    // m_waypointPalette->setEditorController(m_editorController);
    // m_waypointPalette->setBrushIntegrationManager(m_brushManager);
    // setupDockPanel(m_waypointPalette, DockPanelType::WaypointPalette, Qt::LeftDockWidgetArea);
    // connectDockSignals(m_waypointPalette, DockPanelType::WaypointPalette);
}

void DockManager::createPropertiesPanel() {
    if (m_propertiesPanel) {
        return;
    }
    
    // TODO: Create PropertiesPanel when implemented
    // m_propertiesPanel = new palettes::PropertiesPanel();
    // m_propertiesPanel->setEditorController(m_editorController);
    // m_propertiesPanel->setBrushIntegrationManager(m_brushManager);
    // setupDockPanel(m_propertiesPanel, DockPanelType::Properties, Qt::RightDockWidgetArea);
    // connectDockSignals(m_propertiesPanel, DockPanelType::Properties);
}

void DockManager::createMinimapPanel() {
    if (m_minimapPanel) {
        return;
    }
    
    // TODO: Create MinimapPanel when implemented
    // m_minimapPanel = new palettes::MinimapPanel();
    // m_minimapPanel->setEditorController(m_editorController);
    // m_minimapPanel->setBrushIntegrationManager(m_brushManager);
    // setupDockPanel(m_minimapPanel, DockPanelType::Minimap, Qt::RightDockWidgetArea);
    // connectDockSignals(m_minimapPanel, DockPanelType::Minimap);
}

void DockManager::setupDockPanel(QDockWidget* dock, DockPanelType type, Qt::DockWidgetArea defaultArea) {
    if (!dock || !m_mainWindow) {
        return;
    }
    
    dock->setObjectName(getDockPanelName(type));
    dock->setWindowTitle(getDockPanelTitle(type));
    
    // Set dock widget features
    dock->setFeatures(QDockWidget::DockWidgetMovable | 
                     QDockWidget::DockWidgetFloatable | 
                     QDockWidget::DockWidgetClosable);
    
    // Set allowed dock areas
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // Add to main window
    m_mainWindow->addDockWidget(defaultArea, dock);
    
    // Initially hide the dock (will be shown based on saved state or user action)
    dock->hide();
}

void DockManager::connectDockSignals(QDockWidget* dock, DockPanelType type) {
    if (!dock) {
        return;
    }
    
    connect(dock, &QDockWidget::visibilityChanged,
            this, &DockManager::onDockVisibilityChanged);
    connect(dock, &QDockWidget::dockLocationChanged,
            this, &DockManager::onDockLocationChanged);
    connect(dock, &QDockWidget::topLevelChanged,
            this, &DockManager::onDockFloatingChanged);
}

void DockManager::showDockPanel(DockPanelType type, bool show) {
    QDockWidget* dock = nullptr;
    
    switch (type) {
        case DockPanelType::ItemPalette:
            if (!m_itemPalette) createItemPalette();
            dock = m_itemPalette;
            break;
        case DockPanelType::CreaturePalette:
            if (!m_creaturePalette) createCreaturePalette();
            dock = m_creaturePalette;
            break;
        case DockPanelType::HousePalette:
            if (!m_housePalette) createHousePalette();
            dock = m_housePalette;
            break;
        case DockPanelType::WaypointPalette:
            if (!m_waypointPalette) createWaypointPalette();
            dock = m_waypointPalette;
            break;
        case DockPanelType::Properties:
            if (!m_propertiesPanel) createPropertiesPanel();
            dock = m_propertiesPanel;
            break;
        case DockPanelType::Minimap:
            if (!m_minimapPanel) createMinimapPanel();
            dock = m_minimapPanel;
            break;
    }
    
    if (dock) {
        dock->setVisible(show);
        if (show) {
            dock->raise();
        }
    }
}

void DockManager::hideDockPanel(DockPanelType type) {
    showDockPanel(type, false);
}

void DockManager::toggleDockPanel(DockPanelType type) {
    bool isVisible = isDockPanelVisible(type);
    showDockPanel(type, !isVisible);
}

bool DockManager::isDockPanelVisible(DockPanelType type) const {
    QDockWidget* dock = nullptr;
    
    switch (type) {
        case DockPanelType::ItemPalette:
            dock = m_itemPalette;
            break;
        case DockPanelType::CreaturePalette:
            dock = m_creaturePalette;
            break;
        case DockPanelType::HousePalette:
            dock = m_housePalette;
            break;
        case DockPanelType::WaypointPalette:
            dock = m_waypointPalette;
            break;
        case DockPanelType::Properties:
            dock = m_propertiesPanel;
            break;
        case DockPanelType::Minimap:
            dock = m_minimapPanel;
            break;
    }
    
    return dock && dock->isVisible();
}

void DockManager::saveDockLayout() {
    if (!m_mainWindow) {
        return;
    }
    
    QSettings settings;
    settings.setValue("DockManager/geometry", m_mainWindow->saveGeometry());
    settings.setValue("DockManager/state", m_mainWindow->saveState());
    
    // Save individual dock panel states
    if (m_itemPalette) {
        m_itemPalette->saveState();
    }
    if (m_creaturePalette) {
        m_creaturePalette->saveState();
    }
    if (m_housePalette) {
        m_housePalette->saveState();
    }
    if (m_waypointPalette) {
        m_waypointPalette->saveState();
    }
    if (m_propertiesPanel) {
        m_propertiesPanel->saveState();
    }
    if (m_minimapPanel) {
        m_minimapPanel->saveState();
    }
    
    qDebug() << "DockManager::saveDockLayout: Saved dock layout and panel states";
}

void DockManager::loadDockLayout() {
    if (!m_mainWindow) {
        return;
    }
    
    QSettings settings;
    QByteArray geometry = settings.value("DockManager/geometry").toByteArray();
    QByteArray state = settings.value("DockManager/state").toByteArray();
    
    if (!geometry.isEmpty()) {
        m_mainWindow->restoreGeometry(geometry);
    }
    if (!state.isEmpty()) {
        m_mainWindow->restoreState(state);
    }
    
    // Load individual dock panel states
    if (m_itemPalette) {
        m_itemPalette->loadState();
    }
    if (m_creaturePalette) {
        m_creaturePalette->loadState();
    }
    if (m_housePalette) {
        m_housePalette->loadState();
    }
    if (m_waypointPalette) {
        m_waypointPalette->loadState();
    }
    if (m_propertiesPanel) {
        m_propertiesPanel->loadState();
    }
    if (m_minimapPanel) {
        m_minimapPanel->loadState();
    }
    
    qDebug() << "DockManager::loadDockLayout: Loaded dock layout and panel states";
}

void DockManager::resetDockLayout() {
    if (!m_mainWindow) {
        return;
    }
    
    // Reset to default layout
    showDockPanel(DockPanelType::ItemPalette, true);
    showDockPanel(DockPanelType::CreaturePalette, false);
    showDockPanel(DockPanelType::HousePalette, false);
    showDockPanel(DockPanelType::WaypointPalette, false);
    showDockPanel(DockPanelType::Properties, true);
    showDockPanel(DockPanelType::Minimap, true);
    
    // Arrange docks in default positions
    if (m_itemPalette && m_creaturePalette) {
        m_mainWindow->tabifyDockWidget(m_itemPalette, m_creaturePalette);
    }
    
    qDebug() << "DockManager::resetDockLayout: Reset to default dock layout";
}

void DockManager::createDockMenuActions(QMenu* viewMenu) {
    if (!viewMenu) {
        return;
    }
    
    // Create actions for each dock panel
    QAction* itemPaletteAction = new QAction(tr("Item Palette"), this);
    itemPaletteAction->setCheckable(true);
    connect(itemPaletteAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::ItemPalette, checked);
    });
    m_dockActions[DockPanelType::ItemPalette] = itemPaletteAction;
    
    QAction* creaturePaletteAction = new QAction(tr("Creature Palette"), this);
    creaturePaletteAction->setCheckable(true);
    connect(creaturePaletteAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::CreaturePalette, checked);
    });
    m_dockActions[DockPanelType::CreaturePalette] = creaturePaletteAction;
    
    QAction* housePaletteAction = new QAction(tr("House Palette"), this);
    housePaletteAction->setCheckable(true);
    connect(housePaletteAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::HousePalette, checked);
    });
    m_dockActions[DockPanelType::HousePalette] = housePaletteAction;
    
    QAction* waypointPaletteAction = new QAction(tr("Waypoint Palette"), this);
    waypointPaletteAction->setCheckable(true);
    connect(waypointPaletteAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::WaypointPalette, checked);
    });
    m_dockActions[DockPanelType::WaypointPalette] = waypointPaletteAction;
    
    QAction* propertiesAction = new QAction(tr("Properties"), this);
    propertiesAction->setCheckable(true);
    connect(propertiesAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::Properties, checked);
    });
    m_dockActions[DockPanelType::Properties] = propertiesAction;
    
    QAction* minimapAction = new QAction(tr("Minimap"), this);
    minimapAction->setCheckable(true);
    connect(minimapAction, &QAction::triggered, [this](bool checked) {
        showDockPanel(DockPanelType::Minimap, checked);
    });
    m_dockActions[DockPanelType::Minimap] = minimapAction;
    
    // Add actions to menu
    viewMenu->addSeparator();
    viewMenu->addAction(itemPaletteAction);
    viewMenu->addAction(creaturePaletteAction);
    viewMenu->addAction(housePaletteAction);
    viewMenu->addAction(waypointPaletteAction);
    viewMenu->addSeparator();
    viewMenu->addAction(propertiesAction);
    viewMenu->addAction(minimapAction);
}

void DockManager::updateDockMenuActions() {
    for (auto it = m_dockActions.begin(); it != m_dockActions.end(); ++it) {
        DockPanelType type = it.key();
        QAction* action = it.value();
        
        bool isVisible = isDockPanelVisible(type);
        action->setChecked(isVisible);
    }
}

void DockManager::onDockVisibilityChanged(bool visible) {
    QDockWidget* dock = qobject_cast<QDockWidget*>(sender());
    if (!dock) {
        return;
    }
    
    // Find the dock type and emit signal
    DockPanelType type = DockPanelType::ItemPalette; // Default
    if (dock == m_itemPalette) type = DockPanelType::ItemPalette;
    else if (dock == m_creaturePalette) type = DockPanelType::CreaturePalette;
    else if (dock == m_housePalette) type = DockPanelType::HousePalette;
    else if (dock == m_waypointPalette) type = DockPanelType::WaypointPalette;
    else if (dock == m_propertiesPanel) type = DockPanelType::Properties;
    else if (dock == m_minimapPanel) type = DockPanelType::Minimap;
    
    emit dockPanelVisibilityChanged(type, visible);
    updateDockMenuActions();
}

void DockManager::onDockLocationChanged(Qt::DockWidgetArea area) {
    Q_UNUSED(area)
    emit dockLayoutChanged();
}

void DockManager::onDockFloatingChanged(bool floating) {
    Q_UNUSED(floating)
    emit dockLayoutChanged();
}

QString DockManager::getDockPanelName(DockPanelType type) const {
    switch (type) {
        case DockPanelType::ItemPalette: return "ItemPalette";
        case DockPanelType::CreaturePalette: return "CreaturePalette";
        case DockPanelType::HousePalette: return "HousePalette";
        case DockPanelType::WaypointPalette: return "WaypointPalette";
        case DockPanelType::Properties: return "Properties";
        case DockPanelType::Minimap: return "Minimap";
        default: return "UnknownDock";
    }
}

QString DockManager::getDockPanelTitle(DockPanelType type) const {
    switch (type) {
        case DockPanelType::ItemPalette: return tr("Items");
        case DockPanelType::CreaturePalette: return tr("Creatures");
        case DockPanelType::HousePalette: return tr("Houses");
        case DockPanelType::WaypointPalette: return tr("Waypoints");
        case DockPanelType::Properties: return tr("Properties");
        case DockPanelType::Minimap: return tr("Minimap");
        default: return tr("Unknown");
    }
}

Qt::DockWidgetArea DockManager::getDefaultDockArea(DockPanelType type) const {
    switch (type) {
        case DockPanelType::ItemPalette:
        case DockPanelType::CreaturePalette:
        case DockPanelType::HousePalette:
        case DockPanelType::WaypointPalette:
            return Qt::LeftDockWidgetArea;
        case DockPanelType::Properties:
        case DockPanelType::Minimap:
            return Qt::RightDockWidgetArea;
        default:
            return Qt::LeftDockWidgetArea;
    }
}

} // namespace ui
} // namespace RME