#ifndef RME_ADD_ITEM_TO_TILESET_DIALOG_H
#define RME_ADD_ITEM_TO_TILESET_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGroupBox>

// Forward declarations
namespace RME {
namespace core {
namespace assets {
    class MaterialManager;
    class ItemData;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for adding items to existing tileset categories
 * 
 * This dialog allows users to add single items or ranges of items
 * to existing tileset categories. Based on the wxWidgets AddItemWindow.
 */
class AddItemToTilesetDialog : public QDialog {
    Q_OBJECT

public:
    enum class TilesetCategoryType {
        Terrain,
        Doodad,
        Item,
        Wall,
        Carpet,
        Table,
        Raw,
        Collection
    };

    explicit AddItemToTilesetDialog(QWidget* parent = nullptr, 
                                   TilesetCategoryType categoryType = TilesetCategoryType::Item);
    ~AddItemToTilesetDialog() override = default;

    // Get results
    QString getSelectedTileset() const;
    QList<uint16_t> getSelectedItemIds() const;

public slots:
    void accept() override;

private slots:
    void onItemIdChanged();
    void onBrowseItem();
    void onRangeToggled(bool enabled);
    void onRangeFieldChanged();
    void onUseCurrentItem();
    void onQuickRange();
    void onTilesetChanged();

private:
    // UI setup
    void setupUI();
    void setupItemSelection();
    void setupRangeControls();
    void setupTilesetSelection();
    void setupPreview();
    void setupButtonBox();
    void connectSignals();
    
    // Data management
    void loadTilesets();
    void updateItemPreview();
    void updateRangeInfo();
    void validateInput();
    
    // Helper methods
    QString getItemName(uint16_t itemId) const;
    bool isValidItemId(uint16_t itemId) const;
    QList<uint16_t> generateItemRange() const;

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
    
    // Item selection
    QGroupBox* m_itemGroup = nullptr;
    QSpinBox* m_itemIdSpin = nullptr;
    QPushButton* m_browseButton = nullptr;
    QPushButton* m_useCurrentButton = nullptr;
    QLabel* m_itemNameLabel = nullptr;
    QLabel* m_itemPreviewLabel = nullptr;
    
    // Range controls
    QGroupBox* m_rangeGroup = nullptr;
    QCheckBox* m_rangeCheckBox = nullptr;
    QSpinBox* m_rangeStartSpin = nullptr;
    QSpinBox* m_rangeEndSpin = nullptr;
    QPushButton* m_quickRangeButton = nullptr;
    QLabel* m_rangeInfoLabel = nullptr;
    
    // Tileset selection
    QGroupBox* m_tilesetGroup = nullptr;
    QComboBox* m_tilesetCombo = nullptr;
    QLabel* m_categoryLabel = nullptr;
    
    // Data
    TilesetCategoryType m_categoryType;
    uint16_t m_currentItemId = 100;
    bool m_rangeMode = false;
    
    // TODO: Replace with actual MaterialManager when available
    // RME::core::assets::MaterialManager* m_materialManager = nullptr;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_ADD_ITEM_TO_TILESET_DIALOG_H