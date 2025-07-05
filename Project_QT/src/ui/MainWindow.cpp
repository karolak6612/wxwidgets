#include "ui/MainWindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include "core/utils/ResourcePathManager.h"
#include "ui/MainToolBar.h"
#include <QCoreApplication>
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"
#include "core/map/Map.h" // Relative path to header

// Service implementations
#include "core/services/ServiceContainer.h"
#include "core/brush/BrushStateService.h"
#include "core/editor/EditorStateService.h"
#include "core/services/ClientDataService.h"
#include "core/services/WindowManagerService.h"
#include "core/services/ApplicationSettingsService.h"
#include "ui/dialogs/PreferencesDialog.h"
#include "ui/dialogs/AboutDialog.h"
#include "ui/dialogs/ItemFinderDialogQt.h"
#include "ui/dialogs/MapPropertiesDialog.h"
#include "ui/dialogs/ServerHostingDialog.h"
#include "ui/widgets/LiveCollaborationPanel.h"
#include "network/QtLiveClient.h"

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

    // Initialize services first
    initializeServices();

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

    // Create and integrate editor controller
    createEditorController();
    
    // Create dock manager and dock panels
    createDockManager();

    // Use ResourcePathManager to find menubar.xml
    QString menubarPath = RME::core::utils::ResourcePathManager::instance().resolvePath("menubar.xml", "xml");
    
    // If path resolution fails, try Qt resource directly
    if (menubarPath.isEmpty() || !QFile::exists(menubarPath)) {
        menubarPath = ":/menubar.xml";
    }
    
    createMenusFromXML(menubarPath);
    connectMapViewActions(); // Connect actions to MapView slots
    connectEditorController(); // Connect menu actions to editor controller
    connectBrushMaterialActions(); // Connect brush/material editor actions
    connectEditActions(); // Connect edit menu actions (FINAL-03)
    connectViewActions(); // Connect view menu actions (FINAL-04)
    
    // Create live collaboration components (FINAL-05)
    createLiveCollaboration();
    connectLiveActions();
    updateRecentFilesMenu();
    updateMenus();
    updateMenuStatesFromEditor();
    updateWindowTitle(); // Set initial window title

    // Connect services after all components are created
    connectServices();
    
    // Test basic service functionality
    testBasicServiceFunctionality();

    loadWindowSettings(); // Load stored geometry and state
}

MainWindow::~MainWindow() {
    // Cleanup services first
    cleanupServices();
    
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
    // Check if there are unsaved changes before closing
    if (m_editorController && m_editorController->isMapModified()) {
        QString filename = m_editorController->getCurrentMapFilename();
        QString mapName = filename.isEmpty() ? "Untitled Map" : QFileInfo(filename).baseName();
        
        QMessageBox::StandardButton result = QMessageBox::warning(
            this,
            tr("Confirm Close"),
            tr("The map '%1' has unsaved changes.\nDo you want to save before closing?").arg(mapName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
        
        if (result == QMessageBox::Save) {
            if (m_editorController->saveMap()) {
                event->accept();
            } else {
                // Save failed, don't close
                event->ignore();
                return;
            }
        } else if (result == QMessageBox::Discard) {
            event->accept();
        } else {
            // Cancel - don't close
            event->ignore();
            return;
        }
    } else {
        event->accept();
    }
    
    // Save application settings before closing
    if (m_editorController) {
        m_editorController->getAppSettings().saveSettings();
    }

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
        
        // Try using ResourcePathManager first
        QString resolvedPath = RME::core::utils::ResourcePathManager::instance().resolvePath("menubar.xml", "xml");
        if (!resolvedPath.isEmpty() && resolvedPath != xmlFilePath) {
            qInfo() << "Trying ResourcePathManager resolved path:" << resolvedPath;
            file.setFileName(resolvedPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qInfo() << "Successfully opened menubar.xml from ResourcePathManager path:" << resolvedPath;
            }
        }
        
        // If ResourcePathManager failed, try fallback paths
        if (!file.isOpen()) {
            QStringList fallbackPaths = {
                ":/menubar.xml",
                "../XML/menubar.xml",
                "XML/menubar.xml",
                QCoreApplication::applicationDirPath() + "/XML/menubar.xml"
            };
            
            for (const QString& fallbackPath : fallbackPaths) {
                qInfo() << "Trying fallback path:" << fallbackPath;
                file.setFileName(fallbackPath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qInfo() << "Successfully opened menubar.xml from fallback path:" << fallbackPath;
                    break;
                }
            }
        }
        
        if (!file.isOpen()) {
            qCritical() << "Failed to open menubar.xml from any location. Menu will not be created.";
            if (m_statusBar) {
                m_statusBar->showMessage(tr("Error: Could not load menu definition."), 5000);
            }
            return;
        }
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

// Service management methods
void MainWindow::initializeServices()
{
    qDebug() << "MainWindow::initializeServices: Initializing service architecture";
    
    // Create service container
    m_serviceContainer = new RME::core::ServiceContainer(this);
    
    // Create services
    m_brushStateService = new RME::core::brush::BrushStateService(m_brushIntegrationManager, this);
    m_editorStateService = new RME::core::EditorStateService(this);
    m_clientDataService = new RME::core::ClientDataService(this);
    m_windowManagerService = new RME::core::WindowManagerService(this, this);
    m_applicationSettingsService = new RME::core::ApplicationSettingsService(this);
    
    // Register services with container
    m_serviceContainer->registerBrushStateService(m_brushStateService);
    m_serviceContainer->registerEditorStateService(m_editorStateService);
    m_serviceContainer->registerClientDataService(m_clientDataService);
    m_serviceContainer->registerWindowManagerService(m_windowManagerService);
    m_serviceContainer->registerApplicationSettingsService(m_applicationSettingsService);
    
    // Set global service container instance
    RME::core::ServiceContainer::setInstance(m_serviceContainer);
    
    qDebug() << "MainWindow::initializeServices: Services initialized and registered";
}

void MainWindow::connectServices()
{
    qDebug() << "MainWindow::connectServices: Connecting service signals and slots";
    
    // Connect brush state changes to UI updates
    connect(m_brushStateService, &RME::core::IBrushStateService::activeBrushChanged,
            this, &MainWindow::updateMenus);
    connect(m_brushStateService, &RME::core::IBrushStateService::brushSizeChanged,
            this, &MainWindow::updateMenus);
    
    // Connect editor state changes to UI updates
    connect(m_editorStateService, &RME::core::IEditorStateService::editorModeChanged,
            this, &MainWindow::updateMenus);
    connect(m_editorStateService, &RME::core::IEditorStateService::currentFloorChanged,
            this, &MainWindow::updateMenus);
    
    // Connect client data changes to UI updates
    connect(m_clientDataService, &RME::core::IClientDataService::clientVersionChanged,
            this, &MainWindow::updateMenus);
    connect(m_clientDataService, &RME::core::IClientDataService::clientVersionLoaded,
            this, [this](const QString& versionId) {
                m_windowManagerService->updateStatusText(tr("Client version %1 loaded").arg(versionId));
            });
    
    // Connect settings changes to view updates
    connect(m_applicationSettingsService, &RME::core::IApplicationSettingsService::viewSettingsChanged,
            this, &MainWindow::updateMenus);
    
    // Connect window manager service to editor changes
    connect(m_editorStateService, &RME::core::IEditorStateService::activeEditorChanged,
            m_windowManagerService, &RME::core::WindowManagerService::onEditorChanged);
    
    // Connect service container signals
    connect(m_serviceContainer, &RME::core::ServiceContainer::allServicesRegistered,
            this, [this]() {
                qDebug() << "MainWindow: All services are now registered and ready";
                m_windowManagerService->updateStatusText(tr("Services initialized"));
            });
    
    // Connect client data loading progress to status updates
    connect(m_clientDataService, &RME::core::IClientDataService::dataLoadingProgress,
            this, [this](int percentage, const QString& message) {
                m_windowManagerService->updateStatusText(tr("Loading: %1 (%2%)").arg(message).arg(percentage));
            });
    
    // Connect application settings to immediate UI updates
    connect(m_applicationSettingsService, &RME::core::IApplicationSettingsService::doorLockedChanged,
            this, &MainWindow::updateMenus);
    connect(m_applicationSettingsService, &RME::core::IApplicationSettingsService::pastingChanged,
            this, &MainWindow::updateMenus);
    
    qDebug() << "MainWindow::connectServices: Service connections established";
}

void MainWindow::cleanupServices()
{
    qDebug() << "MainWindow::cleanupServices: Cleaning up services";
    
    // Clear global service container instance
    RME::core::ServiceContainer::setInstance(nullptr);
    
    // Services will be deleted automatically by Qt's parent-child system
    // since they are all parented to 'this'
    
    qDebug() << "MainWindow::cleanupServices: Services cleaned up";
}

void MainWindow::testBasicServiceFunctionality()
{
    // Test basic service functionality - remove in production
    qDebug() << "MainWindow: Verifying service initialization";
    
    // Test service container
    if (!m_serviceContainer) {
        qCritical() << "Service container is null!";
        return;
    }
    
    if (!m_serviceContainer->areAllServicesRegistered()) {
        qWarning() << "Not all services are registered. Missing:" << m_serviceContainer->getMissingServices();
        return;
    }
    
    // Test brush state service
    if (m_brushStateService) {
        qDebug() << "MainWindow: BrushStateService initialized";
        m_brushStateService->setBrushSize(5);
        int size = m_brushStateService->getBrushSize();
        qDebug() << "Brush size set to 5, got:" << size;
        
        m_brushStateService->setBrushShape(BrushShape::Circle);
        BrushShape shape = m_brushStateService->getBrushShape();
        qDebug() << "Brush shape set to Circle, got:" << static_cast<int>(shape);
    }
    
    // Test editor state service
    if (m_editorStateService) {
        qDebug() << "MainWindow: EditorStateService initialized";
        m_editorStateService->setCurrentFloor(5);
        int floor = m_editorStateService->getCurrentFloor();
        qDebug() << "Floor set to 5, got:" << floor;
        
        m_editorStateService->setZoomLevel(2.0f);
        float zoom = m_editorStateService->getZoomLevel();
        qDebug() << "Zoom set to 2.0, got:" << zoom;
    }
    
    // Test application settings service
    if (m_applicationSettingsService) {
        qDebug() << "MainWindow: ApplicationSettingsService initialized";
        m_applicationSettingsService->setGridVisible(false);
        bool gridVisible = m_applicationSettingsService->isGridVisible();
        qDebug() << "Grid visibility set to false, got:" << gridVisible;
        
        m_applicationSettingsService->setDefaultBrushSize(3);
        int defaultSize = m_applicationSettingsService->getDefaultBrushSize();
        qDebug() << "Default brush size set to 3, got:" << defaultSize;
    }
    
    // Test window manager service
    if (m_windowManagerService) {
        qDebug() << "Testing WindowManagerService...";
        m_windowManagerService->updateStatusText("Service test in progress...");
        m_windowManagerService->updateWindowTitle("RME - Service Test");
    }
    
    // Test client data service
    if (m_clientDataService) {
        qDebug() << "Testing ClientDataService...";
        bool isLoaded = m_clientDataService->isClientVersionLoaded();
        qDebug() << "Client version loaded:" << isLoaded;
        QString versionId = m_clientDataService->getCurrentVersionId();
        qDebug() << "Current version ID:" << versionId;
    }
    
    qDebug() << "MainWindow::testBasicServiceFunctionality: Service testing completed";
}

void MainWindow::connectEditorController() {
    if (!m_editorController) {
        qWarning() << "MainWindow::connectEditorController: EditorController is null";
        return;
    }
    
    // Connect file operation signals
    connect(m_editorController, &RME::editor_logic::EditorController::mapLoaded,
            this, [this](const QString& filename) {
                updateWindowTitle();
                updateMenuStatesFromEditor();
                statusBar()->showMessage(tr("Map loaded: %1").arg(filename.isEmpty() ? "New Map" : filename), 2000);
            });
    
    connect(m_editorController, &RME::editor_logic::EditorController::mapSaved,
            this, [this](const QString& filename) {
                updateWindowTitle();
                updateMenuStatesFromEditor();
                statusBar()->showMessage(tr("Map saved: %1").arg(filename), 2000);
            });
    
    connect(m_editorController, &RME::editor_logic::EditorController::mapModifiedChanged,
            this, [this](bool modified) {
                updateWindowTitle();
                updateMenuStatesFromEditor();
            });
    
    connect(m_editorController, &RME::editor_logic::EditorController::mapClosed,
            this, [this]() {
                updateWindowTitle();
                updateMenuStatesFromEditor();
                statusBar()->showMessage(tr("Map closed"), 2000);
            });
    
    // Connect undo/redo signals for menu state updates
    if (m_editorController->getUndoStack()) {
        QUndoStack* undoStack = m_editorController->getUndoStack();
        
        // Connect undo/redo state changes to menu updates
        connect(undoStack, &QUndoStack::canUndoChanged, this, [this](bool canUndo) {
            if (QAction* undoAction = m_actions.value("UNDO")) {
                undoAction->setEnabled(canUndo);
            }
        });
        
        connect(undoStack, &QUndoStack::canRedoChanged, this, [this](bool canRedo) {
            if (QAction* redoAction = m_actions.value("REDO")) {
                redoAction->setEnabled(canRedo);
            }
        });
        
        // Update initial states
        if (QAction* undoAction = m_actions.value("UNDO")) {
            undoAction->setEnabled(undoStack->canUndo());
        }
        if (QAction* redoAction = m_actions.value("REDO")) {
            redoAction->setEnabled(undoStack->canRedo());
        }
    }
    
    qDebug() << "MainWindow::connectEditorController: Connected EditorController signals";
}

void MainWindow::connectEditActions() {
    // Helper lambda to disconnect the placeholder and connect the new action
    auto reconnectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qWarning() << "MainWindow::connectEditActions: Action" << actionName << "not found.";
        }
    };

    // Connect edit menu actions
    reconnectAction("UNDO", [this]() { onUndo(); });
    reconnectAction("REDO", [this]() { onRedo(); });
    reconnectAction("CUT", [this]() { onCut(); });
    reconnectAction("COPY", [this]() { onCopy(); });
    reconnectAction("PASTE", [this]() { onPaste(); });
    reconnectAction("PREFERENCES", [this]() { onPreferences(); });

    qDebug() << "MainWindow::connectEditActions: Connected edit menu actions";
}

void MainWindow::connectViewActions() {
    // Helper lambda to disconnect the placeholder and connect the new action
    auto reconnectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qWarning() << "MainWindow::connectViewActions: Action" << actionName << "not found.";
        }
    };

    // Connect zoom actions (these are already connected in connectMapViewActions, but let's ensure consistency)
    reconnectAction("ZOOM_IN", [this]() { onZoomIn(); });
    reconnectAction("ZOOM_OUT", [this]() { onZoomOut(); });
    reconnectAction("ZOOM_NORMAL", [this]() { onZoomNormal(); });
    
    // Connect floor navigation actions
    for (int floor = 0; floor <= 15; ++floor) {
        QString actionName = QString("FLOOR_%1").arg(floor);
        reconnectAction(actionName, [this, floor]() { onSetFloor(floor); });
    }
    
    // Connect visibility toggle actions
    reconnectAction("SHOW_GRID", [this]() { onToggleGrid(); });
    reconnectAction("SHOW_CREATURES", [this]() { onToggleCreatures(); });
    reconnectAction("SHOW_SPAWNS", [this]() { onToggleSpawns(); });
    reconnectAction("SHOW_HOUSES", [this]() { onToggleHouses(); });
    reconnectAction("SHOW_LIGHTS", [this]() { onToggleLights(); });
    reconnectAction("SHOW_TOOLTIPS", [this]() { onToggleTooltips(); });
    
    // Connect tools actions
    reconnectAction("MAP_PROPERTIES", [this]() { onMapProperties(); });
    reconnectAction("FIND_ITEM", [this]() { onFindItem(); });
    
    // Connect help actions
    reconnectAction("ABOUT", [this]() { onAbout(); });

    qDebug() << "MainWindow::connectViewActions: Connected view menu actions";
}

void MainWindow::updateMenuStatesFromEditor() {
    if (!m_editorController) {
        return;
    }
    
    // Update menu states based on editor state
    bool hasMap = !m_editorController->getCurrentMapFilename().isEmpty() || m_editorController->isMapModified();
    bool isModified = m_editorController->isMapModified();
    
    // Enable/disable file operations
    if (m_saveAction) m_saveAction->setEnabled(hasMap && isModified);
    if (m_saveAsAction) m_saveAsAction->setEnabled(hasMap);
    if (m_closeAction) m_closeAction->setEnabled(hasMap);
    
    // Update undo/redo states from undo stack
    if (m_editorController && m_editorController->getUndoStack()) {
        QUndoStack* undoStack = m_editorController->getUndoStack();
        if (QAction* undoAction = m_actions.value("UNDO")) {
            undoAction->setEnabled(undoStack->canUndo());
        }
        if (QAction* redoAction = m_actions.value("REDO")) {
            redoAction->setEnabled(undoStack->canRedo());
        }
    }
}

void MainWindow::updateWindowTitle() {
    QString title = "Remere's Map Editor";
    
    if (m_editorController) {
        QString filename = m_editorController->getCurrentMapFilename();
        bool isModified = m_editorController->isMapModified();
        
        if (!filename.isEmpty()) {
            QFileInfo fileInfo(filename);
            title = fileInfo.baseName();
            if (isModified) {
                title += " *";
            }
            title += " - Remere's Map Editor";
        } else if (isModified) {
            title = "Untitled * - Remere's Map Editor";
        }
    }
    
    setWindowTitle(title);
}

// --- Edit menu operations (FINAL-03) ---

void MainWindow::onUndo() {
    if (m_editorController && m_editorController->getUndoStack()) {
        m_editorController->getUndoStack()->undo();
        statusBar()->showMessage(tr("Undo"), 1000);
    }
}

void MainWindow::onRedo() {
    if (m_editorController && m_editorController->getUndoStack()) {
        m_editorController->getUndoStack()->redo();
        statusBar()->showMessage(tr("Redo"), 1000);
    }
}

void MainWindow::onCut() {
    if (!m_editorController) {
        return;
    }
    
    // Get selection manager and clipboard manager
    auto* selectionManager = m_editorController->getSelectionManager();
    auto* clipboardManager = m_editorController->getClipboardManager();
    
    if (!selectionManager || !clipboardManager) {
        statusBar()->showMessage(tr("Cut operation not available"), 2000);
        return;
    }
    
    // Check if there's a selection
    if (!selectionManager->hasSelection()) {
        statusBar()->showMessage(tr("No selection to cut"), 2000);
        return;
    }
    
    // Copy selection to clipboard
    if (clipboardManager->copySelection()) {
        // Delete the selected tiles (cut operation)
        if (m_editorController->deleteSelection()) {
            statusBar()->showMessage(tr("Selection cut to clipboard"), 2000);
        } else {
            statusBar()->showMessage(tr("Failed to cut selection"), 2000);
        }
    } else {
        statusBar()->showMessage(tr("Failed to copy selection to clipboard"), 2000);
    }
}

void MainWindow::onCopy() {
    if (!m_editorController) {
        return;
    }
    
    // Get selection manager and clipboard manager
    auto* selectionManager = m_editorController->getSelectionManager();
    auto* clipboardManager = m_editorController->getClipboardManager();
    
    if (!selectionManager || !clipboardManager) {
        statusBar()->showMessage(tr("Copy operation not available"), 2000);
        return;
    }
    
    // Check if there's a selection
    if (!selectionManager->hasSelection()) {
        statusBar()->showMessage(tr("No selection to copy"), 2000);
        return;
    }
    
    // Copy selection to clipboard
    if (clipboardManager->copySelection()) {
        statusBar()->showMessage(tr("Selection copied to clipboard"), 2000);
    } else {
        statusBar()->showMessage(tr("Failed to copy selection to clipboard"), 2000);
    }
}

void MainWindow::onPaste() {
    if (!m_editorController) {
        return;
    }
    
    // Get clipboard manager
    auto* clipboardManager = m_editorController->getClipboardManager();
    
    if (!clipboardManager) {
        statusBar()->showMessage(tr("Paste operation not available"), 2000);
        return;
    }
    
    // Check if clipboard has data
    if (!clipboardManager->hasData()) {
        statusBar()->showMessage(tr("Clipboard is empty"), 2000);
        return;
    }
    
    // Paste at current cursor position or map center
    // TODO: Get actual cursor position from MapView when available
    RME::core::Position pastePosition(512, 512, 7); // Default center position
    
    if (clipboardManager->pasteAt(pastePosition)) {
        statusBar()->showMessage(tr("Pasted from clipboard"), 2000);
    } else {
        statusBar()->showMessage(tr("Failed to paste from clipboard"), 2000);
    }
}

void MainWindow::onPreferences() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onPreferences: EditorController not available";
        return;
    }
    
    RME::ui::dialogs::PreferencesDialog dialog(m_editorController->getAppSettings(), this);
    
    if (dialog.exec() == QDialog::Accepted) {
        statusBar()->showMessage(tr("Preferences saved"), 2000);
        // TODO: Apply settings that require immediate update
    }
}

// --- View menu operations (FINAL-04) ---

void MainWindow::onZoomIn() {
    if (m_mapView) {
        m_mapView->setZoom(m_mapView->getZoomFactor() * 1.12);
        statusBar()->showMessage(tr("Zoom: %1%").arg(qRound(m_mapView->getZoomFactor() * 100)), 1000);
    }
}

void MainWindow::onZoomOut() {
    if (m_mapView) {
        m_mapView->setZoom(m_mapView->getZoomFactor() / 1.12);
        statusBar()->showMessage(tr("Zoom: %1%").arg(qRound(m_mapView->getZoomFactor() * 100)), 1000);
    }
}

void MainWindow::onZoomNormal() {
    if (m_mapView) {
        m_mapView->setZoom(1.0);
        statusBar()->showMessage(tr("Zoom: 100%"), 1000);
    }
}

void MainWindow::onFloorUp() {
    if (m_mapView) {
        int currentFloor = m_mapView->getCurrentFloor();
        if (currentFloor > 0) {
            m_mapView->setCurrentFloor(currentFloor - 1);
            statusBar()->showMessage(tr("Floor: %1").arg(currentFloor - 1), 1000);
        }
    }
}

void MainWindow::onFloorDown() {
    if (m_mapView) {
        int currentFloor = m_mapView->getCurrentFloor();
        if (currentFloor < 15) {
            m_mapView->setCurrentFloor(currentFloor + 1);
            statusBar()->showMessage(tr("Floor: %1").arg(currentFloor + 1), 1000);
        }
    }
}

void MainWindow::onSetFloor(int floor) {
    if (m_mapView && floor >= 0 && floor <= 15) {
        m_mapView->setCurrentFloor(floor);
        statusBar()->showMessage(tr("Floor: %1").arg(floor), 1000);
    }
}

void MainWindow::onToggleGrid() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_GRID, false);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_GRID, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Grid enabled") : tr("Grid disabled"), 1000);
}

void MainWindow::onToggleCreatures() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_CREATURES, true);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_CREATURES, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Creatures visible") : tr("Creatures hidden"), 1000);
}

void MainWindow::onToggleSpawns() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_SPAWNS, true);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_SPAWNS, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Spawns visible") : tr("Spawns hidden"), 1000);
}

void MainWindow::onToggleHouses() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_HOUSES, true);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_HOUSES, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Houses visible") : tr("Houses hidden"), 1000);
}

void MainWindow::onToggleLights() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_LIGHTS, false);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_LIGHTS, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Lights visible") : tr("Lights hidden"), 1000);
}

void MainWindow::onToggleTooltips() {
    if (!m_editorController) {
        return;
    }
    
    auto& settings = m_editorController->getAppSettings();
    bool currentState = settings.getBool(RME::core::settings::AppSettings::SHOW_TOOLTIPS, true);
    bool newState = !currentState;
    
    settings.setBool(RME::core::settings::AppSettings::SHOW_TOOLTIPS, newState);
    
    // Update the map view if available
    if (m_mapView) {
        // TODO: Add method to refresh map view rendering
        // m_mapView->refreshRendering();
    }
    
    statusBar()->showMessage(newState ? tr("Tooltips enabled") : tr("Tooltips disabled"), 1000);
}

// --- Tools menu operations (FINAL-04) ---

void MainWindow::onMapProperties() {
    if (!m_editorController || !m_editorController->getMap()) {
        statusBar()->showMessage(tr("No map loaded"), 2000);
        return;
    }
    
    // Create and show the Map Properties dialog
    RME::ui::dialogs::MapPropertiesDialog dialog(m_editorController->getMap(), this);
    
    if (dialog.exec() == QDialog::Accepted) {
        statusBar()->showMessage(tr("Map properties updated"), 2000);
    }
}

void MainWindow::onFindItem() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onFindItem: EditorController not available";
        return;
    }
    
    // Create and show the Item Finder dialog
    RME::ui::dialogs::ItemFinderDialogQt dialog(
        this,
        m_editorController->getAssetManager()->getItemDatabase(),
        false // onlyPickupable = false
    );
    
    // Show the dialog and handle the result
    if (dialog.exec() == QDialog::Accepted) {
        const RME::core::assets::ItemData* selectedItem = dialog.getSelectedItemType();
        if (selectedItem) {
            // Item type found - could be used to set current brush or show item info
            statusBar()->showMessage(tr("Selected item: %1 (ID: %2)")
                                   .arg(selectedItem->name)
                                   .arg(selectedItem->serverID), 3000);
            
            // Integrate with brush system to set the selected item as current raw brush
            if (m_editorController && m_editorController->getBrushStateService()) {
                m_editorController->getBrushStateService()->setCurrentRawItemId(itemId);
                qDebug() << "MainWindow: Set raw brush to item ID" << itemId;
            }
            // if (m_editorController && m_editorController->getBrushStateService()) {
            //     m_editorController->getBrushStateService()->setCurrentRawItemId(selectedItem->serverID);
            // }
        }
    }
}

// --- Live collaboration operations (FINAL-05) ---

void MainWindow::createLiveCollaboration() {
    // Create live client
    m_liveClient = new RME::network::QtLiveClient(this);
    
    // Create live collaboration panel
    m_liveCollaborationPanel = new RME::ui::widgets::LiveCollaborationPanel(this);
    m_liveCollaborationPanel->setLiveClient(m_liveClient);
    
    // Set map context if available
    if (m_editorController) {
        m_liveCollaborationPanel->setMapContext(
            m_editorController->getMap(),
            m_editorController->getUndoStack(),
            m_editorController->getAssetManager()
        );
        m_liveCollaborationPanel->setEditorController(m_editorController);
    }
    
    // Add to dock manager if available
    if (m_dockManager) {
        m_dockManager->addDockWidget("Live Collaboration", m_liveCollaborationPanel, Qt::RightDockWidgetArea);
    }
    
    qDebug() << "MainWindow::createLiveCollaboration: Live collaboration components created";
}

void MainWindow::connectLiveActions() {
    // Helper lambda to disconnect the placeholder and connect the new action
    auto reconnectAction = [this](const QString& actionName, auto&& slot) {
        if (m_actions.contains(actionName)) {
            QAction* action = m_actions[actionName];
            disconnect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
        } else {
            qWarning() << "MainWindow::connectLiveActions: Action" << actionName << "not found.";
        }
    };

    // Connect live collaboration actions
    reconnectAction("ID_MENU_SERVER_HOST", [this]() { onHostServer(); });
    reconnectAction("ID_MENU_SERVER_CONNECT", [this]() { onConnectToServer(); });
    
    // TODO: Add disconnect action when it's defined in menubar.xml
    
    qDebug() << "MainWindow::connectLiveActions: Connected live collaboration actions";
}

void MainWindow::onHostServer() {
    // TODO: Implement actual server hosting when QtLiveServer is integrated
    // For now, show the server hosting dialog
    
    RME::ui::dialogs::ServerHostingDialog dialog(this);
    
    // Connect dialog signals (when actual server implementation is ready)
    connect(&dialog, &RME::ui::dialogs::ServerHostingDialog::startServerRequested,
            this, [this](const RME::ui::dialogs::ServerHostingDialog::ServerSettings& settings) {
                // TODO: Start actual server with these settings
                statusBar()->showMessage(tr("Server hosting not yet fully implemented"), 3000);
            });
    
    dialog.exec();
}

void MainWindow::onConnectToServer() {
    if (m_liveCollaborationPanel) {
        m_liveCollaborationPanel->onConnectToServer();
        statusBar()->showMessage(tr("Connecting to server..."), 2000);
    } else {
        statusBar()->showMessage(tr("Live collaboration not available"), 2000);
    }
}

void MainWindow::onDisconnectFromServer() {
    if (m_liveCollaborationPanel) {
        m_liveCollaborationPanel->onDisconnectFromServer();
        statusBar()->showMessage(tr("Disconnected from server"), 2000);
    }
}

// --- File menu operations ---

void MainWindow::onNewMap() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onNewMap: EditorController not available";
        return;
    }
    
    // Check if current map needs saving
    if (m_editorController->isMapModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            tr("Unsaved Changes"),
            tr("The current map has unsaved changes. Do you want to save before creating a new map?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            onSaveMap();
            if (m_editorController->isMapModified()) {
                return; // Save was cancelled or failed
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    // Create new map
    if (m_editorController->createNewMap()) {
        statusBar()->showMessage(tr("New map created"), 2000);
        updateWindowTitle();
        updateMenuStatesFromEditor();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create new map"));
    }
}

void MainWindow::onOpenMap() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onOpenMap: EditorController not available";
        return;
    }
    
    // Check if current map needs saving
    if (m_editorController->isMapModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            tr("Unsaved Changes"),
            tr("The current map has unsaved changes. Do you want to save before opening another map?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            onSaveMap();
            if (m_editorController->isMapModified()) {
                return; // Save was cancelled or failed
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    // Show file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Map"),
        QString(), // Default directory
        tr("OTBM Map Files (*.otbm);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        if (m_editorController->loadMap(fileName)) {
            addRecentFile(fileName);
            statusBar()->showMessage(tr("Map loaded: %1").arg(QFileInfo(fileName).baseName()), 3000);
            updateWindowTitle();
            updateMenuStatesFromEditor();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load map: %1").arg(fileName));
        }
    }
}

void MainWindow::onSaveMap() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onSaveMap: EditorController not available";
        return;
    }
    
    QString currentFilename = m_editorController->getCurrentMapFilename();
    
    if (currentFilename.isEmpty()) {
        // No filename set, use Save As
        onSaveMapAs();
        return;
    }
    
    if (m_editorController->saveMap(currentFilename)) {
        statusBar()->showMessage(tr("Map saved"), 2000);
        updateWindowTitle();
        updateMenuStatesFromEditor();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save map"));
    }
}

void MainWindow::onSaveMapAs() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onSaveMapAs: EditorController not available";
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Map As"),
        QString(), // Default directory
        tr("OTBM Map Files (*.otbm);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        // Ensure .otbm extension
        if (!fileName.endsWith(".otbm", Qt::CaseInsensitive)) {
            fileName += ".otbm";
        }
        
        if (m_editorController->saveMap(fileName)) {
            addRecentFile(fileName);
            statusBar()->showMessage(tr("Map saved as: %1").arg(QFileInfo(fileName).baseName()), 3000);
            updateWindowTitle();
            updateMenuStatesFromEditor();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save map as: %1").arg(fileName));
        }
    }
}

void MainWindow::onCloseMap() {
    if (!m_editorController) {
        return;
    }
    
    // Check if current map needs saving
    if (m_editorController->isMapModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            tr("Unsaved Changes"),
            tr("The current map has unsaved changes. Do you want to save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            onSaveMap();
            if (m_editorController->isMapModified()) {
                return; // Save was cancelled or failed
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    // Close the map
    if (m_editorController->closeMap()) {
        statusBar()->showMessage(tr("Map closed"), 2000);
        updateWindowTitle();
        updateMenuStatesFromEditor();
    }
}

void MainWindow::onImportMap() {
    if (!m_editorController) {
        qWarning() << "MainWindow::onImportMap: EditorController not available";
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Import Map"),
        QString(),
        tr("All Supported (*.otbm *.otmm);;OTBM Files (*.otbm);;OTMM Files (*.otmm);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        if (m_editorController->importMap(fileName)) {
            statusBar()->showMessage(tr("Map imported: %1").arg(QFileInfo(fileName).baseName()), 3000);
            updateWindowTitle();
            updateMenuStatesFromEditor();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to import map: %1").arg(fileName));
        }
    }
}

void MainWindow::onExportMap() {
    if (!m_editorController || !m_editorController->getMap()) {
        statusBar()->showMessage(tr("No map to export"), 2000);
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Map"),
        QString(),
        tr("OTBM Files (*.otbm);;OTMM Files (*.otmm);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        if (m_editorController->exportMap(fileName)) {
            statusBar()->showMessage(tr("Map exported: %1").arg(QFileInfo(fileName).baseName()), 3000);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to export map: %1").arg(fileName));
        }
    }
}

void MainWindow::onExportMinimap() {
    if (!m_editorController || !m_editorController->getMap()) {
        statusBar()->showMessage(tr("No map to export minimap from"), 2000);
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Minimap"),
        QString(),
        tr("PNG Images (*.png);;BMP Images (*.bmp);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        if (m_editorController->exportMinimap(fileName)) {
            statusBar()->showMessage(tr("Minimap exported: %1").arg(QFileInfo(fileName).baseName()), 3000);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to export minimap: %1").arg(fileName));
        }
    }
}

void MainWindow::onRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString fileName = action->data().toString();
        
        if (!m_editorController) {
            qWarning() << "MainWindow::onRecentFile: EditorController not available";
            return;
        }
        
        // Check if current map needs saving
        if (m_editorController->isMapModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, 
                tr("Unsaved Changes"),
                tr("The current map has unsaved changes. Do you want to save before opening another map?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );
            
            if (reply == QMessageBox::Save) {
                onSaveMap();
                if (m_editorController->isMapModified()) {
                    return; // Save was cancelled or failed
                }
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }
        
        if (m_editorController->loadMap(fileName)) {
            statusBar()->showMessage(tr("Map loaded: %1").arg(QFileInfo(fileName).baseName()), 3000);
            updateWindowTitle();
            updateMenuStatesFromEditor();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load map: %1").arg(fileName));
            // Remove from recent files if it failed to load
            // TODO: Implement removeRecentFile method
        }
    }
}

void MainWindow::onExit() {
    close(); // This will trigger closeEvent which handles unsaved changes
}

// --- Help menu operations (FINAL-06) ---

void MainWindow::onAbout() {
    RME::ui::dialogs::AboutDialog dialog(this);
    dialog.exec();
}

} // namespace ui
} // namespace RME
