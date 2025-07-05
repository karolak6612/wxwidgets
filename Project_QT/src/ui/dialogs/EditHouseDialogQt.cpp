#include "EditHouseDialogQt.h"
#include "core/houses/HouseData.h"
#include "core/world/TownManager.h"
#include "core/world/TownData.h"

namespace RME {
namespace ui {
namespace dialogs {

EditHouseDialogQt::EditHouseDialogQt(QWidget* parent,
                                     RME::core::houses::HouseData* houseCopy,
                                     RME::core::world::TownManager* townManager)
    : QDialog(parent)
    , m_houseCopy(houseCopy)
    , m_townManager(townManager)
{
    if (m_houseCopy) {
        m_originalHouseId = m_houseCopy->getId();
    }
    
    setWindowTitle("Edit House Properties");
    setModal(true);
    
    setupUI();
    connectSignals();
    loadData();
}

void EditHouseDialogQt::setupUI()
{
    m_formLayout = new QFormLayout(this);
    
    // House name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(255);
    m_formLayout->addRow("Name:", m_nameEdit);
    
    // Town selection
    m_townCombo = new QComboBox(this);
    populateTownCombo();
    m_formLayout->addRow("Town:", m_townCombo);
    
    // Rent
    m_rentSpinBox = new QSpinBox(this);
    m_rentSpinBox->setMinimum(0);
    m_rentSpinBox->setMaximum(999999999); // Large maximum for rent
    m_rentSpinBox->setSuffix(" gp");
    m_formLayout->addRow("Rent:", m_rentSpinBox);
    
    // House ID
    m_idSpinBox = new QSpinBox(this);
    m_idSpinBox->setMinimum(1);
    m_idSpinBox->setMaximum(65535);
    m_formLayout->addRow("House ID:", m_idSpinBox);
    
    // Guildhall status
    m_guildhallCheck = new QCheckBox("Is Guildhall", this);
    m_formLayout->addRow(m_guildhallCheck);
    
    // Dialog buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_formLayout->addRow(m_buttonBox);
    
    // Set focus to name field
    m_nameEdit->setFocus();
}

void EditHouseDialogQt::connectSignals()
{
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &EditHouseDialogQt::onAccepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &EditHouseDialogQt::onRejected);
}

void EditHouseDialogQt::loadData()
{
    if (!m_houseCopy) return;
    
    // Load house data into controls
    m_nameEdit->setText(m_houseCopy->getName());
    m_rentSpinBox->setValue(static_cast<int>(m_houseCopy->getRent()));
    m_idSpinBox->setValue(static_cast<int>(m_houseCopy->getId()));
    m_guildhallCheck->setChecked(m_houseCopy->isGuildhall());
    
    // Set town selection
    quint32 townId = m_houseCopy->getTownId();
    for (int i = 0; i < m_townCombo->count(); ++i) {
        if (m_townCombo->itemData(i).toUInt() == townId) {
            m_townCombo->setCurrentIndex(i);
            break;
        }
    }
}

bool EditHouseDialogQt::applyChanges()
{
    if (!validateInputs()) {
        return false;
    }
    
    if (!m_houseCopy) return false;
    
    // Apply changes to house copy
    m_houseCopy->setName(m_nameEdit->text().trimmed());
    m_houseCopy->setRent(static_cast<quint32>(m_rentSpinBox->value()));
    m_houseCopy->setId(static_cast<quint32>(m_idSpinBox->value()));
    m_houseCopy->setGuildhall(m_guildhallCheck->isChecked());
    
    // Set town ID
    quint32 townId = m_townCombo->currentData().toUInt();
    m_houseCopy->setTownId(townId);
    
    return true;
}

bool EditHouseDialogQt::validateInputs()
{
    // Validate name
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "House name cannot be empty.");
        m_nameEdit->setFocus();
        return false;
    }
    
    // Validate rent (non-negative)
    if (m_rentSpinBox->value() < 0) {
        QMessageBox::warning(this, "Validation Error", "Rent must be non-negative.");
        m_rentSpinBox->setFocus();
        return false;
    }
    
    // Validate ID (unique if changed)
    quint32 newId = static_cast<quint32>(m_idSpinBox->value());
    if (newId != m_originalHouseId) {
        // Add validation for unique ID using HouseManager
        // TODO: Need to pass HouseManager or ClientDataService to constructor for full validation
        // For now, enhanced warning about potential conflicts
        // For now, we'll show a warning about potential conflicts
        if (newId == m_originalHouseId) {
            // Same ID, no problem
        } else {
            // Different ID - in a real implementation, we'd check with HouseManager
            // For now, just warn the user
            int result = QMessageBox::question(this, "ID Change Warning",
                                               QString("You are changing the house ID from %1 to %2. "
                                                       "This may cause conflicts if another house already uses this ID. "
                                                       "Continue anyway?")
                                               .arg(m_originalHouseId).arg(newId),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::No);
            if (result != QMessageBox::Yes) {
                m_idSpinBox->setFocus();
                return false;
            }
        }
    }
    
    return true;
}

void EditHouseDialogQt::populateTownCombo()
{
    if (!m_townManager) return;
    
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
}

void EditHouseDialogQt::onAccepted()
{
    if (applyChanges()) {
        accept();
    }
}

void EditHouseDialogQt::onRejected()
{
    reject();
}

} // namespace dialogs
} // namespace ui
} // namespace RME