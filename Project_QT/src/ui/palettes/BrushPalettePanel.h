#pragma once

#include "BasePalettePanel.h"
#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>

namespace RME {
namespace core {
    class Brush;
    namespace brush {
        class BrushManagerService;
        class BrushStateService;
    }
    namespace services {
        class ServiceContainer;
    }
}
}

namespace RME {
namespace ui {
namespace palettes {

class BrushCategoryTab;
class BrushFilterManager;
class BrushOrganizer;
class AdvancedSearchWidget;
class BrushContextMenu;

/**
 * @brief Main brush palette panel for selecting and managing brushes
 * 
 * This panel provides a comprehensive interface for browsing, searching,
 * and selecting brushes. It organizes brushes by category and supports
 * multiple view modes for optimal user experience.
 */
class BrushPalettePanel : public BasePalettePanel
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
     * @brief Brush categories for organization
     */
    enum BrushCategory {
        TerrainBrushes,    ///< Ground, Wall, Carpet, Table
        ObjectBrushes,     ///< Doodad, Raw items
        EntityBrushes,     ///< Creature, Spawn, Waypoint
        SpecialBrushes,    ///< House, HouseExit, Eraser
        AllBrushes,        ///< All brushes combined
        RecentBrushes      ///< Recently used brushes
    };

    explicit BrushPalettePanel(QWidget* parent = nullptr);
    ~BrushPalettePanel();

    // BasePalettePanel interface
    void setServiceContainer(RME::core::services::ServiceContainer* serviceContainer) override;
    void refreshContent() override;
    void updateFromSettings() override;

    // View mode management
    ViewMode getViewMode() const { return m_viewMode; }
    void setViewMode(ViewMode mode);

    // Category management
    BrushCategory getCurrentCategory() const;
    void setCurrentCategory(BrushCategory category);

    // Search functionality
    QString getSearchText() const;
    void setSearchText(const QString& text);
    void clearSearch();

    // Advanced features
    BrushFilterManager* getFilterManager() const { return m_filterManager; }
    BrushOrganizer* getBrushOrganizer() const { return m_brushOrganizer; }
    
    // Brush selection
    RME::core::Brush* getSelectedBrush() const;
    void setSelectedBrush(RME::core::Brush* brush);

public slots:
    void onBrushSelected(RME::core::Brush* brush);
    void onBrushActivated(RME::core::Brush* brush);
    void onSearchTextChanged();
    void onViewModeChanged();
    void onCategoryChanged();
    void onRefreshRequested();
    
    // Advanced feature slots
    void onFiltersChanged();
    void onBrushContextMenuRequested(const QPoint& position);
    void onBrushFavoriteToggled(RME::core::Brush* brush, bool isFavorite);
    void onBrushCategoryChanged(RME::core::Brush* brush, const QString& category);
    void onBrushUsed(RME::core::Brush* brush);

signals:
    void brushSelected(RME::core::Brush* brush);
    void brushActivated(RME::core::Brush* brush);
    void viewModeChanged(ViewMode mode);
    void categoryChanged(BrushCategory category);
    void searchTextChanged(const QString& text);

protected:
    void setupUI();
    void setupConnections();
    void setupToolbar();
    void setupCategoryTabs();
    void updateStatusText();
    void updateViewModeCombo();
    void populateBrushCategories();

private slots:
    void onSearchTimer();
    void onTabChanged(int index);

private:
    // UI Components
    QVBoxLayout* m_mainLayout = nullptr;
    QHBoxLayout* m_toolbarLayout = nullptr;
    QTabWidget* m_categoryTabs = nullptr;
    QLabel* m_statusLabel = nullptr;

    // Toolbar components (replaced by AdvancedSearchWidget)
    AdvancedSearchWidget* m_advancedSearchWidget = nullptr;
    QComboBox* m_viewModeCombo = nullptr;
    QComboBox* m_iconSizeCombo = nullptr;
    QPushButton* m_refreshButton = nullptr;

    // Category tabs
    BrushCategoryTab* m_terrainTab = nullptr;
    BrushCategoryTab* m_objectTab = nullptr;
    BrushCategoryTab* m_entityTab = nullptr;
    BrushCategoryTab* m_specialTab = nullptr;
    BrushCategoryTab* m_allBrushesTab = nullptr;
    BrushCategoryTab* m_recentTab = nullptr;

    // Services
    RME::core::brush::BrushManagerService* m_brushManagerService = nullptr;
    RME::core::brush::BrushStateService* m_brushStateService = nullptr;
    
    // Advanced features
    BrushFilterManager* m_filterManager = nullptr;
    BrushOrganizer* m_brushOrganizer = nullptr;
    BrushContextMenu* m_contextMenu = nullptr;

    // State
    ViewMode m_viewMode = GridView;
    BrushCategory m_currentCategory = TerrainBrushes;
    QString m_searchText;
    RME::core::Brush* m_selectedBrush = nullptr;

    // Search timer for delayed search
    QTimer* m_searchTimer = nullptr;
    static constexpr int SEARCH_DELAY_MS = 300;

    // Category mapping
    static const QMap<BrushCategory, QString> s_categoryNames;
    static const QMap<ViewMode, QString> s_viewModeNames;
};

} // namespace palettes
} // namespace ui
} // namespace RME