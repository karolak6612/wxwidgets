#include "HousePaletteTab.h"
#include "ui/dialogs/EditHouseDialogQt.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/world/TownManager.h"
#include "core/world/TownData.h"
#include "core/brush/BrushStateManager.h"
#include "core/brush/HouseBrush.h"
#include "core/brush/HouseExitBrush.h"
#include <QApplication>

namespace RME {
namespace ui {
namespace palettes {

HousePaletteTab::HousePaletteTab(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void HousePaletteTab::setHouseManager(RME::core::houses::Houses* houseManager)
{
    m_houseManager = houseManager;
    refreshContent();
}

void HousePaletteTab::setTownManager(RME::core::world::TownManager* townManager)
{
    m_townManager = townManager;
    populateTownCombo();
}

void HousePaletteTab::setBrushStateManager(RME::core::brush::BrushStateManager* brushManager)
{
    m_brushStateManager = brushManager;
}

void HousePaletteTab::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
}

void HousePaletteTab::refreshContent()
{
    populateTownCombo();
    updateHouseList();
}

void HousePaletteTab::loadHousesForTown(quint32 townId)
{
    if (m_updatingUI) return;
    
    m_currentTownId = townId;
    updateHouseList();
}

void HousePaletteTab::loadHousesForNoTown()
{
    loadHousesForTown(0);
}

quint32 HousePaletteTab::getSelectedHouseId() const
{
    QListWidgetItem* currentItem = m_houseList->currentItem();
    if (!currentItem) {
        return 0;
    }
    
    return currentItem->data(Qt::UserRole).toUInt();
}

void HousePaletteTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Town selection combo
    m_townCombo = new QComboBox(this);
    m_mainLayout->addWidget(m_townCombo);
    
    // House list
    m_houseList = new QListWidget(this);
    m_houseList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_houseList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_mainLayout->addWidget(m_houseList);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_addHouseButton = new QPushButton("Add House", this);
    m_editHouseButton = new QPushButton("Edit House", this);
    m_removeHouseButton = new QPushButton("Remove House", this);
    
    m_buttonLayout->addWidget(m_addHouseButton);
    m_buttonLayout->addWidget(m_editHouseButton);
    m_buttonLayout->addWidget(m_removeHouseButton);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Brush mode radio buttons
    m_drawHouseTilesRadio = new QRadioButton("Draw House Tiles", this);
    m_setHouseExitRadio = new QRadioButton("Set House Exit", this);
    m_drawHouseTilesRadio->setChecked(true); // Default selection
    
    m_houseBrushModeGroup = new QButtonGroup(this);
    m_houseBrushModeGroup->addButton(m_drawHouseTilesRadio, 0);
    m_houseBrushModeGroup->addButton(m_setHouseExitRadio, 1);
    
    m_mainLayout->addWidget(m_drawHouseTilesRadio);
    m_mainLayout->addWidget(m_setHouseExitRadio);
    
    // Context menu
    m_contextMenu = new QMenu(this);
    QAction* moveToTownAction = m_contextMenu->addAction("Move to Town...");
    connect(moveToTownAction, &QAction::triggered, this, &HousePaletteTab::onMoveHouseToTown);
    
    // Initial state
    m_editHouseButton->setEnabled(false);
    m_removeHouseButton->setEnabled(false);
}

void HousePaletteTab::connectSignals()
{
    connect(m_townCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HousePaletteTab::onTownSelectionChanged);
    
    connect(m_houseList, &QListWidget::itemSelectionChanged,
            this, &HousePaletteTab::onHouseSelectionChanged);
    
    connect(m_houseList, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem*) { onHouseDoubleClicked(); });
    
    connect(m_houseList, &QListWidget::customContextMenuRequested,
            this, &HousePaletteTab::onHouseContextMenu);
    
    connect(m_addHouseButton, &QPushButton::clicked,
            this, &HousePaletteTab::onAddHouse);
    
    connect(m_editHouseButton, &QPushButton::clicked,
            this, &HousePaletteTab::onEditHouse);
    
    connect(m_removeHouseButton, &QPushButton::clicked,
            this, &HousePaletteTab::onRemoveHouse);
    
    connect(m_houseBrushModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int) { onBrushModeChanged(); });
}

void HousePaletteTab::populateTownCombo()
{
    if (!m_townManager) return;
    
    m_updatingUI = true;
    m_townCombo->clear();
    
    // Add "No Town" option
    m_townCombo->addItem("(No Town)", 0);
    
    // Add all towns
    auto towns = m_townManager->getAllTowns();
    for (const auto* town : towns) {
        if (town) {
            m_townCombo->addItem(town->getName(), town->getId());
        }
    }
    
    m_updatingUI = false;
}

void HousePaletteTab::updateHouseList()
{
    if (!m_houseManager) return;
    
    m_updatingUI = true;
    m_houseList->clear();
    
    auto houses = m_houseManager->getAllHouses();
    for (const auto* house : houses) {
        if (!house) continue;
        
        // Filter by town
        if (m_currentTownId == 0) {
            // Show houses with no town
            if (house->getTownId() != 0) continue;
        } else {
            // Show houses for specific town
            if (house->getTownId() != m_currentTownId) continue;
        }
        
        // Calculate house size
        int size = m_houseManager->calculateHouseSizeInSqms(house->getId());
        
        // Create list item
        QString itemText = formatHouseListItem(house->getId(), house->getName(), size);
        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, house->getId());
        m_houseList->addItem(item);
    }
    
    // Sort the list
    m_houseList->sortItems();
    
    m_updatingUI = false;
    onHouseSelectionChanged(); // Update button states
}

void HousePaletteTab::updateBrushState()
{
    if (!m_brushStateManager) return;
    
    quint32 selectedHouseId = getSelectedHouseId();
    if (selectedHouseId == 0) return;
    
    QString brushType;
    if (m_drawHouseTilesRadio->isChecked()) {
        brushType = "HouseBrush";
    } else if (m_setHouseExitRadio->isChecked()) {
        brushType = "HouseExitBrush";
    }
    
    emit brushModeChanged(brushType, selectedHouseId);
}

void HousePaletteTab::showHouseContextMenu(const QPoint& globalPos)
{
    if (m_houseList->selectedItems().isEmpty()) return;
    
    m_contextMenu->exec(globalPos);
}

QString HousePaletteTab::formatHouseListItem(quint32 houseId, const QString& houseName, int size) const
{
    return QString("%1 (ID: %2, Size: %3 sqm)")
           .arg(houseName)
           .arg(houseId)
           .arg(size);
}

QList<quint32> HousePaletteTab::getSelectedHouseIds() const
{
    QList<quint32> houseIds;
    auto selectedItems = m_houseList->selectedItems();
    
    for (const auto* item : selectedItems) {
        houseIds.append(item->data(Qt::UserRole).toUInt());
    }
    
    return houseIds;
}

void HousePaletteTab::selectHouseInList(quint32 houseId)
{
    for (int i = 0; i < m_houseList->count(); ++i) {
        QListWidgetItem* item = m_houseList->item(i);
        if (item && item->data(Qt::UserRole).toUInt() == houseId) {
            m_houseList->setCurrentItem(item);
            break;
        }
    }
}

void HousePaletteTab::onTownSelectionChanged()
{
    if (m_updatingUI) return;
    
    quint32 townId = m_townCombo->currentData().toUInt();
    loadHousesForTown(townId);
}

void HousePaletteTab::onHouseSelectionChanged()
{
    auto selectedItems = m_houseList->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();
    bool singleSelection = selectedItems.size() == 1;
    
    m_editHouseButton->setEnabled(singleSelection);
    m_removeHouseButton->setEnabled(hasSelection);
    
    if (singleSelection) {
        quint32 houseId = getSelectedHouseId();
        emit houseSelected(houseId);
        updateBrushState();
    }
}

void HousePaletteTab::onHouseDoubleClicked()
{
    onEditHouse();
}

void HousePaletteTab::onHouseContextMenu(const QPoint& position)
{
    QPoint globalPos = m_houseList->mapToGlobal(position);
    showHouseContextMenu(globalPos);
}

void HousePaletteTab::onAddHouse()
{
    if (!m_houseManager || !m_townManager) return;
    
    // Create new house with default values
    auto* newHouse = m_houseManager->createNewHouse();
    if (!newHouse) {
        QMessageBox::warning(this, "Error", "Failed to create new house.");
        return;
    }
    
    // Create a copy for editing
    RME::core::houses::HouseData houseCopy = *newHouse;
    
    // Open edit dialog
    RME::ui::dialogs::EditHouseDialogQt dialog(this, &houseCopy, m_townManager);
    if (dialog.exec() == QDialog::Accepted) {
        // Apply changes to the original house
        *newHouse = houseCopy;
        refreshContent();
        selectHouseInList(newHouse->getId());
    } else {
        // Remove the house if dialog was cancelled
        m_houseManager->removeHouse(newHouse->getId());
    }
}

void HousePaletteTab::onEditHouse()
{
    quint32 houseId = getSelectedHouseId();
    if (houseId == 0 || !m_houseManager || !m_townManager) return;
    
    auto* house = m_houseManager->getHouse(houseId);
    if (!house) return;
    
    // Create a copy for editing
    RME::core::houses::HouseData houseCopy = *house;
    
    // Open edit dialog
    RME::ui::dialogs::EditHouseDialogQt dialog(this, &houseCopy, m_townManager);
    if (dialog.exec() == QDialog::Accepted) {
        // Apply changes to the original house
        *house = houseCopy;
        refreshContent();
        selectHouseInList(houseId);
    }
}

void HousePaletteTab::onRemoveHouse()
{
    auto selectedHouseIds = getSelectedHouseIds();
    if (selectedHouseIds.isEmpty() || !m_houseManager) return;
    
    QString message;
    if (selectedHouseIds.size() == 1) {
        message = "Are you sure you want to remove this house?";
    } else {
        message = QString("Are you sure you want to remove %1 houses?")
                  .arg(selectedHouseIds.size());
    }
    
    int result = QMessageBox::question(this, "Confirm Removal", message,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        for (quint32 houseId : selectedHouseIds) {
            m_houseManager->removeHouse(houseId);
        }
        refreshContent();
    }
}

void HousePaletteTab::onMoveHouseToTown()
{
    auto selectedHouseIds = getSelectedHouseIds();
    if (selectedHouseIds.isEmpty() || !m_houseManager || !m_townManager) return;
    
    // Get list of town names for dialog
    QStringList townNames;
    townNames << "(No Town)";
    
    auto towns = m_townManager->getAllTowns();
    for (const auto* town : towns) {
        if (town) {
            townNames << town->getName();
        }
    }
    
    bool ok;
    QString selectedTownName = QInputDialog::getItem(this, "Move to Town",
                                                     "Select target town:",
                                                     townNames, 0, false, &ok);
    
    if (ok) {
        quint32 targetTownId = 0;
        if (selectedTownName != "(No Town)") {
            auto* town = m_townManager->getTown(selectedTownName);
            if (town) {
                targetTownId = town->getId();
            }
        }
        
        // Update all selected houses
        for (quint32 houseId : selectedHouseIds) {
            auto* house = m_houseManager->getHouse(houseId);
            if (house) {
                house->setTownId(targetTownId);
            }
        }
        
        refreshContent();
    }
}

void HousePaletteTab::onBrushModeChanged()
{
    updateBrushState();
}

} // namespace palettes
} // namespace ui
} // namespace RME