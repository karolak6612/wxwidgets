#include "BrushPalettePanel.h"
#include "BrushCategoryTab.h"
#include "BrushFilterManager.h"
#include "BrushOrganizer.h"
#include "AdvancedSearchWidget.h"
#include "BrushContextMenu.h"
#include "core/services/ServiceContainer.h"
#include "core/brush/BrushManagerService.h"
#include "core/brush/BrushStateService.h"
#include "core/brush/Brush.h"
#include <QDebug>
#include <QApplication>
#include <QStyle>

namespace RME {
namespace ui {
namespace palettes {

// Static member definitions
const QMap<BrushPalettePanel::BrushCategory, QString> BrushPalettePanel::s_categoryNames = {
    {TerrainBrushes, "Terrain"},
    {ObjectBrushes, "Objects"},
    {EntityBrushes, "Entities"},
    {SpecialBrushes, "Special"},
    {AllBrushes, "All Brushes"},
    {RecentBrushes, "Recent"}
};

const QMap<BrushPalettePanel::ViewMode, QString> BrushPalettePanel::s_viewModeNames = {
    {GridView, "Grid"},
    {ListView, "List"},
    {LargeIconView, "Large Icons"},
    {SmallIconView, "Small Icons"}
};

BrushPalettePanel::BrushPalettePanel(QWidget* parent)
    : BasePalettePanel(parent)
    , m_searchTimer(new QTimer(this))
{
    setObjectName("BrushPalettePanel");
    setWindowTitle("Brush Palette");
    
    // Configure search timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(SEARCH_DELAY_MS);
    
    // Create advanced features
    m_filterManager = new BrushFilterManager(this);
    m_brushOrganizer = new BrushOrganizer(this);
    m_contextMenu = new BrushContextMenu(this);
    
    // Configure advanced features
    m_contextMenu->setFilterManager(m_filterManager);
    m_contextMenu->setBrushOrganizer(m_brushOrganizer);
    
    setupUI();
    setupConnections();
    
    qDebug() << "BrushPalettePanel: Created with advanced features";
}

BrushPalettePanel::~BrushPalettePanel()
{
    qDebug() << "BrushPalettePanel: Destroyed";
}

void BrushPalettePanel::setServiceContainer(RME::core::services::ServiceContainer* serviceContainer)
{
    BasePalettePanel::setServiceContainer(serviceContainer);
    
    if (serviceContainer) {
        m_brushManagerService = serviceContainer->getBrushManagerService();
        m_brushStateService = serviceContainer->getBrushStateService();
        
        if (m_brushManagerService) {
            qDebug() << "BrushPalettePanel: BrushManagerService connected";
        }
        
        if (m_brushStateService) {
            qDebug() << "BrushPalettePanel: BrushStateService connected";
            
            // Connect to brush state changes
            connect(m_brushStateService, &RME::core::brush::BrushStateService::activeBrushChanged,
                    this, &BrushPalettePanel::onBrushSelected);
        }
        
        // Populate brush categories now that we have services
        populateBrushCategories();
        refreshContent();
    }
}

void BrushPalettePanel::refreshContent()
{
    if (!m_brushManagerService) {
        qWarning() << "BrushPalettePanel: Cannot refresh - no BrushManagerService";
        return;
    }
    
    qDebug() << "BrushPalettePanel: Refreshing content";
    
    // Refresh all category tabs
    if (m_terrainTab) m_terrainTab->refreshBrushes();
    if (m_objectTab) m_objectTab->refreshBrushes();
    if (m_entityTab) m_entityTab->refreshBrushes();
    if (m_specialTab) m_specialTab->refreshBrushes();
    if (m_allBrushesTab) m_allBrushesTab->refreshBrushes();
    if (m_recentTab) m_recentTab->refreshBrushes();
    
    updateStatusText();
}

void BrushPalettePanel::updateFromSettings()
{
    qDebug() << "BrushPalettePanel: Updating from settings";
    
    // For now, use default settings
    setViewMode(GridView);
    updateViewModeCombo();
}

void BrushPalettePanel::setViewMode(ViewMode mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        
        // Update all category tabs
        if (m_terrainTab) m_terrainTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        if (m_objectTab) m_objectTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        if (m_entityTab) m_entityTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        if (m_specialTab) m_specialTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        if (m_allBrushesTab) m_allBrushesTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        if (m_recentTab) m_recentTab->setViewMode(static_cast<BrushCategoryTab::ViewMode>(mode));
        
        updateViewModeCombo();
        emit viewModeChanged(mode);
        
        qDebug() << "BrushPalettePanel: View mode changed to" << s_viewModeNames.value(mode);
    }
}

BrushPalettePanel::BrushCategory BrushPalettePanel::getCurrentCategory() const
{
    if (!m_categoryTabs) {
        return TerrainBrushes;
    }
    
    int currentIndex = m_categoryTabs->currentIndex();
    switch (currentIndex) {
        case 0: return TerrainBrushes;
        case 1: return ObjectBrushes;
        case 2: return EntityBrushes;
        case 3: return SpecialBrushes;
        case 4: return AllBrushes;
        case 5: return RecentBrushes;
        default: return TerrainBrushes;
    }
}

void BrushPalettePanel::setCurrentCategory(BrushCategory category)
{
    if (!m_categoryTabs) {
        return;
    }
    
    int tabIndex = 0;
    switch (category) {
        case TerrainBrushes: tabIndex = 0; break;
        case ObjectBrushes: tabIndex = 1; break;
        case EntityBrushes: tabIndex = 2; break;
        case SpecialBrushes: tabIndex = 3; break;
        case AllBrushes: tabIndex = 4; break;
        case RecentBrushes: tabIndex = 5; break;
    }
    
    if (m_categoryTabs->currentIndex() != tabIndex) {
        m_categoryTabs->setCurrentIndex(tabIndex);
    }
}

QString BrushPalettePanel::getSearchText() const
{
    return m_advancedSearchWidget ? m_advancedSearchWidget->getSearchText() : QString();
}

void BrushPalettePanel::setSearchText(const QString& text)
{
    if (m_advancedSearchWidget) {
        m_advancedSearchWidget->setSearchText(text);
    }
}

void BrushPalettePanel::clearSearch()
{
    if (m_advancedSearchWidget) {
        m_advancedSearchWidget->clearSearch();
    }
}

RME::core::Brush* BrushPalettePanel::getSelectedBrush() const
{
    return m_selectedBrush;
}

void BrushPalettePanel::setSelectedBrush(RME::core::Brush* brush)
{
    if (m_selectedBrush != brush) {
        m_selectedBrush = brush;
        
        // Update selection in current tab
        BrushCategoryTab* currentTab = qobject_cast<BrushCategoryTab*>(m_categoryTabs->currentWidget());
        if (currentTab) {
            currentTab->setSelectedBrush(brush);
        }
        
        // Record brush usage
        if (brush && m_brushOrganizer) {
            m_brushOrganizer->recordBrushUsage(brush);
        }
        
        updateStatusText();
        emit brushSelected(brush);
    }
}

void BrushPalettePanel::onBrushSelected(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
}

void BrushPalettePanel::onBrushActivated(RME::core::Brush* brush)
{
    setSelectedBrush(brush);
    
    // Set as active brush in brush state service
    if (m_brushStateService && brush) {
        m_brushStateService->setActiveBrush(brush);
        qDebug() << "BrushPalettePanel: Activated brush:" << brush->getName();
    }
    
    emit brushActivated(brush);
}

void BrushPalettePanel::onSearchTextChanged()
{
    // Handled by AdvancedSearchWidget
}

void BrushPalettePanel::onViewModeChanged()
{
    if (m_viewModeCombo) {
        int index = m_viewModeCombo->currentIndex();
        ViewMode newMode = static_cast<ViewMode>(index);
        setViewMode(newMode);
    }
}

void BrushPalettePanel::onCategoryChanged()
{
    BrushCategory newCategory = getCurrentCategory();
    if (newCategory != m_currentCategory) {
        m_currentCategory = newCategory;
        updateStatusText();
        emit categoryChanged(newCategory);
        
        qDebug() << "BrushPalettePanel: Category changed to" << s_categoryNames.value(newCategory);
    }
}

void BrushPalettePanel::onRefreshRequested()
{
    refreshContent();
}

void BrushPalettePanel::onFiltersChanged()
{
    // Apply filters to all category tabs
    if (m_terrainTab) m_terrainTab->refreshBrushes();
    if (m_objectTab) m_objectTab->refreshBrushes();
    if (m_entityTab) m_entityTab->refreshBrushes();
    if (m_specialTab) m_specialTab->refreshBrushes();
    if (m_allBrushesTab) m_allBrushesTab->refreshBrushes();
    if (m_recentTab) m_recentTab->refreshBrushes();
    
    updateStatusText();
}

void BrushPalettePanel::onBrushContextMenuRequested(const QPoint& position)
{
    if (m_contextMenu && m_selectedBrush) {
        m_contextMenu->showForBrush(m_selectedBrush, mapToGlobal(position));
    }
}

void BrushPalettePanel::onBrushFavoriteToggled(RME::core::Brush* brush, bool isFavorite)
{
    Q_UNUSED(brush)
    Q_UNUSED(isFavorite)
    
    // Refresh to update favorite indicators
    refreshContent();
}

void BrushPalettePanel::onBrushCategoryChanged(RME::core::Brush* brush, const QString& category)
{
    Q_UNUSED(brush)
    Q_UNUSED(category)
    
    // Refresh to update category membership
    refreshContent();
}

void BrushPalettePanel::onBrushUsed(RME::core::Brush* brush)
{
    if (m_brushOrganizer) {
        m_brushOrganizer->recordBrushUsage(brush);
    }
}

void BrushPalettePanel::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Setup toolbar
    setupToolbar();
    
    // Setup category tabs
    setupCategoryTabs();
    
    // Create status label
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { color: gray; font-size: 11px; }");
    m_mainLayout->addWidget(m_statusLabel);
    
    // Set initial state
    updateStatusText();
}

void BrushPalettePanel::setupConnections()
{
    // Advanced search widget
    if (m_advancedSearchWidget) {
        connect(m_advancedSearchWidget, &AdvancedSearchWidget::searchChanged,
                this, &BrushPalettePanel::onFiltersChanged);
        connect(m_advancedSearchWidget, &AdvancedSearchWidget::filtersChanged,
                this, &BrushPalettePanel::onFiltersChanged);
    }
    
    // Filter manager
    if (m_filterManager) {
        connect(m_filterManager, &BrushFilterManager::filtersChanged,
                this, &BrushPalettePanel::onFiltersChanged);
    }
    
    // Brush organizer
    if (m_brushOrganizer) {
        connect(m_brushOrganizer, &BrushOrganizer::favoritesChanged,
                this, &BrushPalettePanel::refreshContent);
        connect(m_brushOrganizer, &BrushOrganizer::customCategoriesChanged,
                this, &BrushPalettePanel::refreshContent);
    }
    
    // Context menu
    if (m_contextMenu) {
        connect(m_contextMenu, &BrushContextMenu::brushActivated,
                this, &BrushPalettePanel::onBrushActivated);
        connect(m_contextMenu, &BrushContextMenu::favoriteToggled,
                this, &BrushPalettePanel::onBrushFavoriteToggled);
        connect(m_contextMenu, &BrushContextMenu::categoryChanged,
                this, &BrushPalettePanel::onBrushCategoryChanged);
    }
    
    // View mode combo
    if (m_viewModeCombo) {
        connect(m_viewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &BrushPalettePanel::onViewModeChanged);
    }
    
    // Refresh button
    if (m_refreshButton) {
        connect(m_refreshButton, &QPushButton::clicked, this, &BrushPalettePanel::onRefreshRequested);
    }
    
    // Category tabs
    if (m_categoryTabs) {
        connect(m_categoryTabs, &QTabWidget::currentChanged, this, &BrushPalettePanel::onTabChanged);
    }
}

void BrushPalettePanel::setupToolbar()
{
    // Create toolbar layout
    m_toolbarLayout = new QHBoxLayout();
    m_toolbarLayout->setContentsMargins(0, 0, 0, 0);
    m_toolbarLayout->setSpacing(4);
    
    // Advanced search widget (replaces basic search)
    m_advancedSearchWidget = new AdvancedSearchWidget(this);
    m_advancedSearchWidget->setFilterManager(m_filterManager);
    m_toolbarLayout->addWidget(m_advancedSearchWidget);
    
    // Spacer
    m_toolbarLayout->addStretch();
    
    // View mode combo
    m_viewModeCombo = new QComboBox(this);
    m_viewModeCombo->addItems({"Grid", "List", "Large Icons", "Small Icons"});
    m_viewModeCombo->setCurrentIndex(0); // GridView
    m_viewModeCombo->setMaximumWidth(100);
    m_toolbarLayout->addWidget(m_viewModeCombo);
    
    // Icon size combo (for future use)
    m_iconSizeCombo = new QComboBox(this);
    m_iconSizeCombo->addItems({"Small", "Medium", "Large"});
    m_iconSizeCombo->setCurrentIndex(1); // Medium
    m_iconSizeCombo->setMaximumWidth(80);
    m_toolbarLayout->addWidget(m_iconSizeCombo);
    
    // Refresh button
    m_refreshButton = new QPushButton("Refresh", this);
    m_refreshButton->setMaximumSize(60, 24);
    m_refreshButton->setToolTip("Refresh brush list");
    m_toolbarLayout->addWidget(m_refreshButton);
    
    // Add toolbar to main layout
    m_mainLayout->addLayout(m_toolbarLayout);
}

void BrushPalettePanel::setupCategoryTabs()
{
    // Create tab widget
    m_categoryTabs = new QTabWidget(this);
    m_categoryTabs->setTabPosition(QTabWidget::North);
    
    // Create category tabs
    m_terrainTab = new BrushCategoryTab(BrushCategoryTab::TerrainBrushes, this);
    m_objectTab = new BrushCategoryTab(BrushCategoryTab::ObjectBrushes, this);
    m_entityTab = new BrushCategoryTab(BrushCategoryTab::EntityBrushes, this);
    m_specialTab = new BrushCategoryTab(BrushCategoryTab::SpecialBrushes, this);
    m_allBrushesTab = new BrushCategoryTab(BrushCategoryTab::AllBrushes, this);
    m_recentTab = new BrushCategoryTab(BrushCategoryTab::RecentBrushes, this);
    
    // Add tabs
    m_categoryTabs->addTab(m_terrainTab, "Terrain");
    m_categoryTabs->addTab(m_objectTab, "Objects");
    m_categoryTabs->addTab(m_entityTab, "Entities");
    m_categoryTabs->addTab(m_specialTab, "Special");
    m_categoryTabs->addTab(m_allBrushesTab, "All");
    m_categoryTabs->addTab(m_recentTab, "Recent");
    
    // Connect tab signals
    connect(m_terrainTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_terrainTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    connect(m_objectTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_objectTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    connect(m_entityTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_entityTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    connect(m_specialTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_specialTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    connect(m_allBrushesTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_allBrushesTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    connect(m_recentTab, &BrushCategoryTab::brushSelected, this, &BrushPalettePanel::onBrushSelected);
    connect(m_recentTab, &BrushCategoryTab::brushActivated, this, &BrushPalettePanel::onBrushActivated);
    
    // Add to main layout
    m_mainLayout->addWidget(m_categoryTabs);
}

void BrushPalettePanel::updateStatusText()
{
    if (!m_statusLabel) {
        return;
    }
    
    QString status;
    
    if (m_advancedSearchWidget && m_advancedSearchWidget->hasActiveFilters()) {
        status = m_advancedSearchWidget->getFilterSummary();
    } else if (m_selectedBrush) {
        status = QString("Selected: %1").arg(m_selectedBrush->getName());
    } else {
        BrushCategory category = getCurrentCategory();
        status = QString("%1 brushes").arg(s_categoryNames.value(category));
    }
    
    m_statusLabel->setText(status);
}

void BrushPalettePanel::updateViewModeCombo()
{
    if (m_viewModeCombo) {
        m_viewModeCombo->setCurrentIndex(static_cast<int>(m_viewMode));
    }
}

void BrushPalettePanel::populateBrushCategories()
{
    if (!m_brushManagerService) {
        return;
    }
    
    // Set brush manager service for all tabs
    if (m_terrainTab) m_terrainTab->setBrushManagerService(m_brushManagerService);
    if (m_objectTab) m_objectTab->setBrushManagerService(m_brushManagerService);
    if (m_entityTab) m_entityTab->setBrushManagerService(m_brushManagerService);
    if (m_specialTab) m_specialTab->setBrushManagerService(m_brushManagerService);
    if (m_allBrushesTab) m_allBrushesTab->setBrushManagerService(m_brushManagerService);
    if (m_recentTab) m_recentTab->setBrushManagerService(m_brushManagerService);
    
    qDebug() << "BrushPalettePanel: Brush categories populated with advanced features";
}

void BrushPalettePanel::onTabChanged(int index)
{
    Q_UNUSED(index)
    onCategoryChanged();
}

} // namespace palettes
} // namespace ui
} // namespace RME