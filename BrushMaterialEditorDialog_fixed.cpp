#include "BrushMaterialEditorDialog.h"
#include "core/utils/ResourcePathManager.h"
#include "ItemFinderDialogQt.h"
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QFileDialog>
#include <QStandardPaths>

namespace RME {
namespace ui {
namespace dialogs {

//
// BorderGridEditorWidget Implementation
//

BorderGridEditorWidget::BorderGridEditorWidget(QWidget* parent)
    : QWidget(parent)
    , m_cellSize(40, 40)
{
    setMinimumSize(160, 160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    
    // Initialize border items
    m_borderItems.resize(12);
    for (int i = 0; i < 12; ++i) {
        m_borderItems[i] = BorderItem(static_cast<BorderPosition>(i), 0);
    }
    
    updateLayout();
}

void BorderGridEditorWidget::setItemForPosition(BorderPosition pos, uint16_t itemId)
{
    int index = static_cast<int>(pos);
    if (index >= 0 && index < m_borderItems.size()) {
        m_borderItems[index].itemId = itemId;
        emit itemChanged(pos, itemId);
        update();
    }
}

uint16_t BorderGridEditorWidget::getItemForPosition(BorderPosition pos) const
{
    int index = static_cast<int>(pos);
    if (index >= 0 && index < m_borderItems.size()) {
        return m_borderItems[index].itemId;
    }
    return 0;
}

void BorderGridEditorWidget::clearAllItems()
{
    for (auto& item : m_borderItems) {
        item.itemId = 0;
    }
    update();
}

void BorderGridEditorWidget::setSelectedPosition(BorderPosition pos)
{
    if (m_selectedPosition != pos) {
        m_selectedPosition = pos;
        emit positionSelected(pos);
        update();
    }
}

void BorderGridEditorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), palette().window());
    
    // Draw grid cells
    const QRect gridRect = m_gridRect;
    const int cellWidth = m_cellSize.width();
    const int cellHeight = m_cellSize.height();
    
    // Draw the 3x3 grid representing border positions
    for (int i = 0; i < 12; ++i) {
        BorderPosition pos = static_cast<BorderPosition>(i);
        QRect cellRect = getCellRect(pos);
        bool selected = (pos == m_selectedPosition);
        drawCell(painter, pos, cellRect, selected);
    }
}

void BorderGridEditorWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        BorderPosition pos = getPositionFromPoint(event->pos());
        if (pos != BorderPosition(-1)) { // Valid position
            setSelectedPosition(pos);
        }
    }
    QWidget::mousePressEvent(event);
}

void BorderGridEditorWidget::resizeEvent(QResizeEvent* event)
{
    updateLayout();
    QWidget::resizeEvent(event);
}

void BorderGridEditorWidget::updateLayout()
{
    const int margin = 10;
    const int availableWidth = width() - 2 * margin;
    const int availableHeight = height() - 2 * margin;
    
    // Calculate cell size to fit 3x3 grid
    const int cellWidth = availableWidth / 3;
    const int cellHeight = availableHeight / 3;
    m_cellSize = QSize(qMin(cellWidth, cellHeight), qMin(cellWidth, cellHeight));
    
    // Center the grid
    const int gridWidth = m_cellSize.width() * 3;
    const int gridHeight = m_cellSize.height() * 3;
    const int gridX = (width() - gridWidth) / 2;
    const int gridY = (height() - gridHeight) / 2;
    
    m_gridRect = QRect(gridX, gridY, gridWidth, gridHeight);
}

QRect BorderGridEditorWidget::getCellRect(BorderPosition pos) const
{
    const int cellWidth = m_cellSize.width();
    const int cellHeight = m_cellSize.height();
    const int gridX = m_gridRect.x();
    const int gridY = m_gridRect.y();
    
    // Map border positions to 3x3 grid coordinates
    int x = 0, y = 0;
    switch (pos) {
        case BorderPosition::CORNER_NW: x = 0; y = 0; break;
        case BorderPosition::NORTH: x = 1; y = 0; break;
        case BorderPosition::CORNER_NE: x = 2; y = 0; break;
        case BorderPosition::WEST: x = 0; y = 1; break;
        case BorderPosition::DIAGONAL_NW: x = 1; y = 1; break; // Center represents diagonals
        case BorderPosition::EAST: x = 2; y = 1; break;
        case BorderPosition::CORNER_SW: x = 0; y = 2; break;
        case BorderPosition::SOUTH: x = 1; y = 2; break;
        case BorderPosition::CORNER_SE: x = 2; y = 2; break;
        default: return QRect(); // Invalid position
    }
    
    return QRect(gridX + x * cellWidth, gridY + y * cellHeight, cellWidth, cellHeight);
}

BorderPosition BorderGridEditorWidget::getPositionFromPoint(const QPoint& point) const
{
    if (!m_gridRect.contains(point)) {
        return BorderPosition(-1); // Invalid
    }
    
    const int cellWidth = m_cellSize.width();
    const int cellHeight = m_cellSize.height();
    const int x = (point.x() - m_gridRect.x()) / cellWidth;
    const int y = (point.y() - m_gridRect.y()) / cellHeight;
    
    // Map 3x3 grid coordinates back to border positions
    if (x == 0 && y == 0) return BorderPosition::CORNER_NW;
    if (x == 1 && y == 0) return BorderPosition::NORTH;
    if (x == 2 && y == 0) return BorderPosition::CORNER_NE;
    if (x == 0 && y == 1) return BorderPosition::WEST;
    if (x == 1 && y == 1) return BorderPosition::DIAGONAL_NW; // Center for diagonals
    if (x == 2 && y == 1) return BorderPosition::EAST;
    if (x == 0 && y == 2) return BorderPosition::CORNER_SW;
    if (x == 1 && y == 2) return BorderPosition::SOUTH;
    if (x == 2 && y == 2) return BorderPosition::CORNER_SE;
    
    return BorderPosition(-1); // Invalid
}

QString BorderGridEditorWidget::getPositionName(BorderPosition pos) const
{
    switch (pos) {
        case BorderPosition::NORTH: return "North";
        case BorderPosition::EAST: return "East";
        case BorderPosition::SOUTH: return "South";
        case BorderPosition::WEST: return "West";
        case BorderPosition::CORNER_NW: return "Corner NW";
        case BorderPosition::CORNER_NE: return "Corner NE";
        case BorderPosition::CORNER_SE: return "Corner SE";
        case BorderPosition::CORNER_SW: return "Corner SW";
        case BorderPosition::DIAGONAL_NW: return "Diagonal NW";
        case BorderPosition::DIAGONAL_NE: return "Diagonal NE";
        case BorderPosition::DIAGONAL_SE: return "Diagonal SE";
        case BorderPosition::DIAGONAL_SW: return "Diagonal SW";
        default: return "Unknown";
    }
}

void BorderGridEditorWidget::drawCell(QPainter& painter, BorderPosition pos, const QRect& rect, bool selected) const
{
    if (rect.isEmpty()) return;
    
    // Draw cell background
    QColor bgColor = selected ? palette().highlight().color() : palette().base().color();
    painter.fillRect(rect, bgColor);
    
    // Draw border
    QPen borderPen(selected ? palette().highlightedText().color() : palette().text().color());
    borderPen.setWidth(selected ? 2 : 1);
    painter.setPen(borderPen);
    painter.drawRect(rect);
    
    // Draw item ID if set
    uint16_t itemId = getItemForPosition(pos);
    if (itemId > 0) {
        painter.setPen(selected ? palette().highlightedText().color() : palette().text().color());
        painter.drawText(rect, Qt::AlignCenter, QString::number(itemId));
    }
    
    // Draw position label
    QString posName = getPositionName(pos);
    QFont smallFont = painter.font();
    smallFont.setPointSize(qMax(6, smallFont.pointSize() - 2));
    painter.setFont(smallFont);
    painter.setPen(selected ? palette().highlightedText().color() : palette().text().color());
    
    QRect labelRect = rect.adjusted(2, 2, -2, -2);
    painter.drawText(labelRect, Qt::AlignTop | Qt::AlignLeft, posName.left(2)); // Show first 2 chars
}

//
// BorderPreviewWidget Implementation
//

BorderPreviewWidget::BorderPreviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_cellSize(24, 24)
{
    setMinimumSize(120, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BorderPreviewWidget::updatePreview(const QVector<BorderItem>& items)
{
    m_previewItems = items;
    update();
}

void BorderPreviewWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), palette().window());
    
    drawPreviewGrid(painter);
}

void BorderPreviewWidget::drawPreviewGrid(QPainter& painter) const
{
    const int gridSize = 5; // 5x5 preview grid
    const int cellWidth = width() / gridSize;
    const int cellHeight = height() / gridSize;
    
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            QRect cellRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
            
            // Draw cell border
            painter.setPen(palette().text().color());
            painter.drawRect(cellRect);
            
            // Get item for this position
            uint16_t itemId = getItemForPreviewPosition(x, y);
            if (itemId > 0) {
                // Draw item ID (in a real implementation, this would be the sprite)
                painter.setPen(palette().text().color());
                painter.drawText(cellRect, Qt::AlignCenter, QString::number(itemId));
            }
        }
    }
}

uint16_t BorderPreviewWidget::getItemForPreviewPosition(int x, int y) const
{
    // Center tile (2,2) is the main tile
    if (x == 2 && y == 2) {
        return 0; // No border item in center
    }
    
    // Map preview positions to border positions
    // This is a simplified mapping - in reality, this would be more complex
    if (x == 2 && y == 1) { // North
        for (const auto& item : m_previewItems) {
            if (item.position == BorderPosition::NORTH) {
                return item.itemId;
            }
        }
    }
    // Add more position mappings as needed...
    
    return 0;
}

//
// BrushMaterialEditorDialog Implementation
//

BrushMaterialEditorDialog::BrushMaterialEditorDialog(QWidget* parent, RME::core::assets::MaterialManager* materialManager, RME::core::assets::ItemDatabase* itemDatabase)
    : QDialog(parent), m_materialManager(materialManager), m_itemDatabase(itemDatabase)
{
    setWindowTitle("Brush & Material Editor");
    setModal(true);
    resize(800, 600);
    
    setupUI();
    loadData();
    connectSignals();
}

void BrushMaterialEditorDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    // Setup all tabs
    setupBordersTab();
    setupGroundBrushesTab();
    setupWallBrushesTab();
    setupDoodadBrushesTab();
    
    // Setup button box
    setupButtonBox();
    mainLayout->addWidget(m_buttonBox);
}

void BrushMaterialEditorDialog::setupBordersTab()
{
    m_bordersTab = new QWidget();
    m_tabWidget->addTab(m_bordersTab, "Borders");
    
    auto* layout = new QVBoxLayout(m_bordersTab);
    
    // Properties section
    auto* propsGroup = new QGroupBox("Border Properties");
    auto* propsLayout = new QFormLayout(propsGroup);
    
    m_borderNameEdit = new QLineEdit();
    propsLayout->addRow("Name:", m_borderNameEdit);
    
    m_borderIdSpin = new QSpinBox();
    m_borderIdSpin->setRange(1, 1000);
    propsLayout->addRow("Border ID:", m_borderIdSpin);
    
    m_groupIdSpin = new QSpinBox();
    m_groupIdSpin->setRange(0, 1000);
    propsLayout->addRow("Group ID:", m_groupIdSpin);
    
    m_optionalCheck = new QCheckBox("Optional");
    propsLayout->addRow(m_optionalCheck);
    
    m_groundBorderCheck = new QCheckBox("Is Ground Border");
    propsLayout->addRow(m_groundBorderCheck);
    
    m_borderCombo = new QComboBox();
    propsLayout->addRow("Load Border:", m_borderCombo);
    
    layout->addWidget(propsGroup);
    
    // Main editing area
    m_bordersSplitter = new QSplitter(Qt::Horizontal);
    layout->addWidget(m_bordersSplitter);
    
    // Left side - Grid editor and controls
    auto* leftWidget = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftWidget);
    
    // Grid editor
    auto* gridGroup = new QGroupBox("Border Grid Editor");
    auto* gridLayout = new QVBoxLayout(gridGroup);
    
    m_borderGrid = new BorderGridEditorWidget();
    gridLayout->addWidget(m_borderGrid);
    
    m_selectedPositionLabel = new QLabel("Selected: North");
    gridLayout->addWidget(m_selectedPositionLabel);
    
    leftLayout->addWidget(gridGroup);
    
    // Item assignment controls
    auto* itemGroup = new QGroupBox("Item Assignment");
    auto* itemLayout = new QFormLayout(itemGroup);
    
    m_borderItemIdSpin = new QSpinBox();
    m_borderItemIdSpin->setRange(0, 65535);
    itemLayout->addRow("Item ID:", m_borderItemIdSpin);
    
    auto* itemButtonLayout = new QHBoxLayout();
    m_browseBorderItemButton = new QPushButton("Browse...");
    m_applyBorderItemButton = new QPushButton("Apply to Selected");
    itemButtonLayout->addWidget(m_browseBorderItemButton);
    itemButtonLayout->addWidget(m_applyBorderItemButton);
    itemLayout->addRow(itemButtonLayout);
    
    leftLayout->addWidget(itemGroup);
    
    // Action buttons
    auto* actionLayout = new QHBoxLayout();
    m_saveBorderButton = new QPushButton("Save Border");
    m_clearBorderButton = new QPushButton("Clear Grid");
    actionLayout->addWidget(m_saveBorderButton);
    actionLayout->addWidget(m_clearBorderButton);
    leftLayout->addLayout(actionLayout);
    
    leftLayout->addStretch();
    m_bordersSplitter->addWidget(leftWidget);
    
    // Right side - Preview
    auto* rightWidget = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightWidget);
    
    auto* previewGroup = new QGroupBox("Border Preview");
    auto* previewLayout = new QVBoxLayout(previewGroup);
    
    m_borderPreview = new BorderPreviewWidget();
    previewLayout->addWidget(m_borderPreview);
    
    rightLayout->addWidget(previewGroup);
    rightLayout->addStretch();
    m_bordersSplitter->addWidget(rightWidget);
    
    // Set splitter proportions
    m_bordersSplitter->setStretchFactor(0, 2);
    m_bordersSplitter->setStretchFactor(1, 1);
}

void BrushMaterialEditorDialog::setupGroundBrushesTab()
{
    m_groundTab = new QWidget();
    m_tabWidget->addTab(m_groundTab, "Ground Brushes");
    
    auto* layout = new QVBoxLayout(m_groundTab);
    
    // Properties section
    auto* propsGroup = new QGroupBox("Ground Brush Properties");
    auto* propsLayout = new QFormLayout(propsGroup);
    
    m_brushNameEdit = new QLineEdit();
    propsLayout->addRow("Brush Name:", m_brushNameEdit);
    
    m_serverLookIdSpin = new QSpinBox();
    m_serverLookIdSpin->setRange(0, 65535);
    propsLayout->addRow("Server Look ID:", m_serverLookIdSpin);
    
    m_zOrderSpin = new QSpinBox();
    m_zOrderSpin->setRange(0, 100);
    propsLayout->addRow("Z-Order:", m_zOrderSpin);
    
    m_tilesetCombo = new QComboBox();
    propsLayout->addRow("Target Tileset:", m_tilesetCombo);
    
    m_groundBrushCombo = new QComboBox();
    propsLayout->addRow("Load Brush:", m_groundBrushCombo);
    
    layout->addWidget(propsGroup);
    
    // Items section
    auto* itemsGroup = new QGroupBox("Ground Items");
    auto* itemsLayout = new QVBoxLayout(itemsGroup);
    
    m_groundItemsTable = new QTableWidget(0, 2);
    m_groundItemsTable->setHorizontalHeaderLabels({"Item ID", "Chance"});
    m_groundItemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsLayout->addWidget(m_groundItemsTable);
    
    auto* itemButtonLayout = new QHBoxLayout();
    m_addGroundItemButton = new QPushButton("Add Item");
    m_removeGroundItemButton = new QPushButton("Remove Item");
    m_editGroundItemButton = new QPushButton("Edit Item");
    itemButtonLayout->addWidget(m_addGroundItemButton);
    itemButtonLayout->addWidget(m_removeGroundItemButton);
    itemButtonLayout->addWidget(m_editGroundItemButton);
    itemsLayout->addLayout(itemButtonLayout);
    
    layout->addWidget(itemsGroup);
    
    // Border association section
    auto* borderGroup = new QGroupBox("Border Association");
    auto* borderLayout = new QFormLayout(borderGroup);
    
    m_borderAssocIdSpin = new QSpinBox();
    m_borderAssocIdSpin->setRange(0, 1000);
    borderLayout->addRow("Border ID:", m_borderAssocIdSpin);
    
    m_borderAlignmentCombo = new QComboBox();
    m_borderAlignmentCombo->addItems({"outer", "inner"});
    borderLayout->addRow("Border Alignment:", m_borderAlignmentCombo);
    
    m_includeToNoneCheck = new QCheckBox("Add 'to=none' border variant");
    borderLayout->addRow(m_includeToNoneCheck);
    
    m_includeInnerCheck = new QCheckBox("Add inner border variant");
    borderLayout->addRow(m_includeInnerCheck);
    
    layout->addWidget(borderGroup);
    
    // Action buttons
    auto* actionLayout = new QHBoxLayout();
    m_saveGroundBrushButton = new QPushButton("Save Ground Brush");
    actionLayout->addWidget(m_saveGroundBrushButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
}

void BrushMaterialEditorDialog::setupWallBrushesTab()
{
    m_wallTab = new QWidget();
    m_tabWidget->addTab(m_wallTab, "Wall Brushes");
    
    auto* layout = new QVBoxLayout(m_wallTab);
    
    // Properties section
    auto* propsGroup = new QGroupBox("Wall Brush Properties");
    auto* propsLayout = new QFormLayout(propsGroup);
    
    m_wallBrushNameEdit = new QLineEdit();
    propsLayout->addRow("Brush Name:", m_wallBrushNameEdit);
    
    m_wallServerLookIdSpin = new QSpinBox();
    m_wallServerLookIdSpin->setRange(0, 65535);
    propsLayout->addRow("Server Look ID:", m_wallServerLookIdSpin);
    
    m_wallTilesetCombo = new QComboBox();
    propsLayout->addRow("Target Tileset:", m_wallTilesetCombo);
    
    m_wallBrushCombo = new QComboBox();
    propsLayout->addRow("Load Brush:", m_wallBrushCombo);
    
    layout->addWidget(propsGroup);
    
    // Wall items section
    auto* itemsGroup = new QGroupBox("Wall Items (Basic)");
    auto* itemsLayout = new QFormLayout(itemsGroup);
    
    m_horizontalWallSpin = new QSpinBox();
    m_horizontalWallSpin->setRange(0, 65535);
    itemsLayout->addRow("Horizontal Wall:", m_horizontalWallSpin);
    
    m_verticalWallSpin = new QSpinBox();
    m_verticalWallSpin->setRange(0, 65535);
    itemsLayout->addRow("Vertical Wall:", m_verticalWallSpin);
    
    m_wallPoleSpin = new QSpinBox();
    m_wallPoleSpin->setRange(0, 65535);
    itemsLayout->addRow("Wall Pole:", m_wallPoleSpin);
    
    layout->addWidget(itemsGroup);
    
    // Note about future expansion
    auto* noteLabel = new QLabel("Note: This is a basic implementation. Future versions will include "
                                 "a visual grid editor for all 12+ wall segment types and door/window definitions.");
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addWidget(noteLabel);
    
    // Action buttons
    auto* actionLayout = new QHBoxLayout();
    m_saveWallBrushButton = new QPushButton("Save Wall Brush");
    actionLayout->addWidget(m_saveWallBrushButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
    
    layout->addStretch();
}

void BrushMaterialEditorDialog::setupDoodadBrushesTab()
{
    m_doodadTab = new QWidget();
    m_tabWidget->addTab(m_doodadTab, "Doodad Brushes");
    
    auto* layout = new QVBoxLayout(m_doodadTab);
    
    // Properties section
    auto* propsGroup = new QGroupBox("Doodad Brush Properties");
    auto* propsLayout = new QFormLayout(propsGroup);
    
    m_doodadBrushNameEdit = new QLineEdit();
    propsLayout->addRow("Brush Name:", m_doodadBrushNameEdit);
    
    m_doodadServerLookIdSpin = new QSpinBox();
    m_doodadServerLookIdSpin->setRange(0, 65535);
    propsLayout->addRow("Server Look ID:", m_doodadServerLookIdSpin);
    
    m_doodadTilesetCombo = new QComboBox();
    propsLayout->addRow("Target Tileset:", m_doodadTilesetCombo);
    
    m_doodadBrushCombo = new QComboBox();
    propsLayout->addRow("Load Brush:", m_doodadBrushCombo);
    
    layout->addWidget(propsGroup);
    
    // Properties checkboxes
    auto* behaviorGroup = new QGroupBox("Doodad Properties");
    auto* behaviorLayout = new QHBoxLayout(behaviorGroup);
    
    m_draggableCheck = new QCheckBox("Draggable");
    m_blockingCheck = new QCheckBox("Blocking");
    behaviorLayout->addWidget(m_draggableCheck);
    behaviorLayout->addWidget(m_blockingCheck);
    behaviorLayout->addStretch();
    
    layout->addWidget(behaviorGroup);
    
    // Composite items section
    auto* itemsGroup = new QGroupBox("Composite Items");
    auto* itemsLayout = new QVBoxLayout(itemsGroup);
    
    m_doodadItemsTable = new QTableWidget(0, 4);
    m_doodadItemsTable->setHorizontalHeaderLabels({"Item ID", "X-Offset", "Y-Offset", "Z-Offset"});
    m_doodadItemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsLayout->addWidget(m_doodadItemsTable);
    
    auto* itemButtonLayout = new QHBoxLayout();
    m_addDoodadItemButton = new QPushButton("Add Item");
    m_removeDoodadItemButton = new QPushButton("Remove Item");
    m_editDoodadItemButton = new QPushButton("Edit Item");
    itemButtonLayout->addWidget(m_addDoodadItemButton);
    itemButtonLayout->addWidget(m_removeDoodadItemButton);
    itemButtonLayout->addWidget(m_editDoodadItemButton);
    itemsLayout->addLayout(itemButtonLayout);
    
    layout->addWidget(itemsGroup);
    
    // Note about future expansion
    auto* noteLabel = new QLabel("Note: Future versions will include a QGraphicsView-based visual editor "
                                 "for placing items relative to an origin for composite doodads.");
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addWidget(noteLabel);
    
    // Action buttons
    auto* actionLayout = new QHBoxLayout();
    m_saveDoodadBrushButton = new QPushButton("Save Doodad Brush");
    actionLayout->addWidget(m_saveDoodadBrushButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
    
    layout->addStretch();
}

void BrushMaterialEditorDialog::setupButtonBox()
{
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
}

void BrushMaterialEditorDialog::loadData()
{
    loadExistingBorders();
    loadExistingGroundBrushes();
    loadExistingWallBrushes();
    loadExistingDoodadBrushes();
    loadTilesets();
}

void BrushMaterialEditorDialog::loadExistingBorders()
{
    // TODO: Load borders from borders.xml
    m_borderCombo->addItem("(New Border)");
    // Example borders - replace with actual XML loading
    m_borderCombo->addItem("Border 1");
    m_borderCombo->addItem("Border 2");
}

void BrushMaterialEditorDialog::loadExistingGroundBrushes()
{
    // TODO: Load ground brushes from grounds.xml
    m_groundBrushCombo->addItem("(New Ground Brush)");
    // Example brushes - replace with actual XML loading
    m_groundBrushCombo->addItem("Grass Brush");
    m_groundBrushCombo->addItem("Stone Brush");
}

void BrushMaterialEditorDialog::loadExistingWallBrushes()
{
    // TODO: Load wall brushes from walls.xml
    m_wallBrushCombo->addItem("(New Wall Brush)");
    // Example brushes - replace with actual XML loading
    m_wallBrushCombo->addItem("Stone Wall");
    m_wallBrushCombo->addItem("Wood Wall");
}

void BrushMaterialEditorDialog::loadExistingDoodadBrushes()
{
    // TODO: Load doodad brushes from doodads.xml
    m_doodadBrushCombo->addItem("(New Doodad Brush)");
    // Example brushes - replace with actual XML loading
    m_doodadBrushCombo->addItem("Tree Doodad");
    m_doodadBrushCombo->addItem("Rock Doodad");
}

void BrushMaterialEditorDialog::loadTilesets()
{
    // TODO: Load tilesets from tilesets.xml
    QStringList tilesets = {"Terrain", "Doodads", "Items", "Walls", "Custom"};
    
    m_tilesetCombo->addItems(tilesets);
    m_wallTilesetCombo->addItems(tilesets);
    m_doodadTilesetCombo->addItems(tilesets);
}

void BrushMaterialEditorDialog::connectSignals()
{
    // Dialog buttons
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &BrushMaterialEditorDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &BrushMaterialEditorDialog::reject);
    
    // Tab changes
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &BrushMaterialEditorDialog::onTabChanged);
    
    // Borders tab signals
    connect(m_borderGrid, &BorderGridEditorWidget::positionSelected, 
            this, &BrushMaterialEditorDialog::onBorderPositionSelected);
    connect(m_borderGrid, &BorderGridEditorWidget::itemChanged, 
            this, [this](BorderPosition, uint16_t) { markAsModified(); });
    
    connect(m_browseBorderItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onBrowseBorderItem);
    connect(m_applyBorderItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onApplyBorderItem);
    connect(m_saveBorderButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onSaveBorder);
    connect(m_clearBorderButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onClearBorderGrid);
    
    // Border property changes
    connect(m_borderNameEdit, &QLineEdit::textChanged, this, &BrushMaterialEditorDialog::onBorderPropertyChanged);
    connect(m_borderIdSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &BrushMaterialEditorDialog::onBorderPropertyChanged);
    connect(m_groupIdSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &BrushMaterialEditorDialog::onBorderPropertyChanged);
    connect(m_optionalCheck, &QCheckBox::toggled, this, &BrushMaterialEditorDialog::onBorderPropertyChanged);
    connect(m_groundBorderCheck, &QCheckBox::toggled, this, &BrushMaterialEditorDialog::onBorderPropertyChanged);
    
    // Ground brushes tab signals
    connect(m_addGroundItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onAddGroundItem);
    connect(m_removeGroundItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onRemoveGroundItem);
    connect(m_editGroundItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onEditGroundItem);
    connect(m_saveGroundBrushButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onSaveGroundBrush);
    
    // Wall brushes tab signals
    connect(m_saveWallBrushButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onSaveWallBrush);
    
    // Doodad brushes tab signals
    connect(m_addDoodadItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onAddDoodadItem);
    connect(m_removeDoodadItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onRemoveDoodadItem);
    connect(m_editDoodadItemButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onEditDoodadItem);
    connect(m_saveDoodadBrushButton, &QPushButton::clicked, 
            this, &BrushMaterialEditorDialog::onSaveDoodadBrush);
}

//
// Slot Implementations
//

void BrushMaterialEditorDialog::accept()
{
    if (m_wasModified) {
        auto result = QMessageBox::question(this, "Unsaved Changes", 
                                          "You have unsaved changes. Do you want to save them before closing?",
                                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (result == QMessageBox::Cancel) {
            return;
        } else if (result == QMessageBox::Save) {
            // Save current tab data
            int currentTab = m_tabWidget->currentIndex();
            switch (currentTab) {
                case 0: onSaveBorder(); break;
                case 1: onSaveGroundBrush(); break;
                case 2: onSaveWallBrush(); break;
                case 3: onSaveDoodadBrush(); break;
            }
        }
    }
    
    QDialog::accept();
}

void BrushMaterialEditorDialog::reject()
{
    if (m_wasModified) {
        auto result = QMessageBox::question(this, "Unsaved Changes", 
                                          "You have unsaved changes. Are you sure you want to discard them?",
                                          QMessageBox::Discard | QMessageBox::Cancel);
        
        if (result == QMessageBox::Cancel) {
            return;
        }
    }
    
    QDialog::reject();
}

// Borders tab slots
void BrushMaterialEditorDialog::onBorderPositionSelected(BorderPosition pos)
{
    QString posName = m_borderGrid->getPositionName(pos);
    m_selectedPositionLabel->setText(QString("Selected: %1").arg(posName));
    
    // Update item ID spin box with current item for this position
    uint16_t itemId = m_borderGrid->getItemForPosition(pos);
    m_borderItemIdSpin->setValue(itemId);
}

void BrushMaterialEditorDialog::onBorderItemIdChanged()
{
    // This would be connected to the spin box value change if needed
    markAsModified();
}

void BrushMaterialEditorDialog::onBrowseBorderItem()
{
    // TODO: Replace nullptr with actual ItemManager instance
    ItemFinderDialogQt dialog(this, nullptr);
    if (dialog.exec() == QDialog::Accepted) {
        // TODO: Get selected item type and set ID
        // auto* itemType = dialog.getSelectedItemType();
        // if (itemType) {
        //     m_borderItemIdSpin->setValue(itemType->id);
        // }
        
        // For now, just show a placeholder message
        QMessageBox::information(this, "Item Finder", "Item finder integration will be implemented when ItemManager is available.");
    }
}

void BrushMaterialEditorDialog::onApplyBorderItem()
{
    uint16_t itemId = m_borderItemIdSpin->value();
    BorderPosition pos = m_borderGrid->getSelectedPosition();
    
    m_borderGrid->setItemForPosition(pos, itemId);
    
    // Update preview
    QVector<BorderItem> items;
    for (int i = 0; i < 12; ++i) {
        BorderPosition position = static_cast<BorderPosition>(i);
        uint16_t id = m_borderGrid->getItemForPosition(position);
        if (id > 0) {
            items.append(BorderItem(position, id));
        }
    }
    m_borderPreview->updatePreview(items);
    
    markAsModified();
}

void BrushMaterialEditorDialog::onLoadBorder()
{
    QString borderName = m_borderCombo->currentText();
    if (borderName == "(New Border)") {
        clearBorderData();
        return;
    }
    
    // TODO: Load border data from XML
    QMessageBox::information(this, "Load Border", 
                           QString("Loading border '%1' will be implemented when XML loading is available.").arg(borderName));
}

void BrushMaterialEditorDialog::onSaveBorder()
{
    if (!validateBorderData()) {
        return;
    }
    
    // TODO: Save border data to XML
    QString borderName = m_borderNameEdit->text();
    int borderId = m_borderIdSpin->value();
    
    QMessageBox::information(this, "Save Border", 
                           QString("Saving border '%1' (ID: %2) will be implemented when XML saving is available.")
                           .arg(borderName).arg(borderId));
    
    emit borderSaved(borderId);
    m_wasModified = false;
}

void BrushMaterialEditorDialog::onClearBorderGrid()
{
    auto result = QMessageBox::question(this, "Clear Grid", 
                                      "Are you sure you want to clear all border items?",
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        m_borderGrid->clearAllItems();
        m_borderPreview->updatePreview(QVector<BorderItem>());
        markAsModified();
    }
}

void BrushMaterialEditorDialog::onBorderPropertyChanged()
{
    markAsModified();
}

// Ground brushes tab slots
void BrushMaterialEditorDialog::onAddGroundItem()
{
    // Use ItemFinderDialog to select item
    ItemFinderDialogQt dialog(this, m_itemDatabase); // Use the actual ItemDatabase
    
    if (dialog.exec() == QDialog::Accepted) {
        auto* selectedItemType = dialog.getSelectedItemType();
        if (selectedItemType) {
            bool ok;
            int chance = QInputDialog::getInt(this, "Add Ground Item", "Chance (%):", 100, 1, 100, 1, &ok);
            if (!ok) return;
            
            int row = m_groundItemsTable->rowCount();
            m_groundItemsTable->insertRow(row);
            m_groundItemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(selectedItemType->getID())));
            m_groundItemsTable->setItem(row, 1, new QTableWidgetItem(getItemName(selectedItemType->getID())));
            m_groundItemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(chance)));
            
            markAsModified();
        }
    } else {
        // Fallback if ItemFinderDialog fails or is cancelled
        bool ok;
        int itemId = QInputDialog::getInt(this, "Add Ground Item", "Item ID:", 100, 100, 65535, 1, &ok);
        if (!ok) return;
        
        int chance = QInputDialog::getInt(this, "Add Ground Item", "Chance (%):", 100, 1, 100, 1, &ok);
        if (!ok) return;
        
        int row = m_groundItemsTable->rowCount();
        m_groundItemsTable->insertRow(row);
        m_groundItemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(itemId)));
        m_groundItemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(chance)));
        
        markAsModified();
    }
}

void BrushMaterialEditorDialog::onRemoveGroundItem()
{
    int currentRow = m_groundItemsTable->currentRow();
    if (currentRow >= 0) {
        m_groundItemsTable->removeRow(currentRow);
        markAsModified();
    }
}

void BrushMaterialEditorDialog::onEditGroundItem()
{
    int currentRow = m_groundItemsTable->currentRow();
    if (currentRow < 0) return;
    
    QTableWidgetItem* itemIdItem = m_groundItemsTable->item(currentRow, 0);
    QTableWidgetItem* chanceItem = m_groundItemsTable->item(currentRow, 1);
    
    if (!itemIdItem || !chanceItem) return;
    
    bool ok;
    int itemId = QInputDialog::getInt(this, "Edit Ground Item", "Item ID:", 
                                    itemIdItem->text().toInt(), 100, 65535, 1, &ok);
    if (!ok) return;
    
    int chance = QInputDialog::getInt(this, "Edit Ground Item", "Chance (%):", 
                                    chanceItem->text().toInt(), 1, 100, 1, &ok);
    if (!ok) return;
    
    itemIdItem->setText(QString::number(itemId));
    chanceItem->setText(QString::number(chance));
    
    markAsModified();
}

void BrushMaterialEditorDialog::onBrowseGroundItem()
{
    // TODO: Implement when needed
}

void BrushMaterialEditorDialog::onLoadGroundBrush()
{
    QString brushName = m_groundBrushCombo->currentText();
    if (brushName == "(New Ground Brush)") {
        clearGroundBrushData();
        return;
    }
    
    // TODO: Load ground brush data from XML
    QMessageBox::information(this, "Load Ground Brush", 
                           QString("Loading ground brush '%1' will be implemented when XML loading is available.").arg(brushName));
}

void BrushMaterialEditorDialog::onSaveGroundBrush()
{
    if (!validateGroundBrushData()) {
        return;
    }
    
    // TODO: Save ground brush data to XML
    QString brushName = m_brushNameEdit->text();
    
    QMessageBox::information(this, "Save Ground Brush", 
                           QString("Saving ground brush '%1' will be implemented when XML saving is available.")
                           .arg(brushName));
    
    emit groundBrushSaved(brushName);
    m_wasModified = false;
}

void BrushMaterialEditorDialog::onGroundPropertyChanged()
{
    markAsModified();
}

// Wall brushes tab slots
void BrushMaterialEditorDialog::onLoadWallBrush()
{
    QString brushName = m_wallBrushCombo->currentText();
    if (brushName == "(New Wall Brush)") {
        clearWallBrushData();
        return;
    }
    
    // TODO: Load wall brush data from XML
    QMessageBox::information(this, "Load Wall Brush", 
                           QString("Loading wall brush '%1' will be implemented when XML loading is available.").arg(brushName));
}

void BrushMaterialEditorDialog::onSaveWallBrush()
{
    if (!validateWallBrushData()) {
        return;
    }
    
    // TODO: Save wall brush data to XML
    QString brushName = m_wallBrushNameEdit->text();
    
    QMessageBox::information(this, "Save Wall Brush", 
                           QString("Saving wall brush '%1' will be implemented when XML saving is available.")
                           .arg(brushName));
    
    emit wallBrushSaved(brushName);
    m_wasModified = false;
}

void BrushMaterialEditorDialog::onWallPropertyChanged()
{
    markAsModified();
}

// Doodad brushes tab slots
void BrushMaterialEditorDialog::onAddDoodadItem()
{
    // Use ItemFinderDialog to select item
    ItemFinderDialogQt dialog(this, m_itemDatabase); // Use the actual ItemDatabase
    
    if (dialog.exec() == QDialog::Accepted) {
        auto* selectedItemType = dialog.getSelectedItemType();
        if (selectedItemType) {
            bool ok;
            int xOffset = QInputDialog::getInt(this, "Add Doodad Item", "X Offset:", 0, -10, 10, 1, &ok);
            if (!ok) return;
            
            int yOffset = QInputDialog::getInt(this, "Add Doodad Item", "Y Offset:", 0, -10, 10, 1, &ok);
            if (!ok) return;
            
            int zOffset = QInputDialog::getInt(this, "Add Doodad Item", "Z Offset:", 0, -10, 10, 1, &ok);
            if (!ok) return;
            
            int row = m_doodadItemsTable->rowCount();
            m_doodadItemsTable->insertRow(row);
            m_doodadItemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(selectedItemType->getID())));
            m_doodadItemsTable->setItem(row, 1, new QTableWidgetItem(getItemName(selectedItemType->getID())));
            m_doodadItemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(xOffset)));
            m_doodadItemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(yOffset)));
            m_doodadItemsTable->setItem(row, 4, new QTableWidgetItem(QString::number(zOffset)));
            
            markAsModified();
        }
    } else {
        // Fallback if ItemFinderDialog fails or is cancelled
        bool ok;
        int itemId = QInputDialog::getInt(this, "Add Doodad Item", "Item ID:", 100, 100, 65535, 1, &ok);
        if (!ok) return;
        
        int xOffset = QInputDialog::getInt(this, "Add Doodad Item", "X Offset:", 0, -10, 10, 1, &ok);
        if (!ok) return;
        
        int yOffset = QInputDialog::getInt(this, "Add Doodad Item", "Y Offset:", 0, -10, 10, 1, &ok);
        if (!ok) return;
        
        int zOffset = QInputDialog::getInt(this, "Add Doodad Item", "Z Offset:", 0, -10, 10, 1, &ok);
        if (!ok) return;
        
        int row = m_doodadItemsTable->rowCount();
        m_doodadItemsTable->insertRow(row);
        m_doodadItemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(itemId)));
        m_doodadItemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(xOffset)));
        m_doodadItemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(yOffset)));
        m_doodadItemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(zOffset)));
        
        markAsModified();
    }
}

void BrushMaterialEditorDialog::onRemoveDoodadItem()
{
    int currentRow = m_doodadItemsTable->currentRow();
    if (currentRow >= 0) {
        m_doodadItemsTable->removeRow(currentRow);
        markAsModified();
    }
}

void BrushMaterialEditorDialog::onEditDoodadItem()
{
    int currentRow = m_doodadItemsTable->currentRow();
    if (currentRow < 0) return;
    
    // TODO: Implement editing of doodad item properties
    QMessageBox::information(this, "Edit Doodad Item", "Doodad item editing will be implemented.");
}

void BrushMaterialEditorDialog::onLoadDoodadBrush()
{
    QString brushName = m_doodadBrushCombo->currentText();
    if (brushName == "(New Doodad Brush)") {
        clearDoodadBrushData();
        return;
    }
    
    // TODO: Load doodad brush data from XML
    QMessageBox::information(this, "Load Doodad Brush", 
                           QString("Loading doodad brush '%1' will be implemented when XML loading is available.").arg(brushName));
}

void BrushMaterialEditorDialog::onSaveDoodadBrush()
{
    if (!validateDoodadBrushData()) {
        return;
    }
    
    // TODO: Save doodad brush data to XML
    QString brushName = m_doodadBrushNameEdit->text();
    
    QMessageBox::information(this, "Save Doodad Brush", 
                           QString("Saving doodad brush '%1' will be implemented when XML saving is available.")
                           .arg(brushName));
    
    emit doodadBrushSaved(brushName);
    m_wasModified = false;
}

void BrushMaterialEditorDialog::onDoodadPropertyChanged()
{
    markAsModified();
}

// Helper slots
void BrushMaterialEditorDialog::onTabChanged(int index)
{
    // Update UI based on current tab
    Q_UNUSED(index)
}

//
// Helper Methods
//

void BrushMaterialEditorDialog::markAsModified()
{
    if (!m_wasModified) {
        m_wasModified = true;
        setWindowTitle(windowTitle() + " *");
    }
}

bool BrushMaterialEditorDialog::validateBorderData()
{
    if (m_borderNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Border name cannot be empty.");
        m_borderNameEdit->setFocus();
        return false;
    }
    
    if (m_borderIdSpin->value() <= 0) {
        QMessageBox::warning(this, "Validation Error", "Border ID must be greater than 0.");
        m_borderIdSpin->setFocus();
        return false;
    }
    
    return true;
}

bool BrushMaterialEditorDialog::validateGroundBrushData()
{
    if (m_brushNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Brush name cannot be empty.");
        m_brushNameEdit->setFocus();
        return false;
    }
    
    if (m_groundItemsTable->rowCount() == 0) {
        QMessageBox::warning(this, "Validation Error", "Ground brush must have at least one item.");
        return false;
    }
    
    return true;
}

bool BrushMaterialEditorDialog::validateWallBrushData()
{
    if (m_wallBrushNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Wall brush name cannot be empty.");
        m_wallBrushNameEdit->setFocus();
        return false;
    }
    
    return true;
}

bool BrushMaterialEditorDialog::validateDoodadBrushData()
{
    if (m_doodadBrushNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Doodad brush name cannot be empty.");
        m_doodadBrushNameEdit->setFocus();
        return false;
    }
    
    return true;
}

void BrushMaterialEditorDialog::clearBorderData()
{
    m_borderNameEdit->clear();
    m_borderIdSpin->setValue(1);
    m_groupIdSpin->setValue(0);
    m_optionalCheck->setChecked(false);
    m_groundBorderCheck->setChecked(false);
    m_borderGrid->clearAllItems();
    m_borderPreview->updatePreview(QVector<BorderItem>());
}

void BrushMaterialEditorDialog::clearGroundBrushData()
{
    m_brushNameEdit->clear();
    m_serverLookIdSpin->setValue(0);
    m_zOrderSpin->setValue(0);
    m_groundItemsTable->setRowCount(0);
    m_borderAssocIdSpin->setValue(0);
    m_borderAlignmentCombo->setCurrentIndex(0);
    m_includeToNoneCheck->setChecked(false);
    m_includeInnerCheck->setChecked(false);
}

void BrushMaterialEditorDialog::clearWallBrushData()
{
    m_wallBrushNameEdit->clear();
    m_wallServerLookIdSpin->setValue(0);
    m_horizontalWallSpin->setValue(0);
    m_verticalWallSpin->setValue(0);
    m_wallPoleSpin->setValue(0);
}

void BrushMaterialEditorDialog::clearDoodadBrushData()
{
    m_doodadBrushNameEdit->clear();
    m_doodadServerLookIdSpin->setValue(0);
    m_doodadItemsTable->setRowCount(0);
    m_draggableCheck->setChecked(false);
    m_blockingCheck->setChecked(false);
}

void BrushMaterialEditorDialog::updateGroundItemsTable()
{
    // TODO: Implement when needed
}

void BrushMaterialEditorDialog::updateDoodadItemsTable()
{
    // TODO: Implement when needed
}

QString BrushMaterialEditorDialog::getItemName(uint16_t itemId) const
{

    // Get item name from ItemDatabase if available
    if (m_itemDatabase) {
        // TODO: Use actual ItemDatabase API when available
        // auto itemData = m_itemDatabase->getItemData(itemId);
        // if (itemData) {
        //     return itemData->name;
        // }
    }
    return QString("Item %1").arg(itemId);
}

QString BrushMaterialEditorDialog::getXmlFilePath(const QString& filename) const
{
    // Use ResourcePathManager to resolve the path
    QString resolvedPath = RME::core::utils::ResourcePathManager::instance().resolvePath(filename, "xml");
    
    // If the file doesn't exist at the resolved path, try Qt resource system
    if (resolvedPath.isEmpty() || !QFile::exists(resolvedPath)) {
        QString qtResourcePath = ":/" + filename;
        if (QFile::exists(qtResourcePath)) {
            return qtResourcePath;
        }
    } else {
        return resolvedPath;
    }
    
    // If not found in ResourcePathManager or Qt resources, use app data path for creation
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/XML/" + filename;
}

bool BrushMaterialEditorDialog::ensureXmlDirectoryExists() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString xmlDirPath = appDataPath + "/XML";
    
    QDir xmlDir(xmlDirPath);
    if (!xmlDir.exists()) {
        return xmlDir.mkpath(".");
    }
    return true;
}

// XML operations (placeholder implementations)
bool BrushMaterialEditorDialog::saveBorderToXml()
{
    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString bordersPath = getXmlFilePath("borders.xml");
    QFile file(bordersPath);
    
    // Read existing borders first
    QDomDocument doc;
    QDomElement rootElement;
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::warning(this, "Error", "Could not parse existing borders.xml file.");
            return false;
        }
        file.close();
        rootElement = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(xmlDeclaration);
        rootElement = doc.createElement("borders");
        doc.appendChild(rootElement);
    }
    
    // Create new border element
    QDomElement borderElement = doc.createElement("border");
    borderElement.setAttribute("name", m_borderNameEdit->text());
    borderElement.setAttribute("id", m_borderIdSpin->value());
    
    // Add border items from grid
    for (int i = 0; i < 9; ++i) {
        BorderPosition pos = static_cast<BorderPosition>(i);
        uint16_t itemId = m_borderGridWidget->getItemForPosition(pos);
        if (itemId > 0) {
            QDomElement itemElement = doc.createElement("item");
            itemElement.setAttribute("position", i);
            itemElement.setAttribute("id", itemId);
            borderElement.appendChild(itemElement);
        }
    }
    
    // Remove existing border with same name if it exists
    QDomNodeList existingBorders = rootElement.elementsByTagName("border");
    for (int i = 0; i < existingBorders.count(); ++i) {
        QDomElement existing = existingBorders.at(i).toElement();
        if (existing.attribute("name") == m_borderNameEdit->text()) {
            rootElement.removeChild(existing);
            break;
        }
    }
    
    // Add new border
    rootElement.appendChild(borderElement);
    
    // Save to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open borders.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    
bool BrushMaterialEditorDialog::saveBorderToXml()
{
    // TODO: Implement XML saving

    return true;
}

bool BrushMaterialEditorDialog::saveGroundBrushToXml()
{

    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString groundsPath = getXmlFilePath("grounds.xml");
    QFile file(groundsPath);
    
    // Read existing brushes first
    QDomDocument doc;
    QDomElement rootElement;
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::warning(this, "Error", "Could not parse existing grounds.xml file.");
            return false;
        }
        file.close();
        rootElement = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(xmlDeclaration);
        rootElement = doc.createElement("groundbrushes");
        doc.appendChild(rootElement);
    }
    
    // Create new brush element
    QDomElement brushElement = doc.createElement("brush");
    brushElement.setAttribute("name", m_groundBrushNameEdit->text());
    brushElement.setAttribute("id", m_groundBrushIdSpin->value());
    
    // Add items from table
    for (int row = 0; row < m_groundItemsTable->rowCount(); ++row) {
        QTableWidgetItem* idItem = m_groundItemsTable->item(row, 0);
        QTableWidgetItem* chanceItem = m_groundItemsTable->item(row, 2);
        
        if (idItem && chanceItem) {
            QDomElement itemElement = doc.createElement("item");
            itemElement.setAttribute("id", idItem->text().toInt());
            itemElement.setAttribute("chance", chanceItem->text().toInt());
            brushElement.appendChild(itemElement);
        }
    }
    
    // Remove existing brush with same name if it exists
    QDomNodeList existingBrushes = rootElement.elementsByTagName("brush");
    for (int i = 0; i < existingBrushes.count(); ++i) {
        QDomElement existing = existingBrushes.at(i).toElement();
        if (existing.attribute("name") == m_groundBrushNameEdit->text()) {
            rootElement.removeChild(existing);
            break;
        }
    }
    
    // Add new brush
    rootElement.appendChild(brushElement);
    
    // Save to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open grounds.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    

    // TODO: Implement XML saving

    return true;
}

bool BrushMaterialEditorDialog::saveWallBrushToXml()
{

    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString wallsPath = getXmlFilePath("walls.xml");
    QFile file(wallsPath);
    
    // Read existing brushes first
    QDomDocument doc;
    QDomElement rootElement;
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::warning(this, "Error", "Could not parse existing walls.xml file.");
            return false;
        }
        file.close();
        rootElement = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(xmlDeclaration);
        rootElement = doc.createElement("wallbrushes");
        doc.appendChild(rootElement);
    }
    
    // Create new brush element
    QDomElement brushElement = doc.createElement("brush");
    brushElement.setAttribute("name", m_wallBrushNameEdit->text());
    brushElement.setAttribute("id", m_wallBrushIdSpin->value());
    
    // Add wall configuration
    brushElement.setAttribute("lookid", m_wallLookIdSpin->value());
    brushElement.setAttribute("server_lookid", m_wallServerLookIdSpin->value());
    
    // Remove existing brush with same name if it exists
    QDomNodeList existingBrushes = rootElement.elementsByTagName("brush");
    for (int i = 0; i < existingBrushes.count(); ++i) {
        QDomElement existing = existingBrushes.at(i).toElement();
        if (existing.attribute("name") == m_wallBrushNameEdit->text()) {
            rootElement.removeChild(existing);
            break;
        }
    }
    
    // Add new brush
    rootElement.appendChild(brushElement);
    
    // Save to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open walls.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    

    // TODO: Implement XML saving

    return true;
}

bool BrushMaterialEditorDialog::saveDoodadBrushToXml()
{

    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString doodadsPath = getXmlFilePath("doodads.xml");
    QFile file(doodadsPath);
    
    // Read existing brushes first
    QDomDocument doc;
    QDomElement rootElement;
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::warning(this, "Error", "Could not parse existing doodads.xml file.");
            return false;
        }
        file.close();
        rootElement = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(xmlDeclaration);
        rootElement = doc.createElement("doodadbrushes");
        doc.appendChild(rootElement);
    }
    
    // Create new brush element
    QDomElement brushElement = doc.createElement("brush");
    brushElement.setAttribute("name", m_doodadBrushNameEdit->text());
    brushElement.setAttribute("id", m_doodadBrushIdSpin->value());
    
    // Add items from table
    for (int row = 0; row < m_doodadItemsTable->rowCount(); ++row) {
        QTableWidgetItem* idItem = m_doodadItemsTable->item(row, 0);
        QTableWidgetItem* xItem = m_doodadItemsTable->item(row, 2);
        QTableWidgetItem* yItem = m_doodadItemsTable->item(row, 3);
        QTableWidgetItem* zItem = m_doodadItemsTable->item(row, 4);
        
        if (idItem && xItem && yItem && zItem) {
            QDomElement itemElement = doc.createElement("item");
            itemElement.setAttribute("id", idItem->text().toInt());
            itemElement.setAttribute("x", xItem->text().toInt());
            itemElement.setAttribute("y", yItem->text().toInt());
            itemElement.setAttribute("z", zItem->text().toInt());
            brushElement.appendChild(itemElement);
        }
    }
    
    // Remove existing brush with same name if it exists
    QDomNodeList existingBrushes = rootElement.elementsByTagName("brush");
    for (int i = 0; i < existingBrushes.count(); ++i) {
        QDomElement existing = existingBrushes.at(i).toElement();
        if (existing.attribute("name") == m_doodadBrushNameEdit->text()) {
            rootElement.removeChild(existing);
            break;
        }
    }
    
    // Add new brush
    rootElement.appendChild(brushElement);
    
    // Save to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open doodads.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    

    // TODO: Implement XML saving

    return true;
}

} // namespace dialogs
} // namespace ui
} // namespace RME