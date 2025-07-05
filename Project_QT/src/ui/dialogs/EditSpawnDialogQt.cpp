#include "EditSpawnDialogQt.h"
#include "CreatureFinderDialogQt.h"
#include "core/Tile.h"
#include "core/assets/CreatureDatabase.h"
#include "core/spawns/Spawn.h"

namespace RME {
namespace ui {
namespace dialogs {

EditSpawnDialogQt::EditSpawnDialogQt(QWidget* parent,
                                     RME::core::Tile* tileDataSource,
                                     RME::core::assets::CreatureDatabase* creatureDatabase)
    : QDialog(parent)
    , m_tileDataSource(tileDataSource)
    , m_creatureDatabase(creatureDatabase)
{
    setWindowTitle("Edit Spawn Properties");
    setModal(true);
    setMinimumSize(400, 500);
    
    setupUI();
    connectSignals();
    loadData();
    updateButtonStates();
}

int EditSpawnDialogQt::getSpawnRadius() const
{
    return m_currentRadius;
}

int EditSpawnDialogQt::getRespawnTime() const
{
    return m_currentRespawnTime;
}

QStringList EditSpawnDialogQt::getCreatureList() const
{
    return m_currentCreatureList;
}

bool EditSpawnDialogQt::hasValidData() const
{
    return m_dataLoaded && 
           m_currentRadius >= 0 && 
           m_currentRespawnTime >= 0;
}

void EditSpawnDialogQt::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Spawn properties group
    m_propertiesGroup = new QGroupBox("Spawn Properties", this);
    m_formLayout = new QFormLayout(m_propertiesGroup);
    
    // Spawn radius
    m_radiusSpinBox = new QSpinBox(this);
    m_radiusSpinBox->setObjectName("radiusSpinBox");
    m_radiusSpinBox->setMinimum(0);
    m_radiusSpinBox->setMaximum(50);
    m_radiusSpinBox->setSuffix(" tiles");
    m_radiusSpinBox->setToolTip("Radius of the spawn area in tiles");
    m_formLayout->addRow("Spawn Radius:", m_radiusSpinBox);
    
    // Respawn time
    m_respawnTimeSpinBox = new QSpinBox(this);
    m_respawnTimeSpinBox->setObjectName("respawnTimeSpinBox");
    m_respawnTimeSpinBox->setMinimum(0);
    m_respawnTimeSpinBox->setMaximum(86400); // 24 hours in seconds
    m_respawnTimeSpinBox->setSuffix(" seconds");
    m_respawnTimeSpinBox->setToolTip("Time between creature respawns in seconds");
    m_formLayout->addRow("Respawn Time:", m_respawnTimeSpinBox);
    
    m_mainLayout->addWidget(m_propertiesGroup);
    
    // Creature list group
    m_creatureGroup = new QGroupBox("Creatures in Spawn", this);
    QVBoxLayout* creatureLayout = new QVBoxLayout(m_creatureGroup);
    
    // Creature list widget
    m_creatureListWidget = new QListWidget(this);
    m_creatureListWidget->setObjectName("creatureListWidget");
    m_creatureListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_creatureListWidget->setToolTip("List of creatures that can spawn at this location");
    creatureLayout->addWidget(m_creatureListWidget);
    
    // Creature management buttons
    m_creatureButtonLayout = new QHBoxLayout();
    m_addCreatureButton = new QPushButton("Add Creature", this);
    m_addCreatureButton->setObjectName("addCreatureButton");
    m_addCreatureButton->setToolTip("Add a creature to this spawn");
    
    m_removeCreatureButton = new QPushButton("Remove Creature", this);
    m_removeCreatureButton->setObjectName("removeCreatureButton");
    m_removeCreatureButton->setToolTip("Remove selected creatures from this spawn");
    
    m_creatureButtonLayout->addWidget(m_addCreatureButton);
    m_creatureButtonLayout->addWidget(m_removeCreatureButton);
    m_creatureButtonLayout->addStretch();
    
    creatureLayout->addLayout(m_creatureButtonLayout);
    m_mainLayout->addWidget(m_creatureGroup);
    
    // Dialog buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->setObjectName("buttonBox");
    m_mainLayout->addWidget(m_buttonBox);
    
    // Set focus to radius spinbox
    m_radiusSpinBox->setFocus();
}

void EditSpawnDialogQt::connectSignals()
{
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &EditSpawnDialogQt::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    connect(m_addCreatureButton, &QPushButton::clicked, this, &EditSpawnDialogQt::onAddCreature);
    connect(m_removeCreatureButton, &QPushButton::clicked, this, &EditSpawnDialogQt::onRemoveCreature);
    
    connect(m_creatureListWidget, &QListWidget::itemSelectionChanged,
            this, &EditSpawnDialogQt::onCreatureSelectionChanged);
    
    connect(m_radiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &EditSpawnDialogQt::onSpawnDataChanged);
    
    connect(m_respawnTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &EditSpawnDialogQt::onSpawnDataChanged);
}

void EditSpawnDialogQt::loadData()
{
    if (!m_tileDataSource) {
        m_dataLoaded = false;
        return;
    }
    
    m_updatingUI = true;
    
    // Load spawn radius
    m_currentRadius = m_tileDataSource->getSpawnRadius();
    m_radiusSpinBox->setValue(m_currentRadius);
    
    // Load respawn time
    m_currentRespawnTime = m_tileDataSource->getSpawnIntervalSeconds();
    m_respawnTimeSpinBox->setValue(m_currentRespawnTime);
    
    // Load creature list
    m_currentCreatureList = m_tileDataSource->getSpawnCreatureList();
    m_creatureListWidget->clear();
    
    for (const QString& creatureName : m_currentCreatureList) {
        QListWidgetItem* item = new QListWidgetItem(creatureName);
        item->setToolTip(QString("Creature: %1").arg(creatureName));
        m_creatureListWidget->addItem(item);
    }
    
    // Sort the creature list
    m_creatureListWidget->sortItems();
    
    m_dataLoaded = true;
    m_updatingUI = false;
}

bool EditSpawnDialogQt::validateInputs()
{
    // Validate radius
    if (m_radiusSpinBox->value() < 0) {
        QMessageBox::warning(this, "Validation Error", 
                             "Spawn radius cannot be negative.");
        m_radiusSpinBox->setFocus();
        return false;
    }
    
    // Validate respawn time
    if (m_respawnTimeSpinBox->value() < 0) {
        QMessageBox::warning(this, "Validation Error", 
                             "Respawn time cannot be negative.");
        m_respawnTimeSpinBox->setFocus();
        return false;
    }
    
    // Validate that if radius > 0, there should be at least one creature
    if (m_radiusSpinBox->value() > 0 && m_currentCreatureList.isEmpty()) {
        int result = QMessageBox::question(this, "Validation Warning",
                                           "You have set a spawn radius but no creatures are defined. "
                                           "This will create an empty spawn. Continue anyway?",
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::No);
        if (result != QMessageBox::Yes) {
            m_creatureListWidget->setFocus();
            return false;
        }
    }
    
    // Validate that if creatures exist, radius should be > 0
    if (!m_currentCreatureList.isEmpty() && m_radiusSpinBox->value() == 0) {
        QMessageBox::warning(this, "Validation Error",
                             "You have defined creatures but the spawn radius is 0. "
                             "Please set a radius greater than 0 or remove all creatures.");
        m_radiusSpinBox->setFocus();
        return false;
    }
    
    return true;
}

void EditSpawnDialogQt::updateButtonStates()
{
    bool hasSelection = !m_creatureListWidget->selectedItems().isEmpty();
    m_removeCreatureButton->setEnabled(hasSelection);
}

void EditSpawnDialogQt::accept()
{
    if (!validateInputs()) {
        return;
    }
    
    // Update internal data
    m_currentRadius = m_radiusSpinBox->value();
    m_currentRespawnTime = m_respawnTimeSpinBox->value();
    
    // Emit signal with the new data
    emit spawnDataChanged(m_currentRadius, m_currentRespawnTime, m_currentCreatureList);
    
    QDialog::accept();
}

void EditSpawnDialogQt::onAddCreature()
{
    if (!m_creatureDatabase) {
        QMessageBox::warning(this, "Error", "Creature database not available.");
        return;
    }
    
    // Open creature finder dialog
    CreatureFinderDialogQt dialog(this, m_creatureDatabase);
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedCreature = dialog.getSelectedCreatureName();
        if (!selectedCreature.isEmpty()) {
            // Check if creature is already in the list
            if (m_currentCreatureList.contains(selectedCreature)) {
                QMessageBox::information(this, "Duplicate Creature",
                                          QString("Creature '%1' is already in the spawn list.")
                                          .arg(selectedCreature));
                return;
            }
            
            // Add to internal list
            m_currentCreatureList.append(selectedCreature);
            
            // Add to UI list
            QListWidgetItem* item = new QListWidgetItem(selectedCreature);
            item->setToolTip(QString("Creature: %1").arg(selectedCreature));
            m_creatureListWidget->addItem(item);
            
            // Sort the list
            m_creatureListWidget->sortItems();
            
            // Select the newly added item
            for (int i = 0; i < m_creatureListWidget->count(); ++i) {
                QListWidgetItem* listItem = m_creatureListWidget->item(i);
                if (listItem && listItem->text() == selectedCreature) {
                    m_creatureListWidget->setCurrentItem(listItem);
                    break;
                }
            }
            
            onSpawnDataChanged();
        }
    }
}

void EditSpawnDialogQt::onRemoveCreature()
{
    auto selectedItems = m_creatureListWidget->selectedItems();
    if (selectedItems.isEmpty()) return;
    
    QString message;
    if (selectedItems.size() == 1) {
        message = QString("Are you sure you want to remove creature '%1' from this spawn?")
                  .arg(selectedItems.first()->text());
    } else {
        message = QString("Are you sure you want to remove %1 creatures from this spawn?")
                  .arg(selectedItems.size());
    }
    
    int result = QMessageBox::question(this, "Confirm Removal", message,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        // Remove from internal list and UI
        for (QListWidgetItem* item : selectedItems) {
            QString creatureName = item->text();
            m_currentCreatureList.removeAll(creatureName);
            delete m_creatureListWidget->takeItem(m_creatureListWidget->row(item));
        }
        
        onSpawnDataChanged();
        updateButtonStates();
    }
}

void EditSpawnDialogQt::onCreatureSelectionChanged()
{
    updateButtonStates();
}

void EditSpawnDialogQt::onSpawnDataChanged()
{
    if (m_updatingUI) return;
    
    // This slot is called when spawn data changes
    // Could be used for real-time validation or preview updates
}

} // namespace dialogs
} // namespace ui
} // namespace RME