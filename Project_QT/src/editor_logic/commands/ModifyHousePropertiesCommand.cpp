#include "editor_logic/commands/ModifyHousePropertiesCommand.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/editor/EditorControllerInterface.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

ModifyHousePropertiesCommand::ModifyHousePropertiesCommand(
    quint32 houseId,
    const QHash<QString, QVariant>& newProperties,
    RME::core::houses::Houses* housesManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_houseId(houseId),
    m_newProperties(newProperties),
    m_housesManager(housesManager),
    m_controller(controller)
{
    Q_ASSERT(m_housesManager);
    Q_ASSERT(m_controller);
    Q_ASSERT(m_houseId > 0);
    
    setText(generateCommandText());
}

void ModifyHousePropertiesCommand::redo() {
    if (!m_housesManager || !m_controller) {
        qWarning("ModifyHousePropertiesCommand::redo: Invalid members.");
        setText(QObject::tr("Modify House Properties (Error)"));
        return;
    }
    
    RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
    if (!house) {
        qWarning("ModifyHousePropertiesCommand::redo: House with ID %u not found.", m_houseId);
        setText(QObject::tr("Modify House Properties (Not Found)"));
        return;
    }
    
    // Backup current properties (only on first execution)
    if (!m_hasBackup) {
        backupCurrentProperties();
        m_hasBackup = true;
    }
    
    // Apply new properties
    applyProperties(m_newProperties);
    
    qDebug() << "ModifyHousePropertiesCommand::redo: Modified" << m_newProperties.size() 
             << "properties for house" << m_houseId;
}

void ModifyHousePropertiesCommand::undo() {
    if (!m_housesManager || !m_controller || !m_hasBackup) {
        qWarning("ModifyHousePropertiesCommand::undo: Invalid state or no backup available.");
        return;
    }
    
    // Restore old properties
    applyProperties(m_oldProperties);
    
    setText(QObject::tr("Undo: ") + generateCommandText());
    
    qDebug() << "ModifyHousePropertiesCommand::undo: Restored" << m_oldProperties.size() 
             << "properties for house" << m_houseId;
}

bool ModifyHousePropertiesCommand::mergeWith(const QUndoCommand* other) {
    const ModifyHousePropertiesCommand* otherCmd = 
        dynamic_cast<const ModifyHousePropertiesCommand*>(other);
    
    if (!otherCmd || otherCmd->m_houseId != m_houseId) {
        return false; // Can only merge commands for the same house
    }
    
    // Merge the new properties, keeping our old backup
    for (auto it = otherCmd->m_newProperties.constBegin(); 
         it != otherCmd->m_newProperties.constEnd(); ++it) {
        m_newProperties[it.key()] = it.value();
    }
    
    // Update command text
    setText(generateCommandText());
    
    return true;
}

void ModifyHousePropertiesCommand::backupCurrentProperties() {
    RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
    if (!house) {
        return;
    }
    
    // Backup only the properties that will be changed
    for (auto it = m_newProperties.constBegin(); it != m_newProperties.constEnd(); ++it) {
        const QString& property = it.key();
        
        if (property == "name") {
            m_oldProperties[property] = house->name;
        } else if (property == "entryPoint") {
            m_oldProperties[property] = QVariant::fromValue(house->entryPoint);
        } else if (property == "townId") {
            m_oldProperties[property] = house->townId;
        } else if (property == "rent") {
            m_oldProperties[property] = house->rent;
        } else if (property == "isGuildhall") {
            m_oldProperties[property] = house->isGuildhall;
        }
    }
}

void ModifyHousePropertiesCommand::applyProperties(const QHash<QString, QVariant>& properties) {
    RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
    if (!house) {
        return;
    }
    
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        const QString& property = it.key();
        const QVariant& value = it.value();
        
        if (property == "name") {
            house->name = value.toString();
        } else if (property == "entryPoint") {
            RME::core::Position newEntryPoint = value.value<RME::core::Position>();
            RME::core::Position oldEntryPoint = house->entryPoint;
            house->entryPoint = newEntryPoint;
            
            // Update house exit if entry point changed
            if (newEntryPoint != oldEntryPoint) {
                m_housesManager->setHouseExit(m_houseId, newEntryPoint);
            }
        } else if (property == "townId") {
            house->townId = value.toUInt();
        } else if (property == "rent") {
            house->rent = value.toUInt();
        } else if (property == "isGuildhall") {
            house->isGuildhall = value.toBool();
        } else {
            qWarning() << "ModifyHousePropertiesCommand::applyProperties: Unknown property" << property;
        }
    }
}

QString ModifyHousePropertiesCommand::generateCommandText() const {
    if (m_newProperties.size() == 1) {
        QString property = m_newProperties.keys().first();
        return QObject::tr("Modify house %1 property '%2'").arg(m_houseId).arg(property);
    } else {
        return QObject::tr("Modify house %1 properties (%2 changes)")
            .arg(m_houseId).arg(m_newProperties.size());
    }
}

} // namespace actions
} // namespace core
} // namespace RME