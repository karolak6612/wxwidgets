#pragma once // Using pragma once for include guard

#include <QMainWindow>
#include <QMap>     // For storing actions
#include <QList>    // For QList<QAction*> m_recentFileActions

// Forward declarations for Qt classes
class QAction;
class QMenu;
class QSettings;
class QCloseEvent;
class QXmlStreamReader; // For argument type in private method
class QMenuBar;         // For argument type in private method


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
    void onPlaceholderActionTriggered(); // Placeholder for all menu actions initially
    void openRecentFile();             // Slot for dynamic recent file actions
    void updateMenus();                // Slot to update enabled/checked state of actions

private:
    void createMenusFromXML(const QString& xmlFilePath);
    void parseMenuNode(QXmlStreamReader& xml, QMenu* parentMenu, QMenuBar* menuBarInstance); // Helper for XML parsing

    void loadWindowSettings();
    void saveWindowSettings();
    void updateRecentFilesMenu();

    // QMenuBar* m_menuBar; // QMainWindow has one implicitly via menuBar()
    QStatusBar* m_statusBar = nullptr; // Initialized in constructor
    QSettings* m_settings = nullptr;   // Initialized in constructor, owned by this class

    QMap<QString, QAction*> m_actions; // Store actions by their XML "action" name for easy access
    QMenu* m_recentFilesMenu = nullptr;  // Pointer to the 'Recent Files' QMenu

    // MaxRecentFiles should be a static const int or similar
    // For simplicity, using a const int member, or it can be defined in .cpp
    static const int MaxRecentFiles = 10;
    QList<QAction*> m_recentFileActions; // To keep track of dynamically created recent file actions for easy clearing
};

} // namespace ui
} // namespace RME
