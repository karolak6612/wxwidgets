#ifndef RME_RAW_ITEMS_PALETTE_TAB_H
#define RME_RAW_ITEMS_PALETTE_TAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QXmlStreamReader>
#include <QStringList>
#include <QHash>
#include <QIcon>

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IClientDataService.h"

// Forward declarations
namespace RME {
namespace core {
    namespace assets { class ItemDatabase; }
    namespace brush { class BrushStateManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Data structure for a RAW item entry
 */
struct RawItemEntry {
    quint16 itemId = 0;
    QString name;
    QString tileset;
    QIcon icon;
    
    RawItemEntry() = default;
    RawItemEntry(quint16 id, const QString& itemName, const QString& tilesetName)
        : itemId(id), name(itemName), tileset(tilesetName) {}
};

/**
 * @brief RAW Items palette tab for the main palette system
 * 
 * Provides UI for browsing and selecting RAW items from XML definitions.
 * RAW items are individual items that can be placed directly on the map
 * without special brush behavior.
 */
class RawItemsPaletteTab : public QWidget {
    Q_OBJECT

public:
    explicit RawItemsPaletteTab(
        RME::core::IBrushStateService* brushStateService,
        RME::core::IClientDataService* clientDataService,
        QWidget* parent = nullptr
    );
    ~RawItemsPaletteTab() override = default;

    // Integration with core systems
    void setItemDatabase(RME::core::assets::ItemDatabase* itemDatabase);
    void setBrushStateManager(RME::core::brush::BrushStateManager* brushManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);

    // Public interface
    void refreshContent();
    void loadRawItemsFromXml(const QString& xmlFilePath);
    quint16 getSelectedItemId() const;
    QString getSelectedTileset() const;

public slots:
    void onTilesetSelectionChanged();
    void onItemSelectionChanged();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);
    void onClearSearch();

signals:
    void itemSelected(quint16 itemId);
    void rawBrushActivated(quint16 itemId);

private:
    void setupUI();
    void connectSignals();
    void parseRawPaletteXml(const QString& xmlFilePath);
    void populateTilesetCombo();
    void updateItemList();
    void updateItemList(const QString& tilesetFilter);
    void applySearchFilter(const QString& searchText);
    void activateRawBrush(quint16 itemId);
    
    // Helper methods
    QIcon getItemIcon(quint16 itemId) const;
    QString getItemName(quint16 itemId) const;
    QString formatItemListEntry(const RawItemEntry& entry) const;
    void addItemsFromRange(quint16 fromId, quint16 toId, const QString& tileset);
    void addSingleItem(quint16 itemId, const QString& tileset);

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QGroupBox* m_filterGroup = nullptr;
    QHBoxLayout* m_filterLayout = nullptr;
    QComboBox* m_tilesetCombo = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_clearSearchButton = nullptr;
    
    QGroupBox* m_itemsGroup = nullptr;
    QVBoxLayout* m_itemsLayout = nullptr;
    QListWidget* m_itemList = nullptr;
    QLabel* m_itemCountLabel = nullptr;
    
    QGroupBox* m_infoGroup = nullptr;
    QVBoxLayout* m_infoLayout = nullptr;
    QLabel* m_selectedItemLabel = nullptr;
    QLabel* m_itemDetailsLabel = nullptr;

    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IClientDataService* m_clientDataService;
    
    // Core system integration (legacy)
    RME::core::assets::ItemDatabase* m_itemDatabase = nullptr;
    RME::core::brush::BrushStateManager* m_brushStateManager = nullptr;
    RME::core::editor::EditorControllerInterface* m_editorController = nullptr;

    // Data
    QList<RawItemEntry> m_rawItems;
    QStringList m_tilesets;
    QHash<QString, QList<RawItemEntry>> m_itemsByTileset;
    QList<RawItemEntry> m_filteredItems;
    
    // State
    QString m_currentTileset;
    QString m_currentSearchText;
    bool m_updatingUI = false;
    
    // Constants
    static const QString ALL_TILESETS_TEXT;
    static const QString XML_FILE_PATH;
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_RAW_ITEMS_PALETTE_TAB_H