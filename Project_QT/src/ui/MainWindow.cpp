#include "ui/MainWindow.h"
#include "ui/MainToolBar.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"
#include "core/map/Map.h" // Relative path to header

#include <QApplication> // For qApp global pointer if needed, or for QGuiApplication for screen info
#include <QScreen>      // For screen geometry to center window initially
#include <QSettings>
#include <QStatusBar>
#include <QMenuBar>     // For menuBar()
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QXmlStreamReader> // For stub argument types
#include <QFile>            // For stub argument types
#include <QFileInfo>        // Added for QFileInfo
#include <QKeySequence>     // Added for QKeySequence::fromString
#include <QDebug>           // For logging in stubs
#include <QTabWidget>       // For editor tab widget
#include <QMessageBox>      // For save confirmation dialogs

namespace RME {
namespace ui {

// Define static const member if not in header (or use anonymous namespace)
// const int MainWindow::MaxRecentFiles; // Already static const int in .h

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // Initialize QSettings
    // Parent 'this' makes Qt handle its deletion when MainWindow is destroyed.
    m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "RME-Qt", "Editor", this);

    setWindowTitle(tr("Remere's Map Editor - Qt"));

    // Set initial size and position (e.g., centered)
    // QScreen *screen = QGuiApplication::primaryScreen(); // Or qApp->primaryScreen()
    // if (screen) {
    //     QRect screenGeometry = screen->availableGeometry();
    //     int height = screenGeometry.height() * 2 / 3;
    //     int width = screenGeometry.width() * 2 / 3;
    //     resize(width, height);
    //     move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);
    // } else {
        resize(1024, 768); // Default size
    // }

    m_statusBar = statusBar(); // QMainWindow creates one by default
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Welcome to RME-Qt!"), 2000); // Message for 2 seconds
    }

    // Setup editor tab widget instead of single MapView
    setupEditorTabWidget();

    // Create and integrate editor controller
    createEditorController();
    
    // Create dock manager and dock panels
    createDockManager();

    // Placeholder calls, actual menu creation will be called after this
    createMenusFromXML(":/menubar.xml"); // Will be called after stubs are filled
    connectMapViewActions(); // Connect actions to MapView slots
    connectEditorController(); // Connect menu actions to editor controller
    connectBrushMaterialActions(); // Connect brush/material editor actions
    updateRecentFilesMenu();
    updateMenus();
    updateMenuStatesFromEditor();
    updateWindowTitle(); // Set initial window title

    loadWindowSettings(); // Load stored geometry and state
}

MainWindow::~MainWindow() {
    // m_settings is parented to 'this', Qt will delete it.
    // m_actions contains QAction pointers. If they are parented to menus/this, Qt handles them.
    // If not, or if added to m_recentFilesMenu and then menu cleared without deleting actions,
    // m_recentFileActions list can be used to delete them.
    // For now, assume Qt's parent-child cleanup is sufficient for QActions added to menus.
    // Clear the list to avoid dangling pointers if actions were manually deleted elsewhere (unlikely here).
    // qDeleteAll(m_recentFileActions); // This is important if actions are not parented to menu
    m_recentFileActions.clear();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Placeholder: Implement logic for "Do you want to save changes?" later.
    // For now, always accept closing and save settings.
    // if (isWindowModified()) { // QMainWindow has isWindowModified()
    //     QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Confirm Close"),
    //         tr("There are unsaved changes. Do you want to save before closing?"),
    //         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    //     if (res == QMessageBox::Save) {
    //         // saveFile(); // Placeholder for actual save
    //         event->accept();
    //     } else if (res == QMessageBox::Discard) {
    //         event->accept();
    //     } else {
    //         event->ignore();
    //         return;
    //     }
    // } else {
        event->accept();
    // }

    if (event->isAccepted()) {
        saveWindowSettings();
    }
}

void MainWindow::loadWindowSettings() {
    if (!m_settings) return;
    if (m_settings->contains("geometry")) {
        restoreGeometry(m_settings->value("geometry").toByteArray());
    }
    if (m_settings->contains("windowState")) {
        restoreState(m_settings->value("windowState").toByteArray());
    }
}

void MainWindow::saveWindowSettings() {
    if (!m_settings) return;
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
}

// --- Stubs for methods to be implemented later ---

void MainWindow::createMenusFromXML(const QString& xmlFilePath) {
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "MainWindow::createMenusFromXML: Could not open file" << xmlFilePath;
        if (m_statusBar) {
            m_statusBar->showMessage(tr("Error: Could not load menu definition."), 5000);
        }
        return;
    }

    QXmlStreamReader xml(&file);
    QMenuBar* menuBarInstance = menuBar(); // Get QMainWindow's menu bar

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == QLatin1String("menubar")) {
                // Found the root <menubar> element, start parsing its children (top-level menus)
                parseMenuNode(xml, nullptr, menuBarInstance);
                // Assuming only one <menubar> root element
                break;
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "MainWindow::createMenusFromXML: XML parsing error:" << xml.errorString()
                   << "at line" << xml.lineNumber() << "column" << xml.columnNumber();
        if (m_statusBar) {
            m_statusBar->showMessage(tr("Error: Could not parse menu definition."), 5000);
        }
    }
    file.close();
}

void MainWindow::parseMenuNode(QXmlStreamReader& xml, QMenu* parentMenu, QMenuBar* menuBarInstance) {
    // This function is recursive. It's called with either a parentMenu or a menuBarInstance.
    // It parses children until it hits the corresponding end element for the current menu/menubar.
    QString currentMenuElementName = parentMenu ? QLatin1String("menu") : QLatin1String("menubar");

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::EndElement && xml.name().toString() == currentMenuElementName) {
            // Reached the end of the current menu or menubar section
            break;
        }

        if (token == QXmlStreamReader::StartElement) {
            QString elementName = xml.name().toString();
            QXmlStreamAttributes attrs = xml.attributes();

            if (elementName == QLatin1String("menu")) {
                QString menuName = attrs.value(QLatin1String("name")).toString();
                menuName.replace(QLatin1String("$"), QLatin1String("&")); // For mnemonics

                QMenu* newMenu = new QMenu(menuName, parentMenu ? parentMenu : this); // Parent correctly for Qt ownership

                if (parentMenu) {
                    parentMenu->addMenu(newMenu);
                } else if (menuBarInstance) {
                    menuBarInstance->addMenu(newMenu);
                } else {
                    qWarning("parseMenuNode: Neither parentMenu nor menuBarInstance provided for new menu.");
                    delete newMenu; // Avoid leak
                    continue;
                }

                if (attrs.value(QLatin1String("special")).toString() == QLatin1String("RECENT_FILES")) {
                    m_recentFilesMenu = newMenu;
                }

                parseMenuNode(xml, newMenu, nullptr); // Recurse for children of this newMenu

            } else if (elementName == QLatin1String("item")) {
                QString itemName = attrs.value(QLatin1String("name")).toString();
                itemName.replace(QLatin1String("$"), QLatin1String("&"));
                QString actionName = attrs.value(QLatin1String("action")).toString();

                QAction* action = new QAction(itemName, this); // Parent action to MainWindow for lifecycle
                action->setObjectName(actionName); // Store action string for identification

                if (attrs.hasAttribute(QLatin1String("hotkey"))) {
                    action->setShortcut(QKeySequence::fromString(attrs.value(QLatin1String("hotkey")).toString()));
                }
                if (attrs.hasAttribute(QLatin1String("help"))) {
                    action->setStatusTip(attrs.value(QLatin1String("help")).toString());
                }
                if (attrs.value(QLatin1String("kind")).toString() == QLatin1String("check")) {
                    action->setCheckable(true);
                    // Placeholder: Load initial checked state from settings if needed
                    // if (m_settings->value("actions/" + actionName + "/checked", false).toBool()) {
                    //    action->setChecked(true);
                    // }
                }
                // TODO: Handle kind="radio" using QActionGroup

                // Don't connect to placeholder initially - will be connected in connectEditorController()
                // connect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);

                if (parentMenu) {
                    parentMenu->addAction(action);
                } else if (menuBarInstance) {
                    // This case is for top-level actions directly in menubar, less common but possible
                    menuBarInstance->addAction(action);
                } else {
                     qWarning("parseMenuNode: Neither parentMenu nor menuBarInstance provided for new item.");
                     delete action; // Avoid leak
                     continue;
                }

                if (!actionName.isEmpty()) {
                    m_actions.insert(actionName, action);
                }

            } else if (elementName == QLatin1String("separator")) {
                if (parentMenu) {
                    parentMenu->addSeparator();
                } else if (menuBarInstance) {
                    // Separator directly in menubar is unusual, but QMenuBar::addSeparator exists
                    menuBarInstance->addSeparator();
                }
            }
            // Silently ignore other element names for now
        }
    }
    if (xml.hasError()) {
         qWarning() << "MainWindow::parseMenuNode: XML parsing error:" << xml.errorString()
                   << "at line" << xml.lineNumber() << "column" << xml.columnNumber();
    }
}

void MainWindow::updateRecentFilesMenu() {
    if (!m_recentFilesMenu) return; // Should have been found by parseMenuNode
    if (!m_settings) return;

    // Clear existing recent file actions
    // qDeleteAll was considered for m_recentFileActions in destructor.
    // If actions are parented to m_recentFilesMenu, menu->clear() might delete them.
    // Or if parented to MainWindow, they are deleted when MainWindow is.
    // Safest: remove from menu, then delete from our tracking list.
    for (QAction* action : m_recentFileActions) {
        m_recentFilesMenu->removeAction(action); // Remove from menu
        delete action; // Delete the QAction object itself
    }
    m_recentFileActions.clear(); // Clear the tracking list

    QStringList files = m_settings->value("recentFiles/fileList").toStringList();

    if (files.isEmpty()) {
        // Add a placeholder if no recent files
        QAction* placeholderAction = new QAction(tr("(No recent files)"), this);
        placeholderAction->setEnabled(false); // Disable it
        m_recentFilesMenu->addAction(placeholderAction);
        m_recentFileActions.append(placeholderAction); // Track it for clearing
    } else {
        for (int i = 0; i < files.size(); ++i) {
            const QString& filePath = files.at(i);
            // Display a shortened path or just filename for readability in menu
            QString menuText = QString("&%1 %2").arg(i + 1).arg(QFileInfo(filePath).fileName());
            if (menuText.length() > 60) { // Limit length
                 menuText = menuText.left(57) + "...";
            }

            QAction* recentAction = new QAction(menuText, this);
            recentAction->setData(filePath); // Store full path in action's data
            recentAction->setStatusTip(filePath); // Show full path in status tip
            connect(recentAction, &QAction::triggered, this, &MainWindow::openRecentFile);

            m_recentFilesMenu->addAction(recentAction);
            m_recentFileActions.append(recentAction); // Track for clearing
        }
    }
    // The placeholder item from menubar.xml with action "RECENT_FILES_EMPTY_PLACEHOLDER"
    // should be removed if actual recent files are added, or handled if it's the only one.
    // The current logic replaces all actions. If the XML placeholder is there, it's removed too.
    // This is fine as this function rebuilds the menu content.
}

void MainWindow::addRecentFile(const QString& filePath) {
    if (!m_settings || filePath.isEmpty()) return;

    // Read current recent files
    QStringList files = m_settings->value("recentFiles/fileList").toStringList();

    // Remove any existing instance of this filePath to move it to the top
    files.removeAll(filePath);

    // Add the new file path to the beginning of the list
    files.prepend(filePath);

    // Keep the list within MaxRecentFiles limit
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    // Save the updated list
    m_settings->setValue("recentFiles/fileList", files);

    updateRecentFilesMenu(); // Refresh the menu
}

void MainWindow::openRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString filePath = action->data().toString();
        if (!filePath.isEmpty()) {
            qDebug() << "Attempting to open recent file:" << filePath;
            if (m_statusBar) {
                m_statusBar->showMessage(tr("Opening: %1").arg(filePath), 2000);
            }
            // Placeholder: Actual file opening logic will be called here
            // e.g., editor->openFile(filePath);
            // After successful opening, it might be good to call addRecentFile(filePath)
            // again to move it to the top, though addRecentFile already does this.
            // For now, just a debug message.
            addRecentFile(filePath); // Ensure it's moved to top if opened successfully
        }
    }
}

void MainWindow::onPlaceholderActionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString actionName = action->objectName(); // Get the 'action' attribute from XML
        QString message = tr("Action '%1' triggered. Text: '%2'").arg(actionName.isEmpty() ? "Unknown" : actionName).arg(action->text());
        qDebug() << message;
        if (m_statusBar) {
            m_statusBar->showMessage(message, 3000); // Show for 3 seconds
        }

        // Placeholder for actual action logic based on actionName
        // For example:
        // if (actionName == QLatin1String("FILE_EXIT")) {
        //     close(); // Trigger closeEvent, which handles save prompts etc.
        // } else if (actionName == QLatin1String("HELP_ABOUT")) {
        //     QMessageBox::about(this, tr("About RME-Qt"), tr("Remere's Map Editor - Qt Version"));
        // }
        // ... etc. Full logic for each action will be in other WBS tasks.
    } else {
        qWarning("MainWindow::onPlaceholderActionTriggered: Sender is not a QAction.");
    }
}

void MainWindow::updateMenus() {
    // This method will be called to update the enabled/checked state of menu items
    // based on the application's current state (e.g., map loaded, selection active).

    // Initial implementation: Enable all actions for testing.
    // Actual logic will depend on services/state managers from other tasks (e.g., REFACTOR-01).
    for (QAction* action : m_actions.values()) {
        if (action) { // Safety check
            action->setEnabled(true);
        }
    }

    // Example of more specific logic for later:
    // bool mapIsLoaded = ... ; // Get from a map service or editor state
    // bool canUndo = ... ;    // Get from undo stack service
    // bool canRedo = ... ;

    // if (m_actions.contains(QLatin1String("FILE_SAVE"))) {
    //     m_actions.value(QLatin1String("FILE_SAVE"))->setEnabled(mapIsLoaded /* && isMapDirty */);
    // }
    // if (m_actions.contains(QLatin1String("EDIT_UNDO"))) {
    //     m_actions.value(QLatin1String("EDIT_UNDO"))->setEnabled(canUndo);
    // }
    // if (m_actions.contains(QLatin1String("EDIT_REDO"))) {
    //     m_actions.value(QLatin1String("EDIT_REDO"))->setEnabled(canRedo);
    // }
    // if (m_actions.contains(QLatin1String("VIEW_TOGGLE_GRID"))) {
    //     bool gridVisible = ...; // Get from view settings
    //     m_actions.value(QLatin1String("VIEW_TOGGLE_GRID"))->setChecked(gridVisible);
    // }
    qDebug() << "MainWindow::updateMenus called (currently enables all actions).";
}

void MainWindow::connectMapViewActions() {
    if (!m_mapView) {
        qWarning("MainWindow::connectMapViewActions: m_mapView is null!");
        return;
    }

    // Helper lambda to disconnect the placeholder and connect the new action
    auto reconnectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qWarning() << "MainWindow::connectMapViewActions: Action" << actionName << "not found.";
        }
    };

    // Zoom actions
    reconnectAction("ZOOM_IN", [this]() {
        if (m_mapView) {
            m_mapView->setZoom(m_mapView->getZoomFactor() * 1.12); // Approx. ZOOM_STEP_MULTIPLIER
        }
    });
    reconnectAction("ZOOM_OUT", [this]() {
        if (m_mapView) {
            m_mapView->setZoom(m_mapView->getZoomFactor() / 1.12); // Approx. ZOOM_STEP_MULTIPLIER
        }
    });
    reconnectAction("ZOOM_NORMAL", [this]() {
        if (m_mapView) {
            m_mapView->setZoom(1.0);
        }
    });

    // Floor actions
    for (int i = 0; i <= 15; ++i) {
        QString actionName = QString("FLOOR_%1").arg(i);
        // Check if action exists before attempting to reconnect
        if (m_actions.contains(actionName)) {
            reconnectAction(actionName, [this, i]() {
                if (m_mapView) {
                    m_mapView->setCurrentFloor(i);
                }
            });
        }
    }
    // Note: menubar.xml does not have direct FLOOR_UP / FLOOR_DOWN actions.
    // These are typically handled by PageUp/PageDown keys directly in MapView's keyPressEvent.
}

void MainWindow::connectBrushMaterialActions() {
    // Helper lambda to disconnect the placeholder and connect the new action
    auto reconnectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qWarning() << "MainWindow::connectBrushMaterialActions: Action" << actionName << "not found.";
        }
    };

    // Connect brush/material editor actions
    reconnectAction("BRUSH_MATERIAL_EDITOR", &MainWindow::onBrushMaterialEditor);
    reconnectAction("NEW_TILESET", &MainWindow::onNewTileset);
    reconnectAction("ADD_ITEM_TO_TILESET", &MainWindow::onAddItemToTileset);
    
    qDebug() << "MainWindow::connectBrushMaterialActions: Connected brush/material editor actions.";
}

void MainWindow::createEditorController() {
    // Create the EditorController instance
    m_editorController = new RME::editor_logic::EditorController(this);
    
    // Connect EditorController to MapView if both exist
    if (m_mapView && m_editorController) {
        // TODO: Connect EditorController signals to MapView slots when available
        // connect(m_editorController, &EditorController::mapChanged, m_mapView, &MapView::setMap);
        // connect(m_mapView, &MapView::tileClicked, m_editorController, &EditorController::handleTileClick);
        qDebug() << "MainWindow::createEditorController: EditorController created and basic connections established.";
    } else {
        qWarning() << "MainWindow::createEditorController: Failed to create EditorController or MapView is null.";
    }
}

void MainWindow::createDockManager() {
    // Create the DockManager instance
    m_dockManager = new RME::ui::DockManager(this);
    
    // Initialize basic dock layout
    if (m_dockManager) {
        // TODO: Set up initial dock panels when palette classes are available
        // m_dockManager->createItemPalette();
        // m_dockManager->createCreaturePalette();
        // m_dockManager->createPropertiesPanel();
        qDebug() << "MainWindow::createDockManager: DockManager created with basic layout.";
    } else {
        qWarning() << "MainWindow::createDockManager: Failed to create DockManager.";
    }
}

void MainWindow::connectEditorController() {
    if (!m_editorController) {
        qWarning() << "MainWindow::connectEditorController: EditorController is null, cannot connect actions.";
        return;
    }

    // Helper lambda to connect actions to EditorController methods
    auto connectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            // Disconnect from placeholder first
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            // Connect to the actual slot
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qDebug() << "MainWindow::connectEditorController: Action" << actionName << "not found in menu.";
        }
    };

    // Connect File menu actions
    connectAction("NEW", &MainWindow::onNewMap);
    connectAction("OPEN", &MainWindow::onOpenMap);
    connectAction("SAVE", &MainWindow::onSaveMap);
    connectAction("SAVE_AS", &MainWindow::onSaveMapAs);
    connectAction("CLOSE", &MainWindow::onCloseMap);
    connectAction("IMPORT_MAP", &MainWindow::onImportMap);
    connectAction("EXPORT_MAP", &MainWindow::onExportMap);
    connectAction("EXPORT_MINIMAP", &MainWindow::onExportMinimap);
    connectAction("EXIT", &MainWindow::onExit);

    // Connect Edit menu actions
    connectAction("UNDO", &MainWindow::onUndo);
    connectAction("REDO", &MainWindow::onRedo);
    connectAction("CUT", &MainWindow::onCut);
    connectAction("COPY", &MainWindow::onCopy);
    connectAction("PASTE", &MainWindow::onPaste);
    connectAction("SELECT_ALL", &MainWindow::onSelectAll);
    connectAction("CLEAR_SELECTION", &MainWindow::onClearSelection);
    connectAction("DELETE", &MainWindow::onDelete);
    connectAction("PREFERENCES", &MainWindow::onPreferences);

    // Connect Map menu actions
    connectAction("BORDERIZE_MAP", &MainWindow::onBorderizeMap);
    connectAction("RANDOMIZE_MAP", &MainWindow::onRandomizeMap);
    connectAction("CLEAR_INVALID_HOUSE_TILES", &MainWindow::onClearInvalidHouseTiles);
    connectAction("CLEAR_MODIFIED_TILE_STATE", &MainWindow::onClearModifiedTileState);
    connectAction("VALIDATE_GROUNDS", &MainWindow::onValidateGrounds);
    connectAction("BORDERIZE_SELECTION", &MainWindow::onBorderizeSelection);
    connectAction("RANDOMIZE_SELECTION", &MainWindow::onRandomizeSelection);
    connectAction("MOVE_SELECTION", &MainWindow::onMoveSelection);
    connectAction("RESIZE_MAP", &MainWindow::onResizeMap);
    connectAction("MAP_PROPERTIES", &MainWindow::onMapProperties);

    // Connect Search menu actions
    connectAction("FIND_ITEM", &MainWindow::onFindItem);
    connectAction("FIND_CREATURE", &MainWindow::onFindCreature);
    connectAction("SEARCH_ON_MAP", &MainWindow::onSearchOnMap);
    connectAction("SEARCH_ON_SELECTION", &MainWindow::onSearchOnSelection);
    connectAction("GO_TO_POSITION", &MainWindow::onGoToPosition);

    // Connect View menu actions
    connectAction("ZOOM_IN", &MainWindow::onZoomIn);
    connectAction("ZOOM_OUT", &MainWindow::onZoomOut);
    connectAction("ZOOM_NORMAL", &MainWindow::onZoomNormal);
    connectAction("ZOOM_FIT", &MainWindow::onZoomFit);
    connectAction("FLOOR_UP", &MainWindow::onFloorUp);
    connectAction("FLOOR_DOWN", &MainWindow::onFloorDown);
    connectAction("GO_TO_FLOOR", &MainWindow::onGoToFloor);
    connectAction("SHOW_GRID", &MainWindow::onShowGrid);
    connectAction("SHOW_CREATURES", &MainWindow::onShowCreatures);
    connectAction("SHOW_SPAWNS", &MainWindow::onShowSpawns);
    connectAction("SHOW_HOUSES", &MainWindow::onShowHouses);
    connectAction("SHOW_WAYPOINTS", &MainWindow::onShowWaypoints);
    connectAction("SHOW_ITEM_PALETTE", &MainWindow::onShowItemPalette);
    connectAction("SHOW_CREATURE_PALETTE", &MainWindow::onShowCreaturePalette);
    connectAction("SHOW_HOUSE_PALETTE", &MainWindow::onShowHousePalette);
    connectAction("SHOW_WAYPOINT_PALETTE", &MainWindow::onShowWaypointPalette);
    connectAction("SHOW_PROPERTIES_PANEL", &MainWindow::onShowPropertiesPanel);
    connectAction("SHOW_MINIMAP", &MainWindow::onShowMinimap);

    // Connect Tools menu actions
    connectAction("SELECT_TOOL", &MainWindow::onSelectTool);
    connectAction("BRUSH_TOOL", &MainWindow::onBrushTool);
    connectAction("HOUSE_EXIT_TOOL", &MainWindow::onHouseExitTool);
    connectAction("WAYPOINT_TOOL", &MainWindow::onWaypointTool);
    connectAction("SPAWN_TOOL", &MainWindow::onSpawnTool);

    // Connect Help menu actions
    connectAction("ABOUT", &MainWindow::onAbout);
    connectAction("ABOUT_QT", &MainWindow::onAboutQt);
    connectAction("HELP", &MainWindow::onHelp);
    connectAction("CHECK_UPDATES", &MainWindow::onCheckUpdates);

    qDebug() << "MainWindow::connectEditorController: Connected" << m_actions.size() << "menu actions to handlers.";
}

void MainWindow::updateMenuStatesFromEditor() {
    if (!m_editorController) {
        qDebug() << "MainWindow::updateMenuStatesFromEditor: EditorController is null, using default states.";
        // Enable basic actions, disable map-specific ones
        updateMenus();
        return;
    }

    // TODO: Get actual state from EditorController when methods are available
    // For now, use placeholder logic based on whether we have a map loaded
    
    // bool hasMap = m_editorController->hasMap();
    // bool hasSelection = m_editorController->hasSelection();
    // bool canUndo = m_editorController->canUndo();
    // bool canRedo = m_editorController->canRedo();
    // bool isMapDirty = m_editorController->isMapDirty();

    // Placeholder: assume we have basic functionality
    bool hasMap = false; // TODO: Get from EditorController
    bool hasSelection = false; // TODO: Get from EditorController
    bool canUndo = false; // TODO: Get from EditorController
    bool canRedo = false; // TODO: Get from EditorController
    bool isMapDirty = false; // TODO: Get from EditorController

    // Update File menu states
    if (m_actions.contains("SAVE")) {
        m_actions["SAVE"]->setEnabled(hasMap && isMapDirty);
    }
    if (m_actions.contains("SAVE_AS")) {
        m_actions["SAVE_AS"]->setEnabled(hasMap);
    }
    if (m_actions.contains("CLOSE")) {
        m_actions["CLOSE"]->setEnabled(hasMap);
    }
    if (m_actions.contains("EXPORT_MINIMAP")) {
        m_actions["EXPORT_MINIMAP"]->setEnabled(hasMap);
    }

    // Update Edit menu states
    if (m_actions.contains("UNDO")) {
        m_actions["UNDO"]->setEnabled(canUndo);
    }
    if (m_actions.contains("REDO")) {
        m_actions["REDO"]->setEnabled(canRedo);
    }
    if (m_actions.contains("CUT")) {
        m_actions["CUT"]->setEnabled(hasSelection);
    }
    if (m_actions.contains("COPY")) {
        m_actions["COPY"]->setEnabled(hasSelection);
    }
    if (m_actions.contains("DELETE")) {
        m_actions["DELETE"]->setEnabled(hasSelection);
    }
    if (m_actions.contains("CLEAR_SELECTION")) {
        m_actions["CLEAR_SELECTION"]->setEnabled(hasSelection);
    }

    // Update Map menu states
    QStringList mapActions = {"BORDERIZE_MAP", "RANDOMIZE_MAP", "CLEAR_INVALID_HOUSE_TILES", 
                             "CLEAR_MODIFIED_TILE_STATE", "VALIDATE_GROUNDS", "RESIZE_MAP", "MAP_PROPERTIES"};
    for (const QString& actionName : mapActions) {
        if (m_actions.contains(actionName)) {
            m_actions[actionName]->setEnabled(hasMap);
        }
    }

    // Update selection-dependent Map menu actions
    QStringList selectionActions = {"BORDERIZE_SELECTION", "RANDOMIZE_SELECTION", "MOVE_SELECTION"};
    for (const QString& actionName : selectionActions) {
        if (m_actions.contains(actionName)) {
            m_actions[actionName]->setEnabled(hasSelection);
        }
    }

    // Update Search menu states
    QStringList searchActions = {"FIND_ITEM", "FIND_CREATURE", "SEARCH_ON_MAP", "GO_TO_POSITION"};
    for (const QString& actionName : searchActions) {
        if (m_actions.contains(actionName)) {
            m_actions[actionName]->setEnabled(hasMap);
        }
    }
    if (m_actions.contains("SEARCH_ON_SELECTION")) {
        m_actions["SEARCH_ON_SELECTION"]->setEnabled(hasSelection);
    }

    // Update View menu states (most should always be enabled)
    QStringList viewActions = {"ZOOM_IN", "ZOOM_OUT", "ZOOM_NORMAL", "ZOOM_FIT", 
                              "FLOOR_UP", "FLOOR_DOWN", "GO_TO_FLOOR"};
    for (const QString& actionName : viewActions) {
        if (m_actions.contains(actionName)) {
            m_actions[actionName]->setEnabled(hasMap);
        }
    }

    // TODO: Update checkable actions based on current view state
    // if (m_actions.contains("SHOW_GRID")) {
    //     m_actions["SHOW_GRID"]->setChecked(m_mapView->isGridVisible());
    // }

    qDebug() << "MainWindow::updateMenuStatesFromEditor: Updated menu states based on editor state.";
}

void MainWindow::createToolBar() {
    // Create the main toolbar
    m_mainToolBar = new RME::ui::MainToolBar(this);
    
    // Add the toolbar to the main window
    addToolBar(Qt::TopToolBarArea, m_mainToolBar);
    
    // Set up integration with editor controller and brush manager
    if (m_editorController) {
        m_mainToolBar->setEditorController(m_editorController);
    }
    if (m_brushIntegrationManager) {
        m_mainToolBar->setBrushIntegrationManager(m_brushIntegrationManager);
    }
    
    // Connect toolbar signals to MainWindow slots
    connect(m_mainToolBar, &RME::ui::MainToolBar::newMapRequested, 
            this, &MainWindow::onNewMap);
    connect(m_mainToolBar, &RME::ui::MainToolBar::openMapRequested, 
            this, &MainWindow::onOpenMap);
    connect(m_mainToolBar, &RME::ui::MainToolBar::saveMapRequested, 
            this, &MainWindow::onSaveMap);
    
    // Connect tool mode changes
    connect(m_mainToolBar, &RME::ui::MainToolBar::toolModeChangeRequested,
            this, [this](int toolMode) {
                if (m_brushIntegrationManager) {
                    // TODO: Set tool mode in brush integration manager
                    // m_brushIntegrationManager->setToolMode(toolMode);
                }
                qDebug() << "MainWindow::createToolBar: Tool mode changed to" << toolMode;
            });
    
    // Connect zoom and floor changes to MapView
    if (m_mapView) {
        connect(m_mainToolBar, &RME::ui::MainToolBar::zoomChangeRequested,
                this, [this](int zoomChange) {
                    // TODO: Implement zoom change in MapView
                    // m_mapView->changeZoom(zoomChange);
                    qDebug() << "MainWindow::createToolBar: Zoom change requested:" << zoomChange;
                });
        
        connect(m_mainToolBar, &RME::ui::MainToolBar::floorChangeRequested,
                this, [this](int floor) {
                    // TODO: Implement floor change in MapView
                    // m_mapView->setCurrentFloor(floor);
                    qDebug() << "MainWindow::createToolBar: Floor change requested:" << floor;
                });
    }
    
    // Connect toolbar state updates to editor state changes
    if (m_editorController) {
        // TODO: Connect editor state change signals to toolbar updates
        // connect(m_editorController, &EditorController::mapStateChanged,
        //         m_mainToolBar, &MainToolBar::onMapStateChanged);
        // connect(m_editorController, &EditorController::selectionChanged,
        //         m_mainToolBar, &MainToolBar::onSelectionChanged);
    }
    
    // Initial toolbar state update
    m_mainToolBar->updateToolStates();
    
    qDebug() << "MainWindow::createToolBar: Main toolbar created and integrated successfully.";
}

void MainWindow::setupEditorTabWidget() {
    m_editorTabWidget = new QTabWidget(this);
    m_editorTabWidget->setTabsClosable(true);
    m_editorTabWidget->setMovable(true);
    m_editorTabWidget->setDocumentMode(true);
    
    // Set as central widget
    setCentralWidget(m_editorTabWidget);
    
    // Connect tab signals
    connect(m_editorTabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onActiveEditorTabChanged);
    connect(m_editorTabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onEditorTabCloseRequested);
    
    // Show message when no tabs are open
    if (m_editorTabWidget->count() == 0) {
        // TODO: Add a welcome widget or placeholder
        updateWindowTitle();
    }
}

EditorInstanceWidget* MainWindow::createNewEditorInstance(RME::core::Map* map, const QString& filePath) {
    auto* editorInstance = new EditorInstanceWidget(map, filePath, this);
    
    // Set up dependencies
    if (m_settings) {
        // TODO: Convert QSettings to AppSettings when available
        // editorInstance->setAppSettings(m_appSettings);
    }
    
    // Connect signals
    connect(editorInstance, &EditorInstanceWidget::modificationChanged,
            this, &MainWindow::onEditorModificationChanged);
    connect(editorInstance, &EditorInstanceWidget::displayNameChanged,
            this, &MainWindow::onEditorDisplayNameChanged);
    connect(editorInstance, &EditorInstanceWidget::requestClose,
            this, [this, editorInstance]() {
                int index = -1;
                for (int i = 0; i < m_editorTabWidget->count(); ++i) {
                    if (getEditorInstance(i) == editorInstance) {
                        index = i;
                        break;
                    }
                }
                if (index >= 0) {
                    onEditorTabCloseRequested(index);
                }
            });
    
    return editorInstance;
}

void MainWindow::addEditorTab(EditorInstanceWidget* editorInstance) {
    if (!editorInstance || !m_editorTabWidget) return;
    
    QString tabTitle = editorInstance->getDisplayName();
    int index = m_editorTabWidget->addTab(editorInstance, tabTitle);
    m_editorTabWidget->setCurrentIndex(index);
    
    // Update current editor instance
    m_currentEditorInstance = editorInstance;
    updateWindowTitle();
}

void MainWindow::closeEditorTab(int index) {
    EditorInstanceWidget* editorInstance = getEditorInstance(index);
    if (!editorInstance) return;
    
    // Check if the editor has unsaved changes
    if (editorInstance->isModified()) {
        if (!promptSaveChanges(editorInstance)) {
            return; // User cancelled
        }
    }
    
    // Remove the tab
    m_editorTabWidget->removeTab(index);
    
    // Update current editor instance
    if (m_currentEditorInstance == editorInstance) {
        m_currentEditorInstance = getCurrentEditorInstance();
    }
    
    // Delete the editor instance
    editorInstance->deleteLater();
    
    updateWindowTitle();
    updateMenuStatesFromEditor();
}

EditorInstanceWidget* MainWindow::getEditorInstance(int index) const {
    if (!m_editorTabWidget || index < 0 || index >= m_editorTabWidget->count()) {
        return nullptr;
    }
    
    return qobject_cast<EditorInstanceWidget*>(m_editorTabWidget->widget(index));
}

EditorInstanceWidget* MainWindow::getCurrentEditorInstance() const {
    if (!m_editorTabWidget) return nullptr;
    
    int currentIndex = m_editorTabWidget->currentIndex();
    return getEditorInstance(currentIndex);
}

void MainWindow::updateWindowTitle() {
    QString title = tr("Remere's Map Editor - Qt");
    
    if (m_currentEditorInstance) {
        QString displayName = m_currentEditorInstance->getDisplayName();
        title = QString("%1 - %2").arg(displayName, tr("Remere's Map Editor - Qt"));
        
        // Set window modified state
        setWindowModified(m_currentEditorInstance->isModified());
    } else {
        setWindowModified(false);
    }
    
    setWindowTitle(title);
}

bool MainWindow::promptSaveChanges(EditorInstanceWidget* editorInstance) {
    if (!editorInstance || !editorInstance->isModified()) {
        return true; // No changes to save
    }
    
    QString fileName = editorInstance->getDisplayName();
    if (fileName.endsWith("*")) {
        fileName.chop(1); // Remove the asterisk
    }
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Save Changes"),
        tr("The map '%1' has unsaved changes.\n\nDo you want to save the changes?").arg(fileName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );
    
    switch (result) {
        case QMessageBox::Save:
            // TODO: Implement save functionality
            // return saveMap(editorInstance);
            return true; // Placeholder
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
    }
}

void MainWindow::onActiveEditorTabChanged(int index) {
    EditorInstanceWidget* newInstance = getEditorInstance(index);
    
    if (m_currentEditorInstance != newInstance) {
        m_currentEditorInstance = newInstance;
        updateWindowTitle();
        updateMenuStatesFromEditor();
        
        // TODO: Update dock panels to show data for current editor
        // if (m_dockManager) {
        //     m_dockManager->setCurrentEditor(m_currentEditorInstance);
        // }
    }
}

void MainWindow::onEditorTabCloseRequested(int index) {
    closeEditorTab(index);
}

void MainWindow::onEditorModificationChanged(bool modified) {
    Q_UNUSED(modified);
    
    // Update window title to reflect modification state
    updateWindowTitle();
    
    // Update menu states
    updateMenuStatesFromEditor();
}

void MainWindow::onEditorDisplayNameChanged(const QString& name) {
    // Find which tab this editor belongs to and update its title
    EditorInstanceWidget* senderInstance = qobject_cast<EditorInstanceWidget*>(sender());
    if (!senderInstance || !m_editorTabWidget) return;
    
    for (int i = 0; i < m_editorTabWidget->count(); ++i) {
        if (getEditorInstance(i) == senderInstance) {
            m_editorTabWidget->setTabText(i, name);
            break;
        }
    }
    
    // Update window title if this is the current editor
    if (senderInstance == m_currentEditorInstance) {
        updateWindowTitle();
    }
}

} // namespace ui
} // namespace RME
