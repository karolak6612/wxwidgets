#include "BrushTooltipWidget.h"
#include "BrushPreviewGenerator.h"
#include "core/brush/Brush.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QDebug>

namespace RME {
namespace ui {
namespace palettes {

BrushTooltipWidget::BrushTooltipWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("BrushTooltipWidget");
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Create timers
    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    m_showTimer->setInterval(m_showDelay);
    
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(m_hideDelay);
    
    setupUI();
    
    // Connect timers
    connect(m_showTimer, &QTimer::timeout, this, &BrushTooltipWidget::onShowTimer);
    connect(m_hideTimer, &QTimer::timeout, this, &BrushTooltipWidget::onHideTimer);
    
    // Initially hidden
    hide();
    
    qDebug() << "BrushTooltipWidget: Created";
}

BrushTooltipWidget::~BrushTooltipWidget()
{
    qDebug() << "BrushTooltipWidget: Destroyed";
}

void BrushTooltipWidget::setBrush(RME::core::Brush* brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        updateContent();
    }
}

void BrushTooltipWidget::setPreviewGenerator(BrushPreviewGenerator* generator)
{
    m_previewGenerator = generator;
    updatePreview();
}

void BrushTooltipWidget::showTooltip(const QPoint& position)
{
    m_pendingPosition = position;
    
    // Cancel hide timer if running
    m_hideTimer->stop();
    
    if (!m_tooltipVisible) {
        // Start show timer
        m_showTimer->start();
    } else {
        // Already visible, just reposition
        positionTooltip(position);
    }
}

void BrushTooltipWidget::hideTooltip()
{
    // Cancel show timer if running
    m_showTimer->stop();
    
    if (m_tooltipVisible) {
        // Start hide timer for delayed hiding
        m_hideTimer->start();
    }
}

bool BrushTooltipWidget::isTooltipVisible() const
{
    return m_tooltipVisible;
}

void BrushTooltipWidget::setShowDelay(int milliseconds)
{
    m_showDelay = milliseconds;
    m_showTimer->setInterval(milliseconds);
}

int BrushTooltipWidget::getShowDelay() const
{
    return m_showDelay;
}

void BrushTooltipWidget::setHideDelay(int milliseconds)
{
    m_hideDelay = milliseconds;
    m_hideTimer->setInterval(milliseconds);
}

int BrushTooltipWidget::getHideDelay() const
{
    return m_hideDelay;
}

void BrushTooltipWidget::onShowTimer()
{
    if (!m_tooltipVisible && m_brush) {
        updateContent();
        positionTooltip(m_pendingPosition);
        show();
        m_tooltipVisible = true;
        emit tooltipShown();
    }
}

void BrushTooltipWidget::onHideTimer()
{
    if (m_tooltipVisible) {
        hide();
        m_tooltipVisible = false;
        emit tooltipHidden();
    }
}

void BrushTooltipWidget::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(6);
    
    // Create header layout (preview + basic info)
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setSpacing(8);
    
    // Preview label
    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedSize(m_previewSize);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("QLabel { border: 1px solid #ccc; background-color: white; }");
    m_headerLayout->addWidget(m_previewLabel);
    
    // Info layout
    m_infoLayout = new QVBoxLayout();
    m_infoLayout->setSpacing(2);
    
    // Name label
    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 12px; color: #333; }");
    m_nameLabel->setWordWrap(true);
    m_infoLayout->addWidget(m_nameLabel);
    
    // Type label
    m_typeLabel = new QLabel(this);
    m_typeLabel->setStyleSheet("QLabel { font-style: italic; font-size: 10px; color: #666; }");
    m_infoLayout->addWidget(m_typeLabel);
    
    // Add info layout to header
    m_headerLayout->addLayout(m_infoLayout);
    m_headerLayout->addStretch();
    
    // Add header to main layout
    m_mainLayout->addLayout(m_headerLayout);
    
    // Description label
    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setStyleSheet("QLabel { font-size: 10px; color: #555; }");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumWidth(250);
    m_mainLayout->addWidget(m_descriptionLabel);
    
    // Usage label
    m_usageLabel = new QLabel(this);
    m_usageLabel->setStyleSheet("QLabel { font-size: 9px; color: #777; font-style: italic; }");
    m_usageLabel->setWordWrap(true);
    m_usageLabel->setMaximumWidth(250);
    m_mainLayout->addWidget(m_usageLabel);
    
    // Set fixed width for consistent appearance
    setFixedWidth(280);
}

void BrushTooltipWidget::updateContent()
{
    if (!m_brush) {
        m_nameLabel->setText("No Brush");
        m_typeLabel->setText("");
        m_descriptionLabel->setText("");
        m_usageLabel->setText("");
        return;
    }
    
    updatePreview();
    updateBrushInfo();
    updateUsageInfo();
    
    // Adjust size to content
    adjustSize();
}

void BrushTooltipWidget::updatePreview()
{
    if (!m_previewLabel) {
        return;
    }
    
    if (m_brush && m_previewGenerator) {
        QPixmap preview = m_previewGenerator->generatePreview(
            m_brush, m_previewSize, BrushPreviewGenerator::ThumbnailStyle);
        m_previewLabel->setPixmap(preview);
    } else {
        // Create placeholder preview
        QPixmap placeholder(m_previewSize);
        placeholder.fill(QColor(240, 240, 240));
        
        QPainter painter(&placeholder);
        painter.setPen(QColor(180, 180, 180));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "No Preview");
        
        m_previewLabel->setPixmap(placeholder);
    }
}

void BrushTooltipWidget::updateBrushInfo()
{
    if (!m_brush) {
        return;
    }
    
    // Update name
    QString name = m_brush->getName();
    if (name.isEmpty()) {
        name = QString("Unnamed %1").arg(m_brush->getType());
    }
    m_nameLabel->setText(name);
    
    // Update type
    m_typeLabel->setText(QString("Type: %1").arg(m_brush->getType()));
    
    // Update description based on brush type
    QString description = getBrushDescription(m_brush->getType());
    m_descriptionLabel->setText(description);
}

void BrushTooltipWidget::updateUsageInfo()
{
    if (!m_brush) {
        return;
    }
    
    QString usage = getBrushUsageInfo(m_brush->getType());
    m_usageLabel->setText(usage);
}

QString BrushTooltipWidget::getBrushDescription(const QString& brushType) const
{
    static const QMap<QString, QString> descriptions = {
        {"GroundBrush", "Paints ground tiles and terrain. Creates the base layer for your map areas."},
        {"WallBrush", "Paints walls and barriers. Use to create boundaries and structures."},
        {"CarpetBrush", "Paints carpet and floor decorations. Adds detail to indoor areas."},
        {"TableBrush", "Paints table and furniture items. Creates functional furniture layouts."},
        {"DoodadBrush", "Paints decorative objects and items. Adds atmosphere and detail to your map."},
        {"RawBrush", "Paints individual items directly. Provides precise control over item placement."},
        {"CreatureBrush", "Places creatures on the map. Use to populate your world with NPCs and monsters."},
        {"SpawnBrush", "Creates creature spawn points. Defines where creatures will appear."},
        {"WaypointBrush", "Creates navigation waypoints. Helps with pathfinding and navigation."},
        {"HouseBrush", "Defines house areas. Marks regions as player housing zones."},
        {"HouseExitBrush", "Creates house entrance/exit points. Defines access points for houses."},
        {"EraserBrush", "Removes items and objects. Use to clean up or modify existing content."}
    };
    
    return descriptions.value(brushType, "Custom brush tool for map editing.");
}

QString BrushTooltipWidget::getBrushUsageInfo(const QString& brushType) const
{
    return "Left-click to select, double-click to activate. Hold and drag to paint continuously.";
}

void BrushTooltipWidget::positionTooltip(const QPoint& position)
{
    if (!parentWidget()) {
        return;
    }
    
    // Convert position to global coordinates
    QPoint globalPos = parentWidget()->mapToGlobal(position);
    
    // Get screen geometry
    QScreen* screen = QApplication::screenAt(globalPos);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    QRect screenGeometry = screen->availableGeometry();
    
    // Calculate tooltip position with offset
    QPoint tooltipPos = globalPos + QPoint(15, 15);
    
    // Ensure tooltip stays within screen bounds
    if (tooltipPos.x() + width() > screenGeometry.right()) {
        tooltipPos.setX(globalPos.x() - width() - 15);
    }
    
    if (tooltipPos.y() + height() > screenGeometry.bottom()) {
        tooltipPos.setY(globalPos.y() - height() - 15);
    }
    
    // Clamp to screen bounds
    tooltipPos.setX(qMax(screenGeometry.left(), qMin(tooltipPos.x(), screenGeometry.right() - width())));
    tooltipPos.setY(qMax(screenGeometry.top(), qMin(tooltipPos.y(), screenGeometry.bottom() - height())));
    
    move(tooltipPos);
}

void BrushTooltipWidget::enterEvent(QEnterEvent* event)
{
    // Cancel hide timer when mouse enters tooltip
    m_hideTimer->stop();
    QWidget::enterEvent(event);
}

void BrushTooltipWidget::leaveEvent(QEvent* event)
{
    // Start hide timer when mouse leaves tooltip
    hideTooltip();
    QWidget::leaveEvent(event);
}

void BrushTooltipWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background with rounded corners and shadow effect
    QRect bgRect = rect().adjusted(2, 2, -2, -2);
    
    // Draw shadow
    painter.setBrush(QColor(0, 0, 0, 50));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(bgRect.adjusted(2, 2, 2, 2), 6, 6);
    
    // Draw background
    painter.setBrush(QColor(255, 255, 255, 240));
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRoundedRect(bgRect, 6, 6);
    
    QWidget::paintEvent(event);
}

} // namespace palettes
} // namespace ui
} // namespace RME