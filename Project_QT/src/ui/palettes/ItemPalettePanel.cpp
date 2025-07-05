#include "ui/palettes/ItemPalettePanel.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/MaterialManager.h"
#include "core/services/IBrushStateService.h"

#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QFont>
#include "core/sprites/SpriteManager.h"

namespace RME {
namespace ui {
namespace palettes {

ItemPalettePanel::ItemPalettePanel(
    RME::core::IBrushStateService* brushStateService,
    RME::core::IClientDataService* clientDataService,
    QWidget* parent
) : BasePalettePanel(tr("Items"), parent)
    , m_brushStateService(brushStateService)
    , m_clientDataService(clientDataService)
{
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_clientDataService);
    
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
    // Refresh categories and items based on current service state
    populateCategories();
    
    // Refresh current category if one is selected
    if (m_categoryTree && m_categoryTree->currentItem()) {
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
    if (!m_categoryTree) {
        return;
    }
    
    m_categoryTree->clear();
    
    // Add standard categories
    addCategoryItem("all", tr("All Items"));
    addCategoryItem("grounds", tr("Grounds"));
    addCategoryItem("walls", tr("Walls"));
    addCategoryItem("doodads", tr("Doodads"));
    addCategoryItem("items", tr("Items"));
    addCategoryItem("containers", tr("Containers"));
    addCategoryItem("doors", tr("Doors"));
    addCategoryItem("creatures", tr("Creatures"));
    addCategoryItem("spawns", tr("Spawns"));
    addCategoryItem("raw", tr("Raw Items"));
    
    // Select first category by default
    if (m_categoryTree->topLevelItemCount() > 0) {
        m_categoryTree->setCurrentItem(m_categoryTree->topLevelItem(0));
    }
}

void ItemPalettePanel::populateItems(const QString& category) {
    if (!m_itemList || !m_clientDataService) {
        return;
    }
    
    m_itemList->clear();
    m_currentItems.clear();
    m_currentMaterials.clear();
    m_currentCategory = category;
    
    // Get data from services
    auto* itemDatabase = m_clientDataService->getItemDatabase();
    auto* materialManager = m_clientDataService->getMaterialManager();
    
    if (!itemDatabase || !materialManager) {
        qWarning() << "ItemPalettePanel: ItemDatabase or MaterialManager not available";
        return;
    }
    
    // Populate items based on category using services
    if (category == "grounds" || category == "walls" || category == "doodads") {
        // Get materials for terrain categories
        const auto& allMaterials = materialManager->getAllMaterials();
        for (auto it = allMaterials.constBegin(); it != allMaterials.constEnd(); ++it) {
            const MaterialData& material = it.value();
            if (materialMatchesCategory(&material, category)) {
                m_currentMaterials.append(&material);
                addMaterialToList(&material);
            }
        }
    } else if (category == "all") {
        // Show all items
        const auto& allItems = itemDatabase->getAllItems();
        for (auto it = allItems.constBegin(); it != allItems.constEnd(); ++it) {
            const ItemData& item = it.value();
            m_currentItems.append(&item);
            addItemToList(&item);
        }
    } else {
        // Get items for specific categories
        const auto& allItems = itemDatabase->getAllItems();
        for (auto it = allItems.constBegin(); it != allItems.constEnd(); ++it) {
            const ItemData& item = it.value();
            if (itemMatchesCategory(&item, category)) {
                m_currentItems.append(&item);
                addItemToList(&item);
            }
        }
    }
    
    qDebug() << "ItemPalettePanel: Populated" << m_itemList->count() << "items for category" << category;
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
    
    // Get actual item data and preview
    QString itemName = QString("Item %1").arg(itemId);
    QString itemInfo = QString("Item ID: %1\nCategory: %2").arg(itemId).arg(m_currentCategory);
    
    // Get item data from database
    if (m_clientDataService) {
        auto* itemDatabase = m_clientDataService->getItemDatabase();
        if (itemDatabase) {
            const auto& allItems = itemDatabase->getAllItems();
            auto it = allItems.find(itemId);
            if (it != allItems.end()) {
                const ItemData& item = it.value();
                itemName = item.name;
                
                // Build detailed item info
                QStringList infoLines;
                infoLines << QString("Name: %1").arg(item.name);
                infoLines << QString("ID: %1").arg(item.serverID);
                infoLines << QString("Category: %1").arg(m_currentCategory);
                
                if (item.weight > 0) {
                    infoLines << QString("Weight: %1 oz").arg(item.weight);
                }
                
                if (!item.description.isEmpty()) {
                    infoLines << QString("Description: %1").arg(item.description);
                }
                
                // Add item properties
                QStringList properties;
                if (item.isStackable) properties << "Stackable";
                if (item.isMoveable) properties << "Moveable";
                if (item.isPickupable) properties << "Pickupable";
                if (item.isContainer) properties << "Container";
                if (item.isBlocking) properties << "Blocking";
                
                if (!properties.isEmpty()) {
                    infoLines << QString("Properties: %1").arg(properties.join(", "));
                }
                
                itemInfo = infoLines.join("\n");
            }
        }
        
        // Get sprite preview
        auto* spriteManager = m_clientDataService->getSpriteManager();
        if (spriteManager) {
            const SpriteData* spriteData = spriteManager->getSpriteData(itemId);
            if (spriteData && !spriteData->frames.isEmpty()) {
                const QImage& spriteImage = spriteData->frames.first().image;
                if (!spriteImage.isNull()) {
                    // Scale sprite for preview (larger than icon)
                    QPixmap sprite = QPixmap::fromImage(spriteImage);
                    QPixmap scaledSprite = sprite.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    
                    // Create a 64x64 canvas and center the sprite
                    QPixmap previewPixmap(64, 64);
                    previewPixmap.fill(Qt::transparent);
                    
                    QPainter painter(&previewPixmap);
                    int x = (64 - scaledSprite.width()) / 2;
                    int y = (64 - scaledSprite.height()) / 2;
                    painter.drawPixmap(x, y, scaledSprite);
                    
                    m_previewLabel->setPixmap(previewPixmap);
                    m_itemInfoLabel->setText(itemInfo);
                    return;
                }
            }
        }
    }
    
    // Fallback to text preview if sprite not available
    m_previewLabel->setText(QString("ID: %1").arg(itemId));
    m_itemInfoLabel->setText(itemInfo);
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
        
        // Update brush state service with selected item
        if (m_brushStateService) {
            m_brushStateService->setCurrentRawItemId(static_cast<uint32_t>(itemId));
        }
        
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
    // Update brush state service
    if (m_brushStateService) {
        m_brushStateService->setBrushSize(size);
    }
    
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::onBrushShapeChanged() {
    if (m_brushStateService && m_brushShapeCombo) {
        int shapeIndex = m_brushShapeCombo->currentIndex();
        RME::core::BrushShape shape = static_cast<RME::core::BrushShape>(shapeIndex);
        m_brushStateService->setBrushShape(shape);
    }
    
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::onAutoAssignChanged(bool enabled) {
    updateBrushConfiguration();
    emit brushConfigurationChanged();
}

void ItemPalettePanel::updateBrushConfiguration() {
    if (!m_brushStateService) {
        return;
    }
    
    // Update brush settings from UI
    if (m_brushSizeSpinBox) {
        m_brushStateService->setBrushSize(m_brushSizeSpinBox->value());
    }
    
    if (m_brushShapeCombo) {
        int shapeIndex = m_brushShapeCombo->currentIndex();
        RME::core::BrushShape shape = static_cast<RME::core::BrushShape>(shapeIndex);
        m_brushStateService->setBrushShape(shape);
    }
    
    // TODO: Handle auto-assign checkbox and other settings
}

void ItemPalettePanel::createBrushFromSelection() {
    if (!m_itemList || !m_brushStateService) {
        return;
    }
    
    QListWidgetItem* current = m_itemList->currentItem();
    if (!current) {
        return;
    }
    
    quint16 itemId = current->data(Qt::UserRole).toUInt();
    
    // Set the raw item ID for the raw brush
    m_brushStateService->setCurrentRawItemId(static_cast<uint32_t>(itemId));
    
    // Activate the raw brush through BrushStateService
    if (m_brushStateService) {
        m_brushStateService->setCurrentRawItemId(static_cast<uint32_t>(itemId));
    }
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

void ItemPalettePanel::addItemToList(const RME::core::assets::ItemData* itemData) {
    if (!itemData || !m_itemList) {
        return;
    }
    
    // Create list item with item data
    QListWidgetItem* listItem = createItemListItem(
        itemData->serverID, 
        itemData->name, 
        getItemIcon(itemData->serverID)
    );
    
    m_itemList->addItem(listItem);
}

void ItemPalettePanel::addMaterialToList(const RME::core::assets::MaterialData* materialData) {
    if (!materialData || !m_itemList) {
        return;
    }
    
    // Create list item with material data
    QListWidgetItem* listItem = createItemListItem(
        materialData->serverLookId, 
        materialData->id, 
        getMaterialIcon(materialData->serverLookId)
    );
    
    m_itemList->addItem(listItem);
}

QIcon ItemPalettePanel::getItemIcon(quint16 itemId) const {
    // Get actual item icon from SpriteManager
    if (m_clientDataService) {
        auto* spriteManager = m_clientDataService->getSpriteManager();
        if (spriteManager) {
            const SpriteData* spriteData = spriteManager->getSpriteData(itemId);
            if (spriteData && !spriteData->frames.isEmpty()) {
                // Get the first frame of the sprite
                const QImage& spriteImage = spriteData->frames.first().image;
                if (!spriteImage.isNull()) {
                    // Scale to icon size while maintaining aspect ratio
                    QPixmap sprite = QPixmap::fromImage(spriteImage);
                    QPixmap scaledSprite = sprite.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    
                    // Create a 32x32 canvas and center the sprite
                    QPixmap iconPixmap(32, 32);
                    iconPixmap.fill(Qt::transparent);
                    
                    QPainter painter(&iconPixmap);
                    int x = (32 - scaledSprite.width()) / 2;
                    int y = (32 - scaledSprite.height()) / 2;
                    painter.drawPixmap(x, y, scaledSprite);
                    
                    return QIcon(iconPixmap);
                }
            }
        }
    }
    
    // Fallback to placeholder only if sprite loading fails
    QPixmap pixmap(32, 32);
    pixmap.fill(QColor(100, 150, 200)); // Blue placeholder for items
    
    // Add item ID text to placeholder
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(itemId));
    
    return QIcon(pixmap);
}

QIcon ItemPalettePanel::getMaterialIcon(quint16 materialId) const {
    // Get actual material icon from SpriteManager
    if (m_clientDataService) {
        auto* materialManager = m_clientDataService->getMaterialManager();
        auto* spriteManager = m_clientDataService->getSpriteManager();
        
        if (materialManager && spriteManager) {
            // Get material data to find the representative item ID
            const auto& allMaterials = materialManager->getAllMaterials();
            for (auto it = allMaterials.constBegin(); it != allMaterials.constEnd(); ++it) {
                const MaterialData& material = it.value();
                if (material.serverLookId == materialId) {
                    // Get the first ground item from the material as representative
                    if (!material.groundItems.isEmpty()) {
                        quint16 itemId = material.groundItems.first().itemId;
                        return getItemIcon(itemId); // Reuse item icon logic
                    }
                    break;
                }
            }
        }
    }
    
    // Fallback to placeholder only if material/sprite loading fails
    QPixmap pixmap(32, 32);
    pixmap.fill(QColor(150, 100, 50)); // Brown placeholder for materials
    
    // Add material ID text to placeholder
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString("M%1").arg(materialId));
    
    return QIcon(pixmap);
}

bool ItemPalettePanel::itemMatchesCategory(const RME::core::assets::ItemData* item, const QString& category) const {
    if (!item) return false;
    
    // TODO: Implement proper item categorization based on item properties
    // For now, use simple heuristics
    
    if (category == "items") {
        return true; // All items match "items" category
    } else if (category == "containers") {
        return item->isContainer;
    } else if (category == "doors") {
        return item->isBlocking; // Using isBlocking as approximation for doors
    } else if (category == "raw") {
        return true; // All items can be raw items
    }
    
    return false;
}

bool ItemPalettePanel::materialMatchesCategory(const RME::core::assets::MaterialData* material, const QString& category) const {
    if (!material) return false;
    
    // TODO: Implement proper material categorization
    // For now, use material name/type to determine category
    QString materialName = material->id.toLower();
    
    if (category == "grounds") {
        return materialName.contains("ground") || materialName.contains("grass") || 
               materialName.contains("dirt") || materialName.contains("stone");
    } else if (category == "walls") {
        return materialName.contains("wall") || materialName.contains("brick");
    } else if (category == "doodads") {
        return materialName.contains("doodad") || materialName.contains("decoration");
    }
    
    return false;
}

} // namespace palettes
} // namespace ui
} // namespace RME