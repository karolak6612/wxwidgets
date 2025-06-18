#include "ui/palettes/ItemPalettePanel.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/MaterialManager.h"

#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QIcon>

namespace RME {
namespace ui {
namespace palettes {

ItemPalettePanel::ItemPalettePanel(QWidget* parent)
    : BasePalettePanel(tr("Items"), parent)
{
    setMinimumWidth(250);
    setMaximumWidth(400);
}

void ItemPalettePanel::setupContentUI() {
    // Create splitter for resizable sections
    m_splitter = new QSplitter(Qt::Vertical);
    m_contentLayout->addWidget(m_splitter);
    
    // Category selection group
    m_categoryGroup = new QGroupBox(tr("Categories"));
    m_categoryTree = new QTreeWidget();
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setMaximumHeight(150);
    
    QVBoxLayout* categoryLayout = new QVBoxLayout(m_categoryGroup);
    categoryLayout->addWidget(m_categoryTree);
    m_splitter->addWidget(m_categoryGroup);
    
    // Item display group
    m_itemGroup = new QGroupBox(tr("Items"));
    m_itemList = new QListWidget();
    m_itemList->setViewMode(QListWidget::IconMode);
    m_itemList->setIconSize(QSize(32, 32));
    m_itemList->setGridSize(QSize(40, 40));
    m_itemList->setResizeMode(QListWidget::Adjust);
    
    QVBoxLayout* itemLayout = new QVBoxLayout(m_itemGroup);
    itemLayout->addWidget(m_itemList);
    m_splitter->addWidget(m_itemGroup);
    
    // Brush configuration group
    m_brushGroup = new QGroupBox(tr("Brush Settings"));
    
    QVBoxLayout* brushLayout = new QVBoxLayout(m_brushGroup);
    
    // Brush shape
    QHBoxLayout* shapeLayout = new QHBoxLayout();
    shapeLayout->addWidget(new QLabel(tr("Shape:")));
    m_brushShapeCombo = new QComboBox();
    m_brushShapeCombo->addItem(tr("Square"), 0);
    m_brushShapeCombo->addItem(tr("Circle"), 1);
    m_brushShapeCombo->addItem(tr("Diamond"), 2);
    shapeLayout->addWidget(m_brushShapeCombo);
    brushLayout->addLayout(shapeLayout);
    
    // Brush size
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel(tr("Size:")));
    m_brushSizeSpinBox = new QSpinBox();
    m_brushSizeSpinBox->setRange(1, 15);
    m_brushSizeSpinBox->setValue(1);
    sizeLayout->addWidget(m_brushSizeSpinBox);
    brushLayout->addLayout(sizeLayout);
    
    // Auto-assign option
    m_autoAssignCheckBox = new QCheckBox(tr("Auto-assign action ID"));
    m_autoAssignCheckBox->setChecked(true);
    brushLayout->addWidget(m_autoAssignCheckBox);
    
    m_splitter->addWidget(m_brushGroup);
    
    // Item preview group
    m_previewGroup = new QGroupBox(tr("Preview"));
    m_previewGroup->setMaximumHeight(120);
    
    QVBoxLayout* previewLayout = new QVBoxLayout(m_previewGroup);
    m_previewLabel = new QLabel();
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumSize(64, 64);
    m_previewLabel->setStyleSheet("QLabel { border: 1px solid gray; background-color: white; }");
    
    m_itemInfoLabel = new QLabel(tr("No item selected"));
    m_itemInfoLabel->setWordWrap(true);
    m_itemInfoLabel->setAlignment(Qt::AlignTop);
    
    previewLayout->addWidget(m_previewLabel);
    previewLayout->addWidget(m_itemInfoLabel);
    m_splitter->addWidget(m_previewGroup);
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0); // Categories - fixed
    m_splitter->setStretchFactor(1, 1); // Items - expandable
    m_splitter->setStretchFactor(2, 0); // Brush settings - fixed
    m_splitter->setStretchFactor(3, 0); // Preview - fixed
    
    // Populate initial content
    populateCategories();
}

void ItemPalettePanel::connectSignals() {
    BasePalettePanel::connectSignals();
    
    if (m_categoryTree) {
        connect(m_categoryTree, &QTreeWidget::currentItemChanged,
                this, &ItemPalettePanel::onCategoryChanged);
    }
    
    if (m_itemList) {
        connect(m_itemList, &QListWidget::currentItemChanged,
                this, &ItemPalettePanel::onItemSelectionChanged);
        connect(m_itemList, &QListWidget::itemActivated,
                this, &ItemPalettePanel::onItemActivated);
    }
    
    if (m_brushSizeSpinBox) {
        connect(m_brushSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPalettePanel::onBrushSizeChanged);
    }
    
    if (m_brushShapeCombo) {
        connect(m_brushShapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ItemPalettePanel::onBrushShapeChanged);
    }
    
    if (m_autoAssignCheckBox) {
        connect(m_autoAssignCheckBox, &QCheckBox::toggled,
                this, &ItemPalettePanel::onAutoAssignChanged);
    }
}

void ItemPalettePanel::refreshContent() {
    if (!m_editorController) {
        return;
    }
    
    // Refresh categories and items based on current editor state
    populateCategories();
    
    // Refresh current category if one is selected
    if (m_categoryTree->currentItem()) {
        QString category = m_categoryTree->currentItem()->data(0, Qt::UserRole).toString();
        populateItems(category);
    }
}

void ItemPalettePanel::clearSelection() {
    BasePalettePanel::clearSelection();
    
    if (m_itemList) {
        m_itemList->clearSelection();
    }
    
    if (m_categoryTree) {
        m_categoryTree->clearSelection();
    }
    
    updateItemPreview(0);
}

void ItemPalettePanel::applySearchFilter(const QString& text) {
    if (!m_itemList) {
        return;
    }
    
    // Filter items based on search text
    for (int i = 0; i < m_itemList->count(); ++i) {
        QListWidgetItem* item = m_itemList->item(i);
        QString itemName = item->text();
        bool matches = matchesSearchFilter(itemName, text);
        item->setHidden(!matches);
    }
}

void ItemPalettePanel::populateCategories() {
    if (!m_categoryTree || !m_editorController) {
        return;
    }
    
    m_categoryTree->clear();
    
    // Add standard categories
    addCategoryItem("grounds", tr("Grounds"));
    addCategoryItem("walls", tr("Walls"));
    addCategoryItem("doodads", tr("Doodads"));
    addCategoryItem("items", tr("Items"));
    addCategoryItem("containers", tr("Containers"));
    addCategoryItem("doors", tr("Doors"));
    addCategoryItem("creatures", tr("Creatures"));
    addCategoryItem("spawns", tr("Spawns"));
    
    // Select first category by default
    if (m_categoryTree->topLevelItemCount() > 0) {
        m_categoryTree->setCurrentItem(m_categoryTree->topLevelItem(0));
    }
}

void ItemPalettePanel::populateItems(const QString& category) {
    if (!m_itemList || !m_editorController) {
        return;
    }
    
    m_itemList->clear();
    m_currentItems.clear();
    m_currentMaterials.clear();
    m_currentCategory = category;
    
    // TODO: Populate items based on category
    // This would integrate with ItemDatabase and MaterialManager
    
    // For now, add placeholder items
    for (int i = 1; i <= 20; ++i) {
        QString itemName = QString("%1 Item %2").arg(category).arg(i);
        QListWidgetItem* item = createItemListItem(100 + i, itemName);
        m_itemList->addItem(item);
    }
}

void ItemPalettePanel::updateItemPreview(quint16 itemId) {
    if (!m_previewLabel || !m_itemInfoLabel) {
        return;
    }
    
    if (itemId == 0) {
        m_previewLabel->clear();
        m_previewLabel->setText(tr("No Preview"));
        m_itemInfoLabel->setText(tr("No item selected"));
        return;
    }
    
    // TODO: Get actual item data and preview
    m_previewLabel->setText(QString("ID: %1").arg(itemId));
    m_itemInfoLabel->setText(QString("Item ID: %1\nCategory: %2").arg(itemId).arg(m_currentCategory));
}

void ItemPalettePanel::onCategoryChanged() {
    QTreeWidgetItem* current = m_categoryTree->currentItem();
    if (current) {
        QString category = current->data(0, Qt::UserRole).toString();
        populateItems(category);
    }
}

void ItemPalettePanel::onItemSelectionChanged() {
    QListWidgetItem* current = m_itemList->currentItem();
    if (current) {
        quint16 itemId = current->data(Qt::UserRole).toUInt();
        updateItemPreview(itemId);
        emit itemSelected(itemId);
    }
}

void ItemPalettePanel::onItemActivated(QListWidgetItem* item) {
    if (item) {
        quint16 itemId = item->data(Qt::UserRole).toUInt();
        updateItemPreview(itemId);
        createBrushFromSelection();
        emit itemSelected(itemId);
    }
}

void ItemPalettePanel::onBrushSizeChanged(int size) {
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::onBrushShapeChanged() {
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::onAutoAssignChanged(bool enabled) {
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::updateBrushConfiguration() {
    // TODO: Update brush configuration based on current settings
    // This would integrate with BrushIntegrationManager
}

void ItemPalettePanel::createBrushFromSelection() {
    // TODO: Create appropriate brush from selected item/material
    // This would integrate with BrushIntegrationManager
}

void ItemPalettePanel::addCategoryItem(const QString& name, const QString& displayName, const QIcon& icon) {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_categoryTree);
    item->setText(0, displayName);
    item->setData(0, Qt::UserRole, name);
    if (!icon.isNull()) {
        item->setIcon(0, icon);
    }
}

QTreeWidgetItem* ItemPalettePanel::findCategoryItem(const QString& name) {
    for (int i = 0; i < m_categoryTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_categoryTree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == name) {
            return item;
        }
    }
    return nullptr;
}

QListWidgetItem* ItemPalettePanel::createItemListItem(quint16 itemId, const QString& name, const QIcon& icon) {
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, itemId);
    item->setToolTip(QString("%1 (ID: %2)").arg(name).arg(itemId));
    
    if (!icon.isNull()) {
        item->setIcon(icon);
    } else {
        // Create placeholder icon
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::lightGray);
        item->setIcon(QIcon(pixmap));
    }
    
    return item;
}

bool ItemPalettePanel::matchesSearchFilter(const QString& itemName, const QString& filter) const {
    if (filter.isEmpty()) {
        return true;
    }
    
    return itemName.contains(filter, Qt::CaseInsensitive);
}

void ItemPalettePanel::filterItemsByCategory(const QString& category) {
    // TODO: Implement category-based filtering
    Q_UNUSED(category)
}

void ItemPalettePanel::filterItemsBySearch(const QString& searchText) {
    applySearchFilter(searchText);
}

} // namespace palettes
} // namespace ui
} // namespace RME