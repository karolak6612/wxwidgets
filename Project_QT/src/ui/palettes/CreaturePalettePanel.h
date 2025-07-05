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
    class CreatureDatabase;
}

// Include for CreatureData
#include "core/assets/CreatureDatabase.h"

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
    void spawnCreatureRequested(const QString& creatureName, int spawnTime);

protected:
    void setupContentUI() override;
    void connectSignals() override;
    void applySearchFilter(const QString& text) override;

private:
    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IClientDataService* m_clientDataService;
    
    // UI components
    QGroupBox* m_searchWidget = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QListWidget* m_creatureList = nullptr;
    QGroupBox* m_creatureInfoWidget = nullptr;
    QLabel* m_creatureInfoLabel = nullptr;
    QGroupBox* m_spawnControlsWidget = nullptr;
    QPushButton* m_spawnButton = nullptr;
    QPushButton* m_editPropertiesButton = nullptr;
    
    // Data
    RME::CreatureDatabase* m_creatureDatabase = nullptr;
    
    // Helper methods
    void setupUI();
    void setupCreatureList();
    void setupSearchControls();
    void setupCreatureInfo();
    void setupSpawnControls();
    void connectSignals();
    void loadCreatures();
    void filterCreatures(const QString& filter);
    void refreshCreatureList();
    void updateCreatureInfo(const QString& creatureName);
    QString getSelectedCreatureName() const;
    void showCreatureInformation(const QString& creatureName);
    void setCreatureDatabase(RME::CreatureDatabase* database);
    
    // Database integration methods
    QString determineCreatureCategory(const CreatureData& creature) const;
    QListWidgetItem* createCreatureListItem(const CreatureData& creature) const;
    QString createCreatureTooltip(const CreatureData& creature) const;
    void loadFallbackCreatures();
    
    // Event handlers
    void onCreatureSelectionChanged();
    void onCreatureDoubleClicked(QListWidgetItem* item);
    void onCreatureContextMenu(const QPoint& position);
    void onSpawnCreature();
    void onEditCreatureProperties();
    void onSearchTextChanged(const QString& text);
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_CREATURE_PALETTE_PANEL_H