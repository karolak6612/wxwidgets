#ifndef RME_NEW_TILESET_DIALOG_H
#define RME_NEW_TILESET_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QComboBox>

// Forward declarations
namespace RME {
namespace core {
namespace assets {
    class MaterialManager;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for creating new tileset categories
 * 
 * This dialog allows users to create new tileset categories and optionally
 * add an initial item to the new tileset. Based on the wxWidgets AddTilesetWindow.
 */
class NewTilesetDialog : public QDialog {
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

    explicit NewTilesetDialog(QWidget* parent = nullptr, 
                             TilesetCategoryType categoryType = TilesetCategoryType::Item);
    ~NewTilesetDialog() override = default;

    // Get results
    QString getTilesetName() const;
    uint16_t getInitialItemId() const;
    TilesetCategoryType getCategoryType() const { return m_categoryType; }

public slots:
    void accept() override;

private slots:
    void onTilesetNameChanged();
    void onItemIdChanged();
    void onBrowseItem();
    void validateInput();

private:
    // UI setup
    void setupUI();
    void setupTilesetInfo();
    void setupInitialItem();
    void setupButtonBox();
    void connectSignals();
    
    // Data management
    void updateItemPreview();
    
    // Helper methods
    QString getItemName(uint16_t itemId) const;
    bool isValidItemId(uint16_t itemId) const;
    bool isValidTilesetName(const QString& name) const;
    QString getCategoryDisplayName() const;

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
    
    // Tileset info
    QGroupBox* m_tilesetGroup = nullptr;
    QLineEdit* m_tilesetNameEdit = nullptr;
    QLabel* m_categoryLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    
    // Initial item
    QGroupBox* m_itemGroup = nullptr;
    QSpinBox* m_itemIdSpin = nullptr;
    QPushButton* m_browseButton = nullptr;
    QLabel* m_itemNameLabel = nullptr;
    QLabel* m_itemPreviewLabel = nullptr;
    
    // Data
    TilesetCategoryType m_categoryType;
    uint16_t m_currentItemId = 100;
    
    // TODO: Replace with actual MaterialManager when available
    // RME::core::assets::MaterialManager* m_materialManager = nullptr;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_NEW_TILESET_DIALOG_H