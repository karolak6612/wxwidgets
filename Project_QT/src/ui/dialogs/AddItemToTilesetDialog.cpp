#include "AddItemToTilesetDialog.h"
#include "ItemFinderDialogQt.h"
#include <QMessageBox>
#include <QApplication>

namespace RME {
namespace ui {
namespace dialogs {

AddItemToTilesetDialog::AddItemToTilesetDialog(QWidget* parent, TilesetCategoryType categoryType)
    : QDialog(parent)
    , m_categoryType(categoryType)
{
    setWindowTitle("Add Item to Tileset");
    setModal(true);
    resize(400, 300);
    
    setupUI();
    loadTilesets();
    connectSignals();
    updateItemPreview();
}

void AddItemToTilesetDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    setupItemSelection();
    setupRangeControls();
    setupTilesetSelection();
    setupButtonBox();
}

void AddItemToTilesetDialog::setupItemSelection()
{
    m_itemGroup = new QGroupBox("Item Selection");
    auto* layout = new QFormLayout(m_itemGroup);
    
    // Item ID input
    m_itemIdSpin = new QSpinBox();
    m_itemIdSpin->setRange(100, 65535);
    m_itemIdSpin->setValue(m_currentItemId);
    layout->addRow("Item ID:", m_itemIdSpin);
    
    // Item name display
    m_itemNameLabel = new QLabel("\"Unknown Item\"");
    m_itemNameLabel->setStyleSheet("QLabel { font-style: italic; }");
    layout->addRow("Name:", m_itemNameLabel);
    
    // Browse and current item buttons
    auto* buttonLayout = new QHBoxLayout();
    m_browseButton = new QPushButton("Browse...");
    m_useCurrentButton = new QPushButton("Use Current Item");
    buttonLayout->addWidget(m_browseButton);
    buttonLayout->addWidget(m_useCurrentButton);
    buttonLayout->addStretch();
    layout->addRow(buttonLayout);
    
    // Item preview (placeholder)
    m_itemPreviewLabel = new QLabel("Item Preview");
    m_itemPreviewLabel->setMinimumSize(64, 64);
    m_itemPreviewLabel->setStyleSheet("QLabel { border: 1px solid gray; background-color: #f0f0f0; }");
    m_itemPreviewLabel->setAlignment(Qt::AlignCenter);
    layout->addRow("Preview:", m_itemPreviewLabel);
    
    m_mainLayout->addWidget(m_itemGroup);
}

void AddItemToTilesetDialog::setupRangeControls()
{
    m_rangeGroup = new QGroupBox("Range Selection");
    auto* layout = new QFormLayout(m_rangeGroup);
    
    // Range checkbox
    m_rangeCheckBox = new QCheckBox("Add range of items");
    layout->addRow(m_rangeCheckBox);
    
    // Range start
    m_rangeStartSpin = new QSpinBox();
    m_rangeStartSpin->setRange(100, 65535);
    m_rangeStartSpin->setValue(m_currentItemId);
    m_rangeStartSpin->setEnabled(false);
    layout->addRow("Range Start:", m_rangeStartSpin);
    
    // Range end
    m_rangeEndSpin = new QSpinBox();
    m_rangeEndSpin->setRange(100, 65535);
    m_rangeEndSpin->setValue(m_currentItemId + 10);
    m_rangeEndSpin->setEnabled(false);
    layout->addRow("Range End:", m_rangeEndSpin);
    
    // Quick range button
    m_quickRangeButton = new QPushButton("Quick +10 Range");
    m_quickRangeButton->setEnabled(false);
    layout->addRow(m_quickRangeButton);
    
    // Range info
    m_rangeInfoLabel = new QLabel("Single item mode");
    m_rangeInfoLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addRow("Info:", m_rangeInfoLabel);
    
    m_mainLayout->addWidget(m_rangeGroup);
}

void AddItemToTilesetDialog::setupTilesetSelection()
{
    m_tilesetGroup = new QGroupBox("Target Tileset");
    auto* layout = new QFormLayout(m_tilesetGroup);
    
    // Category display
    QString categoryName;
    switch (m_categoryType) {
        case TilesetCategoryType::Terrain: categoryName = "Terrain"; break;
        case TilesetCategoryType::Doodad: categoryName = "Doodad"; break;
        case TilesetCategoryType::Item: categoryName = "Item"; break;
        case TilesetCategoryType::Wall: categoryName = "Wall"; break;
        case TilesetCategoryType::Carpet: categoryName = "Carpet"; break;
        case TilesetCategoryType::Table: categoryName = "Table"; break;
        case TilesetCategoryType::Raw: categoryName = "Raw"; break;
        case TilesetCategoryType::Collection: categoryName = "Collection"; break;
    }
    
    m_categoryLabel = new QLabel(categoryName);
    layout->addRow("Category:", m_categoryLabel);
    
    // Tileset combo
    m_tilesetCombo = new QComboBox();
    layout->addRow("Tileset:", m_tilesetCombo);
    
    m_mainLayout->addWidget(m_tilesetGroup);
}

void AddItemToTilesetDialog::setupButtonBox()
{
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_mainLayout->addWidget(m_buttonBox);
}

void AddItemToTilesetDialog::connectSignals()
{
    // Dialog buttons
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &AddItemToTilesetDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Item selection
    connect(m_itemIdSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AddItemToTilesetDialog::onItemIdChanged);
    connect(m_browseButton, &QPushButton::clicked, 
            this, &AddItemToTilesetDialog::onBrowseItem);
    connect(m_useCurrentButton, &QPushButton::clicked, 
            this, &AddItemToTilesetDialog::onUseCurrentItem);
    
    // Range controls
    connect(m_rangeCheckBox, &QCheckBox::toggled, 
            this, &AddItemToTilesetDialog::onRangeToggled);
    connect(m_rangeStartSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AddItemToTilesetDialog::onRangeFieldChanged);
    connect(m_rangeEndSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AddItemToTilesetDialog::onRangeFieldChanged);
    connect(m_quickRangeButton, &QPushButton::clicked, 
            this, &AddItemToTilesetDialog::onQuickRange);
    
    // Tileset selection
    connect(m_tilesetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AddItemToTilesetDialog::onTilesetChanged);
}

void AddItemToTilesetDialog::loadTilesets()
{
    // TODO: Load actual tilesets from MaterialManager
    // For now, add example tilesets based on category
    m_tilesetCombo->clear();
    
    switch (m_categoryType) {
        case TilesetCategoryType::Terrain:
            m_tilesetCombo->addItems({"Grass Terrain", "Stone Terrain", "Sand Terrain", "Snow Terrain"});
            break;
        case TilesetCategoryType::Doodad:
            m_tilesetCombo->addItems({"Trees", "Rocks", "Furniture", "Decorations"});
            break;
        case TilesetCategoryType::Item:
            m_tilesetCombo->addItems({"Weapons", "Armor", "Tools", "Consumables"});
            break;
        case TilesetCategoryType::Wall:
            m_tilesetCombo->addItems({"Stone Walls", "Wood Walls", "Metal Walls"});
            break;
        default:
            m_tilesetCombo->addItems({"Default Tileset", "Custom Tileset"});
            break;
    }
}

void AddItemToTilesetDialog::onItemIdChanged()
{
    m_currentItemId = m_itemIdSpin->value();
    updateItemPreview();
    
    if (m_rangeMode) {
        m_rangeStartSpin->setValue(m_currentItemId);
        updateRangeInfo();
    }
}

void AddItemToTilesetDialog::onBrowseItem()
{
    // TODO: Replace nullptr with actual ItemManager
    ItemFinderDialogQt dialog(this, nullptr);
    if (dialog.exec() == QDialog::Accepted) {
        // TODO: Get selected item and update
        // auto* itemType = dialog.getSelectedItemType();
        // if (itemType) {
        //     m_itemIdSpin->setValue(itemType->id);
        // }
        
        QMessageBox::information(this, "Item Finder", 
                                "Item finder integration will be implemented when ItemManager is available.");
    }
}

void AddItemToTilesetDialog::onRangeToggled(bool enabled)
{
    m_rangeMode = enabled;
    
    // Enable/disable range controls
    m_rangeStartSpin->setEnabled(enabled);
    m_rangeEndSpin->setEnabled(enabled);
    m_quickRangeButton->setEnabled(enabled);
    
    if (enabled) {
        m_rangeStartSpin->setValue(m_currentItemId);
        m_rangeEndSpin->setValue(m_currentItemId + 10);
    }
    
    updateRangeInfo();
}

void AddItemToTilesetDialog::onRangeFieldChanged()
{
    updateRangeInfo();
}

void AddItemToTilesetDialog::onUseCurrentItem()
{
    // TODO: Get current item from editor selection
    QMessageBox::information(this, "Use Current Item", 
                            "This will use the currently selected item in the editor when implemented.");
}

void AddItemToTilesetDialog::onQuickRange()
{
    m_rangeEndSpin->setValue(m_rangeStartSpin->value() + 10);
    updateRangeInfo();
}

void AddItemToTilesetDialog::onTilesetChanged()
{
    // Update UI based on selected tileset if needed
}

void AddItemToTilesetDialog::updateItemPreview()
{
    QString itemName = getItemName(m_currentItemId);
    m_itemNameLabel->setText(QString("\"%1\"").arg(itemName));
    
    // TODO: Update preview with actual item sprite
    m_itemPreviewLabel->setText(QString("ID: %1").arg(m_currentItemId));
}

void AddItemToTilesetDialog::updateRangeInfo()
{
    if (!m_rangeMode) {
        m_rangeInfoLabel->setText("Single item mode");
        return;
    }
    
    int start = m_rangeStartSpin->value();
    int end = m_rangeEndSpin->value();
    int count = qMax(0, end - start + 1);
    
    m_rangeInfoLabel->setText(QString("Range: %1 items (%2 to %3)")
                             .arg(count).arg(start).arg(end));
}

QString AddItemToTilesetDialog::getItemName(uint16_t itemId) const
{
    // TODO: Get actual item name from ItemManager
    return QString("Item %1").arg(itemId);
}

bool AddItemToTilesetDialog::isValidItemId(uint16_t itemId) const
{
    // TODO: Validate against actual ItemManager
    return itemId >= 100 && itemId <= 65535;
}

QList<uint16_t> AddItemToTilesetDialog::generateItemRange() const
{
    QList<uint16_t> items;
    
    if (m_rangeMode) {
        int start = m_rangeStartSpin->value();
        int end = m_rangeEndSpin->value();
        
        for (int id = start; id <= end; ++id) {
            if (isValidItemId(id)) {
                items.append(id);
            }
        }
    } else {
        if (isValidItemId(m_currentItemId)) {
            items.append(m_currentItemId);
        }
    }
    
    return items;
}

QString AddItemToTilesetDialog::getSelectedTileset() const
{
    return m_tilesetCombo->currentText();
}

QList<uint16_t> AddItemToTilesetDialog::getSelectedItemIds() const
{
    return generateItemRange();
}

void AddItemToTilesetDialog::accept()
{
    // Validate input
    if (m_tilesetCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please select a tileset.");
        return;
    }
    
    QList<uint16_t> items = generateItemRange();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "No valid items to add.");
        return;
    }
    
    // TODO: Actually add items to tileset via MaterialManager
    QString tileset = getSelectedTileset();
    QString message = QString("Would add %1 item(s) to tileset '%2':\n").arg(items.size()).arg(tileset);
    
    for (int i = 0; i < qMin(5, items.size()); ++i) {
        message += QString("- Item %1\n").arg(items[i]);
    }
    if (items.size() > 5) {
        message += QString("... and %1 more items").arg(items.size() - 5);
    }
    
    QMessageBox::information(this, "Add Items", message);
    
    QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace RME