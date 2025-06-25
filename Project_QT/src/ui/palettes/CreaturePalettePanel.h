#ifndef RME_CREATURE_PALETTE_PANEL_H
#define RME_CREATURE_PALETTE_PANEL_H

#include "ui/palettes/BasePalettePanel.h"
#include <QTreeWidget>
#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IClientDataService.h"

// Forward declarations
namespace RME {
namespace core {
namespace assets {
    class CreatureData;
}
namespace spawns {
    class SpawnData;
}
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Creature palette panel for creature and spawn management
 * 
 * This panel provides access to all creatures and spawn management functionality.
 * It supports creature browsing, spawn creation and editing, and creature brush
 * configuration.
 */
class CreaturePalettePanel : public BasePalettePanel {
    Q_OBJECT

public:
    explicit CreaturePalettePanel(
        RME::core::IBrushStateService* brushStateService,
        RME::core::IClientDataService* clientDataService,
        QWidget* parent = nullptr
    );
    ~CreaturePalettePanel() override = default;

    // BasePalettePanel interface
    void refreshContent() override;
    void clearSelection() override;

public slots:
    void onCreatureTypeChanged();
    void onCreatureSelectionChanged();
    void onCreatureActivated(QListWidgetItem* item);
    void onSpawnSettingsChanged();
    void onCreateSpawnClicked();
    void onEditSpawnClicked();
    void onDeleteSpawnClicked();

signals:
    void creatureSelected(const QString& creatureName);
    void spawnCreated(const RME::core::spawns::SpawnData& spawnData);
    void spawnModified(const RME::core::spawns::SpawnData& spawnData);

protected:
    void setupContentUI() override;
    void connectSignals() override;
    void applySearchFilter(const QString& text) override;

private:
    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IClientDataService* m_clientDataService;
    
    // UI components
    QSplitter* m_splitter = nullptr;
    
    // Creature type selection
    QGroupBox* m_typeGroup = nullptr;
    QComboBox* m_creatureTypeCombo = nullptr;
    
    // Creature display
    QGroupBox* m_creatureGroup = nullptr;
    QListWidget* m_creatureList = nullptr;
    
    // Spawn settings
    QGroupBox* m_spawnGroup = nullptr;
    QSpinBox* m_spawnRadiusSpinBox = nullptr;
    QSpinBox* m_spawnCountSpinBox = nullptr;
    QSpinBox* m_spawnTimeSpinBox = nullptr;
    QCheckBox* m_autoCreateSpawnCheckBox = nullptr;
    
    // Spawn management
    QGroupBox* m_spawnManagementGroup = nullptr;
    QPushButton* m_createSpawnButton = nullptr;
    QPushButton* m_editSpawnButton = nullptr;
    QPushButton* m_deleteSpawnButton = nullptr;
    QListWidget* m_spawnList = nullptr;
    
    // Creature preview
    QGroupBox* m_previewGroup = nullptr;
    QLabel* m_creaturePreviewLabel = nullptr;
    QLabel* m_creatureInfoLabel = nullptr;
    
    // Data
    QList<const RME::core::assets::CreatureData*> m_currentCreatures;
    QString m_currentCreatureType;
    QString m_selectedCreatureName;
    
    // Helper methods
    void populateCreatureTypes();
    void populateCreatures(const QString& type);
    void populateSpawns();
    void updateCreaturePreview(const QString& creatureName);
    void updateSpawnButtons();
    
    // Creature management
    void addCreatureToList(const RME::core::assets::CreatureData* creatureData);
    QListWidgetItem* createCreatureListItem(const QString& name, const QString& displayName, const QIcon& icon = QIcon());
    
    // Spawn management
    void addSpawnToList(const RME::core::spawns::SpawnData& spawnData);
    QListWidgetItem* createSpawnListItem(const RME::core::spawns::SpawnData& spawnData);
    
    // Search and filtering
    bool matchesSearchFilter(const QString& creatureName, const QString& filter) const;
    void filterCreaturesByType(const QString& type);
    void filterCreaturesBySearch(const QString& searchText);
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_CREATURE_PALETTE_PANEL_H