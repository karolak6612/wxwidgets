#include "editor_logic/commands/RemoveHouseCommand.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/editor/EditorControllerInterface.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

RemoveHouseCommand::RemoveHouseCommand(
    quint32 houseId,
    RME::core::houses::Houses* housesManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_houseId(houseId),
    m_housesManager(housesManager),
    m_controller(controller)
{
    Q_ASSERT(m_housesManager);
    Q_ASSERT(m_controller);
    Q_ASSERT(m_houseId > 0);
    
    // Get house data for backup and command text
    const RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
    if (house) {
        setText(QObject::tr("Remove house '%1' (ID: %2)").arg(house->name).arg(m_houseId));
    } else {
        setText(QObject::tr("Remove house (ID: %1)").arg(m_houseId));
    }
}

void RemoveHouseCommand::redo() {
    if (!m_housesManager || !m_controller) {
        qWarning("RemoveHouseCommand::redo: Invalid members.");
        setText(QObject::tr("Remove House (Error)"));
        return;
    }
    
    // Backup house data and tile positions for undo (only on first execution)
    if (!m_hasBackup) {
        const RME::core::houses::HouseData* house = m_housesManager->getHouse(m_houseId);
        if (!house) {
            qWarning("RemoveHouseCommand::redo: House with ID %u not found.", m_houseId);
            setText(QObject::tr("Remove House (Not Found)"));
            return;
        }
        
        // Backup house data
        m_backupHouseData = *house;
        
        // Backup all tile positions that belong to this house
        m_backupTilePositions = m_housesManager->getHouseTilePositions(m_houseId);
        
        m_hasBackup = true;
        
        qDebug() << "RemoveHouseCommand::redo: Backed up house" << m_houseId 
                 << "with" << m_backupTilePositions.size() << "tiles";
    }
    
    // Remove the house (this will also clean up all tile assignments)
    bool success = m_housesManager->removeHouse(m_houseId);
    if (!success) {
        qWarning("RemoveHouseCommand::redo: Failed to remove house with ID %u.", m_houseId);
        setText(QObject::tr("Remove House (Failed)"));
        return;
    }
    
    qDebug() << "RemoveHouseCommand::redo: Removed house" << m_houseId 
             << "(" << m_backupHouseData.name << ")";
}

void RemoveHouseCommand::undo() {
    if (!m_housesManager || !m_controller || !m_hasBackup) {
        qWarning("RemoveHouseCommand::undo: Invalid state or no backup available.");
        return;
    }
    
    // Restore the house
    bool success = m_housesManager->addExistingHouse(m_backupHouseData);
    if (!success) {
        qWarning("RemoveHouseCommand::undo: Failed to restore house with ID %u.", m_houseId);
        return;
    }
    
    // Restore house exit
    if (m_backupHouseData.entryPoint.isValid()) {
        m_housesManager->setHouseExit(m_houseId, m_backupHouseData.entryPoint);
    }
    
    // Restore all tile assignments
    for (const RME::core::Position& pos : m_backupTilePositions) {
        m_housesManager->linkTileToHouse(m_houseId, pos);
    }
    
    setText(QObject::tr("Undo: Remove house '%1' (ID: %2)").arg(m_backupHouseData.name).arg(m_houseId));
    
    qDebug() << "RemoveHouseCommand::undo: Restored house" << m_houseId 
             << "with" << m_backupTilePositions.size() << "tiles";
}

} // namespace actions
} // namespace core
} // namespace RME