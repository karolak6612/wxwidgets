#ifndef RME_EDIT_SPAWN_DIALOG_QT_H
#define RME_EDIT_SPAWN_DIALOG_QT_H

#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QStringList>

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    namespace assets { class CreatureDatabase; }
    namespace spawns { class Spawn; }
}
namespace ui {
namespace dialogs { class CreatureFinderDialogQt; }
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for editing spawn properties of an existing spawn point
 * 
 * Allows editing of spawn radius, creature list, and respawn time for
 * spawn points that exist on map tiles. Provides validation and proper
 * data handling for spawn modifications.
 */
class EditSpawnDialogQt : public QDialog {
    Q_OBJECT

public:
    explicit EditSpawnDialogQt(QWidget* parent, 
                               RME::core::Tile* tileDataSource,
                               RME::core::assets::CreatureDatabase* creatureDatabase);
    ~EditSpawnDialogQt() override = default;

    // Data access methods
    int getSpawnRadius() const;
    int getRespawnTime() const;
    QStringList getCreatureList() const;

    // Validation
    bool hasValidData() const;

public slots:
    void accept() override;

private slots:
    void onAddCreature();
    void onRemoveCreature();
    void onCreatureSelectionChanged();
    void onSpawnDataChanged();

signals:
    void spawnDataChanged(int radius, int respawnTime, const QStringList& creatureList);

private:
    void setupUI();
    void connectSignals();
    void loadData();
    bool validateInputs();
    void updateButtonStates();

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QFormLayout* m_formLayout = nullptr;
    
    // Spawn properties group
    QGroupBox* m_propertiesGroup = nullptr;
    QSpinBox* m_radiusSpinBox = nullptr;
    QSpinBox* m_respawnTimeSpinBox = nullptr;
    
    // Creature list group
    QGroupBox* m_creatureGroup = nullptr;
    QListWidget* m_creatureListWidget = nullptr;
    QHBoxLayout* m_creatureButtonLayout = nullptr;
    QPushButton* m_addCreatureButton = nullptr;
    QPushButton* m_removeCreatureButton = nullptr;
    
    // Dialog buttons
    QDialogButtonBox* m_buttonBox = nullptr;

    // Data sources
    RME::core::Tile* m_tileDataSource = nullptr;
    RME::core::assets::CreatureDatabase* m_creatureDatabase = nullptr;

    // Internal data copies for modification
    int m_currentRadius = 0;
    int m_currentRespawnTime = 0;
    QStringList m_currentCreatureList;
    
    // State tracking
    bool m_dataLoaded = false;
    bool m_updatingUI = false;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_EDIT_SPAWN_DIALOG_QT_H