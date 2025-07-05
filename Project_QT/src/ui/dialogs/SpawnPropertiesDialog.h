#ifndef RME_SPAWN_PROPERTIES_DIALOG_H
#define RME_SPAWN_PROPERTIES_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>

// Forward declarations
namespace RME {
namespace core {
namespace spawns {
    class Spawn;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Properties dialog for editing spawn properties
 * 
 * Simple dialog for editing spawn radius. Follows the UI-04 specification
 * for SpawnPropertiesDialogQt.
 */
class SpawnPropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit SpawnPropertiesDialog(QWidget* parent, RME::core::spawns::Spawn* spawn);
    ~SpawnPropertiesDialog() override = default;

    // Result access
    bool wasModified() const { return m_wasModified; }
    RME::core::spawns::Spawn* getModifiedSpawn() const { return m_spawn; }

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onSpawnRadiusChanged();

signals:
    void spawnModified(RME::core::spawns::Spawn* spawn);

private:
    // Core data
    RME::core::spawns::Spawn* m_spawn = nullptr;
    RME::core::spawns::Spawn* m_originalSpawn = nullptr; // Backup for cancel
    bool m_wasModified = false;

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QFormLayout* m_formLayout = nullptr;
    QLabel* m_infoLabel = nullptr;
    QSpinBox* m_spawnRadiusSpin = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;

    // Helper methods
    void setupUI();
    void loadSpawnData();
    void saveSpawnData();
    bool validateInput();
    void markAsModified();
    void createBackup();
    void restoreBackup();
    void connectSignals();
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_SPAWN_PROPERTIES_DIALOG_H