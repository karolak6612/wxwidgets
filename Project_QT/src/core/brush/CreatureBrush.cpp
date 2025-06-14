#include "core/brush/CreatureBrush.h"

#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/creatures/Creature.h"
// #include "core/spawns/SpawnData.h" // SpawnData is not directly used by CreatureBrush logic after CORE-10.
// CreatureData is included from CreatureBrush.h
#include "core/settings/AppSettings.h"
#include "core/editor/EditorControllerInterface.h"
// core/Spawn.h is obsolete, ensure it's removed.

#include <QDebug>
#include <QString> // For QStringLiteral and arg

// Anonymous namespace for helper functions if needed
namespace {
// No longer need direct getAppSettings or getMap helpers, will use controller
} // namespace

namespace RME {
namespace core {

CreatureBrush::CreatureBrush(RME::core::editor::EditorControllerInterface* controller, const RME::core::assets::CreatureData* creatureData) :
    RME::core::brush::Brush(controller), // Call base Brush constructor
    m_creatureData(creatureData)
{
    Q_ASSERT(m_creatureData != nullptr && "CreatureData cannot be null for CreatureBrush");
    if (m_creatureData) {
        // Set base brush name and description dynamically based on creature
        // Base class Brush constructor might need adjustment if it expects these, or add setters.
        // For now, assuming base Brush has setters or this logic is handled differently.
        // Brush::setName(m_creatureData->name);
        // Brush::setDescription(QString("Places %1 creatures.").arg(m_creatureData->name));
    }
}

void CreatureBrush::setCreatureData(const RME::core::assets::CreatureData* creatureData) {
    m_creatureData = creatureData;
}

const RME::core::assets::CreatureData* CreatureBrush::getCreatureData() const {
    return m_creatureData;
}

QString CreatureBrush::getName() const {
    if (m_creatureData) {
        return m_creatureData->name;
    }
    return QStringLiteral("Creature Brush (Unset)"); // Fallback name
}

int CreatureBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    // For now, returning 0. If CreatureData had a looktype/itemid, it could be returned.
    return 0;
}

bool CreatureBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& settings) const {
    if (!m_creatureData) {
        qWarning("CreatureBrush::canApply: No creature data set for the brush.");
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
    // This pre-apply check logic should align with canApply or be the sole place for complex checks.
    // For now, keeping it similar to canApply but using controller for AppSettings.
    if (settings.isEraseMode) {
        if (!tile->hasCreature()) {
             qDebug("CreatureBrush::apply (erase): No creature to erase at %s.", qUtf8Printable(pos.toString()));
            return; // Nothing to do
        }
    } else { // Drawing mode
        if (!m_creatureData) { // Check m_creatureData
            qWarning("CreatureBrush::apply (draw): No creature data set for the brush.");
            return;
        }
        if (tile->isBlocking()) {
             qDebug("CreatureBrush::apply (draw): Tile is blocking at %s.", qUtf8Printable(pos.toString()));
            return;
        }
        if (tile->isPZ() && !m_creatureData->flags.testFlag(RME::CreatureTypeFlag::IS_NPC)) {
             qDebug("CreatureBrush::apply (draw): Cannot place non-NPC in PZ at %s.", qUtf8Printable(pos.toString()));
            return;
        }

        // Spawn logic needs to be re-evaluated with new SpawnData system
        // For now, assume that if a tile doesn't have a SpawnDataRef, we might create one if auto-create is on.
        // This part is complex and depends on how CORE-10 was fully implemented.
        // The original logic involving tile->getSpawn() is for the old Spawn class.
        // For now, let's simplify: if auto-create is on and no spawn *center* is here, a spawn might be created.
        // The actual check for "is this tile part of *any* spawn area" is map-level.
        if (!tile->getSpawnDataRef() && !appSettings->isAutoCreateSpawnEnabled()) {
             qDebug("CreatureBrush::apply (draw): Tile %s has no spawn and auto-create spawn is disabled.", qUtf8Printable(pos.toString()));
            // return; // This might be too restrictive if creature can be placed without spawn.
                      // RME typically requires spawns for creatures.
        }
    }
    // --- End Pre-condition check ---


    if (settings.isEraseMode) {
        // tile->hasCreature() was checked by pre-conditions
        const RME::core::assets::CreatureData* creatureTypeToRemove = nullptr;
        if(tile->getCreature()) { // Get type if creature exists
             creatureTypeToRemove = tile->getCreature()->getType();
        }
        // recordRemoveCreature doesn't strictly need creatureTypeToRemove if it removes any creature.
        // Passing it can be for logging or future validation within the command.
        controller->recordRemoveCreature(pos, creatureTypeToRemove);

        // Auto-spawn removal logic (simplified, assumes SpawnData is managed by Map and Tile::spawnDataRef points to it)
        RME::core::spawns::SpawnData* currentSpawnData = tile->getSpawnDataRef(); // Use new SpawnData
        if (currentSpawnData && currentSpawnData->isAutoCreated()) {
            // This logic is tricky: does recordRemoveCreature update the spawn's creature list synchronously?
            // Assuming for now that if a creature is removed, and the spawn was auto-created
            // and *might* now be empty or only contained this creature, it should be removed.
            // A more robust solution would be for recordRemoveCreature to handle this,
            // or for map to have a "cleanup empty auto spawns" function.
            // For this refactor, let's keep it simple: if the creature was the only type in this auto-spawn, remove spawn.
            // This requires knowing the creature type we just removed.
            if (creatureTypeToRemove && currentSpawnData->getCreatureTypes().count() == 1 &&
                currentSpawnData->getCreatureTypes().first() == creatureTypeToRemove->name) {
                controller->recordRemoveSpawn(currentSpawnData->getCenter());
            } else if (currentSpawnData->getCreatureTypes().isEmpty()){
                 // If after (potential) creature removal from spawn list by recordRemoveCreature, list is empty
                 controller->recordRemoveSpawn(currentSpawnData->getCenter());
            }
        }
    } else { // Drawing mode
        if (!m_creatureData) return; // Should have been caught by canApply/pre-check

        // If there's an existing creature, AddCreatureCommand should handle replacing it.
        // No explicit removal here is needed if AddCreatureCommand's redo first pops existing.
        controller->recordAddCreature(pos, m_creatureData);

        // Auto-create spawn logic
        if (appSettings->isAutoCreateSpawnEnabled() && !tile->getSpawnDataRef()) {
            // Check if tile is covered by any *other* spawn first. This is complex.
            // For now, only create if this tile is not already a spawn center.
            RME::core::spawns::SpawnData newSpawnData(
                pos, // Center
                1,   // Radius (default for auto-spawn, as per original SpawnBrush)
                appSettings->getDefaultSpawnTime(), // Assuming AppSettings provides this
                {m_creatureData->name}
            );
            newSpawnData.setIsAutoCreated(true);
            controller->recordAddSpawn(newSpawnData);
        }
    }
    // Tile notification should be handled by commands (AddCreatureCommand, RemoveCreatureCommand, etc.)
    // controller->notifyTileChanged(pos); // This might be redundant if commands do it.
}

} // namespace core
} // namespace RME
