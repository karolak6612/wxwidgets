#include "MapPropertiesDialog.h"
#include "core/Map.h"

#include <QMessageBox>

namespace RME {
namespace ui {
namespace dialogs {

MapPropertiesDialog::MapPropertiesDialog(RME::core::Map* map, QWidget* parent)
    : QDialog(parent)
    , m_map(map)
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
{
    setWindowTitle(tr("Map Properties"));
    setModal(true);
    setMinimumSize(400, 500);
    
    setupUI();
    loadMapProperties();
}

void MapPropertiesDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create general tab
    m_generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(m_generalTab);
    
    // Map information group
    QGroupBox* infoGroup = new QGroupBox(tr("Map Information"));
    QFormLayout* infoLayout = new QFormLayout(infoGroup);
    
    m_nameEdit = new QLineEdit();
    infoLayout->addRow(tr("Name:"), m_nameEdit);
    
    m_authorEdit = new QLineEdit();
    infoLayout->addRow(tr("Author:"), m_authorEdit);
    
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setMaximumHeight(100);
    infoLayout->addRow(tr("Description:"), m_descriptionEdit);
    
    generalLayout->addWidget(infoGroup);
    
    // Map dimensions group
    QGroupBox* dimensionsGroup = new QGroupBox(tr("Dimensions"));
    QFormLayout* dimensionsLayout = new QFormLayout(dimensionsGroup);
    
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 65535);
    m_widthSpin->setEnabled(false); // Read-only for now
    dimensionsLayout->addRow(tr("Width:"), m_widthSpin);
    
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(1, 65535);
    m_heightSpin->setEnabled(false); // Read-only for now
    dimensionsLayout->addRow(tr("Height:"), m_heightSpin);
    
    generalLayout->addWidget(dimensionsGroup);
    generalLayout->addStretch();
    
    m_tabWidget->addTab(m_generalTab, tr("General"));
    
    // Create statistics tab
    m_statisticsTab = new QWidget();
    QVBoxLayout* statisticsLayout = new QVBoxLayout(m_statisticsTab);
    
    QGroupBox* statsGroup = new QGroupBox(tr("Map Statistics"));
    QFormLayout* statsLayout = new QFormLayout(statsGroup);
    
    m_totalTilesLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Total tiles:"), m_totalTilesLabel);
    
    m_usedTilesLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Used tiles:"), m_usedTilesLabel);
    
    m_emptyTilesLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Empty tiles:"), m_emptyTilesLabel);
    
    m_itemCountLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Total items:"), m_itemCountLabel);
    
    m_creatureCountLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Creatures:"), m_creatureCountLabel);
    
    m_spawnCountLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Spawns:"), m_spawnCountLabel);
    
    m_houseCountLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Houses:"), m_houseCountLabel);
    
    m_waypointCountLabel = new QLabel(tr("Calculating..."));
    statsLayout->addRow(tr("Waypoints:"), m_waypointCountLabel);
    
    statisticsLayout->addWidget(statsGroup);
    
    m_calculateButton = new QPushButton(tr("Recalculate Statistics"));
    statisticsLayout->addWidget(m_calculateButton);
    
    statisticsLayout->addStretch();
    
    m_tabWidget->addTab(m_statisticsTab, tr("Statistics"));
    
    // Create button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setDefault(true);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_okButton, &QPushButton::clicked, this, &MapPropertiesDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_calculateButton, &QPushButton::clicked, this, &MapPropertiesDialog::onCalculateStatistics);
}

void MapPropertiesDialog::loadMapProperties() {
    if (!m_map) {
        return;
    }
    
    // Load basic map properties
    m_nameEdit->setText(m_map->getName());
    m_authorEdit->setText(m_map->getAuthor());
    m_descriptionEdit->setPlainText(m_map->getDescription());
    m_widthSpin->setValue(m_map->getWidth());
    m_heightSpin->setValue(m_map->getHeight());
    
    // Calculate initial statistics
    calculateMapStatistics();
}

void MapPropertiesDialog::saveMapProperties() {
    if (!m_map) {
        return;
    }
    
    // Save basic map properties
    m_map->setName(m_nameEdit->text());
    m_map->setAuthor(m_authorEdit->text());
    m_map->setDescription(m_descriptionEdit->toPlainText());
    
    // Mark map as changed
    m_map->setChanged(true);
}

void MapPropertiesDialog::calculateMapStatistics() {
    if (!m_map) {
        m_totalTilesLabel->setText(tr("N/A"));
        m_usedTilesLabel->setText(tr("N/A"));
        m_emptyTilesLabel->setText(tr("N/A"));
        m_itemCountLabel->setText(tr("N/A"));
        m_creatureCountLabel->setText(tr("N/A"));
        m_spawnCountLabel->setText(tr("N/A"));
        m_houseCountLabel->setText(tr("N/A"));
        m_waypointCountLabel->setText(tr("N/A"));
        return;
    }
    
    // Calculate basic statistics
    int totalTiles = m_map->getWidth() * m_map->getHeight() * 16; // 16 floors
    int usedTiles = 0;
    int itemCount = 0;
    int creatureCount = 0;
    
    // TODO: Implement proper tile iteration when Map provides iterator
    // For now, provide basic information
    m_totalTilesLabel->setText(QString::number(totalTiles));
    m_usedTilesLabel->setText(tr("Calculating..."));
    m_emptyTilesLabel->setText(tr("Calculating..."));
    m_itemCountLabel->setText(tr("Calculating..."));
    m_creatureCountLabel->setText(tr("Calculating..."));
    
    // Get counts from map managers
    int spawnCount = 0;
    int houseCount = 0;
    int waypointCount = 0;
    
    // TODO: Get actual counts when managers are available
    // if (m_map->getSpawnManager()) spawnCount = m_map->getSpawnManager()->getSpawnCount();
    // if (m_map->getHouseManager()) houseCount = m_map->getHouseManager()->getHouseCount();
    // if (m_map->getWaypointManager()) waypointCount = m_map->getWaypointManager()->getWaypointCount();
    
    m_spawnCountLabel->setText(QString::number(spawnCount));
    m_houseCountLabel->setText(QString::number(houseCount));
    m_waypointCountLabel->setText(QString::number(waypointCount));
}

void MapPropertiesDialog::accept() {
    saveMapProperties();
    QDialog::accept();
}

void MapPropertiesDialog::onCalculateStatistics() {
    calculateMapStatistics();
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "MapPropertiesDialog.moc" // Removed - Q_OBJECT is in header