#include "BrushListWidget.h"
#include "core/brush/Brush.h"
#include <QListWidgetItem>
#include <QPainter>
#include <QDebug>

namespace RME {
namespace ui {
namespace palettes {

BrushListWidget::BrushListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName("BrushListWidget");
    setupUI();
    setupConnections();
    
    qDebug() << "BrushListWidget: Created";
}

BrushListWidget::~BrushListWidget()
{
    qDebug() << "BrushListWidget: Destroyed";
}

void BrushListWidget::setBrushes(const QList<RME::core::Brush*>& brushes)
{
    if (m_brushes != brushes) {
        m_brushes = brushes;
        populateList();
        
        qDebug() << "BrushListWidget: Set" << brushes.size() << "brushes";
    }
}

QList<RME::core::Brush*> BrushListWidget::getBrushes() const
{
    return m_brushes;
}

RME::core::Brush* BrushListWidget::getSelectedBrush() const
{
    return m_selectedBrush;
}

void BrushListWidget::setSelectedBrush(RME::core::Brush* brush)
{
    if (m_selectedBrush != brush) {
        m_selectedBrush = brush;
        
        // Find and select the corresponding item
        for (int i = 0; i < count(); ++i) {
            QListWidgetItem* item = this->item(i);
            if (item && item->data(Qt::UserRole).value<RME::core::Brush*>() == brush) {
                setCurrentItem(item);
                break;
            }
        }
        
        if (!brush) {
            clearSelection();
        }
    }
}

void BrushListWidget::onItemSelectionChanged()
{
    QListWidgetItem* currentItem = this->currentItem();
    RME::core::Brush* brush = nullptr;
    
    if (currentItem) {
        brush = currentItem->data(Qt::UserRole).value<RME::core::Brush*>();
    }
    
    if (m_selectedBrush != brush) {
        m_selectedBrush = brush;
        emit brushSelected(brush);
    }
}

void BrushListWidget::onItemActivated(QListWidgetItem* item)
{
    if (item) {
        RME::core::Brush* brush = item->data(Qt::UserRole).value<RME::core::Brush*>();
        if (brush) {
            m_selectedBrush = brush;
            emit brushActivated(brush);
        }
    }
}

void BrushListWidget::setupUI()
{
    // Configure list widget
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    setUniformItemSizes(true);
    
    // Set item size
    setIconSize(QSize(24, 24));
    
    // Enable drag and drop (for future use)
    setDragDropMode(QAbstractItemView::NoDragDrop);
    
    // Configure appearance
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Set item spacing
    setSpacing(2);
    
    // Enable tooltips
    setMouseTracking(true);
    
    // Set minimum size
    setMinimumWidth(200);
    
    // Configure selection behavior
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFocusPolicy(Qt::StrongFocus);
}

void BrushListWidget::setupConnections()
{
    connect(this, &QListWidget::itemSelectionChanged, 
            this, &BrushListWidget::onItemSelectionChanged);
    connect(this, &QListWidget::itemActivated, 
            this, &BrushListWidget::onItemActivated);
}

void BrushListWidget::populateList()
{
    // Clear existing items
    clear();
    
    // Add items for each brush
    for (RME::core::Brush* brush : m_brushes) {
        if (brush) {
            QListWidgetItem* item = createBrushItem(brush);
            addItem(item);
        }
    }
    
    // Restore selection if possible
    if (m_selectedBrush) {
        setSelectedBrush(m_selectedBrush);
    }
}

QListWidgetItem* BrushListWidget::createBrushItem(RME::core::Brush* brush)
{
    if (!brush) {
        return nullptr;
    }
    
    QListWidgetItem* item = new QListWidgetItem();
    
    // Set brush data
    item->setData(Qt::UserRole, QVariant::fromValue(brush));
    
    // Set display text with type prefix
    QString displayText = brush->getName();
    if (displayText.isEmpty()) {
        displayText = QString("Unnamed %1").arg(brush->getType());
    }
    
    // Add type indicator for clarity
    QString typePrefix = getBrushTypePrefix(brush->getType());
    if (!typePrefix.isEmpty()) {
        displayText = QString("[%1] %2").arg(typePrefix, displayText);
    }
    
    item->setText(displayText);
    
    // Set rich tooltip with brush information
    QString tooltip = createBrushTooltip(brush);
    item->setToolTip(tooltip);
    
    // Set icon based on brush type
    QIcon icon = createBrushIcon(brush);
    item->setIcon(icon);
    
    // Set item height for consistent appearance
    item->setSizeHint(QSize(-1, 32));
    
    // Set text alignment
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    return item;
}

QString BrushListWidget::getBrushTypePrefix(const QString& brushType) const
{
    static const QMap<QString, QString> typePrefixes = {
        {"GroundBrush", "GND"},
        {"WallBrush", "WAL"},
        {"CarpetBrush", "CAR"},
        {"TableBrush", "TBL"},
        {"DoodadBrush", "DOD"},
        {"RawBrush", "RAW"},
        {"CreatureBrush", "CRE"},
        {"SpawnBrush", "SPN"},
        {"WaypointBrush", "WPT"},
        {"HouseBrush", "HSE"},
        {"HouseExitBrush", "EXT"},
        {"EraserBrush", "ERS"}
    };
    
    return typePrefixes.value(brushType, "BRS");
}

QString BrushListWidget::createBrushTooltip(RME::core::Brush* brush) const
{
    if (!brush) {
        return "No brush information available";
    }
    
    QString tooltip = QString("<b>%1</b><br/>").arg(brush->getName().isEmpty() ? "Unnamed Brush" : brush->getName());
    tooltip += QString("<i>Type:</i> %1<br/>").arg(brush->getType());
    
    // Add type-specific information
    QString typeInfo = getBrushTypeDescription(brush->getType());
    if (!typeInfo.isEmpty()) {
        tooltip += QString("<i>Description:</i> %1<br/>").arg(typeInfo);
    }
    
    // Add usage hint
    tooltip += "<br/><i>Click to select, double-click to activate</i>";
    
    return tooltip;
}

QString BrushListWidget::getBrushTypeDescription(const QString& brushType) const
{
    static const QMap<QString, QString> typeDescriptions = {
        {"GroundBrush", "Paint ground tiles and terrain"},
        {"WallBrush", "Paint walls and barriers"},
        {"CarpetBrush", "Paint carpet and floor decorations"},
        {"TableBrush", "Paint table and furniture items"},
        {"DoodadBrush", "Paint decorative objects and items"},
        {"RawBrush", "Paint individual items directly"},
        {"CreatureBrush", "Place creatures on the map"},
        {"SpawnBrush", "Create creature spawn points"},
        {"WaypointBrush", "Create navigation waypoints"},
        {"HouseBrush", "Define house areas"},
        {"HouseExitBrush", "Create house entrance/exit points"},
        {"EraserBrush", "Remove items and objects"}
    };
    
    return typeDescriptions.value(brushType, "Custom brush tool");
}

QIcon BrushListWidget::createBrushIcon(RME::core::Brush* brush) const
{
    if (!brush) {
        return QIcon();
    }
    
    // Create a simple colored icon based on brush type
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Get color based on brush type
    QColor brushColor = getBrushTypeColor(brush->getType());
    
    // Draw icon shape based on type
    painter.setBrush(brushColor);
    painter.setPen(QPen(brushColor.darker(150), 1));
    
    if (brush->getType().contains("Ground") || brush->getType().contains("Carpet")) {
        // Square for ground/terrain brushes
        painter.drawRect(2, 2, 12, 12);
    } else if (brush->getType().contains("Wall")) {
        // Vertical rectangle for walls
        painter.drawRect(6, 1, 4, 14);
    } else if (brush->getType().contains("Creature") || brush->getType().contains("Spawn")) {
        // Circle for creatures/spawns
        painter.drawEllipse(2, 2, 12, 12);
    } else if (brush->getType().contains("House")) {
        // House shape
        painter.drawRect(3, 6, 10, 8);
        painter.drawPolygon(QPolygon() << QPoint(3, 6) << QPoint(8, 2) << QPoint(13, 6));
    } else {
        // Default diamond shape
        painter.drawPolygon(QPolygon() << QPoint(8, 2) << QPoint(14, 8) << QPoint(8, 14) << QPoint(2, 8));
    }
    
    return QIcon(pixmap);
}

QColor BrushListWidget::getBrushTypeColor(const QString& brushType) const
{
    static const QMap<QString, QColor> typeColors = {
        {"GroundBrush", QColor(76, 175, 80)},     // Green
        {"WallBrush", QColor(158, 158, 158)},     // Gray
        {"CarpetBrush", QColor(121, 85, 72)},     // Brown
        {"TableBrush", QColor(255, 152, 0)},      // Orange
        {"DoodadBrush", QColor(156, 39, 176)},    // Purple
        {"RawBrush", QColor(96, 125, 139)},       // Blue Gray
        {"CreatureBrush", QColor(33, 150, 243)},  // Blue
        {"SpawnBrush", QColor(255, 193, 7)},      // Amber
        {"WaypointBrush", QColor(244, 67, 54)},   // Red
        {"HouseBrush", QColor(255, 87, 34)},      // Deep Orange
        {"HouseExitBrush", QColor(139, 69, 19)},  // Saddle Brown
        {"EraserBrush", QColor(224, 224, 224)}    // Light Gray
    };
    
    return typeColors.value(brushType, QColor(158, 158, 158));
}

} // namespace palettes
} // namespace ui
} // namespace RME