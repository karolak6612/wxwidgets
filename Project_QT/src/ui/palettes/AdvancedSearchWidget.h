#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QCompleter>
#include <QTimer>

namespace RME {
namespace ui {
namespace palettes {

class BrushFilterManager;

/**
 * @brief Advanced search widget with multiple filtering options
 * 
 * This widget provides a comprehensive search interface including
 * text search, category filters, tag filters, and special filters.
 */
class AdvancedSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedSearchWidget(QWidget* parent = nullptr);
    ~AdvancedSearchWidget();

    // Filter manager
    void setFilterManager(BrushFilterManager* filterManager);
    BrushFilterManager* getFilterManager() const { return m_filterManager; }

    // Search interface
    QString getSearchText() const;
    void setSearchText(const QString& text);
    void clearSearch();

    // Filter interface
    void clearAllFilters();
    bool hasActiveFilters() const;
    QString getFilterSummary() const;

    // Widget state
    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

public Q_SLOTS: // Changed
    void onSearchTextChanged();
    void onSearchModeChanged();
    void onCategoryFilterChanged();
    void onTagFilterChanged();
    void onTypeFilterChanged();
    void onSpecialFilterChanged();
    void onClearFiltersClicked();
    void onToggleExpandedClicked();
    void onSearchTimer();

Q_SIGNALS: // Changed
    void searchChanged();
    void filtersChanged();
    void expandedChanged(bool expanded);

protected:
    void setupUI();
    void setupConnections();
    void setupBasicSearch();
    void setupAdvancedFilters();
    void setupFilterSummary();
    void updateFilterSummary();
    void updateExpandedState();
    void updateAvailableOptions();

private:
    // Filter manager
    BrushFilterManager* m_filterManager = nullptr;

    // Main layout
    QVBoxLayout* m_mainLayout = nullptr;
    
    // Basic search section
    QHBoxLayout* m_basicSearchLayout = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_searchModeCombo = nullptr;
    QPushButton* m_clearButton = nullptr;
    QPushButton* m_expandButton = nullptr;

    // Advanced filters section
    QWidget* m_advancedWidget = nullptr;
    QGridLayout* m_advancedLayout = nullptr;
    
    // Category filters
    QGroupBox* m_categoryGroup = nullptr;
    QVBoxLayout* m_categoryLayout = nullptr;
    QCheckBox* m_terrainCheck = nullptr;
    QCheckBox* m_objectsCheck = nullptr;
    QCheckBox* m_entitiesCheck = nullptr;
    QCheckBox* m_specialCheck = nullptr;
    
    // Tag filters
    QGroupBox* m_tagGroup = nullptr;
    QVBoxLayout* m_tagLayout = nullptr;
    QLineEdit* m_tagEdit = nullptr;
    QListWidget* m_tagList = nullptr;
    QPushButton* m_addTagButton = nullptr;
    
    // Type filters
    QGroupBox* m_typeGroup = nullptr;
    QVBoxLayout* m_typeLayout = nullptr;
    QListWidget* m_typeList = nullptr;
    
    // Special filters
    QGroupBox* m_specialGroup = nullptr;
    QVBoxLayout* m_specialLayout = nullptr;
    QCheckBox* m_recentOnlyCheck = nullptr;
    QCheckBox* m_favoritesOnlyCheck = nullptr;
    QCheckBox* m_caseSensitiveCheck = nullptr;

    // Filter summary
    QLabel* m_summaryLabel = nullptr;

    // State
    bool m_expanded = false;
    QTimer* m_searchTimer = nullptr;
    QCompleter* m_tagCompleter = nullptr;

    // Constants
    static constexpr int SEARCH_DELAY_MS = 300;
};

} // namespace palettes
} // namespace ui
} // namespace RME