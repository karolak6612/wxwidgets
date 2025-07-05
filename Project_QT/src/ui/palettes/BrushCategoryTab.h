#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QString>

namespace RME {
namespace core {
    class Brush;
    namespace brush {
        class BrushManagerService;
    }
}
}

namespace RME {
namespace ui {
namespace palettes {

class BrushListWidget;
class BrushGridWidget;

/**
 * @brief Container for displaying brushes of a specific category
 * 
 * This widget manages the display of brushes within a category,
 * supporting different view modes and filtering capabilities.
 */
class BrushCategoryTab : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief View modes for displaying brushes
     */
    enum ViewMode {
        GridView,        ///< Grid layout with icons
        ListView,        ///< List layout with text
        LargeIconView,   ///< Large icons in grid
        SmallIconView    ///< Small icons in grid
    };

    /**
     * @brief Brush categories
     */
    enum BrushCategory {
        TerrainBrushes,    ///< Ground, Wall, Carpet, Table
        ObjectBrushes,     ///< Doodad, Raw items
        EntityBrushes,     ///< Creature, Spawn, Waypoint
        SpecialBrushes,    ///< House, HouseExit, Eraser
        AllBrushes,        ///< All brushes combined
        RecentBrushes      ///< Recently used brushes
    };

    explicit BrushCategoryTab(BrushCategory category, QWidget* parent = nullptr);
    ~BrushCategoryTab();

    // Category management
    BrushCategory getCategory() const { return m_category; }
    QString getCategoryName() const;

    // View mode management
    ViewMode getViewMode() const { return m_viewMode; }
    void setViewMode(ViewMode mode);

    // Brush management
    void setBrushManagerService(RME::core::brush::BrushManagerService* service);
    void refreshBrushes();
    QList<RME::core::Brush*> getBrushes() const;
    QList<RME::core::Brush*> getFilteredBrushes() const;

    // Selection management
    RME::core::Brush* getSelectedBrush() const;
    void setSelectedBrush(RME::core::Brush* brush);

    // Filtering
    QString getSearchFilter() const { return m_searchFilter; }
    void setSearchFilter(const QString& filter);
    void clearFilter();

    // Statistics
    int getTotalBrushCount() const;
    int getVisibleBrushCount() const;

public slots:
    void onBrushSelected(RME::core::Brush* brush);
    void onBrushActivated(RME::core::Brush* brush);
    void onViewModeChanged();
    void onFilterChanged();

signals:
    void brushSelected(RME::core::Brush* brush);
    void brushActivated(RME::core::Brush* brush);
    void viewModeChanged(ViewMode mode);
    void filterChanged(const QString& filter);
    void brushCountChanged(int total, int visible);

protected:
    void setupUI();
    void setupConnections();
    void updateViewWidget();
    void updateEmptyState();
    void populateBrushes();
    bool matchesCategory(RME::core::Brush* brush) const;
    bool matchesFilter(RME::core::Brush* brush) const;

private:
    // Category and state
    BrushCategory m_category;
    ViewMode m_viewMode = GridView;
    QString m_searchFilter;

    // Services
    RME::core::brush::BrushManagerService* m_brushManagerService = nullptr;

    // UI Components
    QVBoxLayout* m_mainLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QLabel* m_emptyLabel = nullptr;

    // View widgets (only one active at a time)
    BrushListWidget* m_listWidget = nullptr;
    BrushGridWidget* m_gridWidget = nullptr;
    QWidget* m_currentViewWidget = nullptr;

    // Brush data
    QList<RME::core::Brush*> m_allBrushes;
    QList<RME::core::Brush*> m_filteredBrushes;
    RME::core::Brush* m_selectedBrush = nullptr;

    // Category names mapping
    static const QMap<BrushCategory, QString> s_categoryNames;
    static const QMap<BrushCategory, QStringList> s_categoryBrushTypes;
};

} // namespace palettes
} // namespace ui
} // namespace RME