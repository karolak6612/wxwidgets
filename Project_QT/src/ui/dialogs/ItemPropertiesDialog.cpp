#include "ui/dialogs/ItemPropertiesDialog.h"
#include "ui/dialogs/ItemFinderDialogQt.h"
#include "core/Item.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/assets/ItemData.h"
#include "core/items/ContainerItem.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

namespace RME {
namespace ui {
namespace dialogs {

ItemPropertiesDialog::ItemPropertiesDialog(QWidget* parent,
                                          const RME::core::Map* map,
                                          const RME::core::Tile* tileContext,
                                          RME::core::Item* itemCopy)
    : QDialog(parent)
    , m_itemCopy(itemCopy)
    , m_map(map)
    , m_tileContext(tileContext)
{
    Q_ASSERT(m_itemCopy);
    Q_ASSERT(m_map);
    
    setWindowTitle(tr("Item Properties"));
    setModal(true);
    resize(500, 600);
    
    // Get item data from item's type provider
    m_itemData = nullptr;
    if (m_itemCopy->getTypeProvider()) {
        m_itemData = m_itemCopy->getTypeProvider()->getItemData(m_itemCopy->getID());
    }
    
    createBackup();
    setupUI();
    loadItemData();
    connectSignals();
    
    // Create dynamic controls based on item type
    createTypeSpecificControls();
    updateTabVisibility();
}

void ItemPropertiesDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    mainLayout->addWidget(m_tabWidget);
    
    // Setup tabs
    setupGeneralTab();
    setupContentsTab();
    setupAdvancedTab();
    
    // Setup button box
    setupButtonBox();
    mainLayout->addWidget(m_buttonBox);
}

void ItemPropertiesDialog::setupGeneralTab() {
    m_generalTab = new QWidget();
    m_generalLayout = new QFormLayout(m_generalTab);
    
    // Item ID (read-only)
    m_itemIdEdit = new QLineEdit();
    m_itemIdEdit->setReadOnly(true);
    m_generalLayout->addRow(tr("Item ID:"), m_itemIdEdit);
    
    // Item Name (read-only, from database)
    m_itemNameEdit = new QLineEdit();
    m_itemNameEdit->setReadOnly(true);
    m_generalLayout->addRow(tr("Name:"), m_itemNameEdit);
    
    // Count
    m_countSpinBox = new QSpinBox();
    m_countSpinBox->setRange(1, 100);
    m_countSpinBox->setValue(1);
    m_generalLayout->addRow(tr("Count:"), m_countSpinBox);
    
    // Action ID
    m_actionIdSpinBox = new QSpinBox();
    m_actionIdSpinBox->setRange(0, 65535);
    m_actionIdSpinBox->setSpecialValueText(tr("None"));
    m_generalLayout->addRow(tr("Action ID:"), m_actionIdSpinBox);
    
    // Unique ID
    m_uniqueIdSpinBox = new QSpinBox();
    m_uniqueIdSpinBox->setRange(0, 65535);
    m_uniqueIdSpinBox->setSpecialValueText(tr("None"));
    m_generalLayout->addRow(tr("Unique ID:"), m_uniqueIdSpinBox);
    
    // Text (for readable items, signs, etc.)
    m_textEdit = new QLineEdit();
    m_textEdit->setMaxLength(255);
    m_generalLayout->addRow(tr("Text:"), m_textEdit);
    
    // Description (for writable items)
    m_descriptionEdit = new QLineEdit();
    m_descriptionEdit->setMaxLength(255);
    m_generalLayout->addRow(tr("Description:"), m_descriptionEdit);
    
    // Dynamic type-specific controls area
    m_typeSpecificWidgetArea = new QWidget();
    m_typeSpecificLayout = new QVBoxLayout(m_typeSpecificWidgetArea);
    m_generalLayout->addRow(m_typeSpecificWidgetArea);
    
    m_tabWidget->addTab(m_generalTab, tr("General"));
}

void ItemPropertiesDialog::setupContentsTab() {
    m_contentsTab = new QWidget();
    m_contentsLayout = new QVBoxLayout(m_contentsTab);
    
    // Container info
    m_containerInfoLabel = new QLabel();
    m_containerInfoLabel->setWordWrap(true);
    m_contentsLayout->addWidget(m_containerInfoLabel);
    
    // Contents view (QListView with IconMode as per specification)
    m_contentsView = new QListView();
    m_contentsView->setViewMode(QListView::IconMode);
    m_contentsView->setMovement(QListView::Snap);
    m_contentsView->setFlow(QListView::LeftToRight);
    m_contentsView->setWrapping(true);
    m_contentsView->setResizeMode(QListView::Adjust);
    m_contentsView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_contentsModel = new QStandardItemModel(this);
    m_contentsView->setModel(m_contentsModel);
    m_contentsLayout->addWidget(m_contentsView);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addItemButton = new QPushButton(tr("Add Item"));
    m_editItemButton = new QPushButton(tr("Edit Item"));
    m_removeItemButton = new QPushButton(tr("Remove Item"));
    m_editItemButton->setEnabled(false);
    m_removeItemButton->setEnabled(false);
    
    buttonLayout->addWidget(m_addItemButton);
    buttonLayout->addWidget(m_editItemButton);
    buttonLayout->addWidget(m_removeItemButton);
    buttonLayout->addStretch();
    
    m_contentsLayout->addLayout(buttonLayout);
    
    m_tabWidget->addTab(m_contentsTab, tr("Contents"));
}

void ItemPropertiesDialog::setupAdvancedTab() {
    m_advancedTab = new QWidget();
    m_advancedLayout = new QVBoxLayout(m_advancedTab);
    
    // Info label
    QLabel* infoLabel = new QLabel(tr("Advanced item attributes (key-value pairs):"));
    m_advancedLayout->addWidget(infoLabel);
    
    // Attributes table (3 columns as per specification: Key, Type, Value)
    m_attributesTable = new QTableWidget(0, 3);
    m_attributesTable->setHorizontalHeaderLabels({tr("Key"), tr("Type"), tr("Value")});
    m_attributesTable->horizontalHeader()->setStretchLastSection(true);
    m_attributesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_advancedLayout->addWidget(m_attributesTable);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addAttributeButton = new QPushButton(tr("Add Attribute"));
    m_removeAttributeButton = new QPushButton(tr("Remove Attribute"));
    m_removeAttributeButton->setEnabled(false);
    m_resetButton = new QPushButton(tr("Reset to Defaults"));
    
    buttonLayout->addWidget(m_addAttributeButton);
    buttonLayout->addWidget(m_removeAttributeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    
    m_advancedLayout->addLayout(buttonLayout);
    
    m_tabWidget->addTab(m_advancedTab, tr("Advanced"));
}

void ItemPropertiesDialog::setupButtonBox() {
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ItemPropertiesDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ItemPropertiesDialog::reject);
}

void ItemPropertiesDialog::loadItemData() {
    loadGeneralProperties();
    loadContentsData();
    loadAdvancedAttributes();
}

void ItemPropertiesDialog::loadGeneralProperties() {
    if (!m_itemCopy) {
        return;
    }
    
    // Load basic properties
    m_itemIdEdit->setText(QString::number(m_itemCopy->getID()));
    
    // Load item name from type provider
    QString itemName = m_itemCopy->getName();
    if (itemName.isEmpty()) {
        itemName = tr("Item %1").arg(m_itemCopy->getID());
    }
    m_itemNameEdit->setText(itemName);
    
    // Load item-specific properties
    m_actionIdSpinBox->setValue(m_itemCopy->getActionID());
    m_uniqueIdSpinBox->setValue(m_itemCopy->getUniqueID());
    
    // Load count if stackable
    if (m_itemCopy->isStackable() && m_itemCopy->getSubtype() > 0) {
        m_countSpinBox->setValue(m_itemCopy->getSubtype());
        m_countSpinBox->setEnabled(true);
    } else {
        m_countSpinBox->setValue(1);
        m_countSpinBox->setEnabled(false);
    }
    
    // Load text if readable/writeable
    if (hasText()) {
        m_textEdit->setText(m_itemCopy->getText());
        m_textEdit->setEnabled(true);
    } else {
        m_textEdit->clear();
        m_textEdit->setEnabled(false);
    }
    
    // Load description
    m_descriptionEdit->setText(m_itemCopy->getAttribute("description").toString());
    
    // No need to call loadTypeSpecificProperties() as createTypeSpecificControls() handles it
}

void ItemPropertiesDialog::loadContentsData() {
    if (!m_contentsModel || !isContainer()) {
        return;
    }
    
    m_contentsModel->clear();
    
    // Load container contents if item is a container
    if (auto* containerItem = dynamic_cast<RME::ContainerItem*>(m_itemCopy)) {
        const auto& contents = containerItem->getContents();
        for (const auto& item : contents) {
            if (item) {
                QStandardItem* listItem = new QStandardItem();
                
                // Set item text (name or ID)
                QString itemText = item->getName();
                if (itemText.isEmpty()) {
                    itemText = tr("Item %1").arg(item->getID());
                }
                
                // Add count if > 1
                if (item->getSubtype() > 1) {
                    itemText += tr(" (%1)").arg(item->getSubtype());
                }
                listItem->setText(itemText);
                
                // TODO: Set item icon from sprite manager
                // QIcon itemIcon = getItemIcon(item->getID());
                // listItem->setIcon(itemIcon);
                
                // Store item pointer for later access
                listItem->setData(QVariant::fromValue(item.get()), Qt::UserRole);
                
                m_contentsModel->appendRow(listItem);
            }
        }
    }
    
    updateContainerInfo();
}

void ItemPropertiesDialog::loadAdvancedAttributes() {
    if (!m_attributesTable) {
        return;
    }
    
    m_attributesTable->setRowCount(0);
    
    // Load item attributes from the item
    const auto& attributes = m_itemCopy->getAllAttributes();
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        const QString& key = it.key();
        const QVariant& value = it.value();
        
        int row = m_attributesTable->rowCount();
        m_attributesTable->insertRow(row);
        
        // Key column
        m_attributesTable->setItem(row, 0, createAttributeItem(key));
        
        // Type column (determine from QVariant type)
        QString typeStr;
        switch (value.type()) {
            case QVariant::Int:
                typeStr = "Integer";
                break;
            case QVariant::Double:
                typeStr = "Float";
                break;
            case QVariant::Bool:
                typeStr = "Boolean";
                break;
            default:
                typeStr = "String";
                break;
        }
        
        QComboBox* typeCombo = new QComboBox();
        typeCombo->addItems({"String", "Integer", "Float", "Boolean"});
        typeCombo->setCurrentText(typeStr);
        m_attributesTable->setCellWidget(row, 1, typeCombo);
        
        // Value column
        m_attributesTable->setItem(row, 2, createAttributeItem(value.toString()));
        
        // Connect type change signal
        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this]() { markAsModified(); });
    }
}

void ItemPropertiesDialog::connectSignals() {
    // General tab signals
    connect(m_countSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    connect(m_actionIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    connect(m_uniqueIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    connect(m_textEdit, &QLineEdit::textChanged,
            this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    connect(m_descriptionEdit, &QLineEdit::textChanged,
            this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    
    // Contents tab signals
    connect(m_addItemButton, &QPushButton::clicked,
            this, &ItemPropertiesDialog::onAddContainerItem);
    connect(m_removeItemButton, &QPushButton::clicked,
            this, &ItemPropertiesDialog::onRemoveContainerItem);
    connect(m_contentsView->selectionModel(), &QItemSelectionModel::selectionChanged, [this]() {
        m_removeItemButton->setEnabled(!m_contentsView->selectionModel()->selectedIndexes().isEmpty());
    });
    
    // Advanced tab signals
    connect(m_addAttributeButton, &QPushButton::clicked,
            this, &ItemPropertiesDialog::onAddAttribute);
    connect(m_removeAttributeButton, &QPushButton::clicked,
            this, &ItemPropertiesDialog::onRemoveAttribute);
    connect(m_resetButton, &QPushButton::clicked,
            this, &ItemPropertiesDialog::onResetToDefaults);
    connect(m_attributesTable, &QTableWidget::itemSelectionChanged, [this]() {
        m_removeAttributeButton->setEnabled(m_attributesTable->currentRow() >= 0);
    });
    connect(m_attributesTable, &QTableWidget::itemChanged,
            this, &ItemPropertiesDialog::onAttributeChanged);
}

void ItemPropertiesDialog::updateDynamicControls() {
    // Show/hide controls based on item type
    bool showText = hasText();
    bool showActionId = hasActionId();
    bool showDoorId = isDoor();
    bool showDepotId = isDepot();
    bool showTeleport = isTeleport();
    
    // Update visibility
    m_textEdit->setVisible(showText);
    m_generalLayout->labelForField(m_textEdit)->setVisible(showText);
    
    m_actionIdSpinBox->setVisible(showActionId);
    m_generalLayout->labelForField(m_actionIdSpinBox)->setVisible(showActionId);
    
    m_doorIdCombo->setVisible(showDoorId);
    m_generalLayout->labelForField(m_doorIdCombo)->setVisible(showDoorId);
    
    m_depotIdCombo->setVisible(showDepotId);
    m_generalLayout->labelForField(m_depotIdCombo)->setVisible(showDepotId);
    
    m_teleportDestCombo->setVisible(showTeleport);
    m_generalLayout->labelForField(m_teleportDestCombo)->setVisible(showTeleport);
}

void ItemPropertiesDialog::createTypeSpecificControls() {
    clearTypeSpecificControls();
    
    if (!m_itemCopy) {
        return;
    }
    
    QFormLayout* typeLayout = new QFormLayout();
    
    // Add controls based on item type
    if (isFluidContainer() || isSplash()) {
        m_liquidTypeCombo = new QComboBox();
        m_liquidTypeCombo->addItem(tr("None"), 0);
        m_liquidTypeCombo->addItem(tr("Water"), 1);
        m_liquidTypeCombo->addItem(tr("Blood"), 2);
        m_liquidTypeCombo->addItem(tr("Beer"), 3);
        m_liquidTypeCombo->addItem(tr("Slime"), 4);
        m_liquidTypeCombo->addItem(tr("Lemonade"), 5);
        m_liquidTypeCombo->addItem(tr("Milk"), 6);
        m_liquidTypeCombo->addItem(tr("Mana Fluid"), 7);
        m_liquidTypeCombo->addItem(tr("Life Fluid"), 8);
        m_liquidTypeCombo->addItem(tr("Oil"), 9);
        m_liquidTypeCombo->addItem(tr("Urine"), 10);
        m_liquidTypeCombo->addItem(tr("Coconut Milk"), 11);
        m_liquidTypeCombo->addItem(tr("Wine"), 12);
        m_liquidTypeCombo->addItem(tr("Mud"), 13);
        m_liquidTypeCombo->addItem(tr("Fruit Juice"), 14);
        m_liquidTypeCombo->addItem(tr("Lava"), 15);
        m_liquidTypeCombo->addItem(tr("Rum"), 16);
        m_liquidTypeCombo->addItem(tr("Swamp Gas"), 17);
        m_liquidTypeCombo->addItem(tr("Tea"), 18);
        m_liquidTypeCombo->addItem(tr("Mead"), 19);
        typeLayout->addRow(tr("Liquid Type:"), m_liquidTypeCombo);
        connect(m_liquidTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    }
    
    if (isDoor()) {
        m_doorIdSpin = new QSpinBox();
        m_doorIdSpin->setRange(0, 65535);
        m_doorIdSpin->setSpecialValueText(tr("None"));
        // Enable only if on house tile
        bool isHouseTile = m_tileContext && false; // TODO: m_tileContext->isHouseTile();
        m_doorIdSpin->setEnabled(isHouseTile);
        typeLayout->addRow(tr("Door ID:"), m_doorIdSpin);
        connect(m_doorIdSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    }
    
    if (isDepot()) {
        m_depotTownCombo = new QComboBox();
        m_depotTownCombo->addItem(tr("None"), 0);
        // TODO: Populate from m_map->getTownManager()->getTowns()
        typeLayout->addRow(tr("Depot Town:"), m_depotTownCombo);
        connect(m_depotTownCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    }
    
    if (isTeleport()) {
        m_destXSpin = new QSpinBox();
        m_destXSpin->setRange(0, 65535);
        typeLayout->addRow(tr("Dest X:"), m_destXSpin);
        
        m_destYSpin = new QSpinBox();
        m_destYSpin->setRange(0, 65535);
        typeLayout->addRow(tr("Dest Y:"), m_destYSpin);
        
        m_destZSpin = new QSpinBox();
        m_destZSpin->setRange(0, 15);
        typeLayout->addRow(tr("Dest Z:"), m_destZSpin);
        
        connect(m_destXSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
        connect(m_destYSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
        connect(m_destZSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    }
    
    if (isTiered()) {
        m_tierSpin = new QSpinBox();
        m_tierSpin->setRange(0, 255);
        typeLayout->addRow(tr("Tier:"), m_tierSpin);
        connect(m_tierSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ItemPropertiesDialog::onGeneralPropertyChanged);
    }
    
    m_typeSpecificLayout->addLayout(typeLayout);
}

void ItemPropertiesDialog::clearTypeSpecificControls() {
    // Clear existing layout
    QLayoutItem* item;
    while ((item = m_typeSpecificLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Reset all pointers
    m_liquidTypeCombo = nullptr;
    m_doorIdSpin = nullptr;
    m_depotTownCombo = nullptr;
    m_destXSpin = nullptr;
    m_destYSpin = nullptr;
    m_destZSpin = nullptr;
    m_podiumDirectionCombo = nullptr;
    m_showOutfitCheck = nullptr;
    m_showMountCheck = nullptr;
    m_showPlatformCheck = nullptr;
    m_lookTypeSpin = nullptr;
    m_lookHeadSpin = nullptr;
    m_lookBodySpin = nullptr;
    m_lookLegsSpin = nullptr;
    m_lookFeetSpin = nullptr;
    m_lookAddonSpin = nullptr;
    m_lookMountSpin = nullptr;
    m_lookMountHeadSpin = nullptr;
    m_lookMountBodySpin = nullptr;
    m_lookMountLegsSpin = nullptr;
    m_lookMountFeetSpin = nullptr;
    m_tierSpin = nullptr;
}

void ItemPropertiesDialog::updateTabVisibility() {
    // Show contents tab only for containers
    int contentsIndex = m_tabWidget->indexOf(m_contentsTab);
    if (isContainer()) {
        if (contentsIndex == -1) {
            m_tabWidget->addTab(m_contentsTab, tr("Contents"));
        }
    } else {
        if (contentsIndex != -1) {
            m_tabWidget->removeTab(contentsIndex);
        }
    }
}

bool ItemPropertiesDialog::isContainer() const {
    return m_itemCopy && m_itemCopy->isContainer();
}

bool ItemPropertiesDialog::isDoor() const {
    return m_itemCopy && m_itemCopy->isDoor();
}

bool ItemPropertiesDialog::isDepot() const {
    return m_itemCopy && m_itemCopy->isDepot();
}

bool ItemPropertiesDialog::isTeleport() const {
    return m_itemCopy && m_itemCopy->isTeleport();
}

bool ItemPropertiesDialog::isPodium() const {
    return m_itemCopy && m_itemCopy->isPodium();
}

bool ItemPropertiesDialog::isFluidContainer() const {
    return m_itemCopy && m_itemCopy->isFluidContainer();
}

bool ItemPropertiesDialog::isSplash() const {
    return m_itemCopy && m_itemCopy->isSplash();
}

bool ItemPropertiesDialog::isTiered() const {
    // TODO: Check if item supports tiers (newer client versions)
    return false; // m_itemCopy && m_itemCopy->supportsTiers();
}

bool ItemPropertiesDialog::hasText() const {
    return m_itemCopy && (m_itemCopy->isReadable() || m_itemCopy->isWriteable());
}

bool ItemPropertiesDialog::hasActionId() const {
    // Most items can have action IDs
    return m_itemCopy != nullptr;
}

void ItemPropertiesDialog::accept() {
    if (validateInput()) {
        saveItemData();
        QDialog::accept();
    }
}

void ItemPropertiesDialog::reject() {
    if (m_wasModified) {
        int result = QMessageBox::question(this, tr("Discard Changes"),
            tr("You have unsaved changes. Do you want to discard them?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (result == QMessageBox::No) {
            return;
        }
        
        restoreBackup();
    }
    
    QDialog::reject();
}

void ItemPropertiesDialog::onGeneralPropertyChanged() {
    markAsModified();
}

void ItemPropertiesDialog::onContentsChanged() {
    markAsModified();
    updateContainerInfo();
}

void ItemPropertiesDialog::onAttributeChanged() {
    markAsModified();
}

void ItemPropertiesDialog::onAddAttribute() {
    addAttribute(tr("new_attribute"), tr("value"));
    markAsModified();
}

void ItemPropertiesDialog::onRemoveAttribute() {
    int currentRow = m_attributesTable->currentRow();
    if (currentRow >= 0) {
        removeAttribute(currentRow);
        markAsModified();
    }
}

void ItemPropertiesDialog::onAddContainerItem() {
    // Show ItemFinderDialog to select an item
    RME::ui::dialogs::ItemFinderDialogQt dialog(this, nullptr); // TODO: Pass actual ItemManager
    if (dialog.exec() == QDialog::Accepted) {
        auto* selectedItemType = dialog.getSelectedItemType();
        if (selectedItemType && isContainer()) {
            // Create a new item from the selected type
            auto newItem = RME::Item::create(selectedItemType->getID(), m_itemCopy->getTypeProvider());
            
            // Add to container if it's a container item
            if (auto* containerItem = dynamic_cast<RME::ContainerItem*>(m_itemCopy)) {
                containerItem->addItem(std::move(newItem));
                loadContentsData(); // Refresh the contents view
                markAsModified();
            }
        }
    }
}

void ItemPropertiesDialog::onRemoveContainerItem() {
    QModelIndexList selectedIndexes = m_contentsView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QModelIndex index = selectedIndexes.first();
        if (index.isValid()) {
            m_contentsModel->removeRow(index.row());
            markAsModified();
        }
    }
}

void ItemPropertiesDialog::onResetToDefaults() {
    int result = QMessageBox::question(this, tr("Reset to Defaults"),
        tr("This will reset all attributes to their default values. Continue?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        m_attributesTable->setRowCount(0);
        // TODO: Load default attributes from item data
        markAsModified();
    }
}

void ItemPropertiesDialog::saveTypeSpecificProperties() {
    // Save type-specific properties based on item type
    if (isFluidContainer() || isSplash()) {
        if (m_liquidTypeCombo) {
            int liquidType = m_liquidTypeCombo->currentData().toInt();
            m_itemCopy->setSubtype(liquidType);
        }
    }
    
    if (isDoor()) {
        if (m_doorIdSpin) {
            m_itemCopy->setAttribute("doorid", m_doorIdSpin->value());
        }
    }
    
    if (isDepot()) {
        if (m_depotTownCombo) {
            m_itemCopy->setAttribute("depotid", m_depotTownCombo->currentData().toInt());
        }
    }
    
    if (isTeleport()) {
        if (m_destXSpin && m_destYSpin && m_destZSpin) {
            m_itemCopy->setAttribute("tele_dest_x", m_destXSpin->value());
            m_itemCopy->setAttribute("tele_dest_y", m_destYSpin->value());
            m_itemCopy->setAttribute("tele_dest_z", m_destZSpin->value());
        }
    }
    
    if (isTiered()) {
        if (m_tierSpin) {
            m_itemCopy->setAttribute("tier", m_tierSpin->value());
        }
    }
}

void ItemPropertiesDialog::saveItemData() {
    saveGeneralProperties();
    saveContentsData();
    saveAdvancedAttributes();
    
    emit itemModified(m_itemCopy);
}

void ItemPropertiesDialog::saveGeneralProperties() {
    if (!m_itemCopy) {
        return;
    }
    
    // Save properties to item
    m_itemCopy->setActionID(m_actionIdSpinBox->value());
    m_itemCopy->setUniqueID(m_uniqueIdSpinBox->value());
    
    // Save count if stackable
    if (m_itemCopy->isStackable() && m_countSpinBox->isEnabled()) {
        m_itemCopy->setSubtype(m_countSpinBox->value());
    }
    
    // Save text if readable/writeable
    if (hasText() && m_textEdit->isEnabled()) {
        m_itemCopy->setText(m_textEdit->text());
    }
    
    // Save description
    if (!m_descriptionEdit->text().isEmpty()) {
        m_itemCopy->setAttribute("description", m_descriptionEdit->text());
    } else {
        m_itemCopy->clearAttribute("description");
    }
    
    // Save type-specific properties
    saveTypeSpecificProperties();
}

void ItemPropertiesDialog::saveContentsData() {
    // TODO: Save container contents
}

void ItemPropertiesDialog::saveAdvancedAttributes() {
    if (!m_itemCopy || !m_attributesTable) {
        return;
    }
    
    // Clear existing attributes
    m_itemCopy->clearAllAttributes();
    
    // Save all attributes from the table
    for (int row = 0; row < m_attributesTable->rowCount(); ++row) {
        QTableWidgetItem* keyItem = m_attributesTable->item(row, 0);
        QTableWidgetItem* valueItem = m_attributesTable->item(row, 2);
        QComboBox* typeCombo = qobject_cast<QComboBox*>(m_attributesTable->cellWidget(row, 1));
        
        if (keyItem && valueItem && typeCombo) {
            QString key = keyItem->text();
            QString valueStr = valueItem->text();
            QString type = typeCombo->currentText();
            
            // Convert value based on type
            QVariant value;
            if (type == "Integer") {
                value = valueStr.toInt();
            } else if (type == "Float") {
                value = valueStr.toDouble();
            } else if (type == "Boolean") {
                value = (valueStr.toLower() == "true" || valueStr == "1");
            } else {
                value = valueStr; // String
            }
            
            m_itemCopy->setAttribute(key, value);
        }
    }
}

bool ItemPropertiesDialog::validateInput() {
    // TODO: Implement validation
    return true;
}

void ItemPropertiesDialog::markAsModified() {
    m_wasModified = true;
    setWindowTitle(tr("Item Properties *"));
}

// Removed unused methods

void ItemPropertiesDialog::updateContainerInfo() {
    if (!isContainer()) {
        return;
    }
    
    int itemCount = m_contentsModel->rowCount();
    // Get container capacity from item data if available
    int capacity = 20; // Default capacity
    if (m_itemData) {
        // Check if the item data has a maxItems attribute
        auto it = m_itemData->genericAttributes.find("maxItems");
        if (it != m_itemData->genericAttributes.end()) {
            capacity = it.value().toInt();
        }
    }
    
    m_containerInfoLabel->setText(tr("Container: %1/%2 items").arg(itemCount).arg(capacity));
}

void ItemPropertiesDialog::addAttribute(const QString& key, const QString& value) {
    int row = m_attributesTable->rowCount();
    m_attributesTable->insertRow(row);
    
    m_attributesTable->setItem(row, 0, createAttributeItem(key));
    m_attributesTable->setItem(row, 1, createAttributeItem(value));
}

void ItemPropertiesDialog::removeAttribute(int row) {
    if (row >= 0 && row < m_attributesTable->rowCount()) {
        m_attributesTable->removeRow(row);
    }
}

QTableWidgetItem* ItemPropertiesDialog::createAttributeItem(const QString& text) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    return item;
}

void ItemPropertiesDialog::createBackup() {
    if (m_itemCopy) {
        m_originalItem = m_itemCopy->deepCopy().release();
    }
}

void ItemPropertiesDialog::restoreBackup() {
    if (m_originalItem && m_itemCopy) {
        *m_itemCopy = *m_originalItem;
        loadItemData(); // Refresh UI
    }
}

} // namespace dialogs
} // namespace ui
} // namespace RME