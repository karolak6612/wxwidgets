#include "core/services/BrushPaletteService.h"
#include <QSettings>
#include <QDebug>

namespace RME {
namespace core {

BrushPaletteService::BrushPaletteService(QObject *parent) 
    : QObject(parent) {
    initializeDefaults();
    loadSettings();
}

BrushPaletteService::~BrushPaletteService() {
    saveSettings();
}

void BrushPaletteService::initializeDefaults() {
    m_viewMode = ViewMode::Grid;
    m_sortMode = SortMode::Category;
    m_filterMode = FilterMode::All;
    m_currentFilter = QString();
    m_searchText = QString();
    
    m_favoriteBrushes.clear();
    m_recentBrushes.clear();
    m_maxRecentBrushes = 20;
    
    m_visibleCategories.clear();
    
    m_iconSize = 32;
    m_showTooltips = true;
    m_showPreview = true;
    
    m_gridColumns = 4;
    m_autoResizeColumns = true;
    
    m_activeTagFilters.clear();
    m_customBrushOrder.clear();
    
    m_brushUsageStats.clear();
    m_lastBrushUsage.clear();
}

// View mode management
BrushPaletteService::ViewMode BrushPaletteService::getViewMode() const {
    return m_viewMode;
}

void BrushPaletteService::setViewMode(ViewMode mode) {
    if (m_viewMode != mode) {
        m_viewMode = mode;
        emit viewModeChanged(mode);
    }
}

// Sort mode management
BrushPaletteService::SortMode BrushPaletteService::getSortMode() const {
    return m_sortMode;
}

void BrushPaletteService::setSortMode(SortMode mode) {
    if (m_sortMode != mode) {
        m_sortMode = mode;
        emit sortModeChanged(mode);
    }
}

// Filter management
BrushPaletteService::FilterMode BrushPaletteService::getFilterMode() const {
    return m_filterMode;
}

void BrushPaletteService::setFilterMode(FilterMode mode) {
    if (m_filterMode != mode) {
        m_filterMode = mode;
        emit filterModeChanged(mode);
    }
}

QString BrushPaletteService::getCurrentFilter() const {
    return m_currentFilter;
}

void BrushPaletteService::setCurrentFilter(const QString& filter) {
    if (m_currentFilter != filter) {
        m_currentFilter = filter;
        emit currentFilterChanged(filter);
    }
}

// Search functionality
QString BrushPaletteService::getSearchText() const {
    return m_searchText;
}

void BrushPaletteService::setSearchText(const QString& text) {
    if (m_searchText != text) {
        m_searchText = text;
        emit searchTextChanged(text);
    }
}

void BrushPaletteService::clearSearch() {
    setSearchText(QString());
}

// Favorites management
QStringList BrushPaletteService::getFavoriteBrushes() const {
    return m_favoriteBrushes;
}

bool BrushPaletteService::isBrushFavorite(const QString& brushName) const {
    return m_favoriteBrushes.contains(brushName);
}

void BrushPaletteService::addBrushToFavorites(const QString& brushName) {
    if (!brushName.isEmpty() && !m_favoriteBrushes.contains(brushName)) {
        m_favoriteBrushes.append(brushName);
        emit favoriteBrushesChanged(m_favoriteBrushes);
        emit brushFavoriteToggled(brushName, true);
    }
}

void BrushPaletteService::removeBrushFromFavorites(const QString& brushName) {
    if (m_favoriteBrushes.removeOne(brushName)) {
        emit favoriteBrushesChanged(m_favoriteBrushes);
        emit brushFavoriteToggled(brushName, false);
    }
}

void BrushPaletteService::toggleBrushFavorite(const QString& brushName) {
    if (isBrushFavorite(brushName)) {
        removeBrushFromFavorites(brushName);
    } else {
        addBrushToFavorites(brushName);
    }
}

void BrushPaletteService::clearFavorites() {
    if (!m_favoriteBrushes.isEmpty()) {
        m_favoriteBrushes.clear();
        emit favoriteBrushesChanged(m_favoriteBrushes);
    }
}

// Recent brushes management
QStringList BrushPaletteService::getRecentBrushes() const {
    return m_recentBrushes;
}

void BrushPaletteService::addRecentBrush(const QString& brushName) {
    if (brushName.isEmpty()) return;
    
    // Remove if already exists
    m_recentBrushes.removeAll(brushName);
    
    // Add to front
    m_recentBrushes.prepend(brushName);
    
    // Limit size
    while (m_recentBrushes.size() > m_maxRecentBrushes) {
        m_recentBrushes.removeLast();
    }
    
    emit recentBrushesChanged(m_recentBrushes);
}

void BrushPaletteService::clearRecentBrushes() {
    if (!m_recentBrushes.isEmpty()) {
        m_recentBrushes.clear();
        emit recentBrushesChanged(m_recentBrushes);
    }
}

int BrushPaletteService::getMaxRecentBrushes() const {
    return m_maxRecentBrushes;
}

void BrushPaletteService::setMaxRecentBrushes(int maxCount) {
    if (maxCount > 0 && m_maxRecentBrushes != maxCount) {
        m_maxRecentBrushes = maxCount;
        
        // Trim current list if needed
        while (m_recentBrushes.size() > m_maxRecentBrushes) {
            m_recentBrushes.removeLast();
        }
        
        if (m_recentBrushes.size() < maxCount) {
            emit recentBrushesChanged(m_recentBrushes);
        }
    }
}

// Category management
QStringList BrushPaletteService::getVisibleCategories() const {
    return m_visibleCategories;
}

void BrushPaletteService::setVisibleCategories(const QStringList& categories) {
    if (m_visibleCategories != categories) {
        m_visibleCategories = categories;
        emit visibleCategoriesChanged(categories);
    }
}

bool BrushPaletteService::isCategoryVisible(const QString& category) const {
    return m_visibleCategories.isEmpty() || m_visibleCategories.contains(category);
}

void BrushPaletteService::setCategoryVisible(const QString& category, bool visible) {
    bool currentlyVisible = isCategoryVisible(category);
    
    if (visible && !currentlyVisible) {
        if (!m_visibleCategories.contains(category)) {
            m_visibleCategories.append(category);
            emit visibleCategoriesChanged(m_visibleCategories);
            emit categoryVisibilityChanged(category, true);
        }
    } else if (!visible && currentlyVisible) {
        if (m_visibleCategories.removeOne(category)) {
            emit visibleCategoriesChanged(m_visibleCategories);
            emit categoryVisibilityChanged(category, false);
        }
    }
}

// Display settings
int BrushPaletteService::getIconSize() const {
    return m_iconSize;
}

void BrushPaletteService::setIconSize(int size) {
    if (size > 0 && m_iconSize != size) {
        m_iconSize = size;
        emit iconSizeChanged(size);
    }
}

bool BrushPaletteService::getShowTooltips() const {
    return m_showTooltips;
}

void BrushPaletteService::setShowTooltips(bool show) {
    if (m_showTooltips != show) {
        m_showTooltips = show;
        emit showTooltipsChanged(show);
    }
}

bool BrushPaletteService::getShowPreview() const {
    return m_showPreview;
}

void BrushPaletteService::setShowPreview(bool show) {
    if (m_showPreview != show) {
        m_showPreview = show;
        emit showPreviewChanged(show);
    }
}

// Layout settings
int BrushPaletteService::getGridColumns() const {
    return m_gridColumns;
}

void BrushPaletteService::setGridColumns(int columns) {
    if (columns > 0 && m_gridColumns != columns) {
        m_gridColumns = columns;
        emit gridColumnsChanged(columns);
    }
}

bool BrushPaletteService::getAutoResizeColumns() const {
    return m_autoResizeColumns;
}

void BrushPaletteService::setAutoResizeColumns(bool autoResize) {
    if (m_autoResizeColumns != autoResize) {
        m_autoResizeColumns = autoResize;
        emit autoResizeColumnsChanged(autoResize);
    }
}

// Advanced filtering
QStringList BrushPaletteService::getActiveTagFilters() const {
    return m_activeTagFilters;
}

void BrushPaletteService::setActiveTagFilters(const QStringList& tags) {
    if (m_activeTagFilters != tags) {
        m_activeTagFilters = tags;
        emit tagFiltersChanged(tags);
    }
}

void BrushPaletteService::addTagFilter(const QString& tag) {
    if (!tag.isEmpty() && !m_activeTagFilters.contains(tag)) {
        m_activeTagFilters.append(tag);
        emit tagFiltersChanged(m_activeTagFilters);
    }
}

void BrushPaletteService::removeTagFilter(const QString& tag) {
    if (m_activeTagFilters.removeOne(tag)) {
        emit tagFiltersChanged(m_activeTagFilters);
    }
}

void BrushPaletteService::clearTagFilters() {
    if (!m_activeTagFilters.isEmpty()) {
        m_activeTagFilters.clear();
        emit tagFiltersChanged(m_activeTagFilters);
    }
}

// Custom organization
QStringList BrushPaletteService::getCustomBrushOrder() const {
    return m_customBrushOrder;
}

void BrushPaletteService::setCustomBrushOrder(const QStringList& brushNames) {
    if (m_customBrushOrder != brushNames) {
        m_customBrushOrder = brushNames;
        emit customBrushOrderChanged(brushNames);
    }
}

void BrushPaletteService::moveBrushInCustomOrder(const QString& brushName, int newPosition) {
    int currentPos = m_customBrushOrder.indexOf(brushName);
    if (currentPos == -1) {
        // Brush not in custom order, add it
        if (newPosition >= 0 && newPosition <= m_customBrushOrder.size()) {
            m_customBrushOrder.insert(newPosition, brushName);
            emit customBrushOrderChanged(m_customBrushOrder);
        }
    } else if (newPosition >= 0 && newPosition < m_customBrushOrder.size() && currentPos != newPosition) {
        // Move existing brush
        m_customBrushOrder.move(currentPos, newPosition);
        emit customBrushOrderChanged(m_customBrushOrder);
    }
}

// Statistics
QMap<QString, int> BrushPaletteService::getBrushUsageStats() const {
    return m_brushUsageStats;
}

void BrushPaletteService::recordBrushUsage(const QString& brushName) {
    if (brushName.isEmpty()) return;
    
    m_brushUsageStats[brushName] = m_brushUsageStats.value(brushName, 0) + 1;
    m_lastBrushUsage[brushName] = QDateTime::currentDateTime();
    
    // Also add to recent brushes
    addRecentBrush(brushName);
    
    emit brushUsageRecorded(brushName, m_brushUsageStats[brushName]);
}

void BrushPaletteService::clearUsageStats() {
    if (!m_brushUsageStats.isEmpty() || !m_lastBrushUsage.isEmpty()) {
        m_brushUsageStats.clear();
        m_lastBrushUsage.clear();
    }
}

// Persistence
void BrushPaletteService::saveSettings() {
    QSettings settings;
    
    // View and sort settings
    settings.setValue(getSettingsKey("viewMode"), static_cast<int>(m_viewMode));
    settings.setValue(getSettingsKey("sortMode"), static_cast<int>(m_sortMode));
    settings.setValue(getSettingsKey("filterMode"), static_cast<int>(m_filterMode));
    settings.setValue(getSettingsKey("currentFilter"), m_currentFilter);
    
    // Favorites and recent
    settings.setValue(getSettingsKey("favoriteBrushes"), m_favoriteBrushes);
    settings.setValue(getSettingsKey("recentBrushes"), m_recentBrushes);
    settings.setValue(getSettingsKey("maxRecentBrushes"), m_maxRecentBrushes);
    
    // Categories
    settings.setValue(getSettingsKey("visibleCategories"), m_visibleCategories);
    
    // Display settings
    settings.setValue(getSettingsKey("iconSize"), m_iconSize);
    settings.setValue(getSettingsKey("showTooltips"), m_showTooltips);
    settings.setValue(getSettingsKey("showPreview"), m_showPreview);
    
    // Layout settings
    settings.setValue(getSettingsKey("gridColumns"), m_gridColumns);
    settings.setValue(getSettingsKey("autoResizeColumns"), m_autoResizeColumns);
    
    // Advanced filtering
    settings.setValue(getSettingsKey("activeTagFilters"), m_activeTagFilters);
    
    // Custom organization
    settings.setValue(getSettingsKey("customBrushOrder"), m_customBrushOrder);
    
    // Statistics
    QStringList usageKeys, usageValues;
    for (auto it = m_brushUsageStats.constBegin(); it != m_brushUsageStats.constEnd(); ++it) {
        usageKeys.append(it.key());
        usageValues.append(QString::number(it.value()));
    }
    settings.setValue(getSettingsKey("brushUsageKeys"), usageKeys);
    settings.setValue(getSettingsKey("brushUsageValues"), usageValues);
}

void BrushPaletteService::loadSettings() {
    QSettings settings;
    
    // View and sort settings
    m_viewMode = static_cast<ViewMode>(settings.value(getSettingsKey("viewMode"), static_cast<int>(ViewMode::Grid)).toInt());
    m_sortMode = static_cast<SortMode>(settings.value(getSettingsKey("sortMode"), static_cast<int>(SortMode::Category)).toInt());
    m_filterMode = static_cast<FilterMode>(settings.value(getSettingsKey("filterMode"), static_cast<int>(FilterMode::All)).toInt());
    m_currentFilter = settings.value(getSettingsKey("currentFilter"), QString()).toString();
    
    // Favorites and recent
    m_favoriteBrushes = settings.value(getSettingsKey("favoriteBrushes"), QStringList()).toStringList();
    m_recentBrushes = settings.value(getSettingsKey("recentBrushes"), QStringList()).toStringList();
    m_maxRecentBrushes = settings.value(getSettingsKey("maxRecentBrushes"), 20).toInt();
    
    // Categories
    m_visibleCategories = settings.value(getSettingsKey("visibleCategories"), QStringList()).toStringList();
    
    // Display settings
    m_iconSize = settings.value(getSettingsKey("iconSize"), 32).toInt();
    m_showTooltips = settings.value(getSettingsKey("showTooltips"), true).toBool();
    m_showPreview = settings.value(getSettingsKey("showPreview"), true).toBool();
    
    // Layout settings
    m_gridColumns = settings.value(getSettingsKey("gridColumns"), 4).toInt();
    m_autoResizeColumns = settings.value(getSettingsKey("autoResizeColumns"), true).toBool();
    
    // Advanced filtering
    m_activeTagFilters = settings.value(getSettingsKey("activeTagFilters"), QStringList()).toStringList();
    
    // Custom organization
    m_customBrushOrder = settings.value(getSettingsKey("customBrushOrder"), QStringList()).toStringList();
    
    // Statistics
    QStringList usageKeys = settings.value(getSettingsKey("brushUsageKeys"), QStringList()).toStringList();
    QStringList usageValues = settings.value(getSettingsKey("brushUsageValues"), QStringList()).toStringList();
    
    m_brushUsageStats.clear();
    for (int i = 0; i < usageKeys.size() && i < usageValues.size(); ++i) {
        m_brushUsageStats[usageKeys[i]] = usageValues[i].toInt();
    }
}

void BrushPaletteService::resetToDefaults() {
    initializeDefaults();
    
    // Emit all change signals
    emit viewModeChanged(m_viewMode);
    emit sortModeChanged(m_sortMode);
    emit filterModeChanged(m_filterMode);
    emit currentFilterChanged(m_currentFilter);
    emit searchTextChanged(m_searchText);
    emit favoriteBrushesChanged(m_favoriteBrushes);
    emit recentBrushesChanged(m_recentBrushes);
    emit visibleCategoriesChanged(m_visibleCategories);
    emit iconSizeChanged(m_iconSize);
    emit showTooltipsChanged(m_showTooltips);
    emit showPreviewChanged(m_showPreview);
    emit gridColumnsChanged(m_gridColumns);
    emit autoResizeColumnsChanged(m_autoResizeColumns);
    emit tagFiltersChanged(m_activeTagFilters);
    emit customBrushOrderChanged(m_customBrushOrder);
}

// Helper methods
QString BrushPaletteService::getSettingsKey(const QString& key) const {
    return QString("BrushPalette/%1").arg(key);
}

} // namespace core
} // namespace RME

// #include "BrushPaletteService.moc" // Removed - Q_OBJECT is in header