#pragma once

#include <QMenu>
#include <QAction>
#include <QActionGroup>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

class BrushFilterManager;
class BrushOrganizer;

/**
 * @brief Context menu for brush operations
 * 
 * This class provides a comprehensive context menu for brush-related
 * operations including favorites, categories, properties, and actions.
 */
class BrushContextMenu : public QMenu
{
    Q_OBJECT

public:
    explicit BrushContextMenu(QWidget* parent = nullptr);
    ~BrushContextMenu();

    // Dependencies
    void setFilterManager(BrushFilterManager* filterManager);
    void setBrushOrganizer(BrushOrganizer* organizer);

    // Menu creation
    void showForBrush(RME::core::Brush* brush, const QPoint& position);
    void showForMultipleBrushes(const QList<RME::core::Brush*>& brushes, const QPoint& position);

public slots:
    void onActivateBrush();
    void onToggleFavorite();
    void onAddToCategory();
    void onRemoveFromCategory();
    void onCreateNewCategory();
    void onAddTag();
    void onRemoveTag();
    void onShowProperties();
    void onCopyBrush();
    void onDeleteBrush();
    void onExportBrush();
    void onShowUsageStatistics();

signals:
    void brushActivated(RME::core::Brush* brush);
    void favoriteToggled(RME::core::Brush* brush, bool isFavorite);
    void categoryChanged(RME::core::Brush* brush, const QString& category);
    void tagChanged(RME::core::Brush* brush, const QStringList& tags);
    void propertiesRequested(RME::core::Brush* brush);
    void brushCopied(RME::core::Brush* brush);
    void brushDeleted(RME::core::Brush* brush);
    void brushExported(RME::core::Brush* brush);
    void usageStatisticsRequested(RME::core::Brush* brush);

protected:
    void setupSingleBrushMenu(RME::core::Brush* brush);
    void setupMultipleBrushMenu(const QList<RME::core::Brush*>& brushes);
    void createBasicActions();
    void createCategoryActions(RME::core::Brush* brush);
    void createTagActions(RME::core::Brush* brush);
    void createAdvancedActions(RME::core::Brush* brush);
    void updateActionStates();

private:
    // Dependencies
    BrushFilterManager* m_filterManager = nullptr;
    BrushOrganizer* m_organizer = nullptr;

    // Current context
    RME::core::Brush* m_currentBrush = nullptr;
    QList<RME::core::Brush*> m_currentBrushes;

    // Basic actions
    QAction* m_activateAction = nullptr;
    QAction* m_favoriteAction = nullptr;
    QAction* m_propertiesAction = nullptr;

    // Category actions
    QMenu* m_categoryMenu = nullptr;
    QAction* m_addToCategoryAction = nullptr;
    QAction* m_removeFromCategoryAction = nullptr;
    QAction* m_createCategoryAction = nullptr;

    // Tag actions
    QMenu* m_tagMenu = nullptr;
    QAction* m_addTagAction = nullptr;
    QAction* m_removeTagAction = nullptr;

    // Advanced actions
    QAction* m_copyAction = nullptr;
    QAction* m_deleteAction = nullptr;
    QAction* m_exportAction = nullptr;
    QAction* m_statisticsAction = nullptr;

    // Separators
    QAction* m_separator1 = nullptr;
    QAction* m_separator2 = nullptr;
    QAction* m_separator3 = nullptr;
};

} // namespace palettes
} // namespace ui
} // namespace RME