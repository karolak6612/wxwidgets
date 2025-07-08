#include "SpawnSettingsWidget.h"
#include "core/settings/AppSettings.h"

namespace RME {
namespace ui {
namespace widgets {

SpawnSettingsWidget::SpawnSettingsWidget(QWidget* parent)
    : QGroupBox("Spawn Settings", parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

int SpawnSettingsWidget::getSpawnRadius() const
{
    return m_spawnRadiusSpinBox->value();
}

void SpawnSettingsWidget::setSpawnRadius(int radius)
{
    if (m_spawnRadiusSpinBox->value() != radius) {
        m_updatingUI = true;
        m_spawnRadiusSpinBox->setValue(radius);
        m_updatingUI = false;
    }
}

int SpawnSettingsWidget::getSpawnTime() const
{
    return m_spawnTimeSpinBox->value();
}

void SpawnSettingsWidget::setSpawnTime(int seconds)
{
    if (m_spawnTimeSpinBox->value() != seconds) {
        m_updatingUI = true;
        m_spawnTimeSpinBox->setValue(seconds);
        m_updatingUI = false;
    }
}

bool SpawnSettingsWidget::isSpawnModeEnabled() const
{
    return m_enableSpawnModeCheckBox->isChecked();
}

void SpawnSettingsWidget::setSpawnModeEnabled(bool enabled)
{
    if (m_enableSpawnModeCheckBox->isChecked() != enabled) {
        m_updatingUI = true;
        m_enableSpawnModeCheckBox->setChecked(enabled);
        m_updatingUI = false;
        updateUI();
    }
}

void SpawnSettingsWidget::loadSettings()
{
    auto* settings = RME::core::settings::AppSettings::getInstance();
    if (settings) {
        // Load default spawn settings
        setSpawnRadius(settings->value("spawn/defaultRadius", 5).toInt());
        setSpawnTime(settings->value("spawn/defaultTime", 60).toInt());
        setSpawnModeEnabled(settings->value("spawn/enableByDefault", false).toBool());
    } else {
        // Fallback defaults
        setSpawnRadius(5);
        setSpawnTime(60);
        setSpawnModeEnabled(false);
    }
}

void SpawnSettingsWidget::saveSettings()
{
    auto* settings = RME::core::settings::AppSettings::getInstance();
    if (settings) {
        settings->setValue("spawn/defaultRadius", getSpawnRadius());
        settings->setValue("spawn/defaultTime", getSpawnTime());
        settings->setValue("spawn/enableByDefault", isSpawnModeEnabled());
    }
}

void SpawnSettingsWidget::setupUI()
{
    m_formLayout = new QFormLayout(this);
    
    // Enable spawn mode checkbox
    m_enableSpawnModeCheckBox = new QCheckBox("Enable Spawn Mode", this);
    m_enableSpawnModeCheckBox->setObjectName("enableSpawnModeCheckBox");
    m_enableSpawnModeCheckBox->setToolTip("When enabled, placing creatures will create spawn areas");
    m_formLayout->addRow(m_enableSpawnModeCheckBox);
    
    // Spawn radius
    m_spawnRadiusSpinBox = new QSpinBox(this);
    m_spawnRadiusSpinBox->setObjectName("spawnRadiusSpinBox");
    m_spawnRadiusSpinBox->setMinimum(1);
    m_spawnRadiusSpinBox->setMaximum(50);
    m_spawnRadiusSpinBox->setValue(5);
    m_spawnRadiusSpinBox->setSuffix(" tiles");
    m_spawnRadiusSpinBox->setToolTip("Radius of the spawn area in tiles");
    m_formLayout->addRow("Spawn Radius:", m_spawnRadiusSpinBox);
    
    // Spawn time
    m_spawnTimeSpinBox = new QSpinBox(this);
    m_spawnTimeSpinBox->setObjectName("spawnTimeSpinBox");
    m_spawnTimeSpinBox->setMinimum(1);
    m_spawnTimeSpinBox->setMaximum(86400); // 24 hours
    m_spawnTimeSpinBox->setValue(60);
    m_spawnTimeSpinBox->setSuffix(" seconds");
    m_spawnTimeSpinBox->setToolTip("Time between creature respawns in seconds");
    m_formLayout->addRow("Spawn Time:", m_spawnTimeSpinBox);
    
    // Help label
    m_helpLabel = new QLabel(this);
    m_helpLabel->setObjectName("helpLabel");
    m_helpLabel->setWordWrap(true);
    m_helpLabel->setStyleSheet("QLabel { color: gray; font-size: 10px; }");
    m_helpLabel->setText("When spawn mode is enabled, placing a creature will create a spawn area "
                         "with the specified radius and respawn time.");
    m_formLayout->addRow(m_helpLabel);
    
    updateUI();
}

void SpawnSettingsWidget::connectSignals()
{
    connect(m_enableSpawnModeCheckBox, &QCheckBox::toggled,
            this, &SpawnSettingsWidget::onSpawnModeToggled);
    
    connect(m_spawnRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SpawnSettingsWidget::onSpawnRadiusChanged);
    
    connect(m_spawnTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SpawnSettingsWidget::onSpawnTimeChanged);
}

void SpawnSettingsWidget::updateUI()
{
    bool spawnModeEnabled = m_enableSpawnModeCheckBox->isChecked();
    m_spawnRadiusSpinBox->setEnabled(spawnModeEnabled);
    m_spawnTimeSpinBox->setEnabled(spawnModeEnabled);
    
    // Update help text based on mode
    if (spawnModeEnabled) {
        m_helpLabel->setText("Spawn mode is active. Placing creatures will create spawn areas.");
        m_helpLabel->setStyleSheet("QLabel { color: green; font-size: 10px; font-weight: bold; }");
    } else {
        m_helpLabel->setText("Spawn mode is disabled. Creatures will be placed individually.");
        m_helpLabel->setStyleSheet("QLabel { color: gray; font-size: 10px; }");
    }
}

void SpawnSettingsWidget::onSpawnRadiusChanged(int radius)
{
    if (m_updatingUI) return;
    
    emit spawnRadiusChanged(radius);
    emit spawnSettingsChanged();
    saveSettings();
}

void SpawnSettingsWidget::onSpawnTimeChanged(int seconds)
{
    if (m_updatingUI) return;
    
    emit spawnTimeChanged(seconds);
    emit spawnSettingsChanged();
    saveSettings();
}

void SpawnSettingsWidget::onSpawnModeToggled(bool enabled)
{
    if (m_updatingUI) return;
    
    updateUI();
    emit spawnModeToggled(enabled);
    emit spawnSettingsChanged();
    saveSettings();
}

} // namespace widgets
} // namespace ui
} // namespace RME

// #include "SpawnSettingsWidget.moc" // Removed - Q_OBJECT is in header