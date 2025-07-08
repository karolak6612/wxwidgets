#include "PreferencesDialog.h"
#include "core/settings/AppSettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QApplication>

namespace RME {
namespace ui {
namespace dialogs {

PreferencesDialog::PreferencesDialog(RME::core::settings::AppSettings& settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
{
    setWindowTitle(tr("Preferences"));
    setModal(true);
    setMinimumSize(600, 500);
    
    setupUI();
    loadSettings();
    connectSignals();
}

void PreferencesDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create tabs
    createGeneralTab();
    createDisplayTab();
    createEditorTab();
    createAdvancedTab();
    
    // Create button box
    createButtonBox();
}

void PreferencesDialog::createGeneralTab() {
    m_generalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_generalTab);
    
    // Auto-save group
    QGroupBox* autoSaveGroup = new QGroupBox(tr("Auto-save"));
    QFormLayout* autoSaveLayout = new QFormLayout(autoSaveGroup);
    
    m_autoSaveCheck = new QCheckBox(tr("Enable auto-save"));
    autoSaveLayout->addRow(m_autoSaveCheck);
    
    m_autoSaveIntervalSpin = new QSpinBox();
    m_autoSaveIntervalSpin->setRange(1, 60);
    m_autoSaveIntervalSpin->setSuffix(tr(" minutes"));
    autoSaveLayout->addRow(tr("Auto-save interval:"), m_autoSaveIntervalSpin);
    
    layout->addWidget(autoSaveGroup);
    
    // Backup group
    QGroupBox* backupGroup = new QGroupBox(tr("Backups"));
    QFormLayout* backupLayout = new QFormLayout(backupGroup);
    
    m_createBackupsCheck = new QCheckBox(tr("Create backups when saving"));
    backupLayout->addRow(m_createBackupsCheck);
    
    m_maxBackupsSpin = new QSpinBox();
    m_maxBackupsSpin->setRange(1, 50);
    backupLayout->addRow(tr("Maximum backups:"), m_maxBackupsSpin);
    
    layout->addWidget(backupGroup);
    
    // Startup group
    QGroupBox* startupGroup = new QGroupBox(tr("Startup"));
    QFormLayout* startupLayout = new QFormLayout(startupGroup);
    
    m_showWelcomeCheck = new QCheckBox(tr("Show welcome dialog"));
    startupLayout->addRow(m_showWelcomeCheck);
    
    m_checkUpdatesCheck = new QCheckBox(tr("Check for updates"));
    startupLayout->addRow(m_checkUpdatesCheck);
    
    layout->addWidget(startupGroup);
    
    // Directories group
    QGroupBox* dirGroup = new QGroupBox(tr("Directories"));
    QFormLayout* dirLayout = new QFormLayout(dirGroup);
    
    QHBoxLayout* dataLayout = new QHBoxLayout();
    m_dataDirectoryEdit = new QLineEdit();
    m_browseDataButton = new QPushButton(tr("Browse..."));
    dataLayout->addWidget(m_dataDirectoryEdit);
    dataLayout->addWidget(m_browseDataButton);
    dirLayout->addRow(tr("Data directory:"), dataLayout);
    
    QHBoxLayout* clientLayout = new QHBoxLayout();
    m_clientDirectoryEdit = new QLineEdit();
    m_browseClientButton = new QPushButton(tr("Browse..."));
    clientLayout->addWidget(m_clientDirectoryEdit);
    clientLayout->addWidget(m_browseClientButton);
    dirLayout->addRow(tr("Client directory:"), clientLayout);
    
    layout->addWidget(dirGroup);
    
    layout->addStretch();
    m_tabWidget->addTab(m_generalTab, tr("General"));
}

void PreferencesDialog::createDisplayTab() {
    m_displayTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_displayTab);
    
    // Theme group (FINAL-07)
    QGroupBox* themeGroup = new QGroupBox(tr("Theme"));
    QFormLayout* themeLayout = new QFormLayout(themeGroup);
    
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem(tr("Light"), "light");
    m_themeCombo->addItem(tr("Dark"), "dark");
    themeLayout->addRow(tr("Application theme:"), m_themeCombo);
    
    layout->addWidget(themeGroup);
    
    // Visibility group
    QGroupBox* visibilityGroup = new QGroupBox(tr("Default Visibility"));
    QFormLayout* visibilityLayout = new QFormLayout(visibilityGroup);
    
    m_showGridCheck = new QCheckBox(tr("Show grid"));
    visibilityLayout->addRow(m_showGridCheck);
    
    m_showCreaturesCheck = new QCheckBox(tr("Show creatures"));
    visibilityLayout->addRow(m_showCreaturesCheck);
    
    m_showSpawnsCheck = new QCheckBox(tr("Show spawns"));
    visibilityLayout->addRow(m_showSpawnsCheck);
    
    m_showHousesCheck = new QCheckBox(tr("Show houses"));
    visibilityLayout->addRow(m_showHousesCheck);
    
    m_showLightsCheck = new QCheckBox(tr("Show lights"));
    visibilityLayout->addRow(m_showLightsCheck);
    
    m_showTooltipsCheck = new QCheckBox(tr("Show tooltips"));
    visibilityLayout->addRow(m_showTooltipsCheck);
    
    layout->addWidget(visibilityGroup);
    
    // View group
    QGroupBox* viewGroup = new QGroupBox(tr("View Settings"));
    QFormLayout* viewLayout = new QFormLayout(viewGroup);
    
    m_zoomLevelSpin = new QSpinBox();
    m_zoomLevelSpin->setRange(25, 400);
    m_zoomLevelSpin->setSuffix(tr("%"));
    viewLayout->addRow(tr("Default zoom level:"), m_zoomLevelSpin);
    
    m_transparencySlider = new QSlider(Qt::Horizontal);
    m_transparencySlider->setRange(0, 100);
    viewLayout->addRow(tr("Transparency:"), m_transparencySlider);
    
    layout->addWidget(viewGroup);
    
    // Colors group
    QGroupBox* colorGroup = new QGroupBox(tr("Colors"));
    QFormLayout* colorLayout = new QFormLayout(colorGroup);
    
    m_gridColorButton = new QPushButton();
    m_gridColorButton->setMinimumSize(50, 25);
    colorLayout->addRow(tr("Grid color:"), m_gridColorButton);
    
    m_selectionColorButton = new QPushButton();
    m_selectionColorButton->setMinimumSize(50, 25);
    colorLayout->addRow(tr("Selection color:"), m_selectionColorButton);
    
    m_backgroundColorButton = new QPushButton();
    m_backgroundColorButton->setMinimumSize(50, 25);
    colorLayout->addRow(tr("Background color:"), m_backgroundColorButton);
    
    layout->addWidget(colorGroup);
    
    layout->addStretch();
    m_tabWidget->addTab(m_displayTab, tr("Display"));
}

void PreferencesDialog::createEditorTab() {
    m_editorTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_editorTab);
    
    // Behavior group
    QGroupBox* behaviorGroup = new QGroupBox(tr("Editor Behavior"));
    QFormLayout* behaviorLayout = new QFormLayout(behaviorGroup);
    
    m_autoSelectGroundCheck = new QCheckBox(tr("Auto-select ground brush"));
    behaviorLayout->addRow(m_autoSelectGroundCheck);
    
    m_autoCreateBordersCheck = new QCheckBox(tr("Auto-create borders"));
    behaviorLayout->addRow(m_autoCreateBordersCheck);
    
    m_warnOnLargeOperationsCheck = new QCheckBox(tr("Warn on large operations"));
    behaviorLayout->addRow(m_warnOnLargeOperationsCheck);
    
    m_snapToGridCheck = new QCheckBox(tr("Snap to grid"));
    behaviorLayout->addRow(m_snapToGridCheck);
    
    m_smoothScrollingCheck = new QCheckBox(tr("Smooth scrolling"));
    behaviorLayout->addRow(m_smoothScrollingCheck);
    
    layout->addWidget(behaviorGroup);
    
    // Undo group
    QGroupBox* undoGroup = new QGroupBox(tr("Undo/Redo"));
    QFormLayout* undoLayout = new QFormLayout(undoGroup);
    
    m_undoLimitSpin = new QSpinBox();
    m_undoLimitSpin->setRange(10, 1000);
    undoLayout->addRow(tr("Undo limit:"), m_undoLimitSpin);
    
    layout->addWidget(undoGroup);
    
    // Default brush group
    QGroupBox* brushGroup = new QGroupBox(tr("Default Brush"));
    QFormLayout* brushLayout = new QFormLayout(brushGroup);
    
    m_defaultBrushCombo = new QComboBox();
    m_defaultBrushCombo->addItems({tr("Ground"), tr("Wall"), tr("Doodad"), tr("Item"), tr("Creature")});
    brushLayout->addRow(tr("Default brush type:"), m_defaultBrushCombo);
    
    m_defaultBrushSizeSpin = new QSpinBox();
    m_defaultBrushSizeSpin->setRange(1, 15);
    brushLayout->addRow(tr("Default brush size:"), m_defaultBrushSizeSpin);
    
    layout->addWidget(brushGroup);
    
    layout->addStretch();
    m_tabWidget->addTab(m_editorTab, tr("Editor"));
}

void PreferencesDialog::createAdvancedTab() {
    m_advancedTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_advancedTab);
    
    // Logging group
    QGroupBox* loggingGroup = new QGroupBox(tr("Logging"));
    QFormLayout* loggingLayout = new QFormLayout(loggingGroup);
    
    m_enableLoggingCheck = new QCheckBox(tr("Enable logging"));
    loggingLayout->addRow(m_enableLoggingCheck);
    
    m_logLevelCombo = new QComboBox();
    m_logLevelCombo->addItems({tr("Debug"), tr("Info"), tr("Warning"), tr("Error")});
    loggingLayout->addRow(tr("Log level:"), m_logLevelCombo);
    
    layout->addWidget(loggingGroup);
    
    // Performance group
    QGroupBox* performanceGroup = new QGroupBox(tr("Performance"));
    QFormLayout* performanceLayout = new QFormLayout(performanceGroup);
    
    m_memoryCacheSizeSpin = new QSpinBox();
    m_memoryCacheSizeSpin->setRange(64, 2048);
    m_memoryCacheSizeSpin->setSuffix(tr(" MB"));
    performanceLayout->addRow(tr("Memory cache size:"), m_memoryCacheSizeSpin);
    
    m_useHardwareAccelerationCheck = new QCheckBox(tr("Use hardware acceleration"));
    performanceLayout->addRow(m_useHardwareAccelerationCheck);
    
    layout->addWidget(performanceGroup);
    
    // Network group
    QGroupBox* networkGroup = new QGroupBox(tr("Network"));
    QFormLayout* networkLayout = new QFormLayout(networkGroup);
    
    m_networkTimeoutSpin = new QSpinBox();
    m_networkTimeoutSpin->setRange(5, 120);
    m_networkTimeoutSpin->setSuffix(tr(" seconds"));
    networkLayout->addRow(tr("Network timeout:"), m_networkTimeoutSpin);
    
    layout->addWidget(networkGroup);
    
    // Experimental group
    QGroupBox* experimentalGroup = new QGroupBox(tr("Experimental"));
    QFormLayout* experimentalLayout = new QFormLayout(experimentalGroup);
    
    m_enableDebugModeCheck = new QCheckBox(tr("Enable debug mode"));
    experimentalLayout->addRow(m_enableDebugModeCheck);
    
    m_enableExperimentalFeaturesCheck = new QCheckBox(tr("Enable experimental features"));
    experimentalLayout->addRow(m_enableExperimentalFeaturesCheck);
    
    layout->addWidget(experimentalGroup);
    
    layout->addStretch();
    m_tabWidget->addTab(m_advancedTab, tr("Advanced"));
}

void PreferencesDialog::createButtonBox() {
    m_buttonLayout = new QHBoxLayout();
    
    m_restoreDefaultsButton = new QPushButton(tr("Restore Defaults"));
    m_buttonLayout->addWidget(m_restoreDefaultsButton);
    
    m_buttonLayout->addStretch();
    
    m_applyButton = new QPushButton(tr("Apply"));
    m_buttonLayout->addWidget(m_applyButton);
    
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setDefault(true);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void PreferencesDialog::connectSignals() {
    connect(m_okButton, &QPushButton::clicked, this, &PreferencesDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &PreferencesDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, this, &PreferencesDialog::onRestoreDefaults);
    
    // Directory browse buttons
    connect(m_browseDataButton, &QPushButton::clicked, this, &PreferencesDialog::onBrowseDataDirectory);
    connect(m_browseClientButton, &QPushButton::clicked, this, &PreferencesDialog::onBrowseClientDirectory);
    
    // Color buttons
    connect(m_gridColorButton, &QPushButton::clicked, this, &PreferencesDialog::onColorButtonClicked);
    connect(m_selectionColorButton, &QPushButton::clicked, this, &PreferencesDialog::onColorButtonClicked);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &PreferencesDialog::onColorButtonClicked);
    
    // Enable/disable controls based on checkboxes
    connect(m_autoSaveCheck, &QCheckBox::toggled, m_autoSaveIntervalSpin, &QSpinBox::setEnabled);
    connect(m_createBackupsCheck, &QCheckBox::toggled, m_maxBackupsSpin, &QSpinBox::setEnabled);
    connect(m_enableLoggingCheck, &QCheckBox::toggled, m_logLevelCombo, &QComboBox::setEnabled);
}

void PreferencesDialog::loadSettings() {
    // General settings
    m_autoSaveCheck->setChecked(m_settings.getBool("general/autoSave", false));
    m_autoSaveIntervalSpin->setValue(m_settings.getInt("general/autoSaveInterval", 5));
    m_createBackupsCheck->setChecked(m_settings.getBool("general/createBackups", true));
    m_maxBackupsSpin->setValue(m_settings.getInt("general/maxBackups", 5));
    m_showWelcomeCheck->setChecked(m_settings.getBool("general/showWelcome", true));
    m_checkUpdatesCheck->setChecked(m_settings.getBool("general/checkUpdates", true));
    m_dataDirectoryEdit->setText(m_settings.getString("general/dataDirectory", ""));
    m_clientDirectoryEdit->setText(m_settings.getString("general/clientDirectory", ""));
    
    // Display settings
    QString currentTheme = m_settings.getString("ui/theme", "light");
    int themeIndex = m_themeCombo->findData(currentTheme);
    if (themeIndex >= 0) {
        m_themeCombo->setCurrentIndex(themeIndex);
    }
    
    m_showGridCheck->setChecked(m_settings.getBool("display/showGrid", false));
    m_showCreaturesCheck->setChecked(m_settings.getBool("display/showCreatures", true));
    m_showSpawnsCheck->setChecked(m_settings.getBool("display/showSpawns", true));
    m_showHousesCheck->setChecked(m_settings.getBool("display/showHouses", true));
    m_showLightsCheck->setChecked(m_settings.getBool("display/showLights", false));
    m_showTooltipsCheck->setChecked(m_settings.getBool("display/showTooltips", true));
    m_zoomLevelSpin->setValue(m_settings.getInt("display/defaultZoom", 100));
    m_transparencySlider->setValue(m_settings.getInt("display/transparency", 100));
    
    // Load colors
    QColor gridColor = m_settings.getColor("display/gridColor", QColor(128, 128, 128));
    QColor selectionColor = m_settings.getColor("display/selectionColor", QColor(255, 255, 0));
    QColor backgroundColor = m_settings.getColor("display/backgroundColor", QColor(0, 0, 0));
    
    updateColorButton(m_gridColorButton, gridColor);
    updateColorButton(m_selectionColorButton, selectionColor);
    updateColorButton(m_backgroundColorButton, backgroundColor);
    
    // Editor settings
    m_autoSelectGroundCheck->setChecked(m_settings.getBool("editor/autoSelectGround", true));
    m_autoCreateBordersCheck->setChecked(m_settings.getBool("editor/autoCreateBorders", true));
    m_warnOnLargeOperationsCheck->setChecked(m_settings.getBool("editor/warnOnLargeOperations", true));
    m_undoLimitSpin->setValue(m_settings.getInt("editor/undoLimit", 100));
    m_defaultBrushCombo->setCurrentIndex(m_settings.getInt("editor/defaultBrushType", 0));
    m_defaultBrushSizeSpin->setValue(m_settings.getInt("editor/defaultBrushSize", 1));
    m_smoothScrollingCheck->setChecked(m_settings.getBool("editor/smoothScrolling", true));
    m_snapToGridCheck->setChecked(m_settings.getBool("editor/snapToGrid", false));
    
    // Advanced settings
    m_enableLoggingCheck->setChecked(m_settings.getBool("advanced/enableLogging", true));
    m_logLevelCombo->setCurrentIndex(m_settings.getInt("advanced/logLevel", 1)); // Info
    m_enableDebugModeCheck->setChecked(m_settings.getBool("advanced/debugMode", false));
    m_memoryCacheSizeSpin->setValue(m_settings.getInt("advanced/memoryCacheSize", 256));
    m_useHardwareAccelerationCheck->setChecked(m_settings.getBool("advanced/hardwareAcceleration", true));
    m_networkTimeoutSpin->setValue(m_settings.getInt("advanced/networkTimeout", 30));
    m_enableExperimentalFeaturesCheck->setChecked(m_settings.getBool("advanced/experimentalFeatures", false));
    
    // Update enabled states
    m_autoSaveIntervalSpin->setEnabled(m_autoSaveCheck->isChecked());
    m_maxBackupsSpin->setEnabled(m_createBackupsCheck->isChecked());
    m_logLevelCombo->setEnabled(m_enableLoggingCheck->isChecked());
}

void PreferencesDialog::saveSettings() {
    // General settings
    m_settings.setBool("general/autoSave", m_autoSaveCheck->isChecked());
    m_settings.setInt("general/autoSaveInterval", m_autoSaveIntervalSpin->value());
    m_settings.setBool("general/createBackups", m_createBackupsCheck->isChecked());
    m_settings.setInt("general/maxBackups", m_maxBackupsSpin->value());
    m_settings.setBool("general/showWelcome", m_showWelcomeCheck->isChecked());
    m_settings.setBool("general/checkUpdates", m_checkUpdatesCheck->isChecked());
    m_settings.setString("general/dataDirectory", m_dataDirectoryEdit->text());
    m_settings.setString("general/clientDirectory", m_clientDirectoryEdit->text());
    
    // Display settings
    QString selectedTheme = m_themeCombo->currentData().value<QString>();
    m_settings.setString("ui/theme", selectedTheme);
    
    m_settings.setBool("display/showGrid", m_showGridCheck->isChecked());
    m_settings.setBool("display/showCreatures", m_showCreaturesCheck->isChecked());
    m_settings.setBool("display/showSpawns", m_showSpawnsCheck->isChecked());
    m_settings.setBool("display/showHouses", m_showHousesCheck->isChecked());
    m_settings.setBool("display/showLights", m_showLightsCheck->isChecked());
    m_settings.setBool("display/showTooltips", m_showTooltipsCheck->isChecked());
    m_settings.setInt("display/defaultZoom", m_zoomLevelSpin->value());
    m_settings.setInt("display/transparency", m_transparencySlider->value());
    
    // Save colors
    m_settings.setColor("display/gridColor", m_buttonColors[m_gridColorButton]);
    m_settings.setColor("display/selectionColor", m_buttonColors[m_selectionColorButton]);
    m_settings.setColor("display/backgroundColor", m_buttonColors[m_backgroundColorButton]);
    
    // Editor settings
    m_settings.setBool("editor/autoSelectGround", m_autoSelectGroundCheck->isChecked());
    m_settings.setBool("editor/autoCreateBorders", m_autoCreateBordersCheck->isChecked());
    m_settings.setBool("editor/warnOnLargeOperations", m_warnOnLargeOperationsCheck->isChecked());
    m_settings.setInt("editor/undoLimit", m_undoLimitSpin->value());
    m_settings.setInt("editor/defaultBrushType", m_defaultBrushCombo->currentIndex());
    m_settings.setInt("editor/defaultBrushSize", m_defaultBrushSizeSpin->value());
    m_settings.setBool("editor/smoothScrolling", m_smoothScrollingCheck->isChecked());
    m_settings.setBool("editor/snapToGrid", m_snapToGridCheck->isChecked());
    
    // Advanced settings
    m_settings.setBool("advanced/enableLogging", m_enableLoggingCheck->isChecked());
    m_settings.setInt("advanced/logLevel", m_logLevelCombo->currentIndex());
    m_settings.setBool("advanced/debugMode", m_enableDebugModeCheck->isChecked());
    m_settings.setInt("advanced/memoryCacheSize", m_memoryCacheSizeSpin->value());
    m_settings.setBool("advanced/hardwareAcceleration", m_useHardwareAccelerationCheck->isChecked());
    m_settings.setInt("advanced/networkTimeout", m_networkTimeoutSpin->value());
    m_settings.setBool("advanced/experimentalFeatures", m_enableExperimentalFeaturesCheck->isChecked());
    
    // Save to persistent storage
    m_settings.save();
}

void PreferencesDialog::accept() {
    // Check if theme changed
    QString currentTheme = m_settings.getString("ui/theme", "light");
    QString selectedTheme = m_themeCombo->currentData().value<QString>();
    bool themeChanged = (currentTheme != selectedTheme);
    
    saveSettings();
    
    // Notify user if theme changed
    if (themeChanged) {
        QMessageBox::information(this, tr("Theme Changed"), 
                                tr("The theme will be applied when you restart the application."));
    }
    
    QDialog::accept();
}

void PreferencesDialog::reject() {
    QDialog::reject();
}

void PreferencesDialog::onApply() {
    saveSettings();
}

void PreferencesDialog::onRestoreDefaults() {
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Restore Defaults"),
        tr("Are you sure you want to restore all settings to their default values?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        restoreDefaults();
    }
}

void PreferencesDialog::restoreDefaults() {
    // Reset all controls to default values
    m_autoSaveCheck->setChecked(false);
    m_autoSaveIntervalSpin->setValue(5);
    m_createBackupsCheck->setChecked(true);
    m_maxBackupsSpin->setValue(5);
    m_showWelcomeCheck->setChecked(true);
    m_checkUpdatesCheck->setChecked(true);
    m_dataDirectoryEdit->clear();
    m_clientDirectoryEdit->clear();
    
    m_themeCombo->setCurrentIndex(0); // Default to light theme
    m_showGridCheck->setChecked(false);
    m_showCreaturesCheck->setChecked(true);
    m_showSpawnsCheck->setChecked(true);
    m_showHousesCheck->setChecked(true);
    m_showLightsCheck->setChecked(false);
    m_showTooltipsCheck->setChecked(true);
    m_zoomLevelSpin->setValue(100);
    m_transparencySlider->setValue(100);
    
    updateColorButton(m_gridColorButton, QColor(128, 128, 128));
    updateColorButton(m_selectionColorButton, QColor(255, 255, 0));
    updateColorButton(m_backgroundColorButton, QColor(0, 0, 0));
    
    m_autoSelectGroundCheck->setChecked(true);
    m_autoCreateBordersCheck->setChecked(true);
    m_warnOnLargeOperationsCheck->setChecked(true);
    m_undoLimitSpin->setValue(100);
    m_defaultBrushCombo->setCurrentIndex(0);
    m_defaultBrushSizeSpin->setValue(1);
    m_smoothScrollingCheck->setChecked(true);
    m_snapToGridCheck->setChecked(false);
    
    m_enableLoggingCheck->setChecked(true);
    m_logLevelCombo->setCurrentIndex(1);
    m_enableDebugModeCheck->setChecked(false);
    m_memoryCacheSizeSpin->setValue(256);
    m_useHardwareAccelerationCheck->setChecked(true);
    m_networkTimeoutSpin->setValue(30);
    m_enableExperimentalFeaturesCheck->setChecked(false);
}

void PreferencesDialog::onColorButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QColor currentColor = m_buttonColors.value(button, QColor(255, 255, 255));
    QColor newColor = QColorDialog::getColor(currentColor, this);
    
    if (newColor.isValid()) {
        updateColorButton(button, newColor);
    }
}

void PreferencesDialog::onBrowseDataDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Data Directory"),
        m_dataDirectoryEdit->text()
    );
    
    if (!dir.isEmpty()) {
        m_dataDirectoryEdit->setText(dir);
    }
}

void PreferencesDialog::onBrowseClientDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Client Directory"),
        m_clientDirectoryEdit->text()
    );
    
    if (!dir.isEmpty()) {
        m_clientDirectoryEdit->setText(dir);
    }
}

void PreferencesDialog::updateColorButton(QPushButton* button, const QColor& color) {
    if (!button || !color.isValid()) return;
    
    m_buttonColors[button] = color;
    
    QString styleSheet = QString("QPushButton { background-color: %1; border: 1px solid #888; }")
                        .arg(color.name());
    button->setStyleSheet(styleSheet);
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "PreferencesDialog.moc" // Removed - Q_OBJECT is in header