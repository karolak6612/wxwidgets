#include "core/brush/SpawnBrush.h"
#include "core/map/Map.h"         // For GetTile, IsPositionValid
#include "core/Tile.h"            // For Tile methods (getSpawn, setSpawn)
#include "core/Spawn.h"           // For RME::Spawn object
#include "core/editor/EditorControllerInterface.h" // For recording changes
#include "core/settings/BrushSettings.h"   // For getting spawn radius (brush size)
// No AssetManager/ItemDatabase typically needed for basic spawn brush logic itself

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
        if (oldSpawn) {
            qDebug("SpawnBrush::apply (erase): Cleared spawn at %s", qUtf8Printable(pos.toString()));
            // Using a raw pointer capture for oldSpawn's data for undo lambda.
            // The unique_ptr itself is moved into redo lambda's capture to destroy it on redo.
            Spawn* rawOldSpawnPtr = oldSpawn.get(); // Get raw pointer before move
             RME::Spawn tempOldSpawnCopy = *rawOldSpawnPtr; // Make a copy of data for undo

            controller->recordGenericChange(
                QString("Erase Spawn at %1,%2,%3").arg(pos.x).arg(pos.y).arg(pos.z),
                [tile, capturedOldSpawnUniquePtr = std::move(oldSpawn)]() mutable { // Redo: clear spawn
                    tile->setSpawn(nullptr);
                    // capturedOldSpawnUniquePtr goes out of scope and deletes the Spawn object
                },
                [tile, tempOldSpawnCopy]() mutable { // Undo: restore old spawn from copy
                    tile->setSpawn(std::make_unique<Spawn>(tempOldSpawnCopy));
                }
            );
        }
    } else { // Drawing mode
        int radius = settings.getSize();
        if (radius <= 0) radius = 1;

        QList<SpawnCreatureInfo> creatureList;
        int intervalSeconds = DEFAULT_SPAWN_INTERVAL_SECONDS;

        Spawn* rawOldSpawnPtrForDataCopy = oldSpawn.get(); // Get raw ptr before potential move for redo
        RME::Spawn tempOldSpawnCopyForUndo;
        bool hadOldSpawn = (oldSpawn != nullptr);
        if (hadOldSpawn) {
            tempOldSpawnCopyForUndo = *rawOldSpawnPtrForDataCopy; // Copy data for undo
            creatureList = oldSpawn->getCreatureTypes();
            intervalSeconds = oldSpawn->getIntervalSeconds();
        }

        std::unique_ptr<Spawn> newSpawn = std::make_unique<Spawn>(static_cast<uint16_t>(radius), intervalSeconds);
        for(const auto& creatureInfo : creatureList) {
            newSpawn->addCreatureType(creatureInfo.name);
        }

        qDebug("SpawnBrush::apply (draw): Set spawn at %s with radius %d", qUtf8Printable(pos.toString()), radius);

        // Capture new spawn's state for redo by copying its data
        RME::Spawn newSpawnDataCopy = *newSpawn;

        controller->recordGenericChange(
            QString("Set Spawn at %1,%2,%3").arg(pos.x).arg(pos.y).arg(pos.z),
            [tile, newSpawnDataCopy]() mutable { // Redo: set new/updated spawn
                tile->setSpawn(std::make_unique<Spawn>(newSpawnDataCopy));
            },
            [tile, hadOldSpawn, tempOldSpawnCopyForUndo]() mutable { // Undo: restore old (or clear if old was null)
                if(hadOldSpawn){
                    tile->setSpawn(std::make_unique<Spawn>(tempOldSpawnCopyForUndo));
                } else {
                    tile->setSpawn(nullptr);
                }
            }
        );
        // After capture, if newSpawn wasn't std::moved, it's fine.
        // If the redo lambda std::moves from newSpawn, then the original must not be used after this.
        // The current capture copies data (newSpawnDataCopy) so newSpawn itself is not moved.
    }
    // Tile notification should be handled by the command or explicitly here if command doesn't.
    map->notifyTileChanged(pos);
}

} // namespace brush
} // namespace core
} // namespace RME
