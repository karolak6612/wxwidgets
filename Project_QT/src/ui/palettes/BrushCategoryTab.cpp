#include "BrushCategoryTab.h"
#include "BrushListWidget.h"
#include "BrushGridWidget.h"
#include "core/brush/BrushManagerService.h"
#include "core/brush/Brush.h"
#include <QDebug>

namespace RME {
namespace ui {
namespace palettes {

// Static member definitions
const QMap<BrushCategoryTab::BrushCategory, QString> BrushCategoryTab::s_categoryNames = {
    {TerrainBrushes, "Terrain"},
    {ObjectBrushes, "Objects"},
    {EntityBrushes, "Entities"},
    {SpecialBrushes, "Special"},
    {AllBrushes, "All Brushes"},
    {RecentBrushes, "Recent"}
};

const QMap<BrushCategoryTab::BrushCategory, QStringList> BrushCategoryTab::s_categoryBrushTypes = {
    {TerrainBrushes, {"GroundBrush", "WallBrush", "CarpetBrush", "TableBrush"}},
    {ObjectBrushes, {"DoodadBrush", "RawBrush"}},
    {EntityBrushes, {"CreatureBrush", "SpawnBrush", "WaypointBrush"}},
    {SpecialBrushes, {"HouseBrush", "HouseExitBrush", "EraserBrush"}},
    {AllBrushes, {}}, // Empty means all types
    {RecentBrushes, {}} // Special handling for recent brushes
};

BrushCategoryTab::BrushCategoryTab(BrushCategory category, QWidget* parent)
    : QWidget(parent)
    , m_category(category)
{
    setObjectName(QString("BrushCategoryTab_%1").arg(getCategoryName()));
    
    setupUI();
    setupConnections();
    
    qDebug() << "BrushCategoryTab: Created for category" << getCategoryName();
}

BrushCategoryTab::~BrushCategoryTab()
{
    qDebug() << "BrushCategoryTab: Destroyed for category" << getCategoryName();
}

QString BrushCategoryTab::getCategoryName() const
{
    return s_categoryNames.value(m_category, "Unknown");
}

void BrushCategoryTab::setViewMode(ViewMode mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        updateViewWidget();
        emit viewModeChanged(mode);
        
        qDebug() << "BrushCategoryTab: View mode changed to" << static_cast<int>(mode) 
                 << "for category" << getCategoryName();
    }
}

void BrushCategoryTab::setBrushManagerService(RME::core::brush::BrushManagerService* service)
{
    if (m_brushManagerService != service) {
        m_brushManagerService = service;
        
        if (service) {
            qDebug() << "BrushCategoryTab: BrushManagerService set for category" << getCategoryName();
            refreshBrushes();
        }
    }
}

void BrushCategoryTab::refreshBrushes()
{
    if (!m_brushManagerService) {
        qWarning() << "BrushCategoryTab: Cannot refresh - no BrushManagerService";
        return;
    }
    
    qDebug() << "BrushCategoryTab: Refreshing brushes for category" << getCategoryName();
    
    // Clear existing brushes
    m_allBrushes.clear();
    m_filteredBrushes.clear();
    
    // Populate brushes based on category
    populateBrushes();
    
    // Apply current filter
    onFilterChanged();
    
    // Update view widget with new data
    updateViewWidget();
    updateEmptyState();
    
    emit brushCountChanged(getTotalBrushCount(), getVisibleBrushCount());
}

QList<RME::core::Brush*> BrushCategoryTab::getBrushes() const
{
    return m_allBrushes;
}

QList<RME::core::Brush*> BrushCategoryTab::getFilteredBrushes() const
{
    return m_filteredBrushes;
}

RME::core::Brush* BrushCategoryTab::getSelectedBrush() const
{
    return m_selectedBrush;
}

void BrushCategoryTab::setSelectedBrush(RME::core::Brush* brush)
{
    if (m_selectedBrush != brush) {
        m_selectedBrush = brush;
        
        // Update selection in current view widget
        if (m_currentViewWidget) {
            if (m_listWidget && m_currentViewWidget == m_listWidget) {
                m_listWidget->setSelectedBrush(brush);
            } else if (m_gridWidget && m_currentViewWidget == m_gridWidget) {
                m_gridWidget->setSelectedBrush(brush);
            }
        }
        
        emit brushSelected(brush);
    }
}

void BrushCategoryTab::setSearchFilter(const QString& filter)
{
    if (m_searchFilter != filter) {
        m_searchFilter = filter;
        onFilterChanged();
        emit filterChanged(filter);
        
        qDebug() << "BrushCategoryTab: Search filter set to" << filter 
                 << "for category" << getCategoryName();
    }
}

void BrushCategoryTab::clearFilter()
{
    setSearchFilter("");
}

int BrushCategoryTab::getTotalBrushCount() const
{
    return m_allBrushes.size();
}

int BrushCategoryTab::getVisibleBrushCount() const
{
    return m_filteredBrushes.size();
}

void BrushCategoryTab::onBrushSelected(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
}

void BrushCategoryTab::onBrushActivated(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
    emit brushActivated(brush);
}

void BrushCategoryTab::onViewModeChanged()
{
    updateViewWidget();
}

void BrushCategoryTab::onFilterChanged()
{
    // Filter brushes based on search text
    m_filteredBrushes.clear();
    
    for (RME::core::Brush* brush : m_allBrushes) {
        if (matchesFilter(brush)) {
            m_filteredBrushes.append(brush);
        }
    }
    
    // Update view widget with filtered data
    if (m_currentViewWidget) {
        if (m_listWidget && m_currentViewWidget == m_listWidget) {
            m_listWidget->setBrushes(m_filteredBrushes);
        } else if (m_gridWidget && m_currentViewWidget == m_gridWidget) {
            m_gridWidget->setBrushes(m_filteredBrushes);
        }
    }
    
    updateEmptyState();
    emit brushCountChanged(getTotalBrushCount(), getVisibleBrushCount());
}

void BrushCategoryTab::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget (will be replaced by view widgets)
    m_contentWidget = new QWidget();
    m_scrollArea->setWidget(m_contentWidget);
    
    // Create empty state label
    m_emptyLabel = new QLabel("No brushes found", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    m_emptyLabel->setVisible(false);
    
    // Add to main layout
    m_mainLayout->addWidget(m_scrollArea);
    m_mainLayout->addWidget(m_emptyLabel);
    
    // Create initial view widget
    updateViewWidget();
}

void BrushCategoryTab::setupConnections()
{
    // Connections will be set up when view widgets are created
}

void BrushCategoryTab::updateViewWidget()
{
    // Remove current view widget if it exists
    if (m_currentViewWidget) {
        m_scrollArea->takeWidget();
        m_currentViewWidget = nullptr;
    }
    
    // Create appropriate view widget based on view mode
    switch (m_viewMode) {
        case ListView:
        case SmallIconView: // Use list widget for small icons too
            if (!m_listWidget) {
                m_listWidget = new BrushListWidget(this);
                connect(m_listWidget, &BrushListWidget::brushSelected, 
                        this, &BrushCategoryTab::onBrushSelected);
                connect(m_listWidget, &BrushListWidget::brushActivated, 
                        this, &BrushCategoryTab::onBrushActivated);
            }
            m_currentViewWidget = m_listWidget;
            m_listWidget->setBrushes(m_filteredBrushes);
            m_listWidget->setSelectedBrush(m_selectedBrush);
            break;
            
        case GridView:
        case LargeIconView:
        default:
            if (!m_gridWidget) {
                m_gridWidget = new BrushGridWidget(this);
                connect(m_gridWidget, &BrushGridWidget::brushSelected, 
                        this, &BrushCategoryTab::onBrushSelected);
                connect(m_gridWidget, &BrushGridWidget::brushActivated, 
                        this, &BrushCategoryTab::onBrushActivated);
            }
            m_currentViewWidget = m_gridWidget;
            m_gridWidget->setBrushes(m_filteredBrushes);
            m_gridWidget->setSelectedBrush(m_selectedBrush);
            break;
    }
    
    // Set the view widget in scroll area
    if (m_currentViewWidget) {
        m_scrollArea->setWidget(m_currentViewWidget);
        m_currentViewWidget->show();
    }
}

void BrushCategoryTab::updateEmptyState()
{
    bool isEmpty = m_filteredBrushes.isEmpty();
    
    if (m_emptyLabel) {
        m_emptyLabel->setVisible(isEmpty);
        
        if (isEmpty) {
            if (!m_searchFilter.isEmpty()) {
                m_emptyLabel->setText(QString("No brushes match \"%1\"").arg(m_searchFilter));
            } else {
                m_emptyLabel->setText("No brushes available");
            }
        }
    }
    
    if (m_scrollArea) {
        m_scrollArea->setVisible(!isEmpty);
    }
}

void BrushCategoryTab::populateBrushes()
{
    if (!m_brushManagerService) {
        return;
    }
    
    // Get all available brushes from the service
    // Note: This is a placeholder - actual implementation depends on BrushManagerService API
    QList<RME::core::Brush*> allBrushes; // = m_brushManagerService->getAllBrushes();
    
    // For now, create some dummy brushes for testing
    // TODO: Replace with actual brush enumeration when BrushManagerService is complete
    
    // Filter brushes based on category
    for (RME::core::Brush* brush : allBrushes) {
        if (matchesCategory(brush)) {
            m_allBrushes.append(brush);
        }
    }
    
    qDebug() << "BrushCategoryTab: Populated" << m_allBrushes.size() 
             << "brushes for category" << getCategoryName();
}

bool BrushCategoryTab::matchesCategory(RME::core::Brush* brush) const
{
    if (!brush) {
        return false;
    }
    
    // Special handling for "All Brushes" category
    if (m_category == AllBrushes) {
        return true;
    }
    
    // Special handling for "Recent Brushes" category
    if (m_category == RecentBrushes) {
        // TODO: Implement recent brushes tracking
        return false; // For now, no recent brushes
    }
    
    // Check if brush type matches category
    QString brushType = brush->getType();
    QStringList categoryTypes = s_categoryBrushTypes.value(m_category);
    
    return categoryTypes.contains(brushType);
}

bool BrushCategoryTab::matchesFilter(RME::core::Brush* brush) const
{
    if (!brush) {
        return false;
    }
    
    // If no filter, all brushes match
    if (m_searchFilter.isEmpty()) {
        return true;
    }
    
    // Check if brush name contains filter text (case-insensitive)
    QString brushName = brush->getName().toLower();
    QString filter = m_searchFilter.toLower();
    
    return brushName.contains(filter);
}

} // namespace palettes
} // namespace ui
} // namespace RME