#include "ui/dialogs/CreaturePropertiesDialog.h"
#include "core/creatures/Creature.h"
#include "core/Position.h"

#include <QMessageBox>
#include <QDebug>

namespace RME {
namespace ui {
namespace dialogs {

CreaturePropertiesDialog::CreaturePropertiesDialog(QWidget* parent, RME::core::creatures::Creature* creatureCopy)
    : QDialog(parent)
    , m_creatureCopy(creatureCopy)
{
    Q_ASSERT(m_creatureCopy);
    
    setWindowTitle(tr("Creature Properties"));
    setModal(true);
    setFixedSize(350, 200);
    
    createBackup();
    setupUI();
    loadCreatureData();
    connectSignals();
}

void CreaturePropertiesDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Creature name (read-only)
    m_creatureNameLabel = new QLabel();
    m_creatureNameLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    m_creatureNameLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_creatureNameLabel);
    
    // Position info (read-only)
    m_positionLabel = new QLabel();
    m_positionLabel->setAlignment(Qt::AlignCenter);
    m_positionLabel->setStyleSheet("color: gray;");
    m_mainLayout->addWidget(m_positionLabel);
    
    // Form layout for creature properties
    m_formLayout = new QFormLayout();
    
    // Spawn interval
    m_spawnIntervalSpin = new QSpinBox();
    m_spawnIntervalSpin->setObjectName("spawnIntervalSpinBox");
    m_spawnIntervalSpin->setRange(1, 3600); // 1 second to 1 hour
    m_spawnIntervalSpin->setValue(60); // Default 60 seconds
    m_spawnIntervalSpin->setSuffix(" seconds");
    m_spawnIntervalSpin->setToolTip(tr("Time in seconds between creature respawns"));
    m_formLayout->addRow(tr("Spawn Interval:"), m_spawnIntervalSpin);
    
    // Direction
    m_directionCombo = new QComboBox();
    m_directionCombo->setObjectName("directionComboBox");
    setupDirectionCombo();
    m_directionCombo->setToolTip(tr("Direction the creature faces when spawned"));
    m_formLayout->addRow(tr("Direction:"), m_directionCombo);
    
    m_mainLayout->addLayout(m_formLayout);
    
    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_mainLayout->addWidget(m_buttonBox);
}

void CreaturePropertiesDialog::setupDirectionCombo() {
    // Add direction options as specified in UI-04 task
    m_directionCombo->addItem(tr("North"), static_cast<int>(RME::Direction::NORTH));
    m_directionCombo->addItem(tr("East"), static_cast<int>(RME::Direction::EAST));
    m_directionCombo->addItem(tr("South"), static_cast<int>(RME::Direction::SOUTH));
    m_directionCombo->addItem(tr("West"), static_cast<int>(RME::Direction::WEST));
    m_directionCombo->addItem(tr("Northeast"), static_cast<int>(RME::Direction::NORTHEAST));
    m_directionCombo->addItem(tr("Northwest"), static_cast<int>(RME::Direction::NORTHWEST));
    m_directionCombo->addItem(tr("Southeast"), static_cast<int>(RME::Direction::SOUTHEAST));
    m_directionCombo->addItem(tr("Southwest"), static_cast<int>(RME::Direction::SOUTHWEST));
}

void CreaturePropertiesDialog::loadCreatureData() {
    if (!m_creatureCopy) {
        return;
    }
    
    // Load creature name (read-only)
    m_creatureNameLabel->setText(m_creatureCopy->getName());
    
    // Load position info (read-only)
    m_positionLabel->setText(tr("Position: %1").arg(m_creatureCopy->getPosition().toString()));
    
    // Load spawn interval
    m_spawnIntervalSpin->setValue(m_creatureCopy->getSpawnTime());
    
    // Load direction
    RME::Direction direction = m_creatureCopy->getDirection();
    int directionIndex = m_directionCombo->findData(static_cast<int>(direction));
    if (directionIndex >= 0) {
        m_directionCombo->setCurrentIndex(directionIndex);
    }
}

void CreaturePropertiesDialog::saveCreatureData() {
    if (!m_creatureCopy) {
        return;
    }
    
    // Save spawn interval
    m_creatureCopy->setSpawnTime(m_spawnIntervalSpin->value());
    
    // Save direction
    int directionValue = m_directionCombo->currentData().toInt();
    m_creatureCopy->setDirection(static_cast<RME::Direction>(directionValue));
}

bool CreaturePropertiesDialog::validateInput() {
    // Validate spawn interval
    int interval = m_spawnIntervalSpin->value();
    if (interval < 1 || interval > 3600) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Spawn interval must be between 1 and 3600 seconds."));
        m_spawnIntervalSpin->setFocus();
        return false;
    }
    
    // Validate direction selection
    if (m_directionCombo->currentIndex() < 0) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Please select a valid direction."));
        m_directionCombo->setFocus();
        return false;
    }
    
    return true;
}

void CreaturePropertiesDialog::accept() {
    if (validateInput()) {
        saveCreatureData();
        emit creatureModified(m_creatureCopy);
        QDialog::accept();
    }
}

void CreaturePropertiesDialog::reject() {
    if (m_wasModified) {
        int result = QMessageBox::question(this, tr("Discard Changes"),
            tr("You have unsaved changes. Do you want to discard them?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (result == QMessageBox::No) {
            return;
        }
        
        restoreBackup();
    }
    
    QDialog::reject();
}

void CreaturePropertiesDialog::onSpawnIntervalChanged() {
    markAsModified();
}

void CreaturePropertiesDialog::onDirectionChanged() {
    markAsModified();
}

void CreaturePropertiesDialog::markAsModified() {
    if (!m_wasModified) {
        m_wasModified = true;
        setWindowTitle(tr("Creature Properties *"));
    }
}

void CreaturePropertiesDialog::createBackup() {
    if (m_creatureCopy) {
        m_originalCreature = m_creatureCopy->deepCopy().release();
    }
}

void CreaturePropertiesDialog::restoreBackup() {
    if (m_originalCreature && m_creatureCopy) {
        *m_creatureCopy = *m_originalCreature;
        loadCreatureData(); // Refresh UI
    }
}

void CreaturePropertiesDialog::connectSignals() {
    // Connect button box
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &CreaturePropertiesDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &CreaturePropertiesDialog::reject);
    
    // Connect property change signals
    connect(m_spawnIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CreaturePropertiesDialog::onSpawnIntervalChanged);
    connect(m_directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CreaturePropertiesDialog::onDirectionChanged);
}

} // namespace dialogs
} // namespace ui
} // namespace RME