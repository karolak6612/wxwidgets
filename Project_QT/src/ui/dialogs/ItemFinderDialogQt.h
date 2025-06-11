#ifndef ITEMFINDERDIALOGQT_H
#define ITEMFINDERDIALOGQT_H

#include <QDialog>
#include <QSet> // For storing parsed ID ranges

// Forward declarations for Qt classes
class QRadioButton;
class QSpinBox;
class QLineEdit;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QDialogButtonBox;
class QGroupBox;
class QPushButton;
class QLabel;
class QScrollArea;
class QTimer;

// Forward declarations for mapcore classes
namespace mapcore {
    class ItemManager;
    class ItemType;
    class SpriteManager; // Assuming it's a separate class, though prompt suggests itemManager->getSpriteManager()
}

// Define SLOTP constants if not available elsewhere (example values)
enum SlotPositions {
    SLOTP_NONE = 0,
    SLOTP_HEAD = 1 << 0,
    SLOTP_NECKLACE = 1 << 1,
    SLOTP_BACKPACK = 1 << 2,
    SLOTP_ARMOR = 1 << 3,
    SLOTP_LEGS = 1 << 4,
    SLOTP_FEET = 1 << 5,
    SLOTP_RING = 1 << 6,
    SLOTP_AMMO = 1 << 7
    // Add other slot positions as needed
};


class ItemFinderDialogQt : public QDialog
{
    Q_OBJECT

public:
    ItemFinderDialogQt(QWidget* parent, mapcore::ItemManager* itemManager, bool onlyPickupable = false);
    ~ItemFinderDialogQt() override;

    mapcore::ItemType* getSelectedItemType() const;

private slots:
    void onSearchModeChanged();
    void onFilterCriteriaChanged();
    void triggerRefresh();
    void performSearch();
    void onResultSelectionChanged();
    void onSearchByRangeToggled(bool checked);
    void onEnableIgnoredIdsToggled(bool checked);
    void handleOk();
    void handleCancel();

    // Custom slot for 3-state checkbox behavior if needed, or override nextCheckState
    // For simplicity, we'll assume standard tristate behavior is sufficient initially
    // or that QCheckBox::nextCheckState can be overridden in a derived class if complex cycling is a must.

private:
    void setupUi();
    void connectSignals();
    void updateControlsBasedOnSearchMode();
    QSet<int> parseIds(const QString& text); // Helper for parsing comma-separated IDs
    QList<QPair<int, int>> parseIdRanges(const QString& text); // Helper for parsing ID ranges like "100-200,300"

    // UI Elements

    // Search Mode
    QGroupBox* m_searchModeGroup;
    QRadioButton* m_searchByServerIdRadio;
    QRadioButton* m_searchByClientIdRadio;
    QRadioButton* m_searchByNameRadio;
    QRadioButton* m_searchByTypeRadio;
    QRadioButton* m_searchByPropertiesRadio;

    // Search Inputs
    QGroupBox* m_searchInputsGroup;
    QSpinBox* m_serverIdSpin;
    QCheckBox* m_invalidItemCheck; // Assuming ItemType has an isInvalid() or similar
    QSpinBox* m_clientIdSpin;
    QLineEdit* m_nameEdit;
    QCheckBox* m_searchByRangeCheck;
    QLineEdit* m_idRangeEdit;

    // Item Type
    QGroupBox* m_itemTypeGroup;
    // Example types - more should be added based on wx version's SearchItemType
    QRadioButton* m_typeDepotRadio;
    QRadioButton* m_typeMailboxRadio;
    QRadioButton* m_typeContainerRadio;
    QRadioButton* m_typeDoorRadio;
    QRadioButton* m_typeTeleportRadio;
    QRadioButton* m_typeBedRadio;
    QRadioButton* m_typeKeyRadio;
    QRadioButton* m_typePodiumRadio;
    // ... other type radios

    // Item Properties
    QGroupBox* m_itemPropertiesGroup;
    QScrollArea* m_propertiesScrollArea;
    QWidget* m_propertiesWidget; // Container for checkboxes inside scroll area
    // Example properties - more should be added
    QCheckBox* m_propUnpassableCheck;
    QCheckBox* m_propUnmovableCheck;
    QCheckBox* m_propBlockMissilesCheck;
    QCheckBox* m_propBlockPathfinderCheck; // Assuming this exists
    QCheckBox* m_propPickupableCheck;
    QCheckBox* m_propStackableCheck;
    QCheckBox* m_propRotatableCheck;
    QCheckBox* m_propHangableCheck;
    QCheckBox* m_propHookEastCheck;
    QCheckBox* m_propHookSouthCheck;
    QCheckBox* m_propHasElevationCheck;
    QCheckBox* m_propIgnoreLookCheck;
    QCheckBox* m_propHasLightCheck;
    QCheckBox* m_propFloorChangeCheck; // Assuming ItemType::isFloorChange()
    // Slot properties
    QCheckBox* m_propSlotHeadCheck;
    QCheckBox* m_propSlotNecklaceCheck;
    QCheckBox* m_propSlotBackpackCheck;
    QCheckBox* m_propSlotArmorCheck;
    QCheckBox* m_propSlotLegsCheck;
    QCheckBox* m_propSlotFeetCheck;
    QCheckBox* m_propSlotRingCheck;
    QCheckBox* m_propSlotAmmoCheck;
    // ... other property checkboxes

    // Filters
    QGroupBox* m_filtersGroup;
    QCheckBox* m_enableIgnoredIdsCheck;
    QLineEdit* m_ignoredIdsEdit;

    // Results
    QGroupBox* m_resultsGroup;
    QPushButton* m_refreshButton;
    QCheckBox* m_autoRefreshCheck;
    QLabel* m_maxResultsLabel;
    QSpinBox* m_maxResultsSpin;
    QListWidget* m_resultsListWidget;

    // Dialog Buttons
    QDialogButtonBox* m_buttonBox;

    // Data
    mapcore::ItemManager* m_itemManager;
    bool m_onlyPickupableInitial;
    mapcore::ItemType* m_selectedItemType;
    QTimer* m_refreshTimer;

    // Helper structure for property checkboxes
    struct PropertyCheck {
        QCheckBox* checkBox;
        // Could add a lambda or function pointer here for checking the property on ItemType
        // For example: std::function<bool(const mapcore::ItemType*)> checkFunc;
        // Or map to specific item flags/methods directly in performSearch
        enum class PropertyType {
            IsUnpassable, IsUnmovable, IsPickupable, SlotHead // etc.
        };
        PropertyType type; // Or string identifier
        QString objectName;
        QString tooltipText;
    };
    QList<PropertyCheck> m_propertyCheckboxesList;

    enum class SearchMode {
        ServerID,
        ClientID,
        Name,
        Type,
        Properties
    };
    SearchMode getCurrentSearchMode();


    // Constants for property check tooltips
    const QString PROP_TOOLTIP_IGNORE = "[ ] Ignore";
    const QString PROP_TOOLTIP_MUST_HAVE = "[V] Must Have";
    const QString PROP_TOOLTIP_MUST_NOT_HAVE = "[-] Must NOT Have";
    void setCheckboxTooltip(QCheckBox* cb, Qt::CheckState state);
    void cycleCheckboxState(QCheckBox* cb);

};

#endif // ITEMFINDERDIALOGQT_H
