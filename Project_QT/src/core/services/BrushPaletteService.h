#ifndef RME_BRUSH_PALETTE_SERVICE_H
#define RME_BRUSH_PALETTE_SERVICE_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QDateTime>

// Forward declarations
namespace RME { namespace core { class Brush; }}

namespace RME {
namespace core {

/**
 * @brief Service for managing brush palette state and preferences
 * 
 * This service handles:
 * - Brush palette view mode preferences
 * - Brush favorites and recent usage
 * - Search functionality and filters
 * - Palette layout and organization settings
 */
class BrushPaletteService : public QObject {
    Q_OBJECT

public:
    enum class ViewMode {
        List,
        Grid,
        Compact
    };
    Q_ENUM(ViewMode)

    enum class SortMode {
        Name,
        Category,
        RecentlyUsed,
        MostUsed,
        Custom
    };
    Q_ENUM(SortMode)

    enum class FilterMode {
        All,
        Category,
        Tags,
        Favorites,
        Recent,
        Search
    };
    Q_ENUM(FilterMode)

    explicit BrushPaletteService(QObject *parent = nullptr);
    ~BrushPaletteService() override;

    // View mode management
    ViewMode getViewMode() const;
    void setViewMode(ViewMode mode);
    
    // Sort mode management
    SortMode getSortMode() const;
    void setSortMode(SortMode mode);
    
    // Filter management
    FilterMode getFilterMode() const;
    void setFilterMode(FilterMode mode);
    QString getCurrentFilter() const;
    void setCurrentFilter(const QString& filter);
    
    // Search functionality
    QString getSearchText() const;
    void setSearchText(const QString& text);
    void clearSearch();
    
    // Favorites management
    QStringList getFavoriteBrushes() const;
    bool isBrushFavorite(const QString& brushName) const;
    void addBrushToFavorites(const QString& brushName);
    void removeBrushFromFavorites(const QString& brushName);
    void toggleBrushFavorite(const QString& brushName);
    void clearFavorites();
    
    // Recent brushes management
    QStringList getRecentBrushes() const;
    void addRecentBrush(const QString& brushName);
    void clearRecentBrushes();
    int getMaxRecentBrushes() const;
    void setMaxRecentBrushes(int maxCount);
    
    // Category management
    QStringList getVisibleCategories() const;
    void setVisibleCategories(const QStringList& categories);
    bool isCategoryVisible(const QString& category) const;
    void setCategoryVisible(const QString& category, bool visible);
    
    // Display settings
    int getIconSize() const;
    void setIconSize(int size);
    bool getShowTooltips() const;
    void setShowTooltips(bool show);
    bool getShowPreview() const;
    void setShowPreview(bool show);
    
    // Layout settings
    int getGridColumns() const;
    void setGridColumns(int columns);
    bool getAutoResizeColumns() const;
    void setAutoResizeColumns(bool autoResize);
    
    // Persistence
    void saveSettings();
    void loadSettings();
    void resetToDefaults();
    
    // Advanced filtering
    QStringList getActiveTagFilters() const;
    void setActiveTagFilters(const QStringList& tags);
    void addTagFilter(const QString& tag);
    void removeTagFilter(const QString& tag);
    void clearTagFilters();
    
    // Custom organization
    QStringList getCustomBrushOrder() const;
    void setCustomBrushOrder(const QStringList& brushNames);
    void moveBrushInCustomOrder(const QString& brushName, int newPosition);
    
    // Statistics
    QMap<QString, int> getBrushUsageStats() const;
    void recordBrushUsage(const QString& brushName);
    void clearUsageStats();

signals:
    // View and display changes
    void viewModeChanged(ViewMode mode);
    void sortModeChanged(SortMode mode);
    void filterModeChanged(FilterMode mode);
    void currentFilterChanged(const QString& filter);
    void searchTextChanged(const QString& text);
    
    // Favorites and recent changes
    void favoriteBrushesChanged(const QStringList& favorites);
    void brushFavoriteToggled(const QString& brushName, bool isFavorite);
    void recentBrushesChanged(const QStringList& recent);
    
    // Category and organization changes
    void visibleCategoriesChanged(const QStringList& categories);
    void categoryVisibilityChanged(const QString& category, bool visible);
    void customBrushOrderChanged(const QStringList& order);
    
    // Display settings changes
    void iconSizeChanged(int size);
    void showTooltipsChanged(bool show);
    void showPreviewChanged(bool show);
    void gridColumnsChanged(int columns);
    void autoResizeColumnsChanged(bool autoResize);
    
    // Filter changes
    void tagFiltersChanged(const QStringList& tags);
    
    // Statistics changes
    void brushUsageRecorded(const QString& brushName, int totalUsage);

private:
    // Helper methods
    void initializeDefaults();
    QString getSettingsKey(const QString& key) const;
    
    // View state
    ViewMode m_viewMode;
    SortMode m_sortMode;
    FilterMode m_filterMode;
    QString m_currentFilter;
    QString m_searchText;
    
    // Favorites and recent
    QStringList m_favoriteBrushes;
    QStringList m_recentBrushes;
    int m_maxRecentBrushes;
    
    // Categories
    QStringList m_visibleCategories;
    
    // Display settings
    int m_iconSize;
    bool m_showTooltips;
    bool m_showPreview;
    
    // Layout settings
    int m_gridColumns;
    bool m_autoResizeColumns;
    
    // Advanced filtering
    QStringList m_activeTagFilters;
    
    // Custom organization
    QStringList m_customBrushOrder;
    
    // Statistics
    QMap<QString, int> m_brushUsageStats;
    QMap<QString, QDateTime> m_lastBrushUsage;
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_PALETTE_SERVICE_H