#include "ui/MainWindow.h" // Relative path to header

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

    // Placeholder calls, actual menu creation will be called after this
    createMenusFromXML(":/menubar.xml"); // Will be called after stubs are filled
    updateRecentFilesMenu();
    updateMenus();

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

                connect(action, &QAction::triggered, this, &MainWindow::onPlaceholderActionTriggered);

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

} // namespace ui
} // namespace RME
