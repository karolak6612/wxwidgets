#include "core/brush/CreatureBrush.h"

#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/creatures/Creature.h"
#include "core/spawns/SpawnData.h"
#include "core/assets/CreatureData.h"
#include "core/settings/AppSettings.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/Spawn.h" // For RME::Spawn object on tile (marker)

#include <QDebug> // For temporary debugging, remove later

// Anonymous namespace for helper functions if needed
namespace {
// No longer need direct getAppSettings or getMap helpers, will use controller
} // namespace

namespace RME {
namespace core {

CreatureBrush::CreatureBrush() : m_creatureType(nullptr) {
    // Constructor
}

void CreatureBrush::setCreatureType(const RME::core::assets::CreatureData* type) {
    m_creatureType = type;
}

const RME::core::assets::CreatureData* CreatureBrush::getCreatureType() const {
    return m_creatureType;
}

QString CreatureBrush::getName() const {
    if (m_creatureType) {
        return m_creatureType->name;
    }
    return "Creature Brush";
}

int CreatureBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    return 0;
}

// canApply uses const Map*, so it doesn't use EditorControllerInterface directly for map access
// but it might need AppSettings if some conditions depend on global settings.
// For now, assuming AppSettings check is primarily for the 'apply' phase or complex conditions.
bool CreatureBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& settings) const {
    if (!m_creatureType) {
        qWarning() << "CreatureBrush::canApply: No creature type selected.";
        return false;
    }

    if (!map) {
        qWarning() << "CreatureBrush::canApply called with null map.";
        return false;
    }

    const RME::Tile* tile = map->getTile(pos);
    if (!tile) {
        // qWarning() << "CreatureBrush::canApply: No tile at position" << pos.x << pos.y << pos.z;
        return false;
    }

    if (settings.isEraseMode) {
        return tile->hasCreature();
    }

    // Drawing mode checks:
    if (tile->isBlocking()) {
        // qWarning() << "CreatureBrush::canApply: Tile is blocking at" << pos.x << pos.y << pos.z;
        return false;
    }

    // Creature already on tile? Original RME allowed replacing.
    // The `apply` method will handle removal of existing creature if necessary.
    // So, this check is not strictly needed here if replacement is the policy.

    // Spawn conditions:
    // This is simplified here. The definitive check, especially for AUTO_CREATE_SPAWN,
    // which requires AppSettings, is better handled in `apply` where the controller is available.
    // If AppSettings were available here (e.g., passed or via a service locator):
    // RME::core::AppSettings* appSettings = ...;
    // if (!tile->getSpawn() && !tile->getSpawnDataRef() && !appSettings->isAutoCreateSpawnEnabled()) {
    //     return false; // No existing spawn and auto-create is off
    // }

    // PZ/NPC logic:
    if (tile->isPZ()) {
        if (!m_creatureType->flags.testFlag(RME::CreatureTypeFlag::IS_NPC)) {
            // qWarning() << "CreatureBrush::canApply: Cannot place non-NPC in PZ.";
            return false;
        }
    }

    return true;
}

void CreatureBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    if (!controller) {
        qWarning() << "CreatureBrush::apply called with null controller.";
        return;
    }

    RME::core::map::Map* map = controller->getMap();
    RME::core::AppSettings* appSettings = controller->getAppSettings();

    if (!map || !appSettings) {
        qWarning() << "CreatureBrush::apply: Missing map or appSettings from controller.";
        return;
    }

    // Get tile for editing (might create it if it doesn't exist, or just get it)
    RME::core::Tile* tile = controller->getTileForEditing(pos);
    if (!tile) {
        qWarning() << "CreatureBrush::apply: Failed to get tile for editing at" << pos.x << pos.y << pos.z;
        return;
    }

    // --- Pre-condition check using potentially more context from controller ---
    // (This is a more robust check than the public canApply, using controller resources)
    bool canProceed = true;
    if (settings.isEraseMode) {
        if (!tile->hasCreature()) canProceed = false;
    } else { // Drawing mode
        if (!m_creatureType) canProceed = false;
        else if (tile->isBlocking()) canProceed = false;
        else if (tile->isPZ() && !m_creatureType->flags.testFlag(RME::CreatureTypeFlag::IS_NPC)) canProceed = false;
        else {
            // Check spawn condition precisely
            bool existingSpawnCoversTile = false;
            if (tile->getSpawn() || tile->getSpawnDataRef()) { // Tile is center of a spawn
                existingSpawnCoversTile = true;
            } else { // Check if any spawn area covers this tile
                // This might require a map method: existingSpawnCoversTile = map->isTileCoveredBySpawn(pos);
                // For now, we assume if tile->getSpawn/getSpawnDataRef is null, no *centered* spawn is here.
                // The broader check for "covered by any spawn" is more complex.
            }

            if (!existingSpawnCoversTile && !appSettings->isAutoCreateSpawnEnabled()) {
                canProceed = false; // No existing spawn and auto-create is off
            }
        }
    }

    if (!canProceed) {
        qDebug() << "CreatureBrush::apply: Pre-conditions not met for operation at" << pos.x << pos.y << pos.z;
        return;
    }
    // --- End Pre-condition check ---


    if (settings.isEraseMode) {
        if (tile->hasCreature()) { // Check if there is a creature to erase
            const RME::core::assets::CreatureData* creatureTypeToRemove = tile->getCreature()->getType();
            controller->recordRemoveCreature(pos, creatureTypeToRemove);

            // Now check for auto-spawn removal
            // This assumes that 'tile' pointer is still valid and its SpawnDataRef might have been updated by the controller,
            // or that SpawnData objects are managed in a way that this reference remains valid.
            RME::SpawnData* currentSpawnData = tile->getSpawnDataRef();
            if (currentSpawnData && currentSpawnData->isAutoCreated()) {
                // Condition: if the spawn only contained this one creature type and its count was 1.
                // This is a simplification. A more robust check would be if the SpawnData's creature list
                // becomes empty after this creature is conceptually removed from it by the controller's action.
                // If controller->recordRemoveCreature also implies removal from spawn's list:
                // if (currentSpawnData->getCreatureTypes().isEmpty()) {
                // For now, using the provided logic:
                if (currentSpawnData->getCreatureTypes().count() == 1 &&
                    currentSpawnData->getCreatureTypes().first() == creatureTypeToRemove->name) {
                    // It's also possible the controller has already removed this creature type from currentSpawnData's list.
                    // If so, the check should be currentSpawnData->getCreatureTypes().isEmpty()
                    // This part of the logic is highly dependent on controller implementation details.
                    // Let's assume for now the list in currentSpawnData is NOT YET updated by recordRemoveCreature,
                    // so the check against creatureTypeToRemove name and count == 1 is more appropriate.
                    controller->recordRemoveSpawn(currentSpawnData->getCenter());
                }
            }
        }
    } else { // Drawing mode
        if (!m_creatureType) return;

        // If there's an existing creature, remove it first.
        if (tile->getCreature()) {
            const RME::core::assets::CreatureData* oldCreatureType = tile->getCreature()->getType();
            controller->recordRemoveCreature(pos, oldCreatureType);
            // The tile should be updated by the above call before adding the new one.
            // We might need to re-fetch the tile if the controller invalidates the pointer,
            // but for now, assume tile pointer remains valid or controller handles it.
        }

        controller->recordAddCreature(pos, m_creatureType);

        // Auto-create spawn logic
        bool existingSpawnCoversTile = false;
        if (tile->getSpawn() || tile->getSpawnDataRef()) {
             existingSpawnCoversTile = true;
        } else {
            // existingSpawnCoversTile = map->isTileCoveredBySpawn(pos); // More robust check
        }

        if (appSettings->isAutoCreateSpawnEnabled() && !existingSpawnCoversTile) {
            RME::SpawnData newSpawnData(
                pos, // Center
                1,   // Radius (default for auto-spawn)
                appSettings->getDefaultSpawnTime(),
                {m_creatureType->name}
            );
            newSpawnData.setIsAutoCreated(true); // Set the new flag
            controller->recordAddSpawn(newSpawnData);
        }

    }
    controller->notifyTileChanged(pos); // Notify that the tile might have changed
}

} // namespace core
} // namespace RME
