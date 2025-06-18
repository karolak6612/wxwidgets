#include "ui/palettes/BasePalettePanel.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"

#include <QSettings>
#include <QFrame>
#include <QIcon>
#include <QStyle>

namespace RME {
namespace ui {
namespace palettes {

BasePalettePanel::BasePalettePanel(const QString& title, QWidget* parent)
    : QDockWidget(title, parent)
{
    setObjectName(QString("PalettePanel_%1").arg(title.simplified().replace(' ', '_')));
    
    // Set dock widget features
    setFeatures(QDockWidget::DockWidgetMovable | 
                QDockWidget::DockWidgetFloatable | 
                QDockWidget::DockWidgetClosable);
    
    // Set allowed dock areas
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    setupBaseUI();
    connectSignals();
}

void BasePalettePanel::setEditorController(RME::editor_logic::EditorController* controller) {
    m_editorController = controller;
    refreshContent();
}

void BasePalettePanel::setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager) {
    m_brushManager = manager;
    
    if (m_brushManager) {
        // Connect to brush manager signals for updates
        connect(m_brushManager, &RME::core::brush::BrushIntegrationManager::brushActivated,
                this, &BasePalettePanel::refreshContent);
        connect(m_brushManager, &RME::core::brush::BrushIntegrationManager::toolModeChanged,
                this, &BasePalettePanel::refreshContent);
    }
}

void BasePalettePanel::clearSelection() {
    // Base implementation - override in derived classes
    if (m_searchEdit) {
        m_searchEdit->clear();
    }
}

void BasePalettePanel::saveState() {
    QSettings settings;
    QString key = QString("PalettePanels/%1").arg(objectName());
    
    // Save search text
    if (m_searchEdit) {
        settings.setValue(key + "/searchText", m_searchEdit->text());
    }
    
    // Save dock state
    settings.setValue(key + "/geometry", saveGeometry());
    settings.setValue(key + "/visible", isVisible());
}

void BasePalettePanel::loadState() {
    QSettings settings;
    QString key = QString("PalettePanels/%1").arg(objectName());
    
    // Load search text
    if (m_searchEdit) {
        QString searchText = settings.value(key + "/searchText", QString()).toString();
        m_searchEdit->setText(searchText);
        if (!searchText.isEmpty()) {
            applySearchFilter(searchText);
        }
    }
    
    // Load dock state
    QByteArray geometry = settings.value(key + "/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    
    bool visible = settings.value(key + "/visible", true).toBool();
    setVisible(visible);
}

void BasePalettePanel::setSearchEnabled(bool enabled) {
    if (m_searchWidget) {
        m_searchWidget->setVisible(enabled);
    }
}

bool BasePalettePanel::isSearchEnabled() const {
    return m_searchWidget && m_searchWidget->isVisible();
}

void BasePalettePanel::onSearchTextChanged(const QString& text) {
    applySearchFilter(text);
    emit searchTextChanged(text);
}

void BasePalettePanel::onClearSearch() {
    if (m_searchEdit) {
        m_searchEdit->clear();
    }
}

void BasePalettePanel::onRefreshRequested() {
    refreshContent();
}

void BasePalettePanel::setupBaseUI() {
    // Create central widget
    m_centralWidget = new QWidget();
    setWidget(m_centralWidget);
    
    // Create main layout
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(2);
    
    // Setup search UI
    setupSearchUI();
    
    // Create content widget
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(2);
    
    m_mainLayout->addWidget(m_contentWidget);
    
    // Setup content UI (implemented by derived classes)
    setupContentUI();
}

void BasePalettePanel::setupSearchUI() {
    // Create search widget
    m_searchWidget = new QWidget();
    m_searchLayout = new QHBoxLayout(m_searchWidget);
    m_searchLayout->setContentsMargins(0, 0, 0, 0);
    m_searchLayout->setSpacing(2);
    
    // Create search edit
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText(tr("Search..."));
    m_searchEdit->setClearButtonEnabled(true);
    
    // Create clear button
    m_clearSearchButton = new QPushButton();
    m_clearSearchButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_clearSearchButton->setToolTip(tr("Clear search"));
    m_clearSearchButton->setMaximumSize(24, 24);
    m_clearSearchButton->setFlat(true);
    
    // Add to layout
    m_searchLayout->addWidget(m_searchEdit);
    m_searchLayout->addWidget(m_clearSearchButton);
    
    // Add to main layout
    m_mainLayout->addWidget(m_searchWidget);
    m_mainLayout->addWidget(createSeparator());
}

void BasePalettePanel::connectSignals() {
    if (m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged,
                this, &BasePalettePanel::onSearchTextChanged);
    }
    
    if (m_clearSearchButton) {
        connect(m_clearSearchButton, &QPushButton::clicked,
                this, &BasePalettePanel::onClearSearch);
    }
}

QWidget* BasePalettePanel::createSeparator() {
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setMaximumHeight(2);
    return separator;
}

QPushButton* BasePalettePanel::createToolButton(const QString& text, const QString& tooltip) {
    QPushButton* button = new QPushButton(text);
    if (!tooltip.isEmpty()) {
        button->setToolTip(tooltip);
    }
    button->setMaximumHeight(24);
    return button;
}

QLabel* BasePalettePanel::createSectionLabel(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setStyleSheet("QLabel { font-weight: bold; color: #666; }");
    label->setMaximumHeight(20);
    return label;
}

} // namespace palettes
} // namespace ui
} // namespace RME