#ifndef RME_PREFERENCES_DIALOG_H
#define RME_PREFERENCES_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QColorDialog>
#include <QPushButton>

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
 * @brief Dialog for configuring application preferences
 * 
 * This dialog provides a tabbed interface for configuring various
 * application settings including general preferences, display options,
 * editor behavior, and advanced settings.
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(RME::core::settings::AppSettings& settings, QWidget* parent = nullptr);
    ~PreferencesDialog() override = default;

public slots:
    void accept() override;
    void reject() override;
    void onRestoreDefaults();
    void onApply();

private slots:
    void onColorButtonClicked();
    void onBrowseDataDirectory();
    void onBrowseClientDirectory();

private:
    void setupUI();
    void createGeneralTab();
    void createDisplayTab();
    void createEditorTab();
    void createAdvancedTab();
    void createButtonBox();
    
    void loadSettings();
    void saveSettings();
    void restoreDefaults();
    
    void connectSignals();
    void updateColorButton(QPushButton* button, const QColor& color);

    // Reference to settings
    RME::core::settings::AppSettings& m_settings;
    
    // Main layout
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // General tab
    QWidget* m_generalTab;
    QCheckBox* m_autoSaveCheck;
    QSpinBox* m_autoSaveIntervalSpin;
    QCheckBox* m_createBackupsCheck;
    QSpinBox* m_maxBackupsSpin;
    QCheckBox* m_showWelcomeCheck;
    QCheckBox* m_checkUpdatesCheck;
    QLineEdit* m_dataDirectoryEdit;
    QPushButton* m_browseDataButton;
    QLineEdit* m_clientDirectoryEdit;
    QPushButton* m_browseClientButton;
    
    // Display tab
    QWidget* m_displayTab;
    QComboBox* m_themeCombo; // FINAL-07: Theme selection
    QCheckBox* m_showGridCheck;
    QCheckBox* m_showCreaturesCheck;
    QCheckBox* m_showSpawnsCheck;
    QCheckBox* m_showHousesCheck;
    QCheckBox* m_showLightsCheck;
    QCheckBox* m_showTooltipsCheck;
    QSpinBox* m_zoomLevelSpin;
    QSlider* m_transparencySlider;
    QPushButton* m_gridColorButton;
    QPushButton* m_selectionColorButton;
    QPushButton* m_backgroundColorButton;
    
    // Editor tab
    QWidget* m_editorTab;
    QCheckBox* m_autoSelectGroundCheck;
    QCheckBox* m_autoCreateBordersCheck;
    QCheckBox* m_warnOnLargeOperationsCheck;
    QSpinBox* m_undoLimitSpin;
    QComboBox* m_defaultBrushCombo;
    QSpinBox* m_defaultBrushSizeSpin;
    QCheckBox* m_smoothScrollingCheck;
    QCheckBox* m_snapToGridCheck;
    
    // Advanced tab
    QWidget* m_advancedTab;
    QCheckBox* m_enableLoggingCheck;
    QComboBox* m_logLevelCombo;
    QCheckBox* m_enableDebugModeCheck;
    QSpinBox* m_memoryCacheSizeSpin;
    QCheckBox* m_useHardwareAccelerationCheck;
    QSpinBox* m_networkTimeoutSpin;
    QCheckBox* m_enableExperimentalFeaturesCheck;
    
    // Button box
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QPushButton* m_restoreDefaultsButton;
    
    // Color storage for buttons
    QMap<QPushButton*, QColor> m_buttonColors;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_PREFERENCES_DIALOG_H