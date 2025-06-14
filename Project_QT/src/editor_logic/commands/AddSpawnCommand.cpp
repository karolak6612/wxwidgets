#include "editor_logic/commands/AddSpawnCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/editor/EditorControllerInterface.h"
#include <QObject> // for tr()
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

AddSpawnCommand::AddSpawnCommand(
    RME::core::Map* map,
    const RME::core::assets::SpawnData& spawnData,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    m_spawnData(spawnData), // Make a copy
    m_centerPosition(spawnData.getCenter()),
    m_controller(controller),
    m_oldSpawnRefOnTile(nullptr),
    m_spawnAddedToMapList(false)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_controller);
    setText(QObject::tr("Add Spawn at (%1,%2,%3)")
                .arg(m_centerPosition.x)
                .arg(m_centerPosition.y)
                .arg(m_centerPosition.z));
}

void AddSpawnCommand::redo() {
    if (!m_map || !m_controller) {
        qWarning("AddSpawnCommand::redo: Map or controller is null.");
        return;
    }

    bool tileWasCreated = false;
    RME::core::Tile* tile = m_map->getOrCreateTile(m_centerPosition, tileWasCreated);
    if (!tile) {
        qWarning("AddSpawnCommand::redo: Could not get or create tile at spawn center.");
        return;
    }

    // Store the old spawn ref from the tile before changing it
    m_oldSpawnRefOnTile = tile->getSpawnDataRef();

    // Add the new spawn data to the map's list.
    // Map::addSpawn takes SpawnData&&. We move a copy.
    RME::core::assets::SpawnData spawnDataForMapList = m_spawnData;
    m_map->addSpawn(std::move(spawnDataForMapList)); // m_map owns this instance now
    m_spawnAddedToMapList = true;

    // Find the reference to the SpawnData now in the map's list.
    // This assumes Map::addSpawn adds to a list and we can get that list.
    // Also assumes SpawnData has operator== for comparison.
    RME::core::assets::SpawnData* refInMap = nullptr;
    // Need to iterate in reverse as addSpawn likely appends
    QList<RME::core::assets::SpawnData>& mapSpawns = m_map->getSpawns(); // Assuming getSpawns() returns a modifiable QList&
    for (int i = mapSpawns.size() - 1; i >= 0; --i) {
        if (mapSpawns[i] == m_spawnData) { // Requires SpawnData::operator==
            refInMap = &mapSpawns[i];
            break;
        }
    }

    if (!refInMap) {
        qWarning("AddSpawnCommand::redo: Could not find the added SpawnData in map's list. Tile ref not set.");
        // If addSpawn failed silently or our find logic is flawed, refInMap could be null.
        // To be more robust, Map::addSpawn could return a pointer or handle to the added/merged spawn.
        // For now, if not found, we might be setting a null ref or an incorrect one if duplicates exist.
        // This might be okay if tile->setSpawnDataRef(nullptr) is acceptable for a "failed link".
    }

    tile->setSpawnDataRef(refInMap);
    m_controller->notifyTileChanged(m_centerPosition);
}

void AddSpawnCommand::undo() {
    if (!m_map || !m_controller) {
        qWarning("AddSpawnCommand::undo: Map or controller is null.");
        return;
    }

    RME::core::Tile* tile = m_map->getTile(m_centerPosition); // Tile should exist
    if (tile) {
        // The SpawnData instance (m_spawnData) that was added to the map's list
        // is the one whose reference should have been set on the tile.
        // So, when restoring, we set the m_oldSpawnRefOnTile back.
        tile->setSpawnDataRef(m_oldSpawnRefOnTile);
    } else {
         qWarning("AddSpawnCommand::undo: Tile at spawn center not found!");
    }

    if (m_spawnAddedToMapList) {
        // Remove the specific SpawnData instance that this command added from the Map's list.
        // This requires m_spawnData to be identifiable (e.g., using operator==).
        // Map::removeSpawn takes const SpawnData&.
        bool removed = m_map->removeSpawn(m_spawnData);
        if (!removed) {
            qWarning("AddSpawnCommand::undo: Failed to remove SpawnData from map list. The exact instance might not have been found.");
        }
        m_spawnAddedToMapList = false;
    }
    m_controller->notifyTileChanged(m_centerPosition);
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
