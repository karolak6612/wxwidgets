#include "editor_logic/commands/RemoveSpawnCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/editor/EditorControllerInterface.h"
#include <QObject> // for tr()
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

RemoveSpawnCommand::RemoveSpawnCommand(
    RME::core::Map* map,
    const RME::core::Position& spawnCenterPos,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    m_centerPosition(spawnCenterPos),
    m_controller(controller),
    m_removedSpawnDataCopy(nullptr),
    m_originalSpawnRefOnTile(nullptr),
    m_spawnEffectivelyRemoved(false)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_controller);
    // Text will be set in redo based on whether a spawn was actually found and removed.
    setText(QObject::tr("Remove Spawn at (%1,%2,%3)")
                .arg(m_centerPosition.x)
                .arg(m_centerPosition.y)
                .arg(m_centerPosition.z));
}

void RemoveSpawnCommand::redo() {
    if (!m_map || !m_controller) {
        qWarning("RemoveSpawnCommand::redo: Map or controller is null.");
        m_spawnEffectivelyRemoved = false;
        return;
    }

    RME::core::Tile* tile = m_map->getTile(m_centerPosition);
    if (!tile) {
        qWarning("RemoveSpawnCommand::redo: Tile not found at spawn center (%s) to remove spawn from.", qUtf8Printable(m_centerPosition.toString()));
        setText(QObject::tr("Remove Spawn (tile not found)"));
        m_spawnEffectivelyRemoved = false;
        return;
    }

    m_originalSpawnRefOnTile = tile->getSpawnDataRef(); // Capture before clearing

    if (!m_originalSpawnRefOnTile) {
        qDebug("RemoveSpawnCommand::redo: No spawn reference on tile at %s to remove.", qUtf8Printable(m_centerPosition.toString()));
        setText(QObject::tr("Remove Spawn (none found on tile)"));
        m_spawnEffectivelyRemoved = false;
        return; // Nothing to remove from this tile's perspective
    }

    // Make a deep copy of the SpawnData this tile was pointing to.
    m_removedSpawnDataCopy = std::make_unique<RME::core::assets::SpawnData>(*m_originalSpawnRefOnTile);

    // Remove this SpawnData instance from the map's list
    bool removedFromMapList = m_map->removeSpawn(*m_removedSpawnDataCopy); // Uses operator==

    if (removedFromMapList) {
        tile->setSpawnDataRef(nullptr); // Clear the reference on the tile
        m_spawnEffectivelyRemoved = true;
        setText(QObject::tr("Remove Spawn (Radius:%1) at (%2,%3,%4)")
                    .arg(m_removedSpawnDataCopy->getRadius())
                    .arg(m_centerPosition.x)
                    .arg(m_centerPosition.y)
                    .arg(m_centerPosition.z));
        m_controller->notifyTileChanged(m_centerPosition);
    } else {
        qWarning("RemoveSpawnCommand::redo: Tile's spawnDataRef at %s was not found in Map's spawn list, but clearing tile's ref anyway.", qUtf8Printable(m_centerPosition.toString()));
        tile->setSpawnDataRef(nullptr);
        m_spawnEffectivelyRemoved = true; // Still consider it "removed" from tile's perspective for undo consistency.
                                          // The warning indicates potential data inconsistency in the map's list.
        setText(QObject::tr("Remove Spawn (ref cleared, but not found in map list) at (%1,%2,%3)")
                     .arg(m_centerPosition.x)
                     .arg(m_centerPosition.y)
                     .arg(m_centerPosition.z));
        m_controller->notifyTileChanged(m_centerPosition);
    }
}

void RemoveSpawnCommand::undo() {
    if (!m_map || !m_controller) {
        qWarning("RemoveSpawnCommand::undo: Map or controller is null.");
        return;
    }

    if (!m_spawnEffectivelyRemoved) {
        // If redo didn't do anything, undo shouldn't either.
        // However, if redo cleared a ref that wasn't in map list, we should restore that ref.
        RME::core::Tile* tile = m_map->getTile(m_centerPosition);
        if(tile && tile->getSpawnDataRef() != m_originalSpawnRefOnTile) {
            tile->setSpawnDataRef(m_originalSpawnRefOnTile);
            m_controller->notifyTileChanged(m_centerPosition);
        }
        return;
    }

    if (!m_removedSpawnDataCopy) {
        // This implies m_spawnEffectivelyRemoved was true, but we have no copy. This is an inconsistent state.
        qWarning("RemoveSpawnCommand::undo: Spawn was marked effectively removed, but no copy of SpawnData exists to restore.");
        // Try to restore original ref on tile at least.
        RME::core::Tile* tile = m_map->getTile(m_centerPosition);
        if(tile && tile->getSpawnDataRef() != m_originalSpawnRefOnTile) {
            tile->setSpawnDataRef(m_originalSpawnRefOnTile);
            m_controller->notifyTileChanged(m_centerPosition);
        }
        return;
    }

    // Add the copied SpawnData back to the map's list.
    RME::core::assets::SpawnData spawnDataToRestore = *m_removedSpawnDataCopy;
    m_map->addSpawn(std::move(spawnDataToRestore));

    RME::core::assets::SpawnData* refInMap = nullptr;
    QList<RME::core::assets::SpawnData>& mapSpawns = m_map->getSpawns();
    for (int i = mapSpawns.size() - 1; i >= 0; --i) {
        if (mapSpawns[i] == *m_removedSpawnDataCopy) {
            refInMap = &mapSpawns[i];
            break;
        }
    }

    if (!refInMap) {
        qWarning("RemoveSpawnCommand::undo: Could not find the re-added SpawnData in map's list for tile at %s. Tile ref cannot be restored to this specific instance.", qUtf8Printable(m_centerPosition.toString()));
        // The object is in the list, but we can't get its pointer via this search.
        // This implies an issue with operator== or the list management.
        // The tile's ref will be restored to m_originalSpawnRefOnTile if it's different from current.
    }

    RME::core::Tile* tile = m_map->getTile(m_centerPosition);
    if (tile) {
        // Restore to the specific instance re-added to the map if found, otherwise original (which might be the same if no duplicates)
        tile->setSpawnDataRef(refInMap ? refInMap : m_originalSpawnRefOnTile);
    } else {
        qWarning("RemoveSpawnCommand::undo: Tile at spawn center %s not found! Cannot restore spawn ref.", qUtf8Printable(m_centerPosition.toString()));
    }
    m_controller->notifyTileChanged(m_centerPosition);
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
