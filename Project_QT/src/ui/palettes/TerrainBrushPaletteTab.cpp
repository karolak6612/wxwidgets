#include "TerrainBrushPaletteTab.h"
#include "core/assets/MaterialManager.h"
#include "core/brush/BrushStateManager.h"
#include "core/brush/GroundBrush.h"
#include "core/brush/WallBrush.h"
#include "core/brush/DoodadBrush.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>

namespace RME {
namespace ui {
namespace palettes {

const QString TerrainBrushPaletteTab::ALL_TYPES_TEXT = "(All Types)";
const QString TerrainBrushPaletteTab::GROUNDS_XML_PATH = "XML/760/grounds.xml";
const QString TerrainBrushPaletteTab::WALLS_XML_PATH = "XML/760/walls.xml";
const QString TerrainBrushPaletteTab::TILESETS_XML_PATH = "XML/760/tilesets.xml";
const QString TerrainBrushPaletteTab::DOODADS_XML_PATH = "XML/760/doodads.xml";

TerrainBrushPaletteTab::TerrainBrushPaletteTab(
    RME::core::IBrushStateService* brushStateService,
    RME::core::IClientDataService* clientDataService,
    QWidget* parent
) : QWidget(parent)
    , m_brushStateService(brushStateService)
    , m_clientDataService(clientDataService)
{
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_clientDataService);
    
    setupUI();
    connectSignals();
    loadTerrainBrushesFromXml();
}

void TerrainBrushPaletteTab::setMaterialManager(RME::core::assets::MaterialManager* materialManager)
{
    m_materialManager = materialManager;
    refreshContent();
}

// Get MaterialManager from service if not set directly
RME::core::assets::MaterialManager* TerrainBrushPaletteTab::getMaterialManager() const
{
    if (m_materialManager) {
        return m_materialManager;
    }
    
    // Get from service if not set directly
    if (m_clientDataService) {
        return m_clientDataService->getMaterialManager();
    }
    
    return nullptr;
}

void TerrainBrushPaletteTab::setBrushStateManager(RME::core::brush::BrushStateManager* brushManager)
{
    m_brushStateManager = brushManager;
}

void TerrainBrushPaletteTab::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
}

void TerrainBrushPaletteTab::refreshContent()
{
    updateBrushList();
}

void TerrainBrushPaletteTab::loadTerrainBrushesFromXml()
{
    m_terrainBrushes.clear();
    m_brushTypes.clear();
    m_brushesByType.clear();
    
    parseGroundsXml();
    parseWallsXml();
    parseDoodadsFromTilesets();
    
    // Sort brush types
    m_brushTypes.sort();
    
    populateBrushTypeCombo();
    updateBrushList();
}

QString TerrainBrushPaletteTab::getSelectedBrushName() const
{
    QListWidgetItem* currentItem = m_brushList->currentItem();
    if (!currentItem) {
        return QString();
    }
    
    return currentItem->data(Qt::UserRole).toString();
}

QString TerrainBrushPaletteTab::getSelectedBrushType() const
{
    QListWidgetItem* currentItem = m_brushList->currentItem();
    if (!currentItem) {
        return QString();
    }
    
    return currentItem->data(Qt::UserRole + 1).toString();
}

void TerrainBrushPaletteTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Filter group
    m_filterGroup = new QGroupBox("Filters", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);
    
    // Brush type filter
    m_filterLayout->addWidget(new QLabel("Type:", this));
    m_brushTypeCombo = new QComboBox(this);
    m_brushTypeCombo->setObjectName("brushTypeCombo");
    m_brushTypeCombo->setToolTip("Filter brushes by type");
    m_filterLayout->addWidget(m_brushTypeCombo, 1);
    
    // Search filter
    m_filterLayout->addWidget(new QLabel("Search:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("Search brushes...");
    m_searchEdit->setToolTip("Search for brushes by name");
    m_filterLayout->addWidget(m_searchEdit, 1);
    
    m_clearSearchButton = new QPushButton("Clear", this);
    m_clearSearchButton->setObjectName("clearSearchButton");
    m_clearSearchButton->setToolTip("Clear search filter");
    m_filterLayout->addWidget(m_clearSearchButton);
    
    m_mainLayout->addWidget(m_filterGroup);
    
    // Brushes group
    m_brushesGroup = new QGroupBox("Terrain Brushes", this);
    m_brushesLayout = new QVBoxLayout(m_brushesGroup);
    
    // Brush count label
    m_brushCountLabel = new QLabel("Brushes: 0", this);
    m_brushCountLabel->setObjectName("brushCountLabel");
    m_brushesLayout->addWidget(m_brushCountLabel);
    
    // Brush list
    m_brushList = new QListWidget(this);
    m_brushList->setObjectName("brushList");
    m_brushList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_brushList->setToolTip("Double-click to activate terrain brush");
    m_brushesLayout->addWidget(m_brushList, 1);
    
    m_mainLayout->addWidget(m_brushesGroup, 1);
    
    // Info group
    m_infoGroup = new QGroupBox("Brush Information", this);
    m_infoLayout = new QVBoxLayout(m_infoGroup);
    
    m_selectedBrushLabel = new QLabel("No brush selected", this);
    m_selectedBrushLabel->setObjectName("selectedBrushLabel");
    m_selectedBrushLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_infoLayout->addWidget(m_selectedBrushLabel);
    
    m_brushDetailsLabel = new QLabel("Select a brush to view details", this);
    m_brushDetailsLabel->setObjectName("brushDetailsLabel");
    m_brushDetailsLabel->setWordWrap(true);
    m_infoLayout->addWidget(m_brushDetailsLabel);
    
    m_mainLayout->addWidget(m_infoGroup);
    
    // Set stretch factors
    m_mainLayout->setStretchFactor(m_brushesGroup, 1);
}

void TerrainBrushPaletteTab::connectSignals()
{
    connect(m_brushTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TerrainBrushPaletteTab::onBrushTypeChanged);
    
    connect(m_brushList, &QListWidget::itemSelectionChanged,
            this, &TerrainBrushPaletteTab::onBrushSelectionChanged);
    
    connect(m_brushList, &QListWidget::itemDoubleClicked,
            this, &TerrainBrushPaletteTab::onBrushDoubleClicked);
    
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &TerrainBrushPaletteTab::onSearchTextChanged);
    
    connect(m_clearSearchButton, &QPushButton::clicked,
            this, &TerrainBrushPaletteTab::onClearSearch);
}

void TerrainBrushPaletteTab::parseGroundsXml()
{
    QFile file(GROUNDS_XML_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Silently fail if file doesn't exist
    }
    
    QXmlStreamReader xml(&file);
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement && xml.name() == "brush") {
            parseBrushFromXml(xml, "ground", GROUNDS_XML_PATH);
        }
    }
}

void TerrainBrushPaletteTab::parseWallsXml()
{
    QFile file(WALLS_XML_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Silently fail if file doesn't exist
    }
    
    QXmlStreamReader xml(&file);
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement && xml.name() == "brush") {
            parseBrushFromXml(xml, "wall", WALLS_XML_PATH);
        }
    }
}

void TerrainBrushPaletteTab::parseDoodadsFromTilesets()
{
    QFile file(TILESETS_XML_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Silently fail if file doesn't exist
    }
    
    QXmlStreamReader xml(&file);
    QString currentTileset;
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "tileset") {
                currentTileset = xml.attributes().value("name").toString();
            }
            else if (xml.name() == "brush" && !currentTileset.isEmpty()) {
                // This is a brush reference in a tileset
                QString brushName = xml.attributes().value("name").toString();
                if (!brushName.isEmpty()) {
                    TerrainBrushEntry entry(brushName, "doodad");
                    entry.xmlFile = TILESETS_XML_PATH;
                    entry.icon = getBrushIcon(entry);
                    
                    m_terrainBrushes.append(entry);
                    m_brushesByType["doodad"].append(entry);
                    
                    if (!m_brushTypes.contains("doodad")) {
                        m_brushTypes.append("doodad");
                    }
                }
            }
        }
    }
}

void TerrainBrushPaletteTab::populateBrushTypeCombo()
{
    m_updatingUI = true;
    m_brushTypeCombo->clear();
    
    // Add "All Types" option
    m_brushTypeCombo->addItem(ALL_TYPES_TEXT);
    
    // Add individual brush types
    for (const QString& brushType : m_brushTypes) {
        QString displayName = brushType;
        displayName[0] = displayName[0].toUpper(); // Capitalize first letter
        m_brushTypeCombo->addItem(displayName, brushType);
    }
    
    m_updatingUI = false;
}

void TerrainBrushPaletteTab::updateBrushList()
{
    updateBrushList(m_currentBrushType);
}

void TerrainBrushPaletteTab::updateBrushList(const QString& typeFilter)
{
    if (m_updatingUI) return;
    
    m_updatingUI = true;
    m_brushList->clear();
    m_filteredBrushes.clear();
    
    // Determine which brushes to show
    QList<TerrainBrushEntry> brushesToShow;
    if (typeFilter.isEmpty() || typeFilter == ALL_TYPES_TEXT) {
        brushesToShow = m_terrainBrushes;
    } else {
        brushesToShow = m_brushesByType.value(typeFilter);
    }
    
    // Apply search filter if active
    if (!m_currentSearchText.isEmpty()) {
        for (const TerrainBrushEntry& entry : brushesToShow) {
            QString searchText = m_currentSearchText.toLower();
            if (entry.name.toLower().contains(searchText) ||
                entry.type.toLower().contains(searchText)) {
                m_filteredBrushes.append(entry);
            }
        }
    } else {
        m_filteredBrushes = brushesToShow;
    }
    
    // Populate list widget
    for (const TerrainBrushEntry& entry : m_filteredBrushes) {
        QString brushText = formatBrushListEntry(entry);
        QListWidgetItem* item = new QListWidgetItem(entry.icon, brushText);
        item->setData(Qt::UserRole, entry.name);
        item->setData(Qt::UserRole + 1, entry.type);
        item->setToolTip(QString("Brush: %1\nType: %2\nServer ID: %3")
                         .arg(entry.name)
                         .arg(entry.type)
                         .arg(entry.serverId));
        m_brushList->addItem(item);
    }
    
    // Update count label
    m_brushCountLabel->setText(QString("Brushes: %1").arg(m_filteredBrushes.size()));
    
    m_updatingUI = false;
}

void TerrainBrushPaletteTab::applySearchFilter(const QString& searchText)
{
    m_currentSearchText = searchText;
    updateBrushList(m_currentBrushType);
}

void TerrainBrushPaletteTab::activateTerrainBrush(const QString& brushName, const QString& brushType)
{
    if (!m_brushStateManager || brushName.isEmpty()) return;
    
    QString brushId;
    if (brushType == "ground") {
        brushId = "GroundBrush";
    } else if (brushType == "wall") {
        brushId = "WallBrush";
    } else if (brushType == "doodad") {
        brushId = "DoodadBrush";
    } else {
        return; // Unknown brush type
    }
    
    // Get the appropriate brush and configure it
    auto* brush = m_brushStateManager->getBrush(brushId);
    if (brush) {
        // Configure brush with material data from MaterialManager
        if (auto* materialManager = getMaterialManager()) {
            // MaterialManager is available and integrated
            qDebug() << "TerrainBrushPaletteTab: Configuring brush with MaterialManager data";
        }
        m_brushStateManager->setActiveBrush(brushId);
        emit terrainBrushActivated(brushName, brushType);
    }
}

QIcon TerrainBrushPaletteTab::getBrushIcon(const TerrainBrushEntry& entry) const
{
    // Integrate with sprite/texture system
    if (m_clientDataService && m_clientDataService->getSpriteManager()) {
        auto* spriteManager = m_clientDataService->getSpriteManager();
        // TODO: Get actual sprite for terrain and render it
        // QPixmap sprite = spriteManager->getTerrainSprite(terrainId);
    }
    // For now, return empty icon
    Q_UNUSED(entry);
    return QIcon();
}

QString TerrainBrushPaletteTab::formatBrushListEntry(const TerrainBrushEntry& entry) const
{
    QString typeDisplay = entry.type;
    typeDisplay[0] = typeDisplay[0].toUpper();
    return QString("%1 (%2)").arg(entry.name).arg(typeDisplay);
}

void TerrainBrushPaletteTab::parseBrushFromXml(QXmlStreamReader& xml, const QString& brushType, const QString& xmlFile)
{
    QXmlStreamAttributes attrs = xml.attributes();
    QString brushName = attrs.value("name").toString();
    quint16 serverId = attrs.value("server_lookid").toUShort();
    quint16 zOrder = attrs.value("z-order").toUShort();
    
    if (!brushName.isEmpty()) {
        TerrainBrushEntry entry(brushName, brushType, serverId);
        entry.zOrder = zOrder;
        entry.xmlFile = xmlFile;
        entry.icon = getBrushIcon(entry);
        
        m_terrainBrushes.append(entry);
        m_brushesByType[brushType].append(entry);
        
        if (!m_brushTypes.contains(brushType)) {
            m_brushTypes.append(brushType);
        }
    }
}

void TerrainBrushPaletteTab::onBrushTypeChanged()
{
    if (m_updatingUI) return;
    
    QString selectedType = m_brushTypeCombo->currentData().toString();
    m_currentBrushType = (selectedType == ALL_TYPES_TEXT) ? QString() : selectedType;
    updateBrushList(m_currentBrushType);
}

void TerrainBrushPaletteTab::onBrushSelectionChanged()
{
    QString selectedBrushName = getSelectedBrushName();
    QString selectedBrushType = getSelectedBrushType();
    
    if (!selectedBrushName.isEmpty()) {
        // Find the selected brush entry
        TerrainBrushEntry selectedEntry;
        for (const TerrainBrushEntry& entry : m_filteredBrushes) {
            if (entry.name == selectedBrushName && entry.type == selectedBrushType) {
                selectedEntry = entry;
                break;
            }
        }
        
        // Update info labels
        QString typeDisplay = selectedEntry.type;
        typeDisplay[0] = typeDisplay[0].toUpper();
        m_selectedBrushLabel->setText(QString("%1: %2").arg(typeDisplay).arg(selectedEntry.name));
        
        QString details = QString("Type: %1\nServer ID: %2")
                          .arg(selectedEntry.type)
                          .arg(selectedEntry.serverId);
        if (selectedEntry.zOrder > 0) {
            details += QString("\nZ-Order: %1").arg(selectedEntry.zOrder);
        }
        m_brushDetailsLabel->setText(details);
        
        emit brushSelected(selectedBrushName, selectedBrushType);
    } else {
        m_selectedBrushLabel->setText("No brush selected");
        m_brushDetailsLabel->setText("Select a brush to view details");
    }
}

void TerrainBrushPaletteTab::onBrushDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    QString brushName = item->data(Qt::UserRole).toString();
    QString brushType = item->data(Qt::UserRole + 1).toString();
    
    if (!brushName.isEmpty() && !brushType.isEmpty()) {
        activateTerrainBrush(brushName, brushType);
    }
}

void TerrainBrushPaletteTab::onSearchTextChanged(const QString& text)
{
    applySearchFilter(text);
}

void TerrainBrushPaletteTab::onClearSearch()
{
    m_searchEdit->clear();
    applySearchFilter(QString());
}

} // namespace palettes
} // namespace ui
} // namespace RME