#include "editor_logic/commands/AddSpawnCommand.h"
#include "core/spawns/SpawnManager.h"
#include "core/spawns/Spawn.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning

namespace RME {
namespace core {
namespace actions {

AddSpawnCommand::AddSpawnCommand(
    const RME::core::spawns::Spawn& spawn,
    RME::core::spawns::SpawnManager* spawnManager,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_spawn(spawn),
    m_spawnManager(spawnManager),
    m_controller(controller)
{
    Q_ASSERT(m_spawnManager);
    Q_ASSERT(m_controller);
    
    setText(QObject::tr("Add spawn at (%1, %2, %3)")
                .arg(m_spawn.getCenter().x())
                .arg(m_spawn.getCenter().y())
                .arg(m_spawn.getCenter().z()));
}

void AddSpawnCommand::redo() {
    if (!m_spawnManager || !m_controller) {
        qWarning("AddSpawnCommand::redo: Invalid members.");
        setText(QObject::tr("Add Spawn (Error)"));
        return;
    }
    
    // Add the spawn to the spawn manager
    m_spawnManager->addSpawn(m_spawn.getCenter(), m_spawn);
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_spawn.getCenter());
        m_controller->getMap()->setChanged(true);
    }
    
    m_hasBeenExecuted = true;
    
    qDebug() << "AddSpawnCommand::redo: Added spawn at" << m_spawn.getCenter().toString()
             << "with" << m_spawn.getCreatureTypes().size() << "creature types";
}

void AddSpawnCommand::undo() {
    if (!m_spawnManager || !m_controller || !m_hasBeenExecuted) {
        qWarning("AddSpawnCommand::undo: Invalid state or not executed.");
        return;
    }
    
    // Remove the spawn from the spawn manager
    m_spawnManager->removeSpawn(m_spawn.getCenter());
    
    // Notify map about the change
    if (m_controller->getMap()) {
        m_controller->getMap()->notifyTileChanged(m_spawn.getCenter());
        m_controller->getMap()->setChanged(true);
    }
    
    setText(QObject::tr("Undo: Add spawn at (%1, %2, %3)")
                .arg(m_spawn.getCenter().x())
                .arg(m_spawn.getCenter().y())
                .arg(m_spawn.getCenter().z()));
    
    qDebug() << "AddSpawnCommand::undo: Removed spawn at" << m_spawn.getCenter().toString();
}

} // namespace actions
} // namespace core
} // namespace RME