#include "core/brush/SpawnBrush.h"
#include "core/map/Map.h"         // For GetTile, IsPositionValid
#include "core/Tile.h"            // For Tile methods (getSpawn, setSpawn)
#include "core/Spawn.h"           // For RME::Spawn object
#include "core/editor/EditorControllerInterface.h" // For recording changes
#include "core/settings/BrushSettings.h"   // For getting spawn radius (brush size)
#include "editor_logic/commands/RecordSetSpawnCommand.h" // Added for the new command

#include <QDebug> // For qDebug, qWarning
#include <memory> // For std::unique_ptr

// Define a default spawn time if not specified elsewhere (e.g., in game_constants or AppSettings)
const int DEFAULT_SPAWN_INTERVAL_SECONDS = 60; // Example: 60 seconds

namespace RME {
namespace core {
namespace brush {

SpawnBrush::SpawnBrush() {
    // Constructor
}

QString SpawnBrush::getName() const {
    return QStringLiteral("Spawn Brush");
}

int SpawnBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    // Spawns don't typically have a specific item ID to show as a brush icon.
    // This might return an ID of a generic "spawn icon" if one exists in the item definitions
    // for UI representation in a brush palette. For now, returning 0.
    return 0;
}

bool SpawnBrush::canApply(const RME::core::map::Map* map,
                            const RME::core::Position& pos,
                            const RME::core::BrushSettings& /*settings*/) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    const Tile* tile = map->getTile(pos);
    if (!tile || !tile->getGround()) {
        return false;
    }
    return true;
}

void SpawnBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                         const RME::core::Position& pos,
                         const RME::core::BrushSettings& settings) {
    if (!controller) {
        qWarning("SpawnBrush::apply: Null controller.");
        return;
    }
    Map* map = controller->getMap();
    if (!canApply(map, pos, settings)) {
        qWarning("SpawnBrush::apply: Cannot apply at position %s.", qUtf8Printable(pos.toString()));
        return;
    }

    Tile* tile = map->getTileForEditing(pos);
    if (!tile) {
        qWarning("SpawnBrush::apply: Failed to get tile for editing at %s", qUtf8Printable(pos.toString()));
        return;
    }

    std::unique_ptr<Spawn> oldSpawn = tile->popSpawn();

    if (settings.isEraseMode) {
        if (oldSpawn) { // If there was a spawn to erase
            qDebug("SpawnBrush::apply (erase): Clearing spawn at %s", qUtf8Printable(pos.toString()));
            // newSpawn is nullptr because we are erasing.
            // oldSpawn is moved into the command.
            auto command = std::make_unique<RME_COMMANDS::RecordSetSpawnCommand>(
                tile,           // The tile being modified
                nullptr,        // newSpawn state (nullptr for erase)
                std::move(oldSpawn), // oldSpawn state
                controller      // The editor controller
            );
            controller->pushCommand(std::move(command));
        }
        // If oldSpawn was already null, nothing to do for erase mode.
    } else { // Drawing mode
        int radius = settings.getSize();
        if (radius <= 0) radius = 1;

        QList<SpawnCreatureInfo> creatureList;
        int intervalSeconds = DEFAULT_SPAWN_INTERVAL_SECONDS;

        if (radius <= 0) radius = 1;

        QList<SpawnCreatureInfo> creatureList; // To preserve existing creatures if any
        int intervalSeconds = DEFAULT_SPAWN_INTERVAL_SECONDS; // Default interval

        // If there was an old spawn, preserve its creature list and interval unless overwritten by settings.
        // For this example, we assume brush settings don't specify new creatures/interval,
        // so we reuse from oldSpawn if it exists.
        if (oldSpawn) {
            creatureList = oldSpawn->getCreatureTypes();
            intervalSeconds = oldSpawn->getIntervalSeconds();
        }
        // Note: If BrushSettings had options for creatures or interval, you'd use them here.

        std::unique_ptr<Spawn> newSpawn = std::make_unique<Spawn>(static_cast<uint16_t>(radius), intervalSeconds);
        for(const auto& creatureInfo : creatureList) {
            // Assuming addCreatureType takes const QString& or similar from SpawnCreatureInfo
            newSpawn->addCreatureType(creatureInfo.name);
        }

        qDebug("SpawnBrush::apply (draw): Setting spawn at %s with radius %d", qUtf8Printable(pos.toString()), radius);

        // oldSpawn is already unique_ptr from tile->popSpawn(), newSpawn is created above.
        // Both are std::moved into the command.
        auto command = std::make_unique<RME_COMMANDS::RecordSetSpawnCommand>(
            tile,
            std::move(newSpawn),
            std::move(oldSpawn),
            controller
        );
        controller->pushCommand(std::move(command));
    }
    // map->notifyTileChanged(pos); // This should now be handled by RecordSetSpawnCommand's undo/redo.
}

} // namespace brush
} // namespace core
} // namespace RME
