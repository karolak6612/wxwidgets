#include "BrushFilterManager.h"
#include "core/brush/Brush.h"
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

namespace RME {
namespace ui {
namespace palettes {

BrushFilterManager::BrushFilterManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "BrushFilterManager: Created";
}

BrushFilterManager::~BrushFilterManager()
{
    qDebug() << "BrushFilterManager: Destroyed";
}

void BrushFilterManager::setSearchText(const QString& text)
{
    if (m_searchText != text) {
        m_searchText = text;
        
        // Clear cached regex when search text changes
        m_cachedRegexPattern.clear();
        
        Q_EMIT searchTextChanged(text); // Changed
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Search text set to" << text;
    }
}

void BrushFilterManager::setSearchMode(SearchMode mode)
{
    if (m_searchMode != mode) {
        m_searchMode = mode;
        
        // Clear cached regex when search mode changes
        m_cachedRegexPattern.clear();
        
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Search mode changed to" << static_cast<int>(mode);
    }
}

void BrushFilterManager::setCategoryFilter(const QStringList& categories)
{
    if (m_categoryFilter != categories) {
        m_categoryFilter = categories;
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Category filter set to" << categories;
    }
}

void BrushFilterManager::addCategoryFilter(const QString& category)
{
    if (!m_categoryFilter.contains(category)) {
        m_categoryFilter.append(category);
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::removeCategoryFilter(const QString& category)
{
    if (m_categoryFilter.removeAll(category) > 0) {
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::clearCategoryFilter()
{
    if (!m_categoryFilter.isEmpty()) {
        m_categoryFilter.clear();
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::setTagFilter(const QStringList& tags)
{
    if (m_tagFilter != tags) {
        m_tagFilter = tags;
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Tag filter set to" << tags;
    }
}

void BrushFilterManager::addTagFilter(const QString& tag)
{
    if (!m_tagFilter.contains(tag)) {
        m_tagFilter.append(tag);
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::removeTagFilter(const QString& tag)
{
    if (m_tagFilter.removeAll(tag) > 0) {
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::clearTagFilter()
{
    if (!m_tagFilter.isEmpty()) {
        m_tagFilter.clear();
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::setTypeFilter(const QStringList& types)
{
    if (m_typeFilter != types) {
        m_typeFilter = types;
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Type filter set to" << types;
    }
}

void BrushFilterManager::addTypeFilter(const QString& type)
{
    if (!m_typeFilter.contains(type)) {
        m_typeFilter.append(type);
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::removeTypeFilter(const QString& type)
{
    if (m_typeFilter.removeAll(type) > 0) {
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::clearTypeFilter()
{
    if (!m_typeFilter.isEmpty()) {
        m_typeFilter.clear();
        Q_EMIT filtersChanged(); // Changed
    }
}

void BrushFilterManager::setShowRecentOnly(bool recentOnly)
{
    if (m_showRecentOnly != recentOnly) {
        m_showRecentOnly = recentOnly;
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Show recent only set to" << recentOnly;
    }
}

void BrushFilterManager::setShowFavoritesOnly(bool favoritesOnly)
{
    if (m_showFavoritesOnly != favoritesOnly) {
        m_showFavoritesOnly = favoritesOnly;
        Q_EMIT filtersChanged(); // Changed
        
        qDebug() << "BrushFilterManager: Show favorites only set to" << favoritesOnly;
    }
}

QList<RME::core::Brush*> BrushFilterManager::applyFilters(const QList<RME::core::Brush*>& brushes) const
{
    QList<RME::core::Brush*> filteredBrushes;
    
    for (RME::core::Brush* brush : brushes) {
        if (matchesFilters(brush)) {
            filteredBrushes.append(brush);
        }
    }
    
    return filteredBrushes;
}

bool BrushFilterManager::matchesFilters(RME::core::Brush* brush) const
{
    if (!brush) {
        return false;
    }
    
    // Check all filter types
    if (!matchesTextFilter(brush)) return false;
    if (!matchesCategoryFilter(brush)) return false;
    if (!matchesTagFilter(brush)) return false;
    if (!matchesTypeFilter(brush)) return false;
    if (!matchesRecentFilter(brush)) return false;
    if (!matchesFavoriteFilter(brush)) return false;
    
    return true;
}

void BrushFilterManager::clearAllFilters()
{
    bool hadFilters = hasActiveFilters();
    
    m_searchText.clear();
    m_categoryFilter.clear();
    m_tagFilter.clear();
    m_typeFilter.clear();
    m_showRecentOnly = false;
    m_showFavoritesOnly = false;
    m_cachedRegexPattern.clear();
    
    if (hadFilters) {
        Q_EMIT filtersChanged(); // Changed
        qDebug() << "BrushFilterManager: All filters cleared";
    }
}

bool BrushFilterManager::hasActiveFilters() const
{
    return !m_searchText.isEmpty() ||
           !m_categoryFilter.isEmpty() ||
           !m_tagFilter.isEmpty() ||
           !m_typeFilter.isEmpty() ||
           m_showRecentOnly ||
           m_showFavoritesOnly;
}

QString BrushFilterManager::getFilterSummary() const
{
    QStringList summary;
    
    if (!m_searchText.isEmpty()) {
        summary << QString("Text: \"%1\"").arg(m_searchText);
    }
    
    if (!m_categoryFilter.isEmpty()) {
        summary << QString("Categories: %1").arg(m_categoryFilter.join(", "));
    }
    
    if (!m_tagFilter.isEmpty()) {
        summary << QString("Tags: %1").arg(m_tagFilter.join(", "));
    }
    
    if (!m_typeFilter.isEmpty()) {
        summary << QString("Types: %1").arg(m_typeFilter.join(", "));
    }
    
    if (m_showRecentOnly) {
        summary << "Recent only";
    }
    
    if (m_showFavoritesOnly) {
        summary << "Favorites only";
    }
    
    if (summary.isEmpty()) {
        return "No active filters";
    }
    
    return summary.join(" | ");
}

void BrushFilterManager::addRecentBrush(RME::core::Brush* brush)
{
    if (!brush) {
        return;
    }
    
    // Remove if already exists
    m_recentBrushes.removeAll(brush);
    
    // Add to front
    m_recentBrushes.prepend(brush);
    
    // Limit size
    while (m_recentBrushes.size() > m_maxRecentBrushes) {
        m_recentBrushes.removeLast();
    }
    
    Q_EMIT recentBrushesChanged(); // Changed
}

void BrushFilterManager::clearRecentBrushes()
{
    if (!m_recentBrushes.isEmpty()) {
        m_recentBrushes.clear();
        Q_EMIT recentBrushesChanged(); // Changed
        qDebug() << "BrushFilterManager: Recent brushes cleared";
    }
}

void BrushFilterManager::setMaxRecentBrushes(int max)
{
    if (m_maxRecentBrushes != max && max > 0) {
        m_maxRecentBrushes = max;
        
        // Trim if necessary
        while (m_recentBrushes.size() > max) {
            m_recentBrushes.removeLast();
        }
        
        if (m_recentBrushes.size() != max) {
            Q_EMIT recentBrushesChanged(); // Changed
        }
    }
}

void BrushFilterManager::addFavoriteBrush(RME::core::Brush* brush)
{
    if (brush && !m_favoriteBrushes.contains(brush)) {
        m_favoriteBrushes.insert(brush);
        Q_EMIT favoriteBrushesChanged(); // Changed
        qDebug() << "BrushFilterManager: Added favorite brush" << brush->getName();
    }
}

void BrushFilterManager::removeFavoriteBrush(RME::core::Brush* brush)
{
    if (brush && m_favoriteBrushes.remove(brush)) {
        Q_EMIT favoriteBrushesChanged(); // Changed
        qDebug() << "BrushFilterManager: Removed favorite brush" << brush->getName();
    }
}

void BrushFilterManager::clearFavoriteBrushes()
{
    if (!m_favoriteBrushes.isEmpty()) {
        m_favoriteBrushes.clear();
        Q_EMIT favoriteBrushesChanged(); // Changed
        qDebug() << "BrushFilterManager: Favorite brushes cleared";
    }
}

bool BrushFilterManager::isFavoriteBrush(RME::core::Brush* brush) const
{
    return m_favoriteBrushes.contains(brush);
}

QStringList BrushFilterManager::getAllAvailableTags() const
{
    QSet<QString> allTags;
    
    for (auto it = m_brushTags.begin(); it != m_brushTags.end(); ++it) {
        const QStringList& tags = it.value();
        for (const QString& tag : tags) {
            allTags.insert(tag);
        }
    }
    
    QStringList result = allTags.values();
    result.sort();
    return result;
}

QStringList BrushFilterManager::getTagsForBrush(RME::core::Brush* brush) const
{
    return m_brushTags.value(brush);
}

void BrushFilterManager::setTagsForBrush(RME::core::Brush* brush, const QStringList& tags)
{
    if (!brush) {
        return;
    }
    
    QStringList oldTags = m_brushTags.value(brush);
    if (oldTags != tags) {
        if (tags.isEmpty()) {
            m_brushTags.remove(brush);
        } else {
            m_brushTags[brush] = tags;
        }
        
        Q_EMIT tagsChanged(); // Changed
        Q_EMIT filtersChanged(); // Changed // Tags might affect current filtering
        
        qDebug() << "BrushFilterManager: Tags for brush" << brush->getName() << "set to" << tags;
    }
}

void BrushFilterManager::onBrushUsed(RME::core::Brush* brush)
{
    addRecentBrush(brush);
}

// Filter matching methods

bool BrushFilterManager::matchesTextFilter(RME::core::Brush* brush) const
{
    if (m_searchText.isEmpty()) {
        return true;
    }
    
    QStringList searchableText = getBrushSearchableText(brush);
    
    for (const QString& text : searchableText) {
        switch (m_searchMode) {
            case ContainsSearch:
                if (containsSearch(m_searchText, text)) return true;
                break;
            case StartsWithSearch:
                if (startsWithSearch(m_searchText, text)) return true;
                break;
            case ExactSearch:
                if (exactSearch(m_searchText, text)) return true;
                break;
            case RegexSearch:
                if (regexSearch(m_searchText, text)) return true;
                break;
            case FuzzySearch:
                if (fuzzySearch(m_searchText, text)) return true;
                break;
        }
    }
    
    return false;
}

bool BrushFilterManager::matchesCategoryFilter(RME::core::Brush* brush) const
{
    if (m_categoryFilter.isEmpty()) {
        return true;
    }
    
    QString category = getBrushCategory(brush);
    return m_categoryFilter.contains(category);
}

bool BrushFilterManager::matchesTagFilter(RME::core::Brush* brush) const
{
    if (m_tagFilter.isEmpty()) {
        return true;
    }
    
    QStringList brushTags = getTagsForBrush(brush);
    
    // Check if brush has any of the required tags
    for (const QString& requiredTag : m_tagFilter) {
        if (brushTags.contains(requiredTag)) {
            return true;
        }
    }
    
    return false;
}

bool BrushFilterManager::matchesTypeFilter(RME::core::Brush* brush) const
{
    if (m_typeFilter.isEmpty()) {
        return true;
    }
    
    return m_typeFilter.contains(brush->getType());
}

bool BrushFilterManager::matchesRecentFilter(RME::core::Brush* brush) const
{
    if (!m_showRecentOnly) {
        return true;
    }
    
    return m_recentBrushes.contains(brush);
}

bool BrushFilterManager::matchesFavoriteFilter(RME::core::Brush* brush) const
{
    if (!m_showFavoritesOnly) {
        return true;
    }
    
    return m_favoriteBrushes.contains(brush);
}

// Search algorithms

bool BrushFilterManager::containsSearch(const QString& text, const QString& target) const
{
    return target.contains(text, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

bool BrushFilterManager::startsWithSearch(const QString& text, const QString& target) const
{
    return target.startsWith(text, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

bool BrushFilterManager::exactSearch(const QString& text, const QString& target) const
{
    return target.compare(text, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0;
}

bool BrushFilterManager::regexSearch(const QString& text, const QString& target) const
{
    // Cache regex for performance
    if (m_cachedRegexPattern != text) {
        m_cachedRegexPattern = text;
        m_cachedRegex = QRegularExpression(text);
        if (!m_caseSensitive) {
            m_cachedRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
    }
    
    if (!m_cachedRegex.isValid()) {
        return false;
    }
    
    return m_cachedRegex.match(target).hasMatch();
}

bool BrushFilterManager::fuzzySearch(const QString& text, const QString& target) const
{
    // Simple fuzzy search implementation
    QString lowerText = text.toLower();
    QString lowerTarget = target.toLower();
    
    int textIndex = 0;
    int targetIndex = 0;
    
    while (textIndex < lowerText.length() && targetIndex < lowerTarget.length()) {
        if (lowerText[textIndex] == lowerTarget[targetIndex]) {
            textIndex++;
        }
        targetIndex++;
    }
    
    return textIndex == lowerText.length();
}

// Helper methods

QString BrushFilterManager::getBrushCategory(RME::core::Brush* brush) const
{
    if (!brush) {
        return QString();
    }
    
    QString type = brush->getType();
    
    if (type.contains("Ground") || type.contains("Wall") || type.contains("Carpet") || type.contains("Table")) {
        return "Terrain";
    } else if (type.contains("Doodad") || type.contains("Raw")) {
        return "Objects";
    } else if (type.contains("Creature") || type.contains("Spawn") || type.contains("Waypoint")) {
        return "Entities";
    } else if (type.contains("House") || type.contains("Eraser")) {
        return "Special";
    }
    
    return "Other";
}

QStringList BrushFilterManager::getBrushSearchableText(RME::core::Brush* brush) const
{
    if (!brush) {
        return QStringList();
    }
    
    QStringList searchableText;
    
    // Add brush name
    if (!brush->getName().isEmpty()) {
        searchableText << brush->getName();
    }
    
    // Add brush type
    searchableText << brush->getType();
    
    // Add category
    searchableText << getBrushCategory(brush);
    
    // Add tags
    searchableText << getTagsForBrush(brush);
    
    return searchableText;
}

} // namespace palettes
} // namespace ui
} // namespace RME