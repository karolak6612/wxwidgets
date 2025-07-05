#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QRegularExpression>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Advanced filtering and search manager for brushes
 * 
 * This class provides sophisticated filtering capabilities including
 * text search, tag filtering, category filtering, and custom filters.
 */
class BrushFilterManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Filter types for different filtering modes
     */
    enum FilterType {
        TextFilter,        ///< Text-based search
        CategoryFilter,    ///< Category-based filtering
        TagFilter,         ///< Tag-based filtering
        TypeFilter,        ///< Brush type filtering
        RecentFilter,      ///< Recently used brushes
        FavoriteFilter,    ///< Favorite brushes
        CustomFilter       ///< Custom user-defined filters
    };

    /**
     * @brief Search modes for text filtering
     */
    enum SearchMode {
        ContainsSearch,    ///< Contains substring
        StartsWithSearch,  ///< Starts with text
        ExactSearch,       ///< Exact match
        RegexSearch,       ///< Regular expression
        FuzzySearch        ///< Fuzzy matching
    };

    explicit BrushFilterManager(QObject* parent = nullptr);
    ~BrushFilterManager();

    // Text search
    void setSearchText(const QString& text);
    QString getSearchText() const { return m_searchText; }
    
    void setSearchMode(SearchMode mode);
    SearchMode getSearchMode() const { return m_searchMode; }

    // Category filtering
    void setCategoryFilter(const QStringList& categories);
    QStringList getCategoryFilter() const { return m_categoryFilter; }
    void addCategoryFilter(const QString& category);
    void removeCategoryFilter(const QString& category);
    void clearCategoryFilter();

    // Tag filtering
    void setTagFilter(const QStringList& tags);
    QStringList getTagFilter() const { return m_tagFilter; }
    void addTagFilter(const QString& tag);
    void removeTagFilter(const QString& tag);
    void clearTagFilter();

    // Type filtering
    void setTypeFilter(const QStringList& types);
    QStringList getTypeFilter() const { return m_typeFilter; }
    void addTypeFilter(const QString& type);
    void removeTypeFilter(const QString& type);
    void clearTypeFilter();

    // Special filters
    void setShowRecentOnly(bool recentOnly);
    bool isShowRecentOnly() const { return m_showRecentOnly; }
    
    void setShowFavoritesOnly(bool favoritesOnly);
    bool isShowFavoritesOnly() const { return m_showFavoritesOnly; }

    // Filter application
    QList<RME::core::Brush*> applyFilters(const QList<RME::core::Brush*>& brushes) const;
    bool matchesFilters(RME::core::Brush* brush) const;

    // Filter management
    void clearAllFilters();
    bool hasActiveFilters() const;
    QString getFilterSummary() const;

    // Recent brushes management
    void addRecentBrush(RME::core::Brush* brush);
    void clearRecentBrushes();
    QList<RME::core::Brush*> getRecentBrushes() const { return m_recentBrushes; }
    void setMaxRecentBrushes(int max);

    // Favorites management
    void addFavoriteBrush(RME::core::Brush* brush);
    void removeFavoriteBrush(RME::core::Brush* brush);
    void clearFavoriteBrushes();
    QList<RME::core::Brush*> getFavoriteBrushes() const { return m_favoriteBrushes; }
    bool isFavoriteBrush(RME::core::Brush* brush) const;

    // Tag management
    QStringList getAllAvailableTags() const;
    QStringList getTagsForBrush(RME::core::Brush* brush) const;
    void setTagsForBrush(RME::core::Brush* brush, const QStringList& tags);

public slots:
    void onBrushUsed(RME::core::Brush* brush);

signals:
    void filtersChanged();
    void searchTextChanged(const QString& text);
    void recentBrushesChanged();
    void favoriteBrushesChanged();
    void tagsChanged();

protected:
    // Filter matching methods
    bool matchesTextFilter(RME::core::Brush* brush) const;
    bool matchesCategoryFilter(RME::core::Brush* brush) const;
    bool matchesTagFilter(RME::core::Brush* brush) const;
    bool matchesTypeFilter(RME::core::Brush* brush) const;
    bool matchesRecentFilter(RME::core::Brush* brush) const;
    bool matchesFavoriteFilter(RME::core::Brush* brush) const;

    // Search algorithms
    bool containsSearch(const QString& text, const QString& target) const;
    bool startsWithSearch(const QString& text, const QString& target) const;
    bool exactSearch(const QString& text, const QString& target) const;
    bool regexSearch(const QString& text, const QString& target) const;
    bool fuzzySearch(const QString& text, const QString& target) const;

    // Helper methods
    QString getBrushCategory(RME::core::Brush* brush) const;
    QStringList getBrushSearchableText(RME::core::Brush* brush) const;

private:
    // Search settings
    QString m_searchText;
    SearchMode m_searchMode = ContainsSearch;
    bool m_caseSensitive = false;

    // Filter settings
    QStringList m_categoryFilter;
    QStringList m_tagFilter;
    QStringList m_typeFilter;
    bool m_showRecentOnly = false;
    bool m_showFavoritesOnly = false;

    // Recent brushes
    QList<RME::core::Brush*> m_recentBrushes;
    int m_maxRecentBrushes = 20;

    // Favorite brushes
    QSet<RME::core::Brush*> m_favoriteBrushes;

    // Brush tags
    QMap<RME::core::Brush*, QStringList> m_brushTags;

    // Cached regex for performance
    mutable QRegularExpression m_cachedRegex;
    mutable QString m_cachedRegexPattern;
};

} // namespace palettes
} // namespace ui
} // namespace RME