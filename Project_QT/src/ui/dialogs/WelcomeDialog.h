#ifndef RME_WELCOME_DIALOG_H
#define RME_WELCOME_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextBrowser>
#include <QCheckBox>
#include <QSplitter>

// Forward declarations
namespace RME {
namespace core {
namespace settings {
    class AppSettings;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Welcome dialog shown on application startup
 * 
 * This dialog provides quick access to common actions like creating a new map,
 * opening an existing map, accessing preferences, viewing recent files, and
 * displaying what's new information.
 */
class WelcomeDialog : public QDialog {
    Q_OBJECT

public:
    explicit WelcomeDialog(RME::core::settings::AppSettings& settings, QWidget* parent = nullptr);
    ~WelcomeDialog() override = default;

signals:
    void createNewMapRequested();
    void openMapRequested();
    void openSpecificMapRequested(const QString& filePath);

public slots:
    void onNewMapClicked();
    void onOpenMapClicked();
    void onPreferencesClicked();
    void onRecentFileDoubleClicked(QListWidgetItem* item);
    void onShowOnStartupToggled(bool checked);

private:
    void setupUI();
    void createLeftPanel();
    void createCenterPanel();
    void createRightPanel();
    void createBottomPanel();
    
    void loadSettings();
    void loadRecentFiles();
    void loadWhatsNew();
    
    QString getWhatsNewContent() const;
    QPixmap createLogo() const;
    
    // Reference to settings
    RME::core::settings::AppSettings& m_settings;
    
    // Main layout
    QHBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // Left panel (logo and actions)
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    QLabel* m_logoLabel;
    QLabel* m_titleLabel;
    QLabel* m_versionLabel;
    QPushButton* m_newMapButton;
    QPushButton* m_openMapButton;
    QPushButton* m_preferencesButton;
    
    // Center panel (what's new)
    QWidget* m_centerPanel;
    QVBoxLayout* m_centerLayout;
    QLabel* m_whatsNewLabel;
    QTextBrowser* m_whatsNewText;
    
    // Right panel (recent files)
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    QLabel* m_recentFilesLabel;
    QListWidget* m_recentFilesList;
    
    // Bottom panel (show on startup checkbox)
    QWidget* m_bottomPanel;
    QHBoxLayout* m_bottomLayout;
    QCheckBox* m_showOnStartupCheck;
    QPushButton* m_closeButton;
    
    // Constants
    static constexpr int LOGO_SIZE = 64;
    static constexpr int BUTTON_HEIGHT = 40;
    static constexpr int MAX_RECENT_FILES = 10;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_WELCOME_DIALOG_H