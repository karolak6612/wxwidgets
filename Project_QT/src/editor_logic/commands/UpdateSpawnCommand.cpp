#include "editor_logic/commands/UpdateSpawnCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h" // Needed for notifying tile change at spawn center
#include "core/editor/EditorControllerInterface.h"
#include <QObject> // for tr()
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

UpdateSpawnCommand::UpdateSpawnCommand(
    RME::core::Map* map,
    // const RME::core::Position& spawnCenterPos,
    const RME::core::assets::SpawnData& oldSpawnData,
    const RME::core::assets::SpawnData& newSpawnData,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    // m_spawnCenterPos(spawnCenterPos),
    m_oldSpawnData(oldSpawnData), // Store copies
    m_newSpawnData(newSpawnData),
    m_controller(controller)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_controller);
    // The text should reflect what changed, which can be complex.
    // For now, a generic text.
    setText(QObject::tr("Update Spawn at (%1,%2,%3)")
                .arg(m_oldSpawnData.getCenter().x)
                .arg(m_oldSpawnData.getCenter().y)
                .arg(m_oldSpawnData.getCenter().z));
}

bool UpdateSpawnCommand::applySpawnData(const RME::core::assets::SpawnData& dataToApply, const RME::core::assets::SpawnData& originalDataToFind) {
    if (!m_map) return false;

    QList<RME::core::assets::SpawnData>& spawnsInMap = m_map->getSpawns();
    for (int i = 0; i < spawnsInMap.size(); ++i) {
        // We need to identify the *exact* spawn instance.
        // This relies heavily on SpawnData::operator== and careful state management.
        // originalDataToFind is used to locate the spawn to update.
        if (spawnsInMap[i] == originalDataToFind) {
            // Check if center position is changing.
            bool centerChanged = (spawnsInMap[i].getCenter() != dataToApply.getCenter());
            RME::core::Position oldCenterForNotification = spawnsInMap[i].getCenter();

            // Update the SpawnData instance in the map's list.
            spawnsInMap[i] = dataToApply;

            if (m_controller) {
                // Notify change at the new center position.
                m_controller->notifyTileChanged(dataToApply.getCenter());

                // If center changed, also notify the old center position.
                // Also, update tile references.
                if (centerChanged) {
                    m_controller->notifyTileChanged(oldCenterForNotification);

                    // Clear old tile's ref if it pointed to this spawn (now identified by oldDataToFind criteria)
                    RME::core::Tile* oldTile = m_map->getTile(oldCenterForNotification);
                    if (oldTile && oldTile->getSpawnDataRef() && (*oldTile->getSpawnDataRef() == originalDataToFind)) {
                        oldTile->setSpawnDataRef(nullptr);
                    }

                    // Set new tile's ref to the updated spawn in the list
                    RME::core::Tile* newTile = m_map->getOrCreateTile(dataToApply.getCenter());
                    if (newTile) {
                        newTile->setSpawnDataRef(&spawnsInMap[i]); // Point to the updated instance in the list
                    }
                } else {
                    // If center hasn't changed, ensure the tile at that position still points to this updated spawn.
                    // This is important if operator== doesn't guarantee pointer stability or if direct list modification happened.
                    RME::core::Tile* tileAtCenter = m_map->getTile(dataToApply.getCenter());
                    if (tileAtCenter) {
                        tileAtCenter->setSpawnDataRef(&spawnsInMap[i]);
                    }
                }
            }
            return true;
        }
    }
    qWarning("UpdateSpawnCommand::applySpawnData: Could not find original spawn data in map to apply update. Searched for spawn matching center: (%s), radius: %d.",
        qUtf8Printable(originalDataToFind.getCenter().toString()), originalDataToFind.getRadius());
    return false;
}

void UpdateSpawnCommand::redo() {
    // Apply the new state, finding the spawn by its old state.
    if (!applySpawnData(m_newSpawnData, m_oldSpawnData)) {
        qWarning("UpdateSpawnCommand::redo: Failed to apply new spawn data.");
    }
}

void UpdateSpawnCommand::undo() {
    // Apply the old state, finding the spawn by its new state (which is current state on map after redo).
    if (!applySpawnData(m_oldSpawnData, m_newSpawnData)) {
         qWarning("UpdateSpawnCommand::undo: Failed to apply old spawn data (revert).");
    }
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
