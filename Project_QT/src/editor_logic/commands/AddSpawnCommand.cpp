#include "editor_logic/commands/AddSpawnCommand.h"
#include "core/spawns/SpawnManager.h"
#include "core/spawns/SpawnData.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

AddSpawnCommand::AddSpawnCommand(
    const RME::core::spawns::SpawnData& spawnData,
    RME::core::spawns::SpawnManager* spawnManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_spawnData(spawnData),
    m_spawnManager(spawnManager),
    m_controller(controller)
{
    Q_ASSERT(m_spawnManager);
    Q_ASSERT(m_controller);
    
    setText(QObject::tr("Add spawn at (%1, %2, %3)")
                .arg(m_spawnData.position.x)
                .arg(m_spawnData.position.y)
                .arg(m_spawnData.position.z));
}

void AddSpawnCommand::redo() {
    if (!m_spawnManager || !m_controller) {
        qWarning("AddSpawnCommand::redo: Invalid members.");
        setText(QObject::tr("Add Spawn (Error)"));
        return;
    }
    
    // Add the spawn to the spawn manager
    bool success = m_spawnManager->addSpawn(m_spawnData);
    if (!success) {
        qWarning("AddSpawnCommand::redo: Failed to add spawn at position (%d, %d, %d).",
                 m_spawnData.position.x, m_spawnData.position.y, m_spawnData.position.z);
        setText(QObject::tr("Add Spawn (Failed)"));
        return;
    }
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_spawnData.position);
        m_controller->getMap()->setChanged(true);
    }
    
    m_hasBeenExecuted = true;
    
    qDebug() << "AddSpawnCommand::redo: Added spawn at" << m_spawnData.position.toString()
             << "with" << m_spawnData.creatures.size() << "creature types";
}

void AddSpawnCommand::undo() {
    if (!m_spawnManager || !m_controller || !m_hasBeenExecuted) {
        qWarning("AddSpawnCommand::undo: Invalid state or not executed.");
        return;
    }
    
    // Remove the spawn from the spawn manager
    bool success = m_spawnManager->removeSpawn(m_spawnData.position);
    if (!success) {
        qWarning("AddSpawnCommand::undo: Failed to remove spawn at position (%d, %d, %d).",
                 m_spawnData.position.x, m_spawnData.position.y, m_spawnData.position.z);
        return;
    }
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_spawnData.position);
        m_controller->getMap()->setChanged(true);
    }
    
    setText(QObject::tr("Undo: Add spawn at (%1, %2, %3)")
                .arg(m_spawnData.position.x)
                .arg(m_spawnData.position.y)
                .arg(m_spawnData.position.z));
    
    qDebug() << "AddSpawnCommand::undo: Removed spawn at" << m_spawnData.position.toString();
}

} // namespace actions
} // namespace core
} // namespace RME