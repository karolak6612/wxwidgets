#ifndef RME_ITEM_PROPERTIES_DIALOG_H
#define RME_ITEM_PROPERTIES_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>

// Forward declarations
namespace RME {
namespace core {
    class Item;
    class Position;
    class Map;
    class Tile;
    namespace assets {
        class ItemData;
    }
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Properties dialog for editing item properties
 * 
 * This dialog provides comprehensive item property editing with multiple tabs:
 * - General: Basic item properties (ID, name, action ID, etc.)
 * - Contents: Container item management (for containers)
 * - Advanced: Key-value attribute editing
 * 
 * The dialog adapts its interface based on the item type.
 */
class ItemPropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemPropertiesDialog(QWidget* parent, 
                                 const RME::core::Map* map, 
                                 const RME::core::Tile* tileContext, 
                                 RME::core::Item* itemCopy);
    ~ItemPropertiesDialog() override = default;

    // Result access
    bool wasModified() const { return m_wasModified; }
    RME::core::Item* getModifiedItem() const { return m_itemCopy; }

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onGeneralPropertyChanged();
    void onContentsChanged();
    void onAttributeChanged();
    void onAddAttribute();
    void onRemoveAttribute();
    void onAddContainerItem();
    void onRemoveContainerItem();
    void onResetToDefaults();

signals:
    void itemModified(RME::core::Item* item);

private:
    // Core data
    RME::core::Item* m_itemCopy = nullptr;
    RME::core::Item* m_originalItem = nullptr; // Backup for cancel
    const RME::core::Map* m_map = nullptr;
    const RME::core::Tile* m_tileContext = nullptr;
    const RME::core::assets::ItemData* m_itemData = nullptr;
    bool m_wasModified = false;

    // UI components
    QTabWidget* m_tabWidget = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;

    // General tab
    QWidget* m_generalTab = nullptr;
    QFormLayout* m_generalLayout = nullptr;
    QLineEdit* m_itemIdEdit = nullptr;
    QLineEdit* m_itemNameEdit = nullptr;
    QSpinBox* m_actionIdSpinBox = nullptr;
    QSpinBox* m_uniqueIdSpinBox = nullptr;
    QSpinBox* m_countSpinBox = nullptr;
    QLineEdit* m_textEdit = nullptr;
    QLineEdit* m_descriptionEdit = nullptr;
    
    // Dynamic type-specific controls area
    QWidget* m_typeSpecificWidgetArea = nullptr;
    QVBoxLayout* m_typeSpecificLayout = nullptr;
    
    // Type-specific controls (created dynamically)
    QComboBox* m_liquidTypeCombo = nullptr;
    QSpinBox* m_doorIdSpin = nullptr;
    QComboBox* m_depotTownCombo = nullptr;
    QSpinBox* m_destXSpin = nullptr;
    QSpinBox* m_destYSpin = nullptr;
    QSpinBox* m_destZSpin = nullptr;
    QComboBox* m_podiumDirectionCombo = nullptr;
    QCheckBox* m_showOutfitCheck = nullptr;
    QCheckBox* m_showMountCheck = nullptr;
    QCheckBox* m_showPlatformCheck = nullptr;
    QSpinBox* m_lookTypeSpin = nullptr;
    QSpinBox* m_lookHeadSpin = nullptr;
    QSpinBox* m_lookBodySpin = nullptr;
    QSpinBox* m_lookLegsSpin = nullptr;
    QSpinBox* m_lookFeetSpin = nullptr;
    QSpinBox* m_lookAddonSpin = nullptr;
    QSpinBox* m_lookMountSpin = nullptr;
    QSpinBox* m_lookMountHeadSpin = nullptr;
    QSpinBox* m_lookMountBodySpin = nullptr;
    QSpinBox* m_lookMountLegsSpin = nullptr;
    QSpinBox* m_lookMountFeetSpin = nullptr;
    QSpinBox* m_tierSpin = nullptr;
    
    // Contents tab (for containers)
    QWidget* m_contentsTab = nullptr;
    QVBoxLayout* m_contentsLayout = nullptr;
    QListView* m_contentsView = nullptr;
    QStandardItemModel* m_contentsModel = nullptr;
    QPushButton* m_addItemButton = nullptr;
    QPushButton* m_editItemButton = nullptr;
    QPushButton* m_removeItemButton = nullptr;
    QLabel* m_containerInfoLabel = nullptr;
    
    // Advanced tab
    QWidget* m_advancedTab = nullptr;
    QVBoxLayout* m_advancedLayout = nullptr;
    QTableWidget* m_attributesTable = nullptr; // 3 columns: Key, Type, Value
    QPushButton* m_addAttributeButton = nullptr;
    QPushButton* m_removeAttributeButton = nullptr;
    QPushButton* m_resetButton = nullptr;
    
    // Helper methods
    void setupUI();
    void setupGeneralTab();
    void setupContentsTab();
    void setupAdvancedTab();
    void setupButtonBox();
    
    void loadItemData();
    void loadGeneralProperties();
    void loadContentsData();
    void loadAdvancedAttributes();
    
    void saveItemData();
    void saveGeneralProperties();
    void saveTypeSpecificProperties();
    void saveContentsData();
    void saveAdvancedAttributes();
    
    void updateDynamicControls();
    void createTypeSpecificControls();
    void clearTypeSpecificControls();
    void updateContentsTab();
    void updateTabVisibility();
    
    // Item type specific methods
    bool isContainer() const;
    bool isDoor() const;
    bool isDepot() const;
    bool isTeleport() const;
    bool isPodium() const;
    bool isFluidContainer() const;
    bool isSplash() const;
    bool isTiered() const;
    bool hasText() const;
    bool hasActionId() const;
    
    // Validation
    bool validateInput();
    void markAsModified();
    
    // Container management
    void updateContainerInfo();
    
    // Attribute management
    void addAttribute(const QString& key, const QString& value);
    void removeAttribute(int row);
    QTableWidgetItem* createAttributeItem(const QString& text);
    
    // Utility methods
    void connectSignals();
    void disconnectSignals();
    void createBackup();
    void restoreBackup();
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_ITEM_PROPERTIES_DIALOG_H