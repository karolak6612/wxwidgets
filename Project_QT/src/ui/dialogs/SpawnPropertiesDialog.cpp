#include "ui/dialogs/SpawnPropertiesDialog.h"
#include "core/spawns/Spawn.h"

#include <QMessageBox>
#include <QDebug>

namespace RME {
namespace ui {
namespace dialogs {

SpawnPropertiesDialog::SpawnPropertiesDialog(QWidget* parent, RME::core::spawns::Spawn* spawn)
    : QDialog(parent)
    , m_spawn(spawn)
{
    Q_ASSERT(m_spawn);
    
    setWindowTitle(tr("Spawn Properties"));
    setModal(true);
    setFixedSize(300, 150);
    
    createBackup();
    setupUI();
    loadSpawnData();
    connectSignals();
}

void SpawnPropertiesDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Info label
    m_infoLabel = new QLabel();
    m_infoLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_infoLabel);
    
    // Form layout for spawn properties
    m_formLayout = new QFormLayout();
    
    // Spawn radius
    m_spawnRadiusSpin = new QSpinBox();
    m_spawnRadiusSpin->setObjectName("spawnRadiusSpinBox");
    m_spawnRadiusSpin->setRange(1, 50); // Reasonable range for spawn radius
    m_spawnRadiusSpin->setValue(3); // Default value
    m_spawnRadiusSpin->setToolTip(tr("The radius of the spawn area in tiles"));
    m_formLayout->addRow(tr("Spawn Radius:"), m_spawnRadiusSpin);
    
    m_mainLayout->addLayout(m_formLayout);
    
    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_mainLayout->addWidget(m_buttonBox);
}

void SpawnPropertiesDialog::loadSpawnData() {
    if (!m_spawn) {
        return;
    }
    
    // Load spawn radius
    m_spawnRadiusSpin->setValue(m_spawn->getRadius());
    
    // Update info label
    QString info = tr("Spawn at position %1")
                      .arg(m_spawn->getCenter().toString());
    m_infoLabel->setText(info);
}

void SpawnPropertiesDialog::saveSpawnData() {
    if (!m_spawn) {
        return;
    }
    
    // Save spawn radius
    m_spawn->setRadius(m_spawnRadiusSpin->value());
}

bool SpawnPropertiesDialog::validateInput() {
    // Validate spawn radius
    int radius = m_spawnRadiusSpin->value();
    if (radius < 1 || radius > 50) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Spawn radius must be between 1 and 50 tiles."));
        m_spawnRadiusSpin->setFocus();
        return false;
    }
    
    return true;
}

void SpawnPropertiesDialog::accept() {
    if (validateInput()) {
        saveSpawnData();
        emit spawnModified(m_spawn);
        QDialog::accept();
    }
}

void SpawnPropertiesDialog::reject() {
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

void SpawnPropertiesDialog::onSpawnRadiusChanged() {
    markAsModified();
}

void SpawnPropertiesDialog::markAsModified() {
    if (!m_wasModified) {
        m_wasModified = true;
        setWindowTitle(tr("Spawn Properties *"));
    }
}

void SpawnPropertiesDialog::createBackup() {
    if (m_spawn) {
        m_originalSpawn = new RME::core::spawns::Spawn(m_spawn->deepCopy());
    }
}

void SpawnPropertiesDialog::restoreBackup() {
    if (m_originalSpawn && m_spawn) {
        *m_spawn = *m_originalSpawn;
        loadSpawnData(); // Refresh UI
    }
}

void SpawnPropertiesDialog::connectSignals() {
    // Connect button box
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SpawnPropertiesDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SpawnPropertiesDialog::reject);
    
    // Connect property change signals
    connect(m_spawnRadiusSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SpawnPropertiesDialog::onSpawnRadiusChanged);
}

} // namespace dialogs
} // namespace ui
} // namespace RME