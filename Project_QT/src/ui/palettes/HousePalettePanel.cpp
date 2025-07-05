#include "ui/palettes/HousePalettePanel.h"
#include "ui/dialogs/EditHouseDialog.h"
#include "core/houses/HouseData.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

namespace RME {
namespace ui {

HousePalettePanel::HousePalettePanel(
    RME::core::IBrushStateService* brushStateService,
    RME::core::IClientDataService* clientDataService,
    QWidget* parent
) : BasePalettePanel(parent)
    , m_brushStateService(brushStateService)
    , m_clientDataService(clientDataService)
    , m_searchWidget(nullptr)
    , m_searchEdit(nullptr)
    , m_houseList(nullptr)
    , m_houseInfoWidget(nullptr)
    , m_houseInfoLabel(nullptr)
    , m_houseControlsWidget(nullptr)
    , m_createButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
{
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_clientDataService);
    
    setObjectName("HousePalettePanel");
    setWindowTitle(tr("House Palette"));
    
    setupUI();
    connectSignals();
    loadHouses();
}

HousePalettePanel::~HousePalettePanel()
{
    // Qt handles cleanup through parent-child relationships
}

void HousePalettePanel::setupUI()
{
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    
    // Setup search controls
    setupSearchControls();
    
    // Create splitter for house list and info
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // Setup house list
    setupHouseList();
    splitter->addWidget(m_houseList);
    
    // Setup house info panel
    setupHouseInfo();
    splitter->addWidget(m_houseInfoWidget);
    
    // Setup house controls
    setupHouseControls();
    
    // Add to main layout
    mainLayout->addWidget(m_searchWidget);
    mainLayout->addWidget(splitter, 1); // Give splitter most space
    mainLayout->addWidget(m_houseControlsWidget);
    
    // Set splitter proportions (70% list, 30% info)
    splitter->setStretchFactor(0, 7);
    splitter->setStretchFactor(1, 3);
}

void HousePalettePanel::setupHouseList()
{
    m_houseList = new QListWidget(this);
    m_houseList->setObjectName("houseList");
    m_houseList->setAlternatingRowColors(true);
    m_houseList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_houseList->setSortingEnabled(true);
    m_houseList->setContextMenuPolicy(Qt::CustomContextMenu);
}

void HousePalettePanel::setupSearchControls()
{
    m_searchWidget = new QGroupBox(tr("Search"), this);
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchWidget);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("houseSearchEdit");
    m_searchEdit->setPlaceholderText(tr("Search houses..."));
    m_searchEdit->setClearButtonEnabled(true);
    
    searchLayout->addWidget(m_searchEdit);
}

void HousePalettePanel::setupHouseInfo()
{
    m_houseInfoWidget = new QGroupBox(tr("House Information"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout(m_houseInfoWidget);
    
    m_houseInfoLabel = new QLabel(tr("Select a house to view information"), this);
    m_houseInfoLabel->setObjectName("houseInfoLabel");
    m_houseInfoLabel->setWordWrap(true);
    m_houseInfoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_houseInfoLabel->setMinimumHeight(80);
    
    infoLayout->addWidget(m_houseInfoLabel);
}

void HousePalettePanel::setupHouseControls()
{
    m_houseControlsWidget = new QGroupBox(tr("House Controls"), this);
    QHBoxLayout* controlsLayout = new QHBoxLayout(m_houseControlsWidget);
    
    m_createButton = new QPushButton(tr("Create House"), this);
    m_createButton->setObjectName("createHouseButton");
    
    m_editButton = new QPushButton(tr("Edit House"), this);
    m_editButton->setObjectName("editHouseButton");
    m_editButton->setEnabled(false); // Disabled until house selected
    
    m_deleteButton = new QPushButton(tr("Delete House"), this);
    m_deleteButton->setObjectName("deleteHouseButton");
    m_deleteButton->setEnabled(false); // Disabled until house selected
    
    controlsLayout->addWidget(m_createButton);
    controlsLayout->addWidget(m_editButton);
    controlsLayout->addWidget(m_deleteButton);
    controlsLayout->addStretch(); // Push buttons to left
}

void HousePalettePanel::connectSignals()
{
    // Search functionality
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &HousePalettePanel::onSearchTextChanged);
    
    // House list interactions
    connect(m_houseList, &QListWidget::itemSelectionChanged,
            this, &HousePalettePanel::onHouseSelectionChanged);
    connect(m_houseList, &QListWidget::itemDoubleClicked,
            this, &HousePalettePanel::onHouseDoubleClicked);
    connect(m_houseList, &QListWidget::customContextMenuRequested,
            this, &HousePalettePanel::onHouseContextMenu);
    
    // House controls
    connect(m_createButton, &QPushButton::clicked,
            this, &HousePalettePanel::onCreateHouse);
    connect(m_editButton, &QPushButton::clicked,
            this, &HousePalettePanel::onEditHouse);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &HousePalettePanel::onDeleteHouse);
}

void HousePalettePanel::loadHouses()
{
    if (!m_houseList) {
        return;
    }
    
    m_houseList->clear();
    
    // Load houses from Houses system via ClientDataService
    if (m_clientDataService) {
        // Try to get houses from the map
        auto* map = m_clientDataService->getCurrentMap();
        if (map) {
            auto* houses = map->getHouses();
            if (houses) {
                const auto& allHouses = houses->getAllHouses();
                
                // Sort houses by ID for consistent display
                QList<HouseData> sortedHouses;
                for (auto it = allHouses.constBegin(); it != allHouses.constEnd(); ++it) {
                    sortedHouses.append(it.value());
                }
                
                std::sort(sortedHouses.begin(), sortedHouses.end(), 
                         [](const HouseData& a, const HouseData& b) {
                             return a.id < b.id;
                         });
                
                // Add houses to list
                for (const HouseData& house : sortedHouses) {
                    QListWidgetItem* item = createHouseListItem(house);
                    m_houseList->addItem(item);
                }
                
                qInfo() << "HousePalettePanel: Loaded" << sortedHouses.size() << "houses from map";
            } else {
                qWarning() << "HousePalettePanel: No houses system available in current map";
                loadFallbackHouses();
            }
        } else {
            qWarning() << "HousePalettePanel: No current map available";
            loadFallbackHouses();
        }
    } else {
        qWarning() << "HousePalettePanel: No ClientDataService available";
        loadFallbackHouses();
    }
    
    qDebug() << "HousePalettePanel: Loaded" << m_houseList->count() << "houses";
}

void HousePalettePanel::refreshHouseList()
{
    loadHouses();
    
    // Reapply current filter if any
    if (m_searchEdit && !m_searchEdit->text().isEmpty()) {
        filterHouses(m_searchEdit->text());
    }
}

void HousePalettePanel::filterHouses(const QString& filter)
{
    if (!m_houseList) {
        return;
    }
    
    for (int i = 0; i < m_houseList->count(); ++i) {
        QListWidgetItem* item = m_houseList->item(i);
        if (item) {
            bool visible = filter.isEmpty() || 
                          item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!visible);
        }
    }
}

int HousePalettePanel::getSelectedHouseId() const
{
    if (!m_houseList) {
        return -1;
    }
    
    QListWidgetItem* currentItem = m_houseList->currentItem();
    if (!currentItem) {
        return -1;
    }
    
    return currentItem->data(Qt::UserRole).toInt();
}

QString HousePalettePanel::getSelectedHouseName() const
{
    if (!m_houseList) {
        return QString();
    }
    
    QListWidgetItem* currentItem = m_houseList->currentItem();
    return currentItem ? currentItem->text() : QString();
}

void HousePalettePanel::selectHouse(int houseId)
{
    if (!m_houseList) {
        return;
    }
    
    for (int i = 0; i < m_houseList->count(); ++i) {
        QListWidgetItem* item = m_houseList->item(i);
        if (item && item->data(Qt::UserRole).toInt() == houseId) {
            m_houseList->setCurrentItem(item);
            break;
        }
    }
}

void HousePalettePanel::onHouseSelectionChanged()
{
    int houseId = getSelectedHouseId();
    bool hasSelection = (houseId != -1);
    
    // Enable/disable buttons based on selection
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        updateHouseInfo(houseId);
        emit houseSelected(houseId);
    } else {
        m_houseInfoLabel->setText(tr("Select a house to view information"));
    }
}

void HousePalettePanel::onHouseDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        int houseId = item->data(Qt::UserRole).toInt();
        emit houseDoubleClicked(houseId);
        // Also trigger edit for convenience
        onEditHouse();
    }
}

void HousePalettePanel::onHouseContextMenu(const QPoint& position)
{
    QListWidgetItem* item = m_houseList->itemAt(position);
    if (!item) {
        return;
    }
    
    QMenu contextMenu(this);
    
    QAction* editAction = contextMenu.addAction(tr("Edit House"));
    QAction* deleteAction = contextMenu.addAction(tr("Delete House"));
    contextMenu.addSeparator();
    QAction* infoAction = contextMenu.addAction(tr("Show Information"));
    
    QAction* selectedAction = contextMenu.exec(m_houseList->mapToGlobal(position));
    
    if (selectedAction == editAction) {
        onEditHouse();
    } else if (selectedAction == deleteAction) {
        onDeleteHouse();
    } else if (selectedAction == infoAction) {
        int houseId = item->data(Qt::UserRole).toInt();
        showHouseInformation(houseId);
    }
}

void HousePalettePanel::onCreateHouse()
{
    EditHouseDialog dialog(this, -1); // -1 for new house
    if (dialog.exec() == QDialog::Accepted) {
        int newHouseId = dialog.getHouseId();
        QString houseName = dialog.getHouseName();
        
        // Add house to actual house system
        if (m_clientDataService) {
            auto* map = m_clientDataService->getCurrentMap();
            if (map && map->getHouses()) {
                HouseData newHouse;
                newHouse.id = newHouseId;
                newHouse.name = houseName;
                newHouse.owner = "";
                newHouse.rent = 0;
                newHouse.size = 0;
                newHouse.beds = 0;
                map->getHouses()->addHouse(newHouse);
                qDebug() << "HousePalettePanel: Added house to house system";
            }
        }
        
        // Add to list for UI
        QListWidgetItem* item = new QListWidgetItem(houseName);
        item->setData(Qt::UserRole, newHouseId);
        m_houseList->addItem(item);
        m_houseList->setCurrentItem(item);
        
        emit createHouseRequested();
        
        qDebug() << "HousePalettePanel: Created house" << houseName << "with ID" << newHouseId;
    }
}

void HousePalettePanel::onEditHouse()
{
    int houseId = getSelectedHouseId();
    if (houseId == -1) {
        QMessageBox::information(this, tr("No Selection"), 
                               tr("Please select a house to edit."));
        return;
    }
    
    QString houseName = getSelectedHouseName();
    
    EditHouseDialog dialog(this, houseId);
    // Get actual owner from house system
    QString owner = "";
    if (m_clientDataService) {
        auto* map = m_clientDataService->getCurrentMap();
        if (map && map->getHouses()) {
            const HouseData* houseData = map->getHouses()->getHouse(houseId);
            if (houseData) {
                owner = houseData->owner;
            }
        }
    }
    dialog.setHouseData(houseId, houseName, owner);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString newName = dialog.getHouseName();
        
        // Update the list item
        QListWidgetItem* currentItem = m_houseList->currentItem();
        if (currentItem) {
            currentItem->setText(newName);
        }
        
        emit editHouseRequested(houseId);
        
        qDebug() << "HousePalettePanel: Edited house" << houseId << "new name:" << newName;
    }
}

void HousePalettePanel::onDeleteHouse()
{
    int houseId = getSelectedHouseId();
    if (houseId == -1) {
        QMessageBox::information(this, tr("No Selection"), 
                               tr("Please select a house to delete."));
        return;
    }
    
    QString houseName = getSelectedHouseName();
    
    int result = QMessageBox::question(this, tr("Delete House"), 
                                     tr("Are you sure you want to delete house '%1'?").arg(houseName),
                                     QMessageBox::Yes | QMessageBox::No, 
                                     QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        // Remove from list
        QListWidgetItem* currentItem = m_houseList->currentItem();
        if (currentItem) {
            delete m_houseList->takeItem(m_houseList->row(currentItem));
        }
        
        emit deleteHouseRequested(houseId);
        
        qDebug() << "HousePalettePanel: Deleted house" << houseId;
    }
}

void HousePalettePanel::onSearchTextChanged(const QString& text)
{
    filterHouses(text);
}

void HousePalettePanel::updateHouseInfo(int houseId)
{
    if (!m_houseInfoLabel) {
        return;
    }
    
    QString info;
    
    // Try to get actual house information from the current selection
    QListWidgetItem* currentItem = m_houseList->currentItem();
    if (currentItem) {
        QString houseName = currentItem->data(Qt::UserRole + 1).toString();
        QString owner = currentItem->data(Qt::UserRole + 2).toString();
        int rent = currentItem->data(Qt::UserRole + 3).toInt();
        
        info = tr("<b>%1</b><br>").arg(houseName);
        info += tr("House ID: %1<br>").arg(houseId);
        
        if (!owner.isEmpty()) {
            info += tr("Owner: %1<br>").arg(owner);
        } else {
            info += tr("Owner: Not set<br>");
        }
        
        info += tr("Rent: %1 gold<br>").arg(rent);
        
        // Try to get additional house data from the houses system
        if (m_clientDataService) {
            auto* map = m_clientDataService->getCurrentMap();
            if (map) {
                auto* houses = map->getHouses();
                if (houses) {
                    const HouseData* houseData = houses->getHouse(houseId);
                    if (houseData) {
                        info += tr("Size: %1 tiles<br>").arg(houseData->size);
                        info += tr("Beds: %1<br>").arg(houseData->beds);
                        
                        if (!houseData->description.isEmpty()) {
                            info += tr("Description: %1<br>").arg(houseData->description);
                        }
                    }
                }
            }
        }
        
        info += tr("<br>Double-click to edit house properties.");
    } else {
        info = tr("Select a house to view information");
    }
    
    m_houseInfoLabel->setText(info);
}

void HousePalettePanel::showHouseInformation(int houseId)
{
    QString houseName = getSelectedHouseName();
    QString detailedInfo = tr("<h3>%1</h3>").arg(houseName);
    
    // TODO: Add comprehensive house information when house system is available
    detailedInfo += tr("<p><b>House ID:</b> %1</p>").arg(houseId);
    detailedInfo += tr("<p><b>Owner:</b> Not set</p>");
    detailedInfo += tr("<p><b>Rent:</b> 0 gold per month</p>");
    detailedInfo += tr("<p><b>Description:</b> A house that can be owned by players.</p>");
    
    detailedInfo += tr("<p><b>Usage:</b></p>");
    detailedInfo += tr("<ul>");
    detailedInfo += tr("<li>Double-click to edit house properties</li>");
    detailedInfo += tr("<li>Use 'Edit House' button to modify settings</li>");
    detailedInfo += tr("<li>Use 'Delete House' to remove from map</li>");
    detailedInfo += tr("</ul>");
    
    QMessageBox::information(this, tr("House Information"), detailedInfo);
}

QListWidgetItem* HousePalettePanel::createHouseListItem(const HouseData& house) const
{
    QListWidgetItem* item = new QListWidgetItem();
    
    // Set house text with detailed information
    QString houseText = QString("House #%1: %2").arg(house.id).arg(house.name);
    if (!house.owner.isEmpty()) {
        houseText += QString(" (Owner: %1)").arg(house.owner);
    }
    item->setText(houseText);
    
    // Store house data
    item->setData(Qt::UserRole, house.id);
    item->setData(Qt::UserRole + 1, house.name);
    item->setData(Qt::UserRole + 2, house.owner);
    item->setData(Qt::UserRole + 3, house.rent);
    
    // Set house icon (generic house icon for now)
    // TODO: Add actual house icon when available
    item->setIcon(QIcon(":/icons/house.png"));
    
    // Set tooltip with detailed house information
    QString tooltip = createHouseTooltip(house);
    item->setToolTip(tooltip);
    
    return item;
}

QString HousePalettePanel::createHouseTooltip(const HouseData& house) const
{
    QStringList tooltipParts;
    tooltipParts << QString("<b>%1</b>").arg(house.name);
    tooltipParts << QString("House ID: %1").arg(house.id);
    
    if (!house.owner.isEmpty()) {
        tooltipParts << QString("Owner: %1").arg(house.owner);
    } else {
        tooltipParts << "Owner: Not set";
    }
    
    tooltipParts << QString("Rent: %1 gold").arg(house.rent);
    tooltipParts << QString("Size: %1 tiles").arg(house.size);
    tooltipParts << QString("Beds: %1").arg(house.beds);
    
    if (!house.description.isEmpty()) {
        tooltipParts << QString("Description: %1").arg(house.description);
    }
    
    return tooltipParts.join("<br>");
}

void HousePalettePanel::loadFallbackHouses()
{
    // Add some fallback houses for testing when house system is not available
    QStringList fallbackHouses = {
        "House #1 - Thais", "House #2 - Carlin", "House #3 - Venore",
        "House #4 - Ab'Dendriel", "House #5 - Kazordoon", "House #6 - Ankrahmun",
        "House #7 - Port Hope", "House #8 - Liberty Bay", "House #9 - Yalahar",
        "House #10 - Farmine", "Villa #1 - Premium", "Villa #2 - Luxury",
        "Shop #1 - Market", "Shop #2 - Equipment", "Guild Hall #1"
    };
    
    for (int i = 0; i < fallbackHouses.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(fallbackHouses[i]);
        item->setData(Qt::UserRole, i + 1); // Fallback house ID
        item->setToolTip(QString("Fallback house: %1").arg(fallbackHouses[i]));
        m_houseList->addItem(item);
    }
}

} // namespace ui
} // namespace RME