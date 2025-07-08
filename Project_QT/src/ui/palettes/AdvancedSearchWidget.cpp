#include "AdvancedSearchWidget.h"
#include "BrushFilterManager.h"
#include <QDebug>
#include <QStringListModel>

namespace RME {
namespace ui {
namespace palettes {

AdvancedSearchWidget::AdvancedSearchWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("AdvancedSearchWidget");
    
    // Create search timer
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(SEARCH_DELAY_MS);
    
    setupUI();
    setupConnections();
    
    qDebug() << "AdvancedSearchWidget: Created";
}

AdvancedSearchWidget::~AdvancedSearchWidget()
{
    qDebug() << "AdvancedSearchWidget: Destroyed";
}

void AdvancedSearchWidget::setFilterManager(BrushFilterManager* filterManager)
{
    if (m_filterManager != filterManager) {
        // Disconnect old filter manager
        if (m_filterManager) {
            disconnect(m_filterManager, nullptr, this, nullptr);
        }
        
        m_filterManager = filterManager;
        
        // Connect new filter manager
        if (m_filterManager) {
            connect(m_filterManager, &BrushFilterManager::filtersChanged,
                    this, &AdvancedSearchWidget::updateFilterSummary);
            connect(m_filterManager, &BrushFilterManager::tagsChanged,
                    this, &AdvancedSearchWidget::updateAvailableOptions);
            
            // Update UI with current filter state
            updateAvailableOptions();
            updateFilterSummary();
            
            qDebug() << "AdvancedSearchWidget: Filter manager set";
        }
    }
}

QString AdvancedSearchWidget::getSearchText() const
{
    return m_searchEdit ? m_searchEdit->text() : QString();
}

void AdvancedSearchWidget::setSearchText(const QString& text)
{
    if (m_searchEdit && m_searchEdit->text() != text) {
        m_searchEdit->setText(text);
    }
}

void AdvancedSearchWidget::clearSearch()
{
    if (m_searchEdit) {
        m_searchEdit->clear();
    }
}

void AdvancedSearchWidget::clearAllFilters()
{
    if (m_filterManager) {
        m_filterManager->clearAllFilters();
    }
    
    // Reset UI state
    if (m_searchEdit) m_searchEdit->clear();
    if (m_searchModeCombo) m_searchModeCombo->setCurrentIndex(0);
    if (m_terrainCheck) m_terrainCheck->setChecked(false);
    if (m_objectsCheck) m_objectsCheck->setChecked(false);
    if (m_entitiesCheck) m_entitiesCheck->setChecked(false);
    if (m_specialCheck) m_specialCheck->setChecked(false);
    if (m_tagEdit) m_tagEdit->clear();
    if (m_tagList) m_tagList->clear();
    if (m_typeList) m_typeList->clearSelection();
    if (m_recentOnlyCheck) m_recentOnlyCheck->setChecked(false);
    if (m_favoritesOnlyCheck) m_favoritesOnlyCheck->setChecked(false);
    if (m_caseSensitiveCheck) m_caseSensitiveCheck->setChecked(false);
}

bool AdvancedSearchWidget::hasActiveFilters() const
{
    return m_filterManager ? m_filterManager->hasActiveFilters() : false;
}

QString AdvancedSearchWidget::getFilterSummary() const
{
    return m_filterManager ? m_filterManager->getFilterSummary() : QString();
}

void AdvancedSearchWidget::setExpanded(bool expanded)
{
    if (m_expanded != expanded) {
        m_expanded = expanded;
        updateExpandedState();
        Q_EMIT expandedChanged(expanded); // Changed
    }
}

void AdvancedSearchWidget::onSearchTextChanged()
{
    // Start/restart search timer
    m_searchTimer->stop();
    m_searchTimer->start();
}

void AdvancedSearchWidget::onSearchModeChanged()
{
    if (m_filterManager && m_searchModeCombo) {
        auto mode = static_cast<BrushFilterManager::SearchMode>(m_searchModeCombo->currentIndex());
        m_filterManager->setSearchMode(mode);
    }
}

void AdvancedSearchWidget::onCategoryFilterChanged()
{
    if (!m_filterManager) {
        return;
    }
    
    QStringList categories;
    
    if (m_terrainCheck && m_terrainCheck->isChecked()) {
        categories << "Terrain";
    }
    if (m_objectsCheck && m_objectsCheck->isChecked()) {
        categories << "Objects";
    }
    if (m_entitiesCheck && m_entitiesCheck->isChecked()) {
        categories << "Entities";
    }
    if (m_specialCheck && m_specialCheck->isChecked()) {
        categories << "Special";
    }
    
    m_filterManager->setCategoryFilter(categories);
}

void AdvancedSearchWidget::onTagFilterChanged()
{
    if (!m_filterManager || !m_tagList) {
        return;
    }
    
    QStringList selectedTags;
    for (int i = 0; i < m_tagList->count(); ++i) {
        QListWidgetItem* item = m_tagList->item(i);
        if (item && item->checkState() == Qt::Checked) {
            selectedTags << item->text();
        }
    }
    
    m_filterManager->setTagFilter(selectedTags);
}

void AdvancedSearchWidget::onTypeFilterChanged()
{
    if (!m_filterManager || !m_typeList) {
        return;
    }
    
    QStringList selectedTypes;
    for (int i = 0; i < m_typeList->count(); ++i) {
        QListWidgetItem* item = m_typeList->item(i);
        if (item && item->isSelected()) {
            selectedTypes << item->text();
        }
    }
    
    m_filterManager->setTypeFilter(selectedTypes);
}

void AdvancedSearchWidget::onSpecialFilterChanged()
{
    if (!m_filterManager) {
        return;
    }
    
    if (m_recentOnlyCheck) {
        m_filterManager->setShowRecentOnly(m_recentOnlyCheck->isChecked());
    }
    
    if (m_favoritesOnlyCheck) {
        m_filterManager->setShowFavoritesOnly(m_favoritesOnlyCheck->isChecked());
    }
}

void AdvancedSearchWidget::onClearFiltersClicked()
{
    clearAllFilters();
}

void AdvancedSearchWidget::onToggleExpandedClicked()
{
    setExpanded(!m_expanded);
}

void AdvancedSearchWidget::onSearchTimer()
{
    if (m_filterManager && m_searchEdit) {
        m_filterManager->setSearchText(m_searchEdit->text());
    }
}

void AdvancedSearchWidget::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Setup sections
    setupBasicSearch();
    setupAdvancedFilters();
    setupFilterSummary();
    
    // Initially collapsed
    updateExpandedState();
}

void AdvancedSearchWidget::setupConnections()
{
    // Search timer
    connect(m_searchTimer, &QTimer::timeout, this, &AdvancedSearchWidget::onSearchTimer);
    
    // Basic search
    if (m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged, this, &AdvancedSearchWidget::onSearchTextChanged);
    }
    
    if (m_searchModeCombo) {
        connect(m_searchModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &AdvancedSearchWidget::onSearchModeChanged);
    }
    
    if (m_clearButton) {
        connect(m_clearButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onClearFiltersClicked);
    }
    
    if (m_expandButton) {
        connect(m_expandButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onToggleExpandedClicked);
    }
    
    // Category filters
    if (m_terrainCheck) {
        connect(m_terrainCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onCategoryFilterChanged);
    }
    if (m_objectsCheck) {
        connect(m_objectsCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onCategoryFilterChanged);
    }
    if (m_entitiesCheck) {
        connect(m_entitiesCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onCategoryFilterChanged);
    }
    if (m_specialCheck) {
        connect(m_specialCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onCategoryFilterChanged);
    }
    
    // Tag filters
    if (m_tagList) {
        connect(m_tagList, &QListWidget::itemChanged, this, &AdvancedSearchWidget::onTagFilterChanged);
    }
    
    if (m_addTagButton) {
        connect(m_addTagButton, &QPushButton::clicked, [this]() {
            if (m_tagEdit && m_tagList && !m_tagEdit->text().isEmpty()) {
                QString tag = m_tagEdit->text().trimmed();
                if (!tag.isEmpty()) {
                    // Add tag to list if not already present
                    bool found = false;
                    for (int i = 0; i < m_tagList->count(); ++i) {
                        if (m_tagList->item(i)->text() == tag) {
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        QListWidgetItem* item = new QListWidgetItem(tag);
                        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                        item->setCheckState(Qt::Checked);
                        m_tagList->addItem(item);
                        onTagFilterChanged();
                    }
                    
                    m_tagEdit->clear();
                }
            }
        });
    }
    
    // Type filters
    if (m_typeList) {
        connect(m_typeList, &QListWidget::itemSelectionChanged, this, &AdvancedSearchWidget::onTypeFilterChanged);
    }
    
    // Special filters
    if (m_recentOnlyCheck) {
        connect(m_recentOnlyCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onSpecialFilterChanged);
    }
    if (m_favoritesOnlyCheck) {
        connect(m_favoritesOnlyCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onSpecialFilterChanged);
    }
}

void AdvancedSearchWidget::setupBasicSearch()
{
    // Basic search layout
    m_basicSearchLayout = new QHBoxLayout();
    m_basicSearchLayout->setSpacing(4);
    
    // Search edit
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search brushes...");
    m_basicSearchLayout->addWidget(m_searchEdit);
    
    // Search mode combo
    m_searchModeCombo = new QComboBox(this);
    m_searchModeCombo->addItems({"Contains", "Starts With", "Exact", "Regex", "Fuzzy"});
    m_searchModeCombo->setToolTip("Search mode");
    m_searchModeCombo->setMaximumWidth(100);
    m_basicSearchLayout->addWidget(m_searchModeCombo);
    
    // Clear button
    m_clearButton = new QPushButton("Clear", this);
    m_clearButton->setMaximumWidth(60);
    m_clearButton->setToolTip("Clear all filters");
    m_basicSearchLayout->addWidget(m_clearButton);
    
    // Expand button
    m_expandButton = new QPushButton("▼", this);
    m_expandButton->setMaximumSize(24, 24);
    m_expandButton->setToolTip("Show advanced filters");
    m_basicSearchLayout->addWidget(m_expandButton);
    
    m_mainLayout->addLayout(m_basicSearchLayout);
}

void AdvancedSearchWidget::setupAdvancedFilters()
{
    // Advanced filters widget
    m_advancedWidget = new QWidget(this);
    m_advancedLayout = new QGridLayout(m_advancedWidget);
    m_advancedLayout->setContentsMargins(0, 4, 0, 0);
    m_advancedLayout->setSpacing(4);
    
    // Category filters
    m_categoryGroup = new QGroupBox("Categories", m_advancedWidget);
    m_categoryLayout = new QVBoxLayout(m_categoryGroup);
    m_categoryLayout->setSpacing(2);
    
    m_terrainCheck = new QCheckBox("Terrain", m_categoryGroup);
    m_objectsCheck = new QCheckBox("Objects", m_categoryGroup);
    m_entitiesCheck = new QCheckBox("Entities", m_categoryGroup);
    m_specialCheck = new QCheckBox("Special", m_categoryGroup);
    
    m_categoryLayout->addWidget(m_terrainCheck);
    m_categoryLayout->addWidget(m_objectsCheck);
    m_categoryLayout->addWidget(m_entitiesCheck);
    m_categoryLayout->addWidget(m_specialCheck);
    
    m_advancedLayout->addWidget(m_categoryGroup, 0, 0);
    
    // Tag filters
    m_tagGroup = new QGroupBox("Tags", m_advancedWidget);
    m_tagLayout = new QVBoxLayout(m_tagGroup);
    m_tagLayout->setSpacing(2);
    
    QHBoxLayout* tagInputLayout = new QHBoxLayout();
    m_tagEdit = new QLineEdit(m_tagGroup);
    m_tagEdit->setPlaceholderText("Add tag...");
    m_addTagButton = new QPushButton("+", m_tagGroup);
    m_addTagButton->setMaximumSize(24, 24);
    
    tagInputLayout->addWidget(m_tagEdit);
    tagInputLayout->addWidget(m_addTagButton);
    
    m_tagList = new QListWidget(m_tagGroup);
    m_tagList->setMaximumHeight(80);
    
    m_tagLayout->addLayout(tagInputLayout);
    m_tagLayout->addWidget(m_tagList);
    
    m_advancedLayout->addWidget(m_tagGroup, 0, 1);
    
    // Type filters
    m_typeGroup = new QGroupBox("Types", m_advancedWidget);
    m_typeLayout = new QVBoxLayout(m_typeGroup);
    
    m_typeList = new QListWidget(m_typeGroup);
    m_typeList->setMaximumHeight(100);
    m_typeList->setSelectionMode(QAbstractItemView::MultiSelection);
    
    // Add common brush types
    QStringList brushTypes = {
        "GroundBrush", "WallBrush", "CarpetBrush", "TableBrush",
        "DoodadBrush", "RawBrush", "CreatureBrush", "SpawnBrush",
        "WaypointBrush", "HouseBrush", "HouseExitBrush", "EraserBrush"
    };
    
    for (const QString& type : brushTypes) {
        m_typeList->addItem(type);
    }
    
    m_typeLayout->addWidget(m_typeList);
    m_advancedLayout->addWidget(m_typeGroup, 1, 0);
    
    // Special filters
    m_specialGroup = new QGroupBox("Special", m_advancedWidget);
    m_specialLayout = new QVBoxLayout(m_specialGroup);
    m_specialLayout->setSpacing(2);
    
    m_recentOnlyCheck = new QCheckBox("Recent only", m_specialGroup);
    m_favoritesOnlyCheck = new QCheckBox("Favorites only", m_specialGroup);
    m_caseSensitiveCheck = new QCheckBox("Case sensitive", m_specialGroup);
    
    m_specialLayout->addWidget(m_recentOnlyCheck);
    m_specialLayout->addWidget(m_favoritesOnlyCheck);
    m_specialLayout->addWidget(m_caseSensitiveCheck);
    
    m_advancedLayout->addWidget(m_specialGroup, 1, 1);
    
    m_mainLayout->addWidget(m_advancedWidget);
}

void AdvancedSearchWidget::setupFilterSummary()
{
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("QLabel { font-size: 10px; color: #666; font-style: italic; }");
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setText("No active filters");
    m_mainLayout->addWidget(m_summaryLabel);
}

void AdvancedSearchWidget::updateFilterSummary()
{
    if (m_summaryLabel) {
        QString summary = getFilterSummary();
        m_summaryLabel->setText(summary);
    }
}

void AdvancedSearchWidget::updateExpandedState()
{
    if (m_advancedWidget) {
        m_advancedWidget->setVisible(m_expanded);
    }
    
    if (m_expandButton) {
        m_expandButton->setText(m_expanded ? "▲" : "▼");
        m_expandButton->setToolTip(m_expanded ? "Hide advanced filters" : "Show advanced filters");
    }
}

void AdvancedSearchWidget::updateAvailableOptions()
{
    if (!m_filterManager) {
        return;
    }
    
    // Update tag completer
    QStringList availableTags = m_filterManager->getAllAvailableTags();
    if (m_tagEdit) {
        delete m_tagCompleter;
        m_tagCompleter = new QCompleter(availableTags, this);
        m_tagCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_tagEdit->setCompleter(m_tagCompleter);
    }
}

} // namespace palettes
} // namespace ui
} // namespace RME