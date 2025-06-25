#include "core/brush/CreatureBrush.h"

#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/creatures/Creature.h"
#include "core/spawns/SpawnData.h"
#include "core/settings/AppSettings.h"
#include "core/editor/EditorControllerInterface.h"

#include <QDebug>
#include <QString>

// No helper functions needed

namespace RME {
namespace core {

CreatureBrush::CreatureBrush(RME::core::editor::EditorControllerInterface* controller, const RME::core::assets::CreatureData* creatureData) :
    Brush(), // Call base Brush constructor
    m_creatureData(creatureData)
{
    // No need to assert here as we allow null creatureData initially
    // Name and description are handled by getName() method
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

    const RME::core::Tile* tile = map->getTile(pos);
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
        if (!m_creatureData->flags.testFlag(RME::core::CreatureTypeFlag::IS_NPC)) {
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

        // Check if tile has a spawn reference or if auto-create spawn is enabled
        if (!tile->getSpawnDataRef() && !appSettings->isAutoCreateSpawnEnabled()) {
            qDebug("CreatureBrush::apply (draw): Tile %s has no spawn and auto-create spawn is disabled.", qUtf8Printable(pos.toString()));
            // In the original RME, creatures could only be placed in spawn areas
            // For now, we'll allow placing creatures without spawns for flexibility
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

        // Auto-spawn removal logic
        RME::core::spawns::SpawnData* currentSpawnData = tile->getSpawnDataRef();
        if (currentSpawnData && currentSpawnData->isAutoCreated()) {
            // If this was an auto-created spawn and the creature we're removing is the only one
            // or if the spawn's creature list is now empty, remove the spawn
            if (creatureTypeToRemove && currentSpawnData->getCreatureTypes().count() == 1 &&
                currentSpawnData->getCreatureTypes().first() == creatureTypeToRemove->name) {
                controller->recordRemoveSpawn(currentSpawnData->getCenter());
            } else if (currentSpawnData->getCreatureTypes().isEmpty()) {
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
    // Tile notification is handled by the commands
}

// Legacy compatibility methods for direct map manipulation
void CreatureBrush::draw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) {
    Q_UNUSED(map)
    
    if (!tile || !settings) {
        qWarning() << "CreatureBrush::draw: Invalid parameters (tile or settings is null)";
        return;
    }
    
    if (!m_creatureData) {
        qWarning() << "CreatureBrush::draw: No creature data set";
        return;
    }
    
    // Create and place creature directly on tile
    auto creature = std::make_unique<RME::core::Creature>(m_creatureData);
    tile->setCreature(std::move(creature));
    
    qDebug() << "CreatureBrush::draw: Placed creature" << m_creatureData->name << "on tile";
}

void CreatureBrush::undraw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) {
    Q_UNUSED(map)
    Q_UNUSED(settings)
    
    if (!tile) {
        qWarning() << "CreatureBrush::undraw: Invalid tile parameter";
        return;
    }
    
    // Remove creature from tile
    tile->setCreature(nullptr);
    qDebug() << "CreatureBrush::undraw: Removed creature from tile";
}

} // namespace core
} // namespace RME
