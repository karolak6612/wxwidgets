#include "BrushContextMenu.h"
#include "BrushFilterManager.h"
#include "BrushOrganizer.h"
#include "core/brush/Brush.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

namespace RME {
namespace ui {
namespace palettes {

BrushContextMenu::BrushContextMenu(QWidget* parent)
    : QMenu(parent)
{
    setObjectName("BrushContextMenu");
    createBasicActions();
    
    qDebug() << "BrushContextMenu: Created";
}

BrushContextMenu::~BrushContextMenu()
{
    qDebug() << "BrushContextMenu: Destroyed";
}

void BrushContextMenu::setFilterManager(BrushFilterManager* filterManager)
{
    m_filterManager = filterManager;
}

void BrushContextMenu::setBrushOrganizer(BrushOrganizer* organizer)
{
    m_organizer = organizer;
}

void BrushContextMenu::showForBrush(RME::core::Brush* brush, const QPoint& position)
{
    if (!brush) {
        return;
    }
    
    m_currentBrush = brush;
    m_currentBrushes.clear();
    m_currentBrushes.append(brush);
    
    // Clear existing menu
    clear();
    
    // Setup menu for single brush
    setupSingleBrushMenu(brush);
    
    // Show menu
    popup(position);
    
    qDebug() << "BrushContextMenu: Showing menu for brush" << brush->getName();
}

void BrushContextMenu::showForMultipleBrushes(const QList<RME::core::Brush*>& brushes, const QPoint& position)
{
    if (brushes.isEmpty()) {
        return;
    }
    
    m_currentBrush = nullptr;
    m_currentBrushes = brushes;
    
    // Clear existing menu
    clear();
    
    // Setup menu for multiple brushes
    setupMultipleBrushMenu(brushes);
    
    // Show menu
    popup(position);
    
    qDebug() << "BrushContextMenu: Showing menu for" << brushes.size() << "brushes";
}

void BrushContextMenu::onActivateBrush()
{
    if (m_currentBrush) {
        emit brushActivated(m_currentBrush);
    }
}

void BrushContextMenu::onToggleFavorite()
{
    if (!m_organizer) {
        return;
    }
    
    if (m_currentBrush) {
        bool isFavorite = m_organizer->isFavorite(m_currentBrush);
        if (isFavorite) {
            m_organizer->removeFromFavorites(m_currentBrush);
        } else {
            m_organizer->addToFavorites(m_currentBrush);
        }
        emit favoriteToggled(m_currentBrush, !isFavorite);
    } else {
        // Handle multiple brushes
        for (RME::core::Brush* brush : m_currentBrushes) {
            if (brush) {
                m_organizer->addToFavorites(brush);
                emit favoriteToggled(brush, true);
            }
        }
    }
}

void BrushContextMenu::onAddToCategory()
{
    if (!m_organizer) {
        return;
    }
    
    // Get available categories
    QStringList categories = m_organizer->getCustomCategories();
    
    if (categories.isEmpty()) {
        QMessageBox::information(this, "No Categories", 
                                "No custom categories available. Create a category first.");
        return;
    }
    
    bool ok;
    QString category = QInputDialog::getItem(this, "Add to Category", 
                                           "Select category:", categories, 0, false, &ok);
    
    if (ok && !category.isEmpty()) {
        if (m_currentBrush) {
            m_organizer->addBrushToCategory(m_currentBrush, category);
            emit categoryChanged(m_currentBrush, category);
        } else {
            // Handle multiple brushes
            for (RME::core::Brush* brush : m_currentBrushes) {
                if (brush) {
                    m_organizer->addBrushToCategory(brush, category);
                    emit categoryChanged(brush, category);
                }
            }
        }
    }
}

void BrushContextMenu::onRemoveFromCategory()
{
    if (!m_organizer || !m_currentBrush) {
        return;
    }
    
    // Get categories for current brush
    QStringList categories = m_organizer->getCategoriesForBrush(m_currentBrush);
    
    if (categories.isEmpty()) {
        QMessageBox::information(this, "No Categories", 
                                "This brush is not in any custom categories.");
        return;
    }
    
    bool ok;
    QString category = QInputDialog::getItem(this, "Remove from Category", 
                                           "Select category to remove from:", categories, 0, false, &ok);
    
    if (ok && !category.isEmpty()) {
        m_organizer->removeBrushFromCategory(m_currentBrush, category);
        emit categoryChanged(m_currentBrush, QString());
    }
}

void BrushContextMenu::onCreateNewCategory()
{
    if (!m_organizer) {
        return;
    }
    
    bool ok;
    QString categoryName = QInputDialog::getText(this, "Create Category", 
                                               "Category name:", QLineEdit::Normal, QString(), &ok);
    
    if (ok && !categoryName.isEmpty()) {
        m_organizer->addCustomCategory(categoryName);
        
        // Add current brush(es) to new category
        if (m_currentBrush) {
            m_organizer->addBrushToCategory(m_currentBrush, categoryName);
            emit categoryChanged(m_currentBrush, categoryName);
        } else {
            for (RME::core::Brush* brush : m_currentBrushes) {
                if (brush) {
                    m_organizer->addBrushToCategory(brush, categoryName);
                    emit categoryChanged(brush, categoryName);
                }
            }
        }
    }
}

void BrushContextMenu::onAddTag()
{
    if (!m_filterManager) {
        return;
    }
    
    bool ok;
    QString tag = QInputDialog::getText(this, "Add Tag", 
                                      "Tag name:", QLineEdit::Normal, QString(), &ok);
    
    if (ok && !tag.isEmpty()) {
        if (m_currentBrush) {
            QStringList tags = m_filterManager->getTagsForBrush(m_currentBrush);
            if (!tags.contains(tag)) {
                tags.append(tag);
                m_filterManager->setTagsForBrush(m_currentBrush, tags);
                emit tagChanged(m_currentBrush, tags);
            }
        } else {
            // Handle multiple brushes
            for (RME::core::Brush* brush : m_currentBrushes) {
                if (brush) {
                    QStringList tags = m_filterManager->getTagsForBrush(brush);
                    if (!tags.contains(tag)) {
                        tags.append(tag);
                        m_filterManager->setTagsForBrush(brush, tags);
                        emit tagChanged(brush, tags);
                    }
                }
            }
        }
    }
}

void BrushContextMenu::onRemoveTag()
{
    if (!m_filterManager || !m_currentBrush) {
        return;
    }
    
    QStringList tags = m_filterManager->getTagsForBrush(m_currentBrush);
    
    if (tags.isEmpty()) {
        QMessageBox::information(this, "No Tags", 
                                "This brush has no tags.");
        return;
    }
    
    bool ok;
    QString tag = QInputDialog::getItem(this, "Remove Tag", 
                                      "Select tag to remove:", tags, 0, false, &ok);
    
    if (ok && !tag.isEmpty()) {
        tags.removeAll(tag);
        m_filterManager->setTagsForBrush(m_currentBrush, tags);
        emit tagChanged(m_currentBrush, tags);
    }
}

void BrushContextMenu::onShowProperties()
{
    if (m_currentBrush) {
        emit propertiesRequested(m_currentBrush);
    }
}

void BrushContextMenu::onCopyBrush()
{
    if (m_currentBrush) {
        emit brushCopied(m_currentBrush);
    }
}

void BrushContextMenu::onDeleteBrush()
{
    if (m_currentBrush) {
        int ret = QMessageBox::question(this, "Delete Brush", 
                                      QString("Are you sure you want to delete the brush '%1'?")
                                      .arg(m_currentBrush->getName()),
                                      QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            emit brushDeleted(m_currentBrush);
        }
    } else if (!m_currentBrushes.isEmpty()) {
        int ret = QMessageBox::question(this, "Delete Brushes", 
                                      QString("Are you sure you want to delete %1 brushes?")
                                      .arg(m_currentBrushes.size()),
                                      QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            for (RME::core::Brush* brush : m_currentBrushes) {
                if (brush) {
                    emit brushDeleted(brush);
                }
            }
        }
    }
}

void BrushContextMenu::onExportBrush()
{
    if (m_currentBrush) {
        emit brushExported(m_currentBrush);
    }
}

void BrushContextMenu::onShowUsageStatistics()
{
    if (m_currentBrush) {
        emit usageStatisticsRequested(m_currentBrush);
    }
}

void BrushContextMenu::setupSingleBrushMenu(RME::core::Brush* brush)
{
    // Basic actions
    addAction(m_activateAction);
    addAction(m_favoriteAction);
    addSeparator();
    
    // Category actions
    createCategoryActions(brush);
    addSeparator();
    
    // Tag actions
    createTagActions(brush);
    addSeparator();
    
    // Advanced actions
    createAdvancedActions(brush);
    
    // Update action states
    updateActionStates();
}

void BrushContextMenu::setupMultipleBrushMenu(const QList<RME::core::Brush*>& brushes)
{
    // For multiple brushes, show limited actions
    QAction* addToFavoritesAction = addAction("Add to Favorites");
    connect(addToFavoritesAction, &QAction::triggered, this, &BrushContextMenu::onToggleFavorite);
    
    addSeparator();
    
    QAction* addToCategoryAction = addAction("Add to Category...");
    connect(addToCategoryAction, &QAction::triggered, this, &BrushContextMenu::onAddToCategory);
    
    QAction* createCategoryAction = addAction("Create New Category...");
    connect(createCategoryAction, &QAction::triggered, this, &BrushContextMenu::onCreateNewCategory);
    
    addSeparator();
    
    QAction* addTagAction = addAction("Add Tag...");
    connect(addTagAction, &QAction::triggered, this, &BrushContextMenu::onAddTag);
    
    addSeparator();
    
    QAction* deleteAction = addAction("Delete Brushes...");
    connect(deleteAction, &QAction::triggered, this, &BrushContextMenu::onDeleteBrush);
}

void BrushContextMenu::createBasicActions()
{
    // Activate action
    m_activateAction = new QAction("Activate", this);
    connect(m_activateAction, &QAction::triggered, this, &BrushContextMenu::onActivateBrush);
    
    // Favorite action
    m_favoriteAction = new QAction("Add to Favorites", this);
    m_favoriteAction->setCheckable(true);
    connect(m_favoriteAction, &QAction::triggered, this, &BrushContextMenu::onToggleFavorite);
    
    // Properties action
    m_propertiesAction = new QAction("Properties...", this);
    connect(m_propertiesAction, &QAction::triggered, this, &BrushContextMenu::onShowProperties);
}

void BrushContextMenu::createCategoryActions(RME::core::Brush* brush)
{
    if (!m_organizer) {
        return;
    }
    
    // Category submenu
    m_categoryMenu = addMenu("Categories");
    
    // Add to category
    m_addToCategoryAction = m_categoryMenu->addAction("Add to Category...");
    connect(m_addToCategoryAction, &QAction::triggered, this, &BrushContextMenu::onAddToCategory);
    
    // Remove from category
    m_removeFromCategoryAction = m_categoryMenu->addAction("Remove from Category...");
    connect(m_removeFromCategoryAction, &QAction::triggered, this, &BrushContextMenu::onRemoveFromCategory);
    
    m_categoryMenu->addSeparator();
    
    // Create new category
    m_createCategoryAction = m_categoryMenu->addAction("Create New Category...");
    connect(m_createCategoryAction, &QAction::triggered, this, &BrushContextMenu::onCreateNewCategory);
    
    // Show current categories
    QStringList currentCategories = m_organizer->getCategoriesForBrush(brush);
    if (!currentCategories.isEmpty()) {
        m_categoryMenu->addSeparator();
        QAction* currentAction = m_categoryMenu->addAction(QString("Current: %1").arg(currentCategories.join(", ")));
        currentAction->setEnabled(false);
    }
}

void BrushContextMenu::createTagActions(RME::core::Brush* brush)
{
    if (!m_filterManager) {
        return;
    }
    
    // Tag submenu
    m_tagMenu = addMenu("Tags");
    
    // Add tag
    m_addTagAction = m_tagMenu->addAction("Add Tag...");
    connect(m_addTagAction, &QAction::triggered, this, &BrushContextMenu::onAddTag);
    
    // Remove tag
    m_removeTagAction = m_tagMenu->addAction("Remove Tag...");
    connect(m_removeTagAction, &QAction::triggered, this, &BrushContextMenu::onRemoveTag);
    
    // Show current tags
    QStringList currentTags = m_filterManager->getTagsForBrush(brush);
    if (!currentTags.isEmpty()) {
        m_tagMenu->addSeparator();
        QAction* currentAction = m_tagMenu->addAction(QString("Current: %1").arg(currentTags.join(", ")));
        currentAction->setEnabled(false);
    }
}

void BrushContextMenu::createAdvancedActions(RME::core::Brush* brush)
{
    // Properties
    addAction(m_propertiesAction);
    
    addSeparator();
    
    // Copy action
    m_copyAction = addAction("Copy");
    connect(m_copyAction, &QAction::triggered, this, &BrushContextMenu::onCopyBrush);
    
    // Export action
    m_exportAction = addAction("Export...");
    connect(m_exportAction, &QAction::triggered, this, &BrushContextMenu::onExportBrush);
    
    addSeparator();
    
    // Statistics action
    m_statisticsAction = addAction("Usage Statistics...");
    connect(m_statisticsAction, &QAction::triggered, this, &BrushContextMenu::onShowUsageStatistics);
    
    addSeparator();
    
    // Delete action
    m_deleteAction = addAction("Delete...");
    connect(m_deleteAction, &QAction::triggered, this, &BrushContextMenu::onDeleteBrush);
}

void BrushContextMenu::updateActionStates()
{
    if (!m_currentBrush) {
        return;
    }
    
    // Update favorite action
    if (m_favoriteAction && m_organizer) {
        bool isFavorite = m_organizer->isFavorite(m_currentBrush);
        m_favoriteAction->setText(isFavorite ? "Remove from Favorites" : "Add to Favorites");
        m_favoriteAction->setChecked(isFavorite);
    }
    
    // Update remove from category action
    if (m_removeFromCategoryAction && m_organizer) {
        QStringList categories = m_organizer->getCategoriesForBrush(m_currentBrush);
        m_removeFromCategoryAction->setEnabled(!categories.isEmpty());
    }
    
    // Update remove tag action
    if (m_removeTagAction && m_filterManager) {
        QStringList tags = m_filterManager->getTagsForBrush(m_currentBrush);
        m_removeTagAction->setEnabled(!tags.isEmpty());
    }
}

} // namespace palettes
} // namespace ui
} // namespace RME