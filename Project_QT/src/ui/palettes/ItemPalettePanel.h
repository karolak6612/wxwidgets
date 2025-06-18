#ifndef RME_ITEM_PALETTE_PANEL_H
#define RME_ITEM_PALETTE_PANEL_H

#include "ui/palettes/BasePalettePanel.h"
#include <QTreeWidget>
#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>

// Forward declarations
namespace RME {
namespace core {
namespace assets {
    class ItemData;
    class MaterialData;
}
namespace brush {
    class Brush;
}
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Item palette panel for item and brush selection
 * 
 * This panel provides access to all items, materials, and brushes available
 * in the editor. It supports categorized browsing, search functionality,
 * and brush configuration.
 */
class ItemPalettePanel : public BasePalettePanel {
    Q_OBJECT

public:
    explicit ItemPalettePanel(QWidget* parent = nullptr);
    ~ItemPalettePanel() override = default;

    // BasePalettePanel interface
    void refreshContent() override;
    void clearSelection() override;

public slots:
    void onCategoryChanged();
    void onItemSelectionChanged();
    void onItemActivated(QListWidgetItem* item);
    void onBrushSizeChanged(int size);
    void onBrushShapeChanged();
    void onAutoAssignChanged(bool enabled);

signals:
    void itemSelected(quint16 itemId);
    void brushSelected(RME::core::brush::Brush* brush);
    void brushConfigurationChanged();

protected:
    void setupContentUI() override;
    void connectSignals() override;
    void applySearchFilter(const QString& text) override;

private:
    // UI components
    QSplitter* m_splitter = nullptr;
    
    // Category selection
    QGroupBox* m_categoryGroup = nullptr;
    QTreeWidget* m_categoryTree = nullptr;
    
    // Item display
    QGroupBox* m_itemGroup = nullptr;
    QListWidget* m_itemList = nullptr;
    
    // Brush configuration
    QGroupBox* m_brushGroup = nullptr;
    QComboBox* m_brushShapeCombo = nullptr;
    QSpinBox* m_brushSizeSpinBox = nullptr;
    QCheckBox* m_autoAssignCheckBox = nullptr;
    
    // Item preview
    QGroupBox* m_previewGroup = nullptr;
    QLabel* m_previewLabel = nullptr;
    QLabel* m_itemInfoLabel = nullptr;
    
    // Data
    QList<const RME::core::assets::ItemData*> m_currentItems;
    QList<const RME::core::assets::MaterialData*> m_currentMaterials;
    QString m_currentCategory;
    
    // Helper methods
    void populateCategories();
    void populateItems(const QString& category);
    void updateItemPreview(quint16 itemId);
    void updateBrushConfiguration();
    void createBrushFromSelection();
    
    // Category management
    void addCategoryItem(const QString& name, const QString& displayName, const QIcon& icon = QIcon());
    QTreeWidgetItem* findCategoryItem(const QString& name);
    
    // Item management
    void addItemToList(const RME::core::assets::ItemData* itemData);
    void addMaterialToList(const RME::core::assets::MaterialData* materialData);
    QListWidgetItem* createItemListItem(quint16 itemId, const QString& name, const QIcon& icon = QIcon());
    
    // Search and filtering
    bool matchesSearchFilter(const QString& itemName, const QString& filter) const;
    void filterItemsByCategory(const QString& category);
    void filterItemsBySearch(const QString& searchText);
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_ITEM_PALETTE_PANEL_H