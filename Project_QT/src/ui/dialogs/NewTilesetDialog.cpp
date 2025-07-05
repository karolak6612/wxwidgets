#include "NewTilesetDialog.h"
#include "ItemFinderDialogQt.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace RME {
namespace ui {
namespace dialogs {

NewTilesetDialog::NewTilesetDialog(QWidget* parent, TilesetCategoryType categoryType)
    : QDialog(parent)
    , m_categoryType(categoryType)
{
    setWindowTitle("Create New Tileset");
    setModal(true);
    resize(400, 250);
    
    setupUI();
    connectSignals();
    updateItemPreview();
    validateInput();
}

void NewTilesetDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    setupTilesetInfo();
    setupInitialItem();
    setupButtonBox();
}

void NewTilesetDialog::setupTilesetInfo()
{
    m_tilesetGroup = new QGroupBox("Tileset Information");
    auto* layout = new QFormLayout(m_tilesetGroup);
    
    // Category display
    m_categoryLabel = new QLabel(getCategoryDisplayName());
    m_categoryLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addRow("Category:", m_categoryLabel);
    
    // Tileset name input
    m_tilesetNameEdit = new QLineEdit();
    m_tilesetNameEdit->setPlaceholderText("Enter tileset name...");
    
    // Set up validator for tileset name (alphanumeric, spaces, underscores, hyphens)
    QRegularExpression nameRegex("^[a-zA-Z0-9 _-]+$");
    auto* validator = new QRegularExpressionValidator(nameRegex, this);
    m_tilesetNameEdit->setValidator(validator);
    
    layout->addRow("Tileset Name:", m_tilesetNameEdit);
    
    // Description
    m_descriptionLabel = new QLabel("This will create a new tileset category for organizing " + 
                                   getCategoryDisplayName().toLower() + " items.");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addRow(m_descriptionLabel);
    
    m_mainLayout->addWidget(m_tilesetGroup);
}

void NewTilesetDialog::setupInitialItem()
{
    m_itemGroup = new QGroupBox("Initial Item (Optional)");
    auto* layout = new QFormLayout(m_itemGroup);
    
    // Item ID input
    m_itemIdSpin = new QSpinBox();
    m_itemIdSpin->setRange(100, 65535);
    m_itemIdSpin->setValue(m_currentItemId);
    layout->addRow("Item ID:", m_itemIdSpin);
    
    // Browse button
    m_browseButton = new QPushButton("Browse...");
    layout->addRow("Select Item:", m_browseButton);
    
    // Item name display
    m_itemNameLabel = new QLabel("\"Unknown Item\"");
    m_itemNameLabel->setStyleSheet("QLabel { font-style: italic; }");
    layout->addRow("Name:", m_itemNameLabel);
    
    // Item preview (placeholder)
    m_itemPreviewLabel = new QLabel("Item Preview");
    m_itemPreviewLabel->setMinimumSize(64, 64);
    m_itemPreviewLabel->setStyleSheet("QLabel { border: 1px solid gray; background-color: #f0f0f0; }");
    m_itemPreviewLabel->setAlignment(Qt::AlignCenter);
    layout->addRow("Preview:", m_itemPreviewLabel);
    
    // Info label
    auto* infoLabel = new QLabel("You can add an initial item to the new tileset. "
                                "More items can be added later using the 'Add Item to Tileset' dialog.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: gray; font-style: italic; font-size: 9pt; }");
    layout->addRow(infoLabel);
    
    m_mainLayout->addWidget(m_itemGroup);
}

void NewTilesetDialog::setupButtonBox()
{
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Create Tileset");
    m_mainLayout->addWidget(m_buttonBox);
}

void NewTilesetDialog::connectSignals()
{
    // Dialog buttons
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &NewTilesetDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Tileset name
    connect(m_tilesetNameEdit, &QLineEdit::textChanged, 
            this, &NewTilesetDialog::onTilesetNameChanged);
    
    // Item selection
    connect(m_itemIdSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &NewTilesetDialog::onItemIdChanged);
    connect(m_browseButton, &QPushButton::clicked, 
            this, &NewTilesetDialog::onBrowseItem);
}

void NewTilesetDialog::onTilesetNameChanged()
{
    validateInput();
}

void NewTilesetDialog::onItemIdChanged()
{
    m_currentItemId = m_itemIdSpin->value();
    updateItemPreview();
}

void NewTilesetDialog::onBrowseItem()
{
    // Use actual ClientDataService instead of nullptr
    ItemFinderDialogQt dialog(this, m_clientDataService);
    if (dialog.exec() == QDialog::Accepted) {
        // Get selected item and update
        if (auto selectedItemId = dialog.getSelectedItemId()) {
            m_itemIdSpin->setValue(*selectedItemId);
        }
        // auto* itemType = dialog.getSelectedItemType();
        // if (itemType) {
        //     m_itemIdSpin->setValue(itemType->id);
        // }
        
        QMessageBox::information(this, "Item Finder", 
                                "Item finder integration will be implemented when ItemManager is available.");
    }
}

void NewTilesetDialog::validateInput()
{
    bool valid = isValidTilesetName(m_tilesetNameEdit->text());
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
    
    // Update tileset name field styling
    if (m_tilesetNameEdit->text().isEmpty()) {
        m_tilesetNameEdit->setStyleSheet("");
    } else if (valid) {
        m_tilesetNameEdit->setStyleSheet("QLineEdit { border: 2px solid green; }");
    } else {
        m_tilesetNameEdit->setStyleSheet("QLineEdit { border: 2px solid red; }");
    }
}

void NewTilesetDialog::updateItemPreview()
{
    QString itemName = getItemName(m_currentItemId);
    m_itemNameLabel->setText(QString("\"%1\"").arg(itemName));
    
    // Update preview with actual item sprite
    if (m_clientDataService && m_clientDataService->getSpriteManager()) {
        auto spriteManager = m_clientDataService->getSpriteManager();
        // TODO: Get sprite for item and update preview widget
    }
    m_itemPreviewLabel->setText(QString("ID: %1").arg(m_currentItemId));
}

QString NewTilesetDialog::getItemName(uint16_t itemId) const
{
    // TODO: Get actual item name from ItemManager
    return QString("Item %1").arg(itemId);
}

bool NewTilesetDialog::isValidItemId(uint16_t itemId) const
{
    // TODO: Validate against actual ItemManager
    return itemId >= 100 && itemId <= 65535;
}

bool NewTilesetDialog::isValidTilesetName(const QString& name) const
{
    QString trimmed = name.trimmed();
    
    // Must not be empty
    if (trimmed.isEmpty()) {
        return false;
    }
    
    // Must be at least 2 characters
    if (trimmed.length() < 2) {
        return false;
    }
    
    // Must not exceed reasonable length
    if (trimmed.length() > 50) {
        return false;
    }
    
    // Must match the validator pattern (alphanumeric, spaces, underscores, hyphens)
    QRegularExpression nameRegex("^[a-zA-Z0-9 _-]+$");
    if (!nameRegex.match(trimmed).hasMatch()) {
        return false;
    }
    
    // TODO: Check against existing tileset names in MaterialManager
    
    return true;
}

QString NewTilesetDialog::getCategoryDisplayName() const
{
    switch (m_categoryType) {
        case TilesetCategoryType::Terrain: return "Terrain";
        case TilesetCategoryType::Doodad: return "Doodad";
        case TilesetCategoryType::Item: return "Item";
        case TilesetCategoryType::Wall: return "Wall";
        case TilesetCategoryType::Carpet: return "Carpet";
        case TilesetCategoryType::Table: return "Table";
        case TilesetCategoryType::Raw: return "Raw";
        case TilesetCategoryType::Collection: return "Collection";
        default: return "Unknown";
    }
}

QString NewTilesetDialog::getTilesetName() const
{
    return m_tilesetNameEdit->text().trimmed();
}

uint16_t NewTilesetDialog::getInitialItemId() const
{
    return m_currentItemId;
}

void NewTilesetDialog::accept()
{
    // Final validation
    if (!isValidTilesetName(getTilesetName())) {
        QMessageBox::warning(this, "Invalid Name", 
                           "Please enter a valid tileset name (2-50 characters, alphanumeric, spaces, underscores, hyphens only).");
        m_tilesetNameEdit->setFocus();
        return;
    }
    
    if (!isValidItemId(getInitialItemId())) {
        QMessageBox::warning(this, "Invalid Item", 
                           "The selected item ID is not valid.");
        m_itemIdSpin->setFocus();
        return;
    }
    
    // TODO: Actually create the tileset via MaterialManager
    QString message = QString("Would create new %1 tileset:\n\n")
                     .arg(getCategoryDisplayName().toLower());
    message += QString("Name: %1\n").arg(getTilesetName());
    message += QString("Initial Item: %1 (%2)\n").arg(getInitialItemId()).arg(getItemName(getInitialItemId()));
    message += QString("Category: %1").arg(getCategoryDisplayName());
    
    QMessageBox::information(this, "Create Tileset", message);
    
    QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace RME