#include "editor_logic/commands/RemoveSpawnCommand.h"
#include "core/spawns/SpawnManager.h"
#include "core/spawns/Spawn.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

RemoveSpawnCommand::RemoveSpawnCommand(
    const RME::core::Position& position,
    RME::core::spawns::SpawnManager* spawnManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_position(position),
    m_spawnManager(spawnManager),
    m_controller(controller)
{
    Q_ASSERT(m_spawnManager);
    Q_ASSERT(m_controller);
    
    setText(QObject::tr("Remove spawn at (%1, %2, %3)")
                .arg(m_position.x)
                .arg(m_position.y)
                .arg(m_position.z));
}

void RemoveSpawnCommand::redo() {
    if (!m_spawnManager || !m_controller) {
        qWarning("RemoveSpawnCommand::redo: Invalid members.");
        setText(QObject::tr("Remove Spawn (Error)"));
        return;
    }
    
    // Backup spawn data for undo (only on first execution)
    if (!m_hasBackup) {
        const RME::core::spawns::Spawn* existingSpawn = m_spawnManager->getSpawn(m_position);
        if (!existingSpawn) {
            qWarning("RemoveSpawnCommand::redo: No spawn found at position (%d, %d, %d).",
                     m_position.x, m_position.y, m_position.z);
            setText(QObject::tr("Remove Spawn (Not Found)"));
            return;
        }
        
        // Backup the spawn data
        m_backupSpawn = *existingSpawn;
        m_hasBackup = true;
        
        qDebug() << "RemoveSpawnCommand::redo: Backed up spawn at" << m_position.toString()
                 << "with" << m_backupSpawn.getCreatureTypes().size() << "creature types";
    }
    
    // Remove the spawn from the spawn manager
    m_spawnManager->removeSpawn(m_position);
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_position);
        m_controller->getMap()->setChanged(true);
    }
    
    m_hasBeenExecuted = true;
    
    qDebug() << "RemoveSpawnCommand::redo: Removed spawn at" << m_position.toString();
}

void RemoveSpawnCommand::undo() {
    if (!m_spawnManager || !m_controller || !m_hasBackup || !m_hasBeenExecuted) {
        qWarning("RemoveSpawnCommand::undo: Invalid state, no backup, or not executed.");
        return;
    }
    
    // Restore the spawn from backup
    m_spawnManager->addSpawn(m_position, m_backupSpawn);
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_position);
        m_controller->getMap()->setChanged(true);
    }
    
    setText(QObject::tr("Undo: Remove spawn at (%1, %2, %3)")
                .arg(m_position.x)
                .arg(m_position.y)
                .arg(m_position.z));
    
    qDebug() << "RemoveSpawnCommand::undo: Restored spawn at" << m_position.toString()
             << "with" << m_backupSpawn.getCreatureTypes().size() << "creature types";
}

} // namespace actions
} // namespace core
} // namespace RME