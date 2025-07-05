#include "BrushGridWidget.h"
#include "BrushIconWidget.h"
#include "core/brush/Brush.h"
#include <QDebug>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QTimer>

namespace RME {
namespace ui {
namespace palettes {

BrushGridWidget::BrushGridWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("BrushGridWidget");
    setupUI();
    setupConnections();
    
    qDebug() << "BrushGridWidget: Created";
}

BrushGridWidget::~BrushGridWidget()
{
    clearGrid();
    qDebug() << "BrushGridWidget: Destroyed";
}

void BrushGridWidget::setBrushes(const QList<RME::core::Brush*>& brushes)
{
    if (m_brushes != brushes) {
        m_brushes = brushes;
        populateGrid();
        
        qDebug() << "BrushGridWidget: Set" << brushes.size() << "brushes";
    }
}

QList<RME::core::Brush*> BrushGridWidget::getBrushes() const
{
    return m_brushes;
}

RME::core::Brush* BrushGridWidget::getSelectedBrush() const
{
    return m_selectedBrush;
}

void BrushGridWidget::setSelectedBrush(RME::core::Brush* brush)
{
    if (m_selectedBrush != brush) {
        // Deselect previous brush icon
        if (m_selectedBrush) {
            for (BrushIconWidget* icon : m_brushIcons) {
                if (icon && icon->getBrush() == m_selectedBrush) {
                    icon->setSelected(false);
                    break;
                }
            }
        }
        
        m_selectedBrush = brush;
        
        // Select new brush icon
        if (m_selectedBrush) {
            for (BrushIconWidget* icon : m_brushIcons) {
                if (icon && icon->getBrush() == m_selectedBrush) {
                    icon->setSelected(true);
                    break;
                }
            }
        }
    }
}

void BrushGridWidget::setIconSize(const QSize& size)
{
    if (m_iconSize != size) {
        m_iconSize = size;
        
        // Update all existing icons
        for (BrushIconWidget* icon : m_brushIcons) {
            if (icon) {
                icon->setIconSize(size);
            }
        }
        
        updateGridLayout();
    }
}

QSize BrushGridWidget::getIconSize() const
{
    return m_iconSize;
}

void BrushGridWidget::setColumnsPerRow(int columns)
{
    if (m_columnsPerRow != columns && columns > 0) {
        m_columnsPerRow = columns;
        updateGridLayout();
    }
}

int BrushGridWidget::getColumnsPerRow() const
{
    return m_columnsPerRow;
}

void BrushGridWidget::onBrushIconSelected(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
    emit brushSelected(brush);
}

void BrushGridWidget::onBrushIconActivated(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
    emit brushActivated(brush);
}

void BrushGridWidget::setupUI()
{
    // Create grid layout
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setContentsMargins(4, 4, 4, 4);
    m_gridLayout->setSpacing(4);
    
    // Configure layout
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // Set minimum size
    setMinimumSize(200, 200);
    
    // Enable mouse tracking for hover effects
    setMouseTracking(true);
    
    // Set focus policy
    setFocusPolicy(Qt::StrongFocus);
    
    // Configure size policy
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BrushGridWidget::setupConnections()
{
    // Connections will be set up for individual brush icons
}

void BrushGridWidget::populateGrid()
{
    // Clear existing grid
    clearGrid();
    
    // Create brush icons
    for (RME::core::Brush* brush : m_brushes) {
        if (brush) {
            BrushIconWidget* icon = createBrushIcon(brush);
            if (icon) {
                m_brushIcons.append(icon);
            }
        }
    }
    
    // Update grid layout
    updateGridLayout();
    
    // Restore selection if possible
    if (m_selectedBrush) {
        setSelectedBrush(m_selectedBrush);
    }
}

void BrushGridWidget::clearGrid()
{
    // Remove all brush icons from layout and delete them
    for (BrushIconWidget* icon : m_brushIcons) {
        if (icon) {
            m_gridLayout->removeWidget(icon);
            icon->deleteLater();
        }
    }
    m_brushIcons.clear();
}

BrushIconWidget* BrushGridWidget::createBrushIcon(RME::core::Brush* brush)
{
    if (!brush) {
        return nullptr;
    }
    
    BrushIconWidget* icon = new BrushIconWidget(brush, this);
    icon->setIconSize(m_iconSize);
    
    // Connect signals
    connect(icon, &BrushIconWidget::selected, 
            this, &BrushGridWidget::onBrushIconSelected);
    connect(icon, &BrushIconWidget::activated, 
            this, &BrushGridWidget::onBrushIconActivated);
    
    return icon;
}

void BrushGridWidget::updateGridLayout()
{
    if (!m_gridLayout) {
        return;
    }
    
    // Remove all widgets from layout (but don't delete them)
    for (BrushIconWidget* icon : m_brushIcons) {
        if (icon) {
            m_gridLayout->removeWidget(icon);
        }
    }
    
    // Calculate optimal columns based on widget width and icon size
    int availableWidth = width() - m_gridLayout->contentsMargins().left() - m_gridLayout->contentsMargins().right();
    int iconWidth = m_iconSize.width() + 16; // Add padding
    int optimalColumns = qMax(1, availableWidth / iconWidth);
    
    // Use the smaller of optimal or configured columns
    int actualColumns = qMin(m_columnsPerRow, optimalColumns);
    
    // Add widgets back in grid formation
    int row = 0;
    int col = 0;
    
    for (BrushIconWidget* icon : m_brushIcons) {
        if (icon) {
            m_gridLayout->addWidget(icon, row, col);
            
            col++;
            if (col >= actualColumns) {
                col = 0;
                row++;
            }
        }
    }
    
    // Add stretch to fill remaining space
    if (row > 0 || col > 0) {
        m_gridLayout->setRowStretch(row + 1, 1);
        m_gridLayout->setColumnStretch(actualColumns, 1);
    }
    
    // Update widget size hint
    int totalRows = (m_brushIcons.size() + actualColumns - 1) / actualColumns;
    int preferredHeight = totalRows * (m_iconSize.height() + 32) + 20; // Add padding
    setMinimumHeight(qMin(preferredHeight, 400)); // Cap at reasonable height
}

void BrushGridWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Left:
            selectPreviousBrush();
            event->accept();
            break;
        case Qt::Key_Right:
            selectNextBrush();
            event->accept();
            break;
        case Qt::Key_Up:
            selectBrushAt(-1, 0); // Move up one row
            event->accept();
            break;
        case Qt::Key_Down:
            selectBrushAt(1, 0); // Move down one row
            event->accept();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (m_selectedBrush) {
                emit brushActivated(m_selectedBrush);
            }
            event->accept();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void BrushGridWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // Update grid layout when widget is resized
    QTimer::singleShot(0, this, &BrushGridWidget::updateGridLayout);
}

void BrushGridWidget::contextMenuEvent(QContextMenuEvent* event)
{
    // Find brush at cursor position
    RME::core::Brush* brushAtCursor = nullptr;
    for (BrushIconWidget* icon : m_brushIcons) {
        if (icon && icon->geometry().contains(event->pos())) {
            brushAtCursor = icon->getBrush();
            break;
        }
    }
    
    if (brushAtCursor) {
        // TODO: Create context menu for brush actions
        // For now, just select the brush
        setSelectedBrush(brushAtCursor);
    }
    
    QWidget::contextMenuEvent(event);
}

void BrushGridWidget::selectNextBrush()
{
    if (m_brushIcons.isEmpty()) {
        return;
    }
    
    int currentIndex = getSelectedBrushIndex();
    int nextIndex = (currentIndex + 1) % m_brushIcons.size();
    
    if (nextIndex < m_brushIcons.size() && m_brushIcons[nextIndex]) {
        setSelectedBrush(m_brushIcons[nextIndex]->getBrush());
    }
}

void BrushGridWidget::selectPreviousBrush()
{
    if (m_brushIcons.isEmpty()) {
        return;
    }
    
    int currentIndex = getSelectedBrushIndex();
    int prevIndex = (currentIndex - 1 + m_brushIcons.size()) % m_brushIcons.size();
    
    if (prevIndex < m_brushIcons.size() && m_brushIcons[prevIndex]) {
        setSelectedBrush(m_brushIcons[prevIndex]->getBrush());
    }
}

void BrushGridWidget::selectBrushAt(int rowDelta, int colDelta)
{
    if (m_brushIcons.isEmpty()) {
        return;
    }
    
    int currentIndex = getSelectedBrushIndex();
    if (currentIndex < 0) {
        return;
    }
    
    // Calculate current row and column
    int currentRow = currentIndex / m_columnsPerRow;
    int currentCol = currentIndex % m_columnsPerRow;
    
    // Calculate new position
    int newRow = currentRow + rowDelta;
    int newCol = currentCol + colDelta;
    
    // Clamp to valid range
    int totalRows = (m_brushIcons.size() + m_columnsPerRow - 1) / m_columnsPerRow;
    newRow = qMax(0, qMin(newRow, totalRows - 1));
    newCol = qMax(0, qMin(newCol, m_columnsPerRow - 1));
    
    // Calculate new index
    int newIndex = newRow * m_columnsPerRow + newCol;
    
    // Make sure index is valid
    if (newIndex < m_brushIcons.size() && m_brushIcons[newIndex]) {
        setSelectedBrush(m_brushIcons[newIndex]->getBrush());
    }
}

int BrushGridWidget::getSelectedBrushIndex() const
{
    if (!m_selectedBrush) {
        return -1;
    }
    
    for (int i = 0; i < m_brushIcons.size(); ++i) {
        if (m_brushIcons[i] && m_brushIcons[i]->getBrush() == m_selectedBrush) {
            return i;
        }
    }
    
    return -1;
}

} // namespace palettes
} // namespace ui
} // namespace RME