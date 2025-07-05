#ifndef RME_SPAWN_SETTINGS_WIDGET_H
#define RME_SPAWN_SETTINGS_WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>

namespace RME {
namespace ui {
namespace widgets {

/**
 * @brief Widget for spawn creation settings
 * 
 * Provides UI controls for configuring spawn parameters when creating
 * new spawns via creature placement. This widget is designed to be
 * embedded in the CreaturePalettePanel.
 */
class SpawnSettingsWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit SpawnSettingsWidget(QWidget* parent = nullptr);
    ~SpawnSettingsWidget() override = default;

    // Spawn parameter access
    int getSpawnRadius() const;
    void setSpawnRadius(int radius);
    
    int getSpawnTime() const;
    void setSpawnTime(int seconds);
    
    bool isSpawnModeEnabled() const;
    void setSpawnModeEnabled(bool enabled);

    // Settings persistence
    void loadSettings();
    void saveSettings();

public slots:
    void onSpawnRadiusChanged(int radius);
    void onSpawnTimeChanged(int seconds);
    void onSpawnModeToggled(bool enabled);

signals:
    void spawnRadiusChanged(int radius);
    void spawnTimeChanged(int seconds);
    void spawnModeToggled(bool enabled);
    void spawnSettingsChanged();

private:
    void setupUI();
    void connectSignals();
    void updateUI();

    // UI components
    QFormLayout* m_formLayout = nullptr;
    QSpinBox* m_spawnRadiusSpinBox = nullptr;
    QSpinBox* m_spawnTimeSpinBox = nullptr;
    QCheckBox* m_enableSpawnModeCheckBox = nullptr;
    QLabel* m_helpLabel = nullptr;

    // State
    bool m_updatingUI = false;
};

} // namespace widgets
} // namespace ui
} // namespace RME

#endif // RME_SPAWN_SETTINGS_WIDGET_H