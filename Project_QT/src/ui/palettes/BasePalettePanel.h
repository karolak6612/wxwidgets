#ifndef RME_BASE_PALETTE_PANEL_H
#define RME_BASE_PALETTE_PANEL_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QSplitter>

// Forward declarations
namespace RME {
namespace editor_logic {
    class EditorController;
}
namespace core {
namespace brush {
    class BrushIntegrationManager;
}
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Base class for all palette dock panels
 * 
 * This class provides common functionality for all palette panels including
 * search functionality, layout management, and integration with the editor
 * controller and brush system.
 */
class BasePalettePanel : public QDockWidget {
    Q_OBJECT

public:
    explicit BasePalettePanel(const QString& title, QWidget* parent = nullptr);
    ~BasePalettePanel() override = default;

    // Integration with editor system
    virtual void setEditorController(RME::editor_logic::EditorController* controller);
    virtual void setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager);

    // Panel management
    virtual void refreshContent() = 0;
    virtual void clearSelection();
    virtual void saveState();
    virtual void loadState();

    // Search functionality
    void setSearchEnabled(bool enabled);
    bool isSearchEnabled() const;

public Q_SLOTS: // Changed
    virtual void onSearchTextChanged(const QString& text);
    virtual void onClearSearch();
    virtual void onRefreshRequested();

Q_SIGNALS: // Changed
    void selectionChanged();
    void itemActivated();
    void searchTextChanged(const QString& text);

protected:
    // Core integration
    RME::editor_logic::EditorController* m_editorController = nullptr;
    RME::core::brush::BrushIntegrationManager* m_brushManager = nullptr;

    // UI components
    QWidget* m_centralWidget = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    
    // Search components
    QWidget* m_searchWidget = nullptr;
    QHBoxLayout* m_searchLayout = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_clearSearchButton = nullptr;
    
    // Content area
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    
    // Helper methods
    void setupBaseUI();
    void setupSearchUI();
    virtual void setupContentUI() = 0;
    virtual void connectSignals();
    virtual void applySearchFilter(const QString& text) = 0;
    
    // Utility methods
    QWidget* createSeparator();
    QPushButton* createToolButton(const QString& text, const QString& tooltip = QString());
    QLabel* createSectionLabel(const QString& text);
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_BASE_PALETTE_PANEL_H