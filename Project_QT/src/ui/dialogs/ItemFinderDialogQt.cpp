#include "ItemFinderDialogQt.h"

// Qt includes
#include <QtWidgets>

// mapcore includes (assuming paths)
#include "core/ItemManager.h"
#include "core/ItemType.h"
#include "core/SpriteManager.h" // For item sprites, assuming ItemManager::getSpriteManager()
#include "core/Sprite.h"      // For ItemType::getSprite() or similar to get sprite ID

// Standard library includes
#include <algorithm> // For std::sort, std::remove_if, etc. if needed for parsing

// Define reasonable defaults if ItemManager doesn't provide max IDs
const int DEFAULT_MAX_SERVER_ID = 65535;
const int DEFAULT_MAX_SPRITE_ID = 32767; // Typical client sprite limit

// --- Helper class for tristate checkbox cycling ---
// (Alternative to overriding nextCheckState directly in ItemFinderDialogQt if preferred)
class TriStateCheckBox : public QCheckBox {
public:
    TriStateCheckBox(const QString& text, QWidget* parent = nullptr) : QCheckBox(text, parent) {
        setTristate(true);
    }
protected:
    void nextCheckState() override {
        if (checkState() == Qt::Unchecked) {
            setCheckState(Qt::Checked);
        } else if (checkState() == Qt::Checked) {
            setCheckState(Qt::PartiallyChecked);
        } else {
            setCheckState(Qt::Unchecked);
        }
        // Update tooltip based on new state if ItemFinderDialogQt::setCheckboxTooltip is accessible
        // Or emit a signal to be handled by the dialog
    }
};


ItemFinderDialogQt::ItemFinderDialogQt(QWidget* parent, mapcore::ItemManager* itemManager, bool onlyPickupable)
    : QDialog(parent),
      m_itemManager(itemManager),
      m_onlyPickupableInitial(onlyPickupable),
      m_selectedItemType(nullptr)
{
    Q_ASSERT(m_itemManager);

    setupUi();
    connectSignals();

    // Initial state based on constructor args
    if (m_onlyPickupableInitial && m_propPickupableCheck) {
        m_propPickupableCheck->setCheckState(Qt::Checked);
        m_propPickupableCheck->setEnabled(false);
        setCheckboxTooltip(m_propPickupableCheck, Qt::Checked);
    }

    m_searchByServerIdRadio->setChecked(true); // Default search mode
    updateControlsBasedOnSearchMode(); // Set initial enabled/disabled states

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &ItemFinderDialogQt::performSearch);

    setWindowTitle(tr("Find Item"));
    // Consider setting a default size or restoring geometry from settings
}

ItemFinderDialogQt::~ItemFinderDialogQt()
{
    // Qt handles child widget deletion
}

void ItemFinderDialogQt::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Search Mode GroupBox ---
    m_searchModeGroup = new QGroupBox(tr("Search Mode"));
    m_searchModeGroup->setObjectName("searchModeGroup");
    QHBoxLayout* searchModeLayout = new QHBoxLayout();
    m_searchByServerIdRadio = new QRadioButton(tr("By Server ID"));
    m_searchByServerIdRadio->setObjectName("searchByServerIdRadio");
    m_searchByClientIdRadio = new QRadioButton(tr("By Client ID"));
    m_searchByClientIdRadio->setObjectName("searchByClientIdRadio");
    m_searchByNameRadio = new QRadioButton(tr("By Name"));
    m_searchByNameRadio->setObjectName("searchByNameRadio");
    m_searchByTypeRadio = new QRadioButton(tr("By Type"));
    m_searchByTypeRadio->setObjectName("searchByTypeRadio");
    m_searchByPropertiesRadio = new QRadioButton(tr("By Properties"));
    m_searchByPropertiesRadio->setObjectName("searchByPropertiesRadio");
    searchModeLayout->addWidget(m_searchByServerIdRadio);
    searchModeLayout->addWidget(m_searchByClientIdRadio);
    searchModeLayout->addWidget(m_searchByNameRadio);
    searchModeLayout->addWidget(m_searchByTypeRadio);
    searchModeLayout->addWidget(m_searchByPropertiesRadio);
    m_searchModeGroup->setLayout(searchModeLayout);
    mainLayout->addWidget(m_searchModeGroup);

    // --- Search Inputs GroupBox ---
    m_searchInputsGroup = new QGroupBox(tr("Search Inputs"));
    m_searchInputsGroup->setObjectName("searchInputsGroup");
    QFormLayout* searchInputsLayout = new QFormLayout();
    m_serverIdSpin = new QSpinBox();
    m_serverIdSpin->setObjectName("serverIdSpin");
    m_serverIdSpin->setRange(100, m_itemManager->getMaxServerId() > 0 ? m_itemManager->getMaxServerId() : DEFAULT_MAX_SERVER_ID);
    m_invalidItemCheck = new QCheckBox(tr("Invalid Item")); // Assuming ItemType::isInvalid() or similar logic
    m_invalidItemCheck->setObjectName("invalidItemCheck");
    QHBoxLayout* serverIdLayout = new QHBoxLayout();
    serverIdLayout->addWidget(m_serverIdSpin);
    serverIdLayout->addWidget(m_invalidItemCheck);
    searchInputsLayout->addRow(tr("Server ID:"), serverIdLayout);

    m_clientIdSpin = new QSpinBox();
    m_clientIdSpin->setObjectName("clientIdSpin");
    m_clientIdSpin->setRange(1, m_itemManager->getMaxSpriteId() > 0 ? m_itemManager->getMaxSpriteId() : DEFAULT_MAX_SPRITE_ID);
    searchInputsLayout->addRow(tr("Client ID:"), m_clientIdSpin);

    m_nameEdit = new QLineEdit();
    m_nameEdit->setObjectName("nameEdit");
    searchInputsLayout->addRow(tr("Name:"), m_nameEdit);

    m_searchByRangeCheck = new QCheckBox(tr("Search by Range"));
    m_searchByRangeCheck->setObjectName("searchByRangeCheck");
    m_idRangeEdit = new QLineEdit();
    m_idRangeEdit->setObjectName("idRangeEdit");
    m_idRangeEdit->setPlaceholderText(tr("e.g., 2222,2244-2266"));
    m_idRangeEdit->setToolTip(tr("Enter comma-separated IDs or ID ranges (e.g., 100-200, 305, 400-410)"));
    m_idRangeEdit->setEnabled(false);
    searchInputsLayout->addRow(m_searchByRangeCheck, m_idRangeEdit);
    m_searchInputsGroup->setLayout(searchInputsLayout);
    mainLayout->addWidget(m_searchInputsGroup);

    // --- Item Type GroupBox ---
    m_itemTypeGroup = new QGroupBox(tr("Item Type"));
    m_itemTypeGroup->setObjectName("itemTypeGroup");
    QGridLayout* itemTypeLayout = new QGridLayout(); // Or QVBoxLayout
    // TODO: Populate with QRadioButtons for each SearchItemType from wx version
    // Example:
    m_typeDepotRadio = new QRadioButton(tr("Depot"));       m_typeDepotRadio->setObjectName("typeDepotRadio");
    m_typeMailboxRadio = new QRadioButton(tr("Mailbox"));   m_typeMailboxRadio->setObjectName("typeMailboxRadio");
    m_typeContainerRadio = new QRadioButton(tr("Container")); m_typeContainerRadio->setObjectName("typeContainerRadio");
    m_typeDoorRadio = new QRadioButton(tr("Door"));         m_typeDoorRadio->setObjectName("typeDoorRadio");
    m_typeTeleportRadio = new QRadioButton(tr("Teleport")); m_typeTeleportRadio->setObjectName("typeTeleportRadio");
    m_typeBedRadio = new QRadioButton(tr("Bed"));           m_typeBedRadio->setObjectName("typeBedRadio");
    m_typeKeyRadio = new QRadioButton(tr("Key"));           m_typeKeyRadio->setObjectName("typeKeyRadio");
    m_typePodiumRadio = new QRadioButton(tr("Podium"));     m_typePodiumRadio->setObjectName("typePodiumRadio");

    itemTypeLayout->addWidget(m_typeDepotRadio, 0, 0);
    itemTypeLayout->addWidget(m_typeMailboxRadio, 0, 1);
    itemTypeLayout->addWidget(m_typeContainerRadio, 1, 0);
    itemTypeLayout->addWidget(m_typeDoorRadio, 1, 1);
    itemTypeLayout->addWidget(m_typeTeleportRadio, 2, 0);
    itemTypeLayout->addWidget(m_typeBedRadio, 2, 1);
    itemTypeLayout->addWidget(m_typeKeyRadio, 3, 0);
    itemTypeLayout->addWidget(m_typePodiumRadio, 3, 1);
    // ... add other types
    m_itemTypeGroup->setLayout(itemTypeLayout);
    mainLayout->addWidget(m_itemTypeGroup);

    // --- Item Properties GroupBox ---
    m_itemPropertiesGroup = new QGroupBox(tr("Item Properties"));
    m_itemPropertiesGroup->setObjectName("itemPropertiesGroup");
    m_propertiesScrollArea = new QScrollArea();
    m_propertiesScrollArea->setObjectName("propertiesScrollArea");
    m_propertiesScrollArea->setWidgetResizable(true);
    m_propertiesWidget = new QWidget();
    m_propertiesWidget->setObjectName("propertiesWidget");
    QGridLayout* propertiesLayout = new QGridLayout(m_propertiesWidget); // Use QGridLayout for better alignment
    m_propertiesScrollArea->setWidget(m_propertiesWidget);
    QVBoxLayout* itemPropsOuterLayout = new QVBoxLayout();
    itemPropsOuterLayout->addWidget(m_propertiesScrollArea);
    m_itemPropertiesGroup->setLayout(itemPropsOuterLayout);

    m_propertyCheckboxesList.clear();
    auto addPropertyCheck = [&](const QString& label, const QString& objName, const QString& tooltipBase) {
        // QCheckBox* cb = new QCheckBox(label, m_propertiesWidget);
        TriStateCheckBox* cb = new TriStateCheckBox(label, m_propertiesWidget); // Using custom for cycling
        cb->setObjectName(objName);
        cb->setTristate(true);
        cb->setCheckState(Qt::Unchecked);
        setCheckboxTooltip(cb, Qt::Unchecked); // Initial tooltip
        // Store it for performSearch
        m_propertyCheckboxesList.append({cb, {}, objName, tooltipBase}); // PropertyType needs to be mapped
        return cb;
    };

    // Populate properties - names should match wx version's flags
    // Column 1
    m_propUnpassableCheck = addPropertyCheck(tr("Unpassable"), "propUnpassableCheck", "Unpassable");
    propertiesLayout->addWidget(m_propUnpassableCheck, 0, 0);
    m_propUnmovableCheck = addPropertyCheck(tr("Unmovable"), "propUnmovableCheck", "Unmovable");
    propertiesLayout->addWidget(m_propUnmovableCheck, 1, 0);
    m_propBlockMissilesCheck = addPropertyCheck(tr("Block Missiles"), "propBlockMissilesCheck", "Block Missiles");
    propertiesLayout->addWidget(m_propBlockMissilesCheck, 2, 0);
    m_propBlockPathfinderCheck = addPropertyCheck(tr("Block Pathfinder"), "propBlockPathfinderCheck", "Block Pathfinder");
    propertiesLayout->addWidget(m_propBlockPathfinderCheck, 3, 0);
    m_propPickupableCheck = addPropertyCheck(tr("Pickupable"), "propPickupableCheck", "Pickupable");
    propertiesLayout->addWidget(m_propPickupableCheck, 4, 0);
    m_propStackableCheck = addPropertyCheck(tr("Stackable"), "propStackableCheck", "Stackable");
    propertiesLayout->addWidget(m_propStackableCheck, 5, 0);
    m_propRotatableCheck = addPropertyCheck(tr("Rotatable"), "propRotatableCheck", "Rotatable");
    propertiesLayout->addWidget(m_propRotatableCheck, 6, 0);

    // Column 2
    m_propHangableCheck = addPropertyCheck(tr("Hangable"), "propHangableCheck", "Hangable");
    propertiesLayout->addWidget(m_propHangableCheck, 0, 1);
    m_propHookEastCheck = addPropertyCheck(tr("Hook East"), "propHookEastCheck", "Hook East");
    propertiesLayout->addWidget(m_propHookEastCheck, 1, 1);
    m_propHookSouthCheck = addPropertyCheck(tr("Hook South"), "propHookSouthCheck", "Hook South");
    propertiesLayout->addWidget(m_propHookSouthCheck, 2, 1);
    m_propHasElevationCheck = addPropertyCheck(tr("Has Elevation"), "propHasElevationCheck", "Has Elevation");
    propertiesLayout->addWidget(m_propHasElevationCheck, 3, 1);
    m_propIgnoreLookCheck = addPropertyCheck(tr("Ignore Look"), "propIgnoreLookCheck", "Ignore Look");
    propertiesLayout->addWidget(m_propIgnoreLookCheck, 4, 1);
    m_propHasLightCheck = addPropertyCheck(tr("Has Light"), "propHasLightCheck", "Has Light");
    propertiesLayout->addWidget(m_propHasLightCheck, 5, 1);
    m_propFloorChangeCheck = addPropertyCheck(tr("Floor Change"), "propFloorChangeCheck", "Floor Change");
    propertiesLayout->addWidget(m_propFloorChangeCheck, 6, 1);

    // Column 3 (Slots)
    m_propSlotHeadCheck = addPropertyCheck(tr("Slot: Head"), "propSlotHeadCheck", "Slot Head");
    propertiesLayout->addWidget(m_propSlotHeadCheck, 0, 2);
    m_propSlotNecklaceCheck = addPropertyCheck(tr("Slot: Necklace"), "propSlotNecklaceCheck", "Slot Necklace");
    propertiesLayout->addWidget(m_propSlotNecklaceCheck, 1, 2);
    m_propSlotBackpackCheck = addPropertyCheck(tr("Slot: Backpack"), "propSlotBackpackCheck", "Slot Backpack");
    propertiesLayout->addWidget(m_propSlotBackpackCheck, 2, 2);
    m_propSlotArmorCheck = addPropertyCheck(tr("Slot: Armor"), "propSlotArmorCheck", "Slot Armor");
    propertiesLayout->addWidget(m_propSlotArmorCheck, 3, 2);
    m_propSlotLegsCheck = addPropertyCheck(tr("Slot: Legs"), "propSlotLegsCheck", "Slot Legs");
    propertiesLayout->addWidget(m_propSlotLegsCheck, 4, 2);
    m_propSlotFeetCheck = addPropertyCheck(tr("Slot: Feet"), "propSlotFeetCheck", "Slot Feet");
    propertiesLayout->addWidget(m_propSlotFeetCheck, 5, 2);
    m_propSlotRingCheck = addPropertyCheck(tr("Slot: Ring"), "propSlotRingCheck", "Slot Ring");
    propertiesLayout->addWidget(m_propSlotRingCheck, 6, 2);
    m_propSlotAmmoCheck = addPropertyCheck(tr("Slot: Ammo"), "propSlotAmmoCheck", "Slot Ammo");
    propertiesLayout->addWidget(m_propSlotAmmoCheck, 7, 2); // Example of adding another row if needed

    mainLayout->addWidget(m_itemPropertiesGroup);


    // --- Filters GroupBox ---
    m_filtersGroup = new QGroupBox(tr("Filters"));
    m_filtersGroup->setObjectName("filtersGroup");
    QFormLayout* filtersLayout = new QFormLayout();
    m_enableIgnoredIdsCheck = new QCheckBox(tr("Enable Ignored IDs"));
    m_enableIgnoredIdsCheck->setObjectName("enableIgnoredIdsCheck");
    m_ignoredIdsEdit = new QLineEdit();
    m_ignoredIdsEdit->setObjectName("ignoredIdsEdit");
    m_ignoredIdsEdit->setPlaceholderText(tr("e.g., 1212,1256-1261"));
    m_ignoredIdsEdit->setToolTip(tr("Enter comma-separated Server IDs or ID ranges to ignore."));
    m_ignoredIdsEdit->setEnabled(false);
    filtersLayout->addRow(m_enableIgnoredIdsCheck, m_ignoredIdsEdit);
    m_filtersGroup->setLayout(filtersLayout);
    mainLayout->addWidget(m_filtersGroup);

    // --- Results GroupBox ---
    m_resultsGroup = new QGroupBox(tr("Results"));
    m_resultsGroup->setObjectName("resultsGroup");
    QVBoxLayout* resultsLayout = new QVBoxLayout();
    QHBoxLayout* resultsControlsLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("Refresh"));
    m_refreshButton->setObjectName("refreshButton");
    m_autoRefreshCheck = new QCheckBox(tr("Auto Refresh"));
    m_autoRefreshCheck->setObjectName("autoRefreshCheck");
    m_autoRefreshCheck->setChecked(true);
    m_maxResultsLabel = new QLabel(tr("Max Results:"));
    m_maxResultsSpin = new QSpinBox();
    m_maxResultsSpin->setObjectName("maxResultsSpin");
    m_maxResultsSpin->setRange(10, 1000);
    m_maxResultsSpin->setValue(100); // Default max results
    resultsControlsLayout->addWidget(m_refreshButton);
    resultsControlsLayout->addWidget(m_autoRefreshCheck);
    resultsControlsLayout->addStretch();
    resultsControlsLayout->addWidget(m_maxResultsLabel);
    resultsControlsLayout->addWidget(m_maxResultsSpin);
    resultsLayout->addLayout(resultsControlsLayout);

    m_resultsListWidget = new QListWidget();
    m_resultsListWidget->setObjectName("resultsListWidget");
    m_resultsListWidget->setViewMode(QListView::IconMode);
    m_resultsListWidget->setIconSize(QSize(32, 32)); // Default icon size, adjust as needed
    m_resultsListWidget->setWordWrap(true);
    m_resultsListWidget->setResizeMode(QListView::Adjust); // Adjust items on resize
    resultsLayout->addWidget(m_resultsListWidget);
    m_resultsGroup->setLayout(resultsLayout);
    mainLayout->addWidget(m_resultsGroup);

    // --- Dialog Buttons ---
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->setObjectName("buttonBox");
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false); // OK initially disabled
    mainLayout->addWidget(m_buttonBox);
}

void ItemFinderDialogQt::connectSignals()
{
    // Search Mode
    connect(m_searchByServerIdRadio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onSearchModeChanged);
    connect(m_searchByClientIdRadio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onSearchModeChanged);
    connect(m_searchByNameRadio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onSearchModeChanged);
    connect(m_searchByTypeRadio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onSearchModeChanged);
    connect(m_searchByPropertiesRadio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onSearchModeChanged);

    // Search Inputs
    connect(m_serverIdSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    connect(m_invalidItemCheck, &QCheckBox::toggled, this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    connect(m_clientIdSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    connect(m_searchByRangeCheck, &QCheckBox::toggled, this, &ItemFinderDialogQt::onSearchByRangeToggled);
    connect(m_idRangeEdit, &QLineEdit::textChanged, this, &ItemFinderDialogQt::onFilterCriteriaChanged);

    // Item Types
    QList<QRadioButton*> typeRadios = m_itemTypeGroup->findChildren<QRadioButton*>();
    for (QRadioButton* radio : typeRadios) {
        connect(radio, &QRadioButton::toggled, this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    }

    // Item Properties
    for (const auto& propCheck : m_propertyCheckboxesList) {
        connect(propCheck.checkBox, &QCheckBox::stateChanged, this, &ItemFinderDialogQt::onFilterCriteriaChanged);
        // Connect custom cycling behavior if using TriStateCheckBox and it emits a signal, or if QCheckBox::clicked is used
        // For now, assume TriStateCheckBox's override of nextCheckState is sufficient.
        // If not using TriStateCheckBox, this is where you'd connect clicked() to a slot that calls cycleCheckboxState.
         connect(propCheck.checkBox, &QCheckBox::clicked, this, [this, cb=propCheck.checkBox](){
            // This ensures the tooltip updates after the state has been changed by TriStateCheckBox's nextCheckState
            // or if we manually cycle it here (if not using TriStateCheckBox subclass)
            // If NOT using TriStateCheckBox subclass, call cycleCheckboxState(cb) here FIRST.
             setCheckboxTooltip(cb, cb->checkState());
         });
    }


    // Filters
    connect(m_enableIgnoredIdsCheck, &QCheckBox::toggled, this, &ItemFinderDialogQt::onEnableIgnoredIdsToggled);
    connect(m_ignoredIdsEdit, &QLineEdit::textChanged, this, &ItemFinderDialogQt::onFilterCriteriaChanged);

    // Results
    connect(m_refreshButton, &QPushButton::clicked, this, &ItemFinderDialogQt::performSearch);
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &ItemFinderDialogQt::onFilterCriteriaChanged); // Can also trigger refresh
    connect(m_maxResultsSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ItemFinderDialogQt::onFilterCriteriaChanged);
    connect(m_resultsListWidget, &QListWidget::currentItemChanged, this, &ItemFinderDialogQt::onResultSelectionChanged);
    connect(m_resultsListWidget, &QListWidget::itemDoubleClicked, this, &ItemFinderDialogQt::handleOk);


    // Dialog Buttons
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ItemFinderDialogQt::handleOk);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ItemFinderDialogQt::handleCancel);
}


void ItemFinderDialogQt::setCheckboxTooltip(QCheckBox* cb, Qt::CheckState state) {
    if (!cb) return;
    QString baseText = cb->text().remove('&'); // Remove mnemonics if any
    for(const auto& pCheck : m_propertyCheckboxesList) {
        if(pCheck.checkBox == cb) {
            baseText = pCheck.tooltipText; // Use the stored base tooltip
            break;
        }
    }

    if (state == Qt::Unchecked) {
        cb->setToolTip(QString("%1: %2").arg(baseText, PROP_TOOLTIP_IGNORE));
    } else if (state == Qt::Checked) {
        cb->setToolTip(QString("%1: %2").arg(baseText, PROP_TOOLTIP_MUST_HAVE));
    } else { // PartiallyChecked
        cb->setToolTip(QString("%1: %2").arg(baseText, PROP_TOOLTIP_MUST_NOT_HAVE));
    }
}

void ItemFinderDialogQt::cycleCheckboxState(QCheckBox* cb) {
    if (!cb) return;
    // This is the manual cycle logic if not using TriStateCheckBox subclass
    if (cb->checkState() == Qt::Unchecked) {
        cb->setCheckState(Qt::Checked);
    } else if (cb->checkState() == Qt::Checked) {
        cb->setCheckState(Qt::PartiallyChecked);
    } else {
        cb->setCheckState(Qt::Unchecked);
    }
    setCheckboxTooltip(cb, cb->checkState()); // Update tooltip after manual cycle
}


mapcore::ItemType* ItemFinderDialogQt::getSelectedItemType() const
{
    return m_selectedItemType;
}

void ItemFinderDialogQt::onSearchModeChanged()
{
    updateControlsBasedOnSearchMode();
    if (m_autoRefreshCheck->isChecked()) {
        triggerRefresh();
    }
}

void ItemFinderDialogQt::updateControlsBasedOnSearchMode()
{
    SearchMode mode = getCurrentSearchMode();

    // Disable all input groups first, then enable specific ones
    m_searchInputsGroup->setEnabled(true); // This group is almost always active in some form
    m_itemTypeGroup->setEnabled(false);
    m_itemPropertiesGroup->setEnabled(false);

    // Specific inputs within SearchInputsGroup
    m_serverIdSpin->setEnabled(mode == SearchMode::ServerID);
    m_invalidItemCheck->setEnabled(mode == SearchMode::ServerID);
    m_clientIdSpin->setEnabled(mode == SearchMode::ClientID);
    m_nameEdit->setEnabled(mode == SearchMode::Name);

    bool isIdSearchMode = (mode == SearchMode::ServerID || mode == SearchMode::ClientID);
    m_searchByRangeCheck->setEnabled(isIdSearchMode);
    m_idRangeEdit->setEnabled(isIdSearchMode && m_searchByRangeCheck->isChecked());

    if (mode == SearchMode::Type) {
        m_itemTypeGroup->setEnabled(true);
    } else if (mode == SearchMode::Properties) {
        m_itemPropertiesGroup->setEnabled(true);
    }
}


ItemFinderDialogQt::SearchMode ItemFinderDialogQt::getCurrentSearchMode() {
    if (m_searchByServerIdRadio->isChecked()) return SearchMode::ServerID;
    if (m_searchByClientIdRadio->isChecked()) return SearchMode::ClientID;
    if (m_searchByNameRadio->isChecked()) return SearchMode::Name;
    if (m_searchByTypeRadio->isChecked()) return SearchMode::Type;
    if (m_searchByPropertiesRadio->isChecked()) return SearchMode::Properties;
    return SearchMode::ServerID; // Default
}


void ItemFinderDialogQt::onFilterCriteriaChanged()
{
    if (m_autoRefreshCheck->isChecked()) {
        triggerRefresh();
    }
}

void ItemFinderDialogQt::triggerRefresh()
{
    m_refreshTimer->start(250); // Debounce for 250ms
}

void ItemFinderDialogQt::performSearch()
{
    m_resultsListWidget->clear();
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    m_selectedItemType = nullptr;

    SearchMode searchMode = getCurrentSearchMode();
    int maxResults = m_maxResultsSpin->value();

    QString ignoredIdsText = m_enableIgnoredIdsCheck->isChecked() ? m_ignoredIdsEdit->text() : "";
    QSet<int> ignoredServerIds = parseIds(ignoredIdsText); // Assuming server IDs for now
    // QList<QPair<int, int>> ignoredIdRanges = parseIdRanges(ignoredIdsText); // If ranges are also supported for ignore

    QString idRangeText = (searchMode == SearchMode::ServerID || searchMode == SearchMode::ClientID) && m_searchByRangeCheck->isChecked() ? m_idRangeEdit->text() : "";
    QSet<int> searchSpecificIds = parseIds(idRangeText);
    QList<QPair<int, int>> searchIdRanges = parseIdRanges(idRangeText);


    const auto& allItemTypes = m_itemManager->getItemTypes(); // Assuming this returns const QList<mapcore::ItemType*>& or similar

    for (mapcore::ItemType* itemType : allItemTypes) {
        if (!itemType) continue;

        // 1. Initial 'onlyPickupable' filter (from constructor)
        if (m_onlyPickupableInitial && !itemType->isPickupable()) {
            continue;
        }

        // 2. Ignored IDs filter
        if (m_enableIgnoredIdsCheck->isChecked() && ignoredServerIds.contains(itemType->id())) {
            continue;
        }
        // TODO: Add range check for ignored IDs if parseIdRanges is used for ignoredIdsEdit

        // 3. Search Mode specific filters
        bool match = false;
        switch (searchMode) {
            case SearchMode::ServerID: {
                bool serverIdMatch = false;
                if (!idRangeText.isEmpty()) { // Range/List search
                    if (searchSpecificIds.contains(itemType->id())) serverIdMatch = true;
                    if (!serverIdMatch) {
                        for(const auto& range : searchIdRanges) {
                            if (itemType->id() >= range.first && itemType->id() <= range.second) {
                                serverIdMatch = true;
                                break;
                            }
                        }
                    }
                } else { // Single ID search
                    serverIdMatch = (itemType->id() == m_serverIdSpin->value());
                }
                // Handle m_invalidItemCheck. If ItemType has isInvalid():
                // if (m_invalidItemCheck->isChecked() && !itemType->isInvalid()) continue;
                // if (!m_invalidItemCheck->isChecked() && itemType->isInvalid()) continue;
                // For now, assuming it's a positive match if checked or if the ID matches.
                // The exact logic of "Invalid Item" needs clarification from wx original.
                // If it means "allow non-items" or "search for an ID that isn't a real item",
                // this loop structure isn't right. Assuming it's a filter on ItemType.
                // If no isInvalid() on ItemType, this check might be complex or ignored.
                // For now, let's say if m_invalidItemCheck is checked, this item is a match if its ID matches.
                // This part is a bit ambiguous from the prompt for Qt.
                if (serverIdMatch) { // Simplified: invalidItemCheck might modify this condition
                    match = true;
                }
                break;
            }
            case SearchMode::ClientID: {
                 bool clientIdMatch = false;
                 // Assuming ItemType has getClientId() or similar
                 int currentClientId = itemType->getClientId(); // Or itemType->getSprite(0)->id if that's the way
                 if (!idRangeText.isEmpty()) {
                    if (searchSpecificIds.contains(currentClientId)) clientIdMatch = true;
                    if (!clientIdMatch) {
                        for(const auto& range : searchIdRanges) {
                            if (currentClientId >= range.first && currentClientId <= range.second) {
                                clientIdMatch = true;
                                break;
                            }
                        }
                    }
                 } else {
                    clientIdMatch = (currentClientId == m_clientIdSpin->value());
                 }
                 if (clientIdMatch) match = true;
                 break;
            }
            case SearchMode::Name: {
                if (itemType->name().contains(m_nameEdit->text(), Qt::CaseInsensitive)) {
                    match = true;
                }
                break;
            }
            case SearchMode::Type: {
                // This requires ItemType to have methods like isDepot(), isMailbox(), etc.
                if (m_typeDepotRadio->isChecked() && itemType->isDepot()) match = true;
                else if (m_typeMailboxRadio->isChecked() && itemType->isMailbox()) match = true;
                else if (m_typeContainerRadio->isChecked() && itemType->isContainer()) match = true;
                else if (m_typeDoorRadio->isChecked() && itemType->isDoor()) match = true;
                else if (m_typeTeleportRadio->isChecked() && itemType->isTeleport()) match = true;
                else if (m_typeBedRadio->isChecked() && itemType->isBed()) match = true;
                else if (m_typeKeyRadio->isChecked() && itemType->isKey()) match = true;
                else if (m_typePodiumRadio->isChecked() && itemType->isPodium()) match = true;
                // ... Add checks for all other type radios
                break;
            }
            case SearchMode::Properties: {
                match = true; // Assume match until a property fails
                for (const auto& propCheckDef : m_propertyCheckboxesList) {
                    QCheckBox* cb = propCheckDef.checkBox;
                    if (cb->checkState() == Qt::Unchecked) continue; // Ignore this property

                    bool hasProperty = false; // Placeholder: determine this based on itemType and propCheckDef.objectName
                    // Example property checks (replace with actual ItemType methods)
                    QString objName = cb->objectName();
                    if (objName == "propUnpassableCheck") hasProperty = !itemType->isPassable(); // Note: Unpassable is !Passable
                    else if (objName == "propUnmovableCheck") hasProperty = !itemType->isMoveable(); // Note: Unmovable is !Moveable
                    else if (objName == "propBlockMissilesCheck") hasProperty = itemType->blocksMissiles();
                    else if (objName == "propBlockPathfinderCheck") hasProperty = itemType->blocksPathfinder();
                    else if (objName == "propPickupableCheck") hasProperty = itemType->isPickupable();
                    else if (objName == "propStackableCheck") hasProperty = itemType->isStackable();
                    else if (objName == "propRotatableCheck") hasProperty = itemType->isRotatable();
                    else if (objName == "propHangableCheck") hasProperty = itemType->isHangable();
                    else if (objName == "propHookEastCheck") hasProperty = itemType->canHookEast();
                    else if (objName == "propHookSouthCheck") hasProperty = itemType->canHookSouth();
                    else if (objName == "propHasElevationCheck") hasProperty = itemType->hasElevation();
                    else if (objName == "propIgnoreLookCheck") hasProperty = itemType->ignoresLook();
                    else if (objName == "propHasLightCheck") hasProperty = itemType->hasLight();
                    else if (objName == "propFloorChangeCheck") hasProperty = itemType->isFloorChange();
                    // Slot checks (assuming getSlotPosition() returns a bitmask and SLOTP_ enums are defined)
                    else if (objName == "propSlotHeadCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_HEAD);
                    else if (objName == "propSlotNecklaceCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_NECKLACE);
                    else if (objName == "propSlotBackpackCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_BACKPACK);
                    else if (objName == "propSlotArmorCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_ARMOR);
                    else if (objName == "propSlotLegsCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_LEGS);
                    else if (objName == "propSlotFeetCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_FEET);
                    else if (objName == "propSlotRingCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_RING);
                    else if (objName == "propSlotAmmoCheck") hasProperty = (itemType->getSlotPosition() & SLOTP_AMMO);
                    // ... other properties

                    if (cb->checkState() == Qt::Checked && !hasProperty) { // Must have, but doesn't
                        match = false;
                        break;
                    }
                    if (cb->checkState() == Qt::PartiallyChecked && hasProperty) { // Must NOT have, but does
                        match = false;
                        break;
                    }
                }
                break;
            }
        }

        if (match) {
            QListWidgetItem* listItem = new QListWidgetItem(m_resultsListWidget);
            listItem->setText(itemType->name());
            // Assuming ItemManager has getSpriteManager() and SpriteManager has getSpritePixmap()
            // And ItemType has getSprite(0)->id or similar to get a sprite ID
            if (m_itemManager->getSpriteManager() && itemType->getSpriteCount() > 0) {
                 // Default sprite for the item type.
                int spriteIdToUse = itemType->getSprite(0)->id; // Or itemType->getClientId() if that's the sprite id
                QPixmap pixmap = m_itemManager->getSpriteManager()->getSpritePixmap(spriteIdToUse, 0, 0, 0, 0, false);
                if (!pixmap.isNull()) {
                    listItem->setIcon(QIcon(pixmap));
                }
            }
            listItem->setData(Qt::UserRole, QVariant::fromValue(itemType));
            m_resultsListWidget->addItem(listItem);

            if (m_resultsListWidget->count() >= maxResults) {
                break;
            }
        }
    }

    if (m_resultsListWidget->count() > 0) {
        m_resultsListWidget->setCurrentRow(0); // Select the first item
        // onResultSelectionChanged will enable OK button
    }
}


QSet<int> ItemFinderDialogQt::parseIds(const QString& text) {
    QSet<int> ids;
    if (text.trimmed().isEmpty()) return ids;
    QStringList parts = text.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        int id = part.trimmed().toInt(&ok);
        if (ok) {
            ids.insert(id);
        }
    }
    return ids;
}

QList<QPair<int, int>> ItemFinderDialogQt::parseIdRanges(const QString& text) {
    QList<QPair<int, int>> ranges;
    if (text.trimmed().isEmpty()) return ranges;

    QStringList parts = text.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        QString trimmedPart = part.trimmed();
        QStringList rangeParts = trimmedPart.split('-', Qt::SkipEmptyParts);
        bool ok1, ok2;
        if (rangeParts.size() == 1) {
            int id = rangeParts[0].toInt(&ok1);
            if (ok1) {
                ranges.append({id, id});
            }
        } else if (rangeParts.size() == 2) {
            int startId = rangeParts[0].toInt(&ok1);
            int endId = rangeParts[1].toInt(&ok2);
            if (ok1 && ok2 && startId <= endId) {
                ranges.append({startId, endId});
            }
        }
    }
    std.sort(ranges.begin(), ranges.end()); // Optional: sort and merge overlapping ranges for efficiency
    return ranges;
}


void ItemFinderDialogQt::onResultSelectionChanged()
{
    QListWidgetItem* currentItem = m_resultsListWidget->currentItem();
    bool itemSelected = (currentItem != nullptr);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(itemSelected);
    if (itemSelected) {
        m_selectedItemType = currentItem->data(Qt::UserRole).value<mapcore::ItemType*>();
    } else {
        m_selectedItemType = nullptr;
    }
}

void ItemFinderDialogQt::onSearchByRangeToggled(bool checked)
{
    m_idRangeEdit->setEnabled(checked);
    onFilterCriteriaChanged(); // Refresh if auto-refresh is on
}

void ItemFinderDialogQt::onEnableIgnoredIdsToggled(bool checked)
{
    m_ignoredIdsEdit->setEnabled(checked);
    onFilterCriteriaChanged(); // Refresh if auto-refresh is on
}

void ItemFinderDialogQt::handleOk()
{
    if (m_resultsListWidget->currentItem()) { // Ensure an item is selected
        m_selectedItemType = m_resultsListWidget->currentItem()->data(Qt::UserRole).value<mapcore::ItemType*>();
        if (m_selectedItemType) {
            accept();
            return;
        }
    }
    // If OK is clicked without a selection (e.g. if button wasn't properly disabled)
    // or if invalid item logic needs to be handled here.
    // For now, only accept if a valid item type is selected.
    // Consider a warning if OK is pressed with no selection.
}

void ItemFinderDialogQt::handleCancel()
{
    m_selectedItemType = nullptr;
    reject();
}
