#include "editor_logic/commands/CreateHouseCommand.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/editor/EditorControllerInterface.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

CreateHouseCommand::CreateHouseCommand(
    const QString& houseName,
    const RME::core::Position& entryPoint,
    quint32 townId,
    quint32 rent,
    bool isGuildhall,
    RME::core::houses::Houses* housesManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_houseName(houseName),
    m_entryPoint(entryPoint),
    m_townId(townId),
    m_rent(rent),
    m_isGuildhall(isGuildhall),
    m_housesManager(housesManager),
    m_controller(controller)
{
    Q_ASSERT(m_housesManager);
    Q_ASSERT(m_controller);
    
    setText(QObject::tr("Create house '%1'").arg(m_houseName));
}

void CreateHouseCommand::redo() {
    if (!m_housesManager || !m_controller) {
        qWarning("CreateHouseCommand::redo: Invalid members.");
        setText(QObject::tr("Create House (Error)"));
        return;
    }
    
    if (!m_hasBeenExecuted) {
        // First execution - create the house
        RME::core::houses::HouseData* newHouse = m_housesManager->createNewHouse();
        if (!newHouse) {
            qWarning("CreateHouseCommand::redo: Failed to create new house.");
            setText(QObject::tr("Create House (Failed)"));
            return;
        }
        
        m_createdHouseId = newHouse->id;
        m_hasBeenExecuted = true;
    } else {
        // Re-execution after undo - recreate the house with the same ID
        RME::core::houses::HouseData houseData;
        houseData.id = m_createdHouseId;
        houseData.name = m_houseName;
        houseData.entryPoint = m_entryPoint;
        houseData.townId = m_townId;
        houseData.rent = m_rent;
        houseData.isGuildhall = m_isGuildhall;
        
        if (!m_housesManager->addExistingHouse(houseData)) {
            qWarning("CreateHouseCommand::redo: Failed to re-add house with ID %u.", m_createdHouseId);
            setText(QObject::tr("Create House (Re-add Failed)"));
            return;
        }
    }
    
    // Set house properties
    RME::core::houses::HouseData* house = m_housesManager->getHouse(m_createdHouseId);
    if (house) {
        house->name = m_houseName;
        house->entryPoint = m_entryPoint;
        house->townId = m_townId;
        house->rent = m_rent;
        house->isGuildhall = m_isGuildhall;
        
        // Set house exit if entry point is valid
        if (m_entryPoint.isValid()) {
            m_housesManager->setHouseExit(m_createdHouseId, m_entryPoint);
        }
    }
    
    setText(QObject::tr("Create house '%1' (ID: %2)").arg(m_houseName).arg(m_createdHouseId));
    
    qDebug() << "CreateHouseCommand::redo: Created house" << m_createdHouseId 
             << "with name" << m_houseName;
}

void CreateHouseCommand::undo() {
    if (!m_housesManager || m_createdHouseId == 0) {
        qWarning("CreateHouseCommand::undo: Invalid state.");
        return;
    }
    
    // Remove the house (this will also clean up all tile assignments)
    bool success = m_housesManager->removeHouse(m_createdHouseId);
    if (!success) {
        qWarning("CreateHouseCommand::undo: Failed to remove house with ID %u.", m_createdHouseId);
    }
    
    setText(QObject::tr("Undo: Create house '%1' (ID: %2)").arg(m_houseName).arg(m_createdHouseId));
    
    qDebug() << "CreateHouseCommand::undo: Removed house" << m_createdHouseId;
}

} // namespace actions
} // namespace core
} // namespace RME