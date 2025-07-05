#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QJsonObject>
#include <QJsonArray>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Manages brush organization including custom categories, favorites, and sorting
 * 
 * This class provides functionality for organizing brushes into custom categories,
 * managing favorites, and providing different sorting options.
 */
class BrushOrganizer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Sorting options for brush lists
     */
    enum SortOrder {
        NameAscending,     ///< Sort by name A-Z
        NameDescending,    ///< Sort by name Z-A
        TypeAscending,     ///< Sort by type A-Z
        TypeDescending,    ///< Sort by type Z-A
        RecentlyUsed,      ///< Sort by recent usage
        MostUsed,          ///< Sort by usage frequency
        Custom             ///< Custom user-defined order
    };

    explicit BrushOrganizer(QObject* parent = nullptr);
    ~BrushOrganizer();

    // Custom categories
    QStringList getCustomCategories() const;
    void addCustomCategory(const QString& categoryName);
    void removeCustomCategory(const QString& categoryName);
    void renameCustomCategory(const QString& oldName, const QString& newName);
    
    // Category membership
    QStringList getBrushesInCategory(const QString& categoryName) const;
    void addBrushToCategory(RME::core::Brush* brush, const QString& categoryName);
    void removeBrushFromCategory(RME::core::Brush* brush, const QString& categoryName);
    QStringList getCategoriesForBrush(RME::core::Brush* brush) const;

    // Favorites management
    void addToFavorites(RME::core::Brush* brush);
    void removeFromFavorites(RME::core::Brush* brush);
    bool isFavorite(RME::core::Brush* brush) const;
    QList<RME::core::Brush*> getFavorites() const;
    void clearFavorites();

    // Usage tracking
    void recordBrushUsage(RME::core::Brush* brush);
    int getBrushUsageCount(RME::core::Brush* brush) const;
    QDateTime getLastUsageTime(RME::core::Brush* brush) const;
    QList<RME::core::Brush*> getRecentlyUsedBrushes(int maxCount = 20) const;
    QList<RME::core::Brush*> getMostUsedBrushes(int maxCount = 20) const;

    // Sorting
    QList<RME::core::Brush*> sortBrushes(const QList<RME::core::Brush*>& brushes, SortOrder order) const;
    void setSortOrder(SortOrder order);
    SortOrder getSortOrder() const { return m_sortOrder; }

    // Custom ordering
    void setCustomOrder(const QString& categoryName, const QList<RME::core::Brush*>& brushes);
    QList<RME::core::Brush*> getCustomOrder(const QString& categoryName) const;
    void moveInCustomOrder(const QString& categoryName, RME::core::Brush* brush, int newIndex);

    // Drag and drop support
    void moveBrushToCategory(RME::core::Brush* brush, const QString& fromCategory, const QString& toCategory);
    bool canMoveBrushToCategory(RME::core::Brush* brush, const QString& categoryName) const;

    // Persistence
    QJsonObject saveToJson() const;
    void loadFromJson(const QJsonObject& json);
    void saveToFile(const QString& filename) const;
    void loadFromFile(const QString& filename);

    // Statistics
    int getTotalBrushCount() const;
    int getCategoryBrushCount(const QString& categoryName) const;
    QMap<QString, int> getCategoryStatistics() const;
    QMap<RME::core::Brush*, int> getUsageStatistics() const;

public slots:
    void onBrushUsed(RME::core::Brush* brush);

signals:
    void customCategoriesChanged();
    void favoritesChanged();
    void usageStatisticsChanged();
    void sortOrderChanged(SortOrder order);
    void brushMovedToCategory(RME::core::Brush* brush, const QString& category);

protected:
    // Helper methods
    QString generateUniqueId(RME::core::Brush* brush) const;
    void updateUsageStatistics(RME::core::Brush* brush);
    void cleanupOrphanedData();

private:
    // Custom categories
    QStringList m_customCategories;
    QMap<QString, QStringList> m_categoryBrushes; // category -> brush IDs
    QMap<QString, QStringList> m_brushCategories; // brush ID -> categories

    // Favorites
    QSet<QString> m_favoriteBrushes; // brush IDs

    // Usage tracking
    QMap<QString, int> m_usageCount; // brush ID -> usage count
    QMap<QString, QDateTime> m_lastUsage; // brush ID -> last usage time
    QList<QString> m_recentlyUsed; // ordered list of recently used brush IDs

    // Custom ordering
    QMap<QString, QStringList> m_customOrders; // category -> ordered brush IDs

    // Settings
    SortOrder m_sortOrder = NameAscending;
    int m_maxRecentBrushes = 20;

    // Brush ID mapping (for persistence)
    QMap<RME::core::Brush*, QString> m_brushToId;
    QMap<QString, RME::core::Brush*> m_idToBrush;
};

} // namespace palettes
} // namespace ui
} // namespace RME