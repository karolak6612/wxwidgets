#include "RawItemsPaletteTab.h"
#include "core/assets/ItemDatabase.h"
#include "core/brush/BrushStateManager.h"
#include "core/brush/RawBrush.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>

namespace RME {
namespace ui {
namespace palettes {

const QString RawItemsPaletteTab::ALL_TILESETS_TEXT = "(All Tilesets)";
const QString RawItemsPaletteTab::XML_FILE_PATH = "XML/760/raw_palette.xml";

RawItemsPaletteTab::RawItemsPaletteTab(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadRawItemsFromXml(XML_FILE_PATH);
}

void RawItemsPaletteTab::setItemDatabase(RME::core::assets::ItemDatabase* itemDatabase)
{
    m_itemDatabase = itemDatabase;
    refreshContent();
}

void RawItemsPaletteTab::setBrushStateManager(RME::core::brush::BrushStateManager* brushManager)
{
    m_brushStateManager = brushManager;
}

void RawItemsPaletteTab::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
}

void RawItemsPaletteTab::refreshContent()
{
    updateItemList();
}

void RawItemsPaletteTab::loadRawItemsFromXml(const QString& xmlFilePath)
{
    parseRawPaletteXml(xmlFilePath);
    populateTilesetCombo();
    updateItemList();
}

quint16 RawItemsPaletteTab::getSelectedItemId() const
{
    QListWidgetItem* currentItem = m_itemList->currentItem();
    if (!currentItem) {
        return 0;
    }
    
    return currentItem->data(Qt::UserRole).toUInt();
}

QString RawItemsPaletteTab::getSelectedTileset() const
{
    return m_currentTileset;
}

void RawItemsPaletteTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Filter group
    m_filterGroup = new QGroupBox("Filters", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);
    
    // Tileset filter
    m_filterLayout->addWidget(new QLabel("Tileset:", this));
    m_tilesetCombo = new QComboBox(this);
    m_tilesetCombo->setObjectName("tilesetCombo");
    m_tilesetCombo->setToolTip("Filter items by tileset");
    m_filterLayout->addWidget(m_tilesetCombo, 1);
    
    // Search filter
    m_filterLayout->addWidget(new QLabel("Search:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("Search items...");
    m_searchEdit->setToolTip("Search for items by ID or name");
    m_filterLayout->addWidget(m_searchEdit, 1);
    
    m_clearSearchButton = new QPushButton("Clear", this);
    m_clearSearchButton->setObjectName("clearSearchButton");
    m_clearSearchButton->setToolTip("Clear search filter");
    m_filterLayout->addWidget(m_clearSearchButton);
    
    m_mainLayout->addWidget(m_filterGroup);
    
    // Items group
    m_itemsGroup = new QGroupBox("RAW Items", this);
    m_itemsLayout = new QVBoxLayout(m_itemsGroup);
    
    // Item count label
    m_itemCountLabel = new QLabel("Items: 0", this);
    m_itemCountLabel->setObjectName("itemCountLabel");
    m_itemsLayout->addWidget(m_itemCountLabel);
    
    // Item list
    m_itemList = new QListWidget(this);
    m_itemList->setObjectName("itemList");
    m_itemList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_itemList->setToolTip("Double-click to activate RAW brush with selected item");
    m_itemsLayout->addWidget(m_itemList, 1);
    
    m_mainLayout->addWidget(m_itemsGroup, 1);
    
    // Info group
    m_infoGroup = new QGroupBox("Item Information", this);
    m_infoLayout = new QVBoxLayout(m_infoGroup);
    
    m_selectedItemLabel = new QLabel("No item selected", this);
    m_selectedItemLabel->setObjectName("selectedItemLabel");
    m_selectedItemLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_infoLayout->addWidget(m_selectedItemLabel);
    
    m_itemDetailsLabel = new QLabel("Select an item to view details", this);
    m_itemDetailsLabel->setObjectName("itemDetailsLabel");
    m_itemDetailsLabel->setWordWrap(true);
    m_infoLayout->addWidget(m_itemDetailsLabel);
    
    m_mainLayout->addWidget(m_infoGroup);
    
    // Set stretch factors
    m_mainLayout->setStretchFactor(m_itemsGroup, 1);
}

void RawItemsPaletteTab::connectSignals()
{
    connect(m_tilesetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RawItemsPaletteTab::onTilesetSelectionChanged);
    
    connect(m_itemList, &QListWidget::itemSelectionChanged,
            this, &RawItemsPaletteTab::onItemSelectionChanged);
    
    connect(m_itemList, &QListWidget::itemDoubleClicked,
            this, &RawItemsPaletteTab::onItemDoubleClicked);
    
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &RawItemsPaletteTab::onSearchTextChanged);
    
    connect(m_clearSearchButton, &QPushButton::clicked,
            this, &RawItemsPaletteTab::onClearSearch);
}

void RawItemsPaletteTab::parseRawPaletteXml(const QString& xmlFilePath)
{
    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", 
                             QString("Could not open RAW palette XML file: %1").arg(xmlFilePath));
        return;
    }
    
    m_rawItems.clear();
    m_tilesets.clear();
    m_itemsByTileset.clear();
    
    QXmlStreamReader xml(&file);
    QString currentTileset;
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "tileset") {
                currentTileset = xml.attributes().value("name").toString();
                if (!currentTileset.isEmpty() && !m_tilesets.contains(currentTileset)) {
                    m_tilesets.append(currentTileset);
                }
            }
            else if (xml.name() == "item" && !currentTileset.isEmpty()) {
                QXmlStreamAttributes attrs = xml.attributes();
                
                if (attrs.hasAttribute("id")) {
                    // Single item
                    quint16 itemId = attrs.value("id").toUShort();
                    addSingleItem(itemId, currentTileset);
                }
                else if (attrs.hasAttribute("fromid") && attrs.hasAttribute("toid")) {
                    // Item range
                    quint16 fromId = attrs.value("fromid").toUShort();
                    quint16 toId = attrs.value("toid").toUShort();
                    addItemsFromRange(fromId, toId, currentTileset);
                }
            }
        }
    }
    
    if (xml.hasError()) {
        QMessageBox::warning(this, "XML Parse Error", 
                             QString("Error parsing RAW palette XML: %1").arg(xml.errorString()));
    }
    
    // Sort tilesets alphabetically
    m_tilesets.sort();
}

void RawItemsPaletteTab::populateTilesetCombo()
{
    m_updatingUI = true;
    m_tilesetCombo->clear();
    
    // Add "All Tilesets" option
    m_tilesetCombo->addItem(ALL_TILESETS_TEXT);
    
    // Add individual tilesets
    for (const QString& tileset : m_tilesets) {
        m_tilesetCombo->addItem(tileset);
    }
    
    m_updatingUI = false;
}

void RawItemsPaletteTab::updateItemList()
{
    updateItemList(m_currentTileset);
}

void RawItemsPaletteTab::updateItemList(const QString& tilesetFilter)
{
    if (m_updatingUI) return;
    
    m_updatingUI = true;
    m_itemList->clear();
    m_filteredItems.clear();
    
    // Determine which items to show
    QList<RawItemEntry> itemsToShow;
    if (tilesetFilter.isEmpty() || tilesetFilter == ALL_TILESETS_TEXT) {
        itemsToShow = m_rawItems;
    } else {
        itemsToShow = m_itemsByTileset.value(tilesetFilter);
    }
    
    // Apply search filter if active
    if (!m_currentSearchText.isEmpty()) {
        for (const RawItemEntry& entry : itemsToShow) {
            QString searchText = m_currentSearchText.toLower();
            if (QString::number(entry.itemId).contains(searchText) ||
                entry.name.toLower().contains(searchText) ||
                entry.tileset.toLower().contains(searchText)) {
                m_filteredItems.append(entry);
            }
        }
    } else {
        m_filteredItems = itemsToShow;
    }
    
    // Populate list widget
    for (const RawItemEntry& entry : m_filteredItems) {
        QString itemText = formatItemListEntry(entry);
        QListWidgetItem* item = new QListWidgetItem(entry.icon, itemText);
        item->setData(Qt::UserRole, entry.itemId);
        item->setToolTip(QString("Item ID: %1\nTileset: %2").arg(entry.itemId).arg(entry.tileset));
        m_itemList->addItem(item);
    }
    
    // Update count label
    m_itemCountLabel->setText(QString("Items: %1").arg(m_filteredItems.size()));
    
    m_updatingUI = false;
}

void RawItemsPaletteTab::applySearchFilter(const QString& searchText)
{
    m_currentSearchText = searchText;
    updateItemList(m_currentTileset);
}

void RawItemsPaletteTab::activateRawBrush(quint16 itemId)
{
    if (!m_brushStateManager) return;
    
    // Get or create RAW brush
    auto* rawBrush = dynamic_cast<RME::core::brush::RawBrush*>(
        m_brushStateManager->getBrush("RawBrush"));
    
    if (rawBrush) {
        rawBrush->setCurrentItemId(itemId);
        m_brushStateManager->setActiveBrush("RawBrush");
        emit rawBrushActivated(itemId);
    }
}

QIcon RawItemsPaletteTab::getItemIcon(quint16 itemId) const
{
    // TODO: Integrate with sprite/texture system when available
    // For now, return empty icon
    Q_UNUSED(itemId);
    return QIcon();
}

QString RawItemsPaletteTab::getItemName(quint16 itemId) const
{
    if (m_itemDatabase) {
        // TODO: Get actual item name from database
        // For now, return generic name
        return QString("Item %1").arg(itemId);
    }
    return QString("Item %1").arg(itemId);
}

QString RawItemsPaletteTab::formatItemListEntry(const RawItemEntry& entry) const
{
    return QString("%1 - %2").arg(entry.itemId).arg(entry.name);
}

void RawItemsPaletteTab::addItemsFromRange(quint16 fromId, quint16 toId, const QString& tileset)
{
    for (quint16 id = fromId; id <= toId; ++id) {
        addSingleItem(id, tileset);
    }
}

void RawItemsPaletteTab::addSingleItem(quint16 itemId, const QString& tileset)
{
    QString itemName = getItemName(itemId);
    QIcon itemIcon = getItemIcon(itemId);
    
    RawItemEntry entry(itemId, itemName, tileset);
    entry.icon = itemIcon;
    
    m_rawItems.append(entry);
    m_itemsByTileset[tileset].append(entry);
}

void RawItemsPaletteTab::onTilesetSelectionChanged()
{
    if (m_updatingUI) return;
    
    QString selectedTileset = m_tilesetCombo->currentText();
    m_currentTileset = (selectedTileset == ALL_TILESETS_TEXT) ? QString() : selectedTileset;
    updateItemList(m_currentTileset);
}

void RawItemsPaletteTab::onItemSelectionChanged()
{
    quint16 selectedItemId = getSelectedItemId();
    
    if (selectedItemId > 0) {
        // Find the selected item entry
        RawItemEntry selectedEntry;
        for (const RawItemEntry& entry : m_filteredItems) {
            if (entry.itemId == selectedItemId) {
                selectedEntry = entry;
                break;
            }
        }
        
        // Update info labels
        m_selectedItemLabel->setText(QString("Item %1: %2").arg(selectedEntry.itemId).arg(selectedEntry.name));
        m_itemDetailsLabel->setText(QString("Tileset: %1\nItem ID: %2")
                                    .arg(selectedEntry.tileset)
                                    .arg(selectedEntry.itemId));
        
        emit itemSelected(selectedItemId);
    } else {
        m_selectedItemLabel->setText("No item selected");
        m_itemDetailsLabel->setText("Select an item to view details");
    }
}

void RawItemsPaletteTab::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    quint16 itemId = item->data(Qt::UserRole).toUInt();
    if (itemId > 0) {
        activateRawBrush(itemId);
    }
}

void RawItemsPaletteTab::onSearchTextChanged(const QString& text)
{
    applySearchFilter(text);
}

void RawItemsPaletteTab::onClearSearch()
{
    m_searchEdit->clear();
    applySearchFilter(QString());
}

} // namespace palettes
} // namespace ui
} // namespace RME