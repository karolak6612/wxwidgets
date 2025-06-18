#ifndef RME_HOUSE_PALETTE_TAB_H
#define RME_HOUSE_PALETTE_TAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QtGlobal>

// Forward declarations
namespace RME {
namespace core {
    namespace houses { class Houses; }
    namespace world { class TownManager; }
    namespace brush { class BrushStateManager; }
    namespace editor { class EditorControllerInterface; }
}
namespace ui {
namespace dialogs { class EditHouseDialogQt; }
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief House palette tab for the main palette system
 * 
 * Provides UI for managing houses including town filtering, house selection,
 * and brush mode switching between house tiles and house exit modes.
 */
class HousePaletteTab : public QWidget {
    Q_OBJECT

public:
    explicit HousePaletteTab(QWidget* parent = nullptr);
    ~HousePaletteTab() override = default;

    // Integration with core systems
    void setHouseManager(RME::core::houses::Houses* houseManager);
    void setTownManager(RME::core::world::TownManager* townManager);
    void setBrushStateManager(RME::core::brush::BrushStateManager* brushManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);

    // Public interface
    void refreshContent();
    void loadHousesForTown(quint32 townId);
    void loadHousesForNoTown();
    quint32 getSelectedHouseId() const;

public slots:
    void onTownSelectionChanged();
    void onHouseSelectionChanged();
    void onHouseDoubleClicked();
    void onHouseContextMenu(const QPoint& position);
    void onAddHouse();
    void onEditHouse();
    void onRemoveHouse();
    void onMoveHouseToTown();
    void onBrushModeChanged();

signals:
    void houseSelected(quint32 houseId);
    void brushModeChanged(const QString& brushType, quint32 houseId);

private:
    void setupUI();
    void connectSignals();
    void populateTownCombo();
    void updateHouseList();
    void updateBrushState();
    void showHouseContextMenu(const QPoint& globalPos);
    
    // Helper methods
    QString formatHouseListItem(quint32 houseId, const QString& houseName, int size) const;
    QList<quint32> getSelectedHouseIds() const;
    void selectHouseInList(quint32 houseId);

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QComboBox* m_townCombo = nullptr;
    QListWidget* m_houseList = nullptr;
    QHBoxLayout* m_buttonLayout = nullptr;
    QPushButton* m_addHouseButton = nullptr;
    QPushButton* m_editHouseButton = nullptr;
    QPushButton* m_removeHouseButton = nullptr;
    QRadioButton* m_drawHouseTilesRadio = nullptr;
    QRadioButton* m_setHouseExitRadio = nullptr;
    QButtonGroup* m_houseBrushModeGroup = nullptr;
    QMenu* m_contextMenu = nullptr;

    // Core system integration
    RME::core::houses::Houses* m_houseManager = nullptr;
    RME::core::world::TownManager* m_townManager = nullptr;
    RME::core::brush::BrushStateManager* m_brushStateManager = nullptr;
    RME::core::editor::EditorControllerInterface* m_editorController = nullptr;

    // State
    quint32 m_currentTownId = 0; // 0 means "No Town"
    bool m_updatingUI = false;
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_HOUSE_PALETTE_TAB_H