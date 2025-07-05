#include "BrushIconWidget.h"
#include "core/brush/Brush.h"
#include <QPainter>
#include <QDebug>

namespace RME {
namespace ui {
namespace palettes {

// Static style definitions
const QString BrushIconWidget::SELECTED_STYLE = 
    "BrushIconWidget { background-color: #3daee9; border: 2px solid #2980b9; border-radius: 4px; }";
const QString BrushIconWidget::HOVERED_STYLE = 
    "BrushIconWidget { background-color: #e3f2fd; border: 1px solid #90caf9; border-radius: 4px; }";
const QString BrushIconWidget::NORMAL_STYLE = 
    "BrushIconWidget { background-color: transparent; border: 1px solid transparent; border-radius: 4px; }";

BrushIconWidget::BrushIconWidget(RME::core::Brush* brush, QWidget* parent)
    : QWidget(parent)
    , m_brush(brush)
{
    setObjectName("BrushIconWidget");
    setupUI();
    updateIcon();
    updateLabel();
    updateSelectionState();
    
    // Enable mouse tracking for hover effects
    setMouseTracking(true);
    
    qDebug() << "BrushIconWidget: Created for brush" << (brush ? brush->getName() : "null");
}

BrushIconWidget::~BrushIconWidget()
{
    qDebug() << "BrushIconWidget: Destroyed";
}

void BrushIconWidget::setBrush(RME::core::Brush* brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        updateIcon();
        updateLabel();
        
        qDebug() << "BrushIconWidget: Brush changed to" << (brush ? brush->getName() : "null");
    }
}

void BrushIconWidget::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        updateSelectionState();
        
        if (selected) {
            emit this->selected(m_brush);
        }
    }
}

void BrushIconWidget::setIconSize(const QSize& size)
{
    if (m_iconSize != size) {
        m_iconSize = size;
        
        if (m_iconLabel) {
            m_iconLabel->setFixedSize(size);
        }
        
        updateIcon();
        adjustSize();
    }
}

QSize BrushIconWidget::getIconSize() const
{
    return m_iconSize;
}

void BrushIconWidget::setShowLabel(bool show)
{
    if (m_showLabel != show) {
        m_showLabel = show;
        
        if (m_textLabel) {
            m_textLabel->setVisible(show);
        }
        
        adjustSize();
    }
}

bool BrushIconWidget::getShowLabel() const
{
    return m_showLabel;
}

void BrushIconWidget::setupUI()
{
    // Create layout
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(2);
    m_layout->setAlignment(Qt::AlignCenter);
    
    // Create icon label
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(m_iconSize);
    m_iconLabel->setStyleSheet("QLabel { border: none; background-color: #f0f0f0; }");
    m_layout->addWidget(m_iconLabel);
    
    // Create text label
    m_textLabel = new QLabel(this);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setWordWrap(true);
    m_textLabel->setMaximumWidth(m_iconSize.width() + 16);
    m_textLabel->setStyleSheet("QLabel { font-size: 10px; color: #333; }");
    m_layout->addWidget(m_textLabel);
    
    // Set initial size
    setFixedSize(m_iconSize.width() + 16, m_iconSize.height() + 32);
}

void BrushIconWidget::updateIcon()
{
    if (!m_iconLabel) {
        return;
    }
    
    QPixmap icon(m_iconSize);
    icon.fill(Qt::transparent);
    
    if (m_brush) {
        QPainter painter(&icon);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Get brush type and color
        QString brushType = m_brush->getType();
        QColor brushColor = getBrushTypeColor(brushType);
        
        // Create gradient for more appealing look
        QLinearGradient gradient(0, 0, m_iconSize.width(), m_iconSize.height());
        gradient.setColorAt(0, brushColor.lighter(120));
        gradient.setColorAt(1, brushColor.darker(120));
        
        painter.setBrush(QBrush(gradient));
        painter.setPen(QPen(brushColor.darker(150), 2));
        
        // Draw shape based on brush type
        drawBrushTypeShape(painter, brushType);
        
        // Draw type indicator
        drawTypeIndicator(painter, brushType);
    } else {
        // Draw placeholder for no brush
        QPainter painter(&icon);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(240, 240, 240));
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawRect(icon.rect().adjusted(2, 2, -2, -2));
        
        painter.setPen(QColor(150, 150, 150));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(icon.rect(), Qt::AlignCenter, "?");
    }
    
    m_iconLabel->setPixmap(icon);
}

void BrushIconWidget::drawBrushTypeShape(QPainter& painter, const QString& brushType)
{
    QRect iconRect = QRect(0, 0, m_iconSize.width(), m_iconSize.height());
    QRect drawRect = iconRect.adjusted(3, 3, -3, -3);
    
    if (brushType.contains("Ground") || brushType.contains("Carpet")) {
        // Square pattern for ground/terrain
        painter.drawRect(drawRect);
        
        // Add texture lines
        painter.setPen(QPen(painter.pen().color().darker(130), 1));
        for (int i = 0; i < 3; ++i) {
            int y = drawRect.top() + (drawRect.height() * (i + 1)) / 4;
            painter.drawLine(drawRect.left() + 2, y, drawRect.right() - 2, y);
        }
    } else if (brushType.contains("Wall")) {
        // Brick pattern for walls
        painter.drawRect(drawRect);
        
        // Draw brick lines
        painter.setPen(QPen(painter.pen().color().darker(130), 1));
        int brickHeight = drawRect.height() / 3;
        for (int i = 1; i < 3; ++i) {
            int y = drawRect.top() + i * brickHeight;
            painter.drawLine(drawRect.left(), y, drawRect.right(), y);
        }
        // Vertical lines offset for brick pattern
        for (int i = 0; i < 3; ++i) {
            int y = drawRect.top() + i * brickHeight;
            int offset = (i % 2) * (drawRect.width() / 4);
            painter.drawLine(drawRect.left() + drawRect.width() / 2 + offset, y, 
                           drawRect.left() + drawRect.width() / 2 + offset, y + brickHeight);
        }
    } else if (brushType.contains("Creature") || brushType.contains("Spawn")) {
        // Circle for creatures/spawns
        painter.drawEllipse(drawRect);
        
        // Add inner circle
        QRect innerRect = drawRect.adjusted(drawRect.width() / 4, drawRect.height() / 4, 
                                          -drawRect.width() / 4, -drawRect.height() / 4);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(innerRect);
    } else if (brushType.contains("House")) {
        // House shape
        QRect baseRect = QRect(drawRect.left(), drawRect.top() + drawRect.height() / 3, 
                              drawRect.width(), drawRect.height() * 2 / 3);
        painter.drawRect(baseRect);
        
        // Roof
        QPolygon roof;
        roof << QPoint(drawRect.left(), drawRect.top() + drawRect.height() / 3)
             << QPoint(drawRect.center().x(), drawRect.top())
             << QPoint(drawRect.right(), drawRect.top() + drawRect.height() / 3);
        painter.drawPolygon(roof);
    } else if (brushType.contains("Table")) {
        // Table shape (rectangle with legs)
        QRect topRect = QRect(drawRect.left(), drawRect.top(), 
                             drawRect.width(), drawRect.height() / 3);
        painter.drawRect(topRect);
        
        // Table legs
        int legWidth = 2;
        painter.drawRect(drawRect.left() + 2, topRect.bottom(), legWidth, drawRect.height() / 3);
        painter.drawRect(drawRect.right() - 2 - legWidth, topRect.bottom(), legWidth, drawRect.height() / 3);
    } else if (brushType.contains("Eraser")) {
        // X pattern for eraser
        painter.setPen(QPen(painter.pen().color(), 3));
        painter.drawLine(drawRect.topLeft(), drawRect.bottomRight());
        painter.drawLine(drawRect.topRight(), drawRect.bottomLeft());
    } else {
        // Default diamond shape
        QPolygon diamond;
        diamond << QPoint(drawRect.center().x(), drawRect.top())
                << QPoint(drawRect.right(), drawRect.center().y())
                << QPoint(drawRect.center().x(), drawRect.bottom())
                << QPoint(drawRect.left(), drawRect.center().y());
        painter.drawPolygon(diamond);
    }
}

void BrushIconWidget::drawTypeIndicator(QPainter& painter, const QString& brushType)
{
    // Draw small type indicator in corner
    QString indicator = getTypeIndicator(brushType);
    if (indicator.isEmpty()) {
        return;
    }
    
    QRect indicatorRect(m_iconSize.width() - 12, m_iconSize.height() - 12, 10, 10);
    
    painter.setBrush(QColor(255, 255, 255, 200));
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    painter.drawEllipse(indicatorRect);
    
    painter.setPen(QColor(50, 50, 50));
    painter.setFont(QFont("Arial", 6, QFont::Bold));
    painter.drawText(indicatorRect, Qt::AlignCenter, indicator);
}

QString BrushIconWidget::getTypeIndicator(const QString& brushType) const
{
    static const QMap<QString, QString> indicators = {
        {"GroundBrush", "G"},
        {"WallBrush", "W"},
        {"CarpetBrush", "C"},
        {"TableBrush", "T"},
        {"DoodadBrush", "D"},
        {"RawBrush", "R"},
        {"CreatureBrush", "CR"},
        {"SpawnBrush", "S"},
        {"WaypointBrush", "WP"},
        {"HouseBrush", "H"},
        {"HouseExitBrush", "E"},
        {"EraserBrush", "X"}
    };
    
    return indicators.value(brushType, "?");
}

QColor BrushIconWidget::getBrushTypeColor(const QString& brushType) const
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

void BrushIconWidget::updateLabel()
{
    if (!m_textLabel) {
        return;
    }
    
    QString labelText;
    if (m_brush) {
        labelText = m_brush->getName();
        if (labelText.isEmpty()) {
            labelText = m_brush->getType();
        }
    } else {
        labelText = "No Brush";
    }
    
    m_textLabel->setText(labelText);
    m_textLabel->setVisible(m_showLabel);
}

void BrushIconWidget::updateSelectionState()
{
    if (m_selected) {
        setStyleSheet(SELECTED_STYLE);
    } else if (m_hovered) {
        setStyleSheet(HOVERED_STYLE);
    } else {
        setStyleSheet(NORMAL_STYLE);
    }
    
    update();
}

void BrushIconWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
    }
    
    QWidget::mousePressEvent(event);
}

void BrushIconWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
        emit activated(m_brush);
    }
    
    QWidget::mouseDoubleClickEvent(event);
}

void BrushIconWidget::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    updateSelectionState();
    
    QWidget::enterEvent(event);
}

void BrushIconWidget::leaveEvent(QEvent* event)
{
    m_hovered = false;
    updateSelectionState();
    
    QWidget::leaveEvent(event);
}

void BrushIconWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    // Additional custom painting can be done here if needed
}

} // namespace palettes
} // namespace ui
} // namespace RME