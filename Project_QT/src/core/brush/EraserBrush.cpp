#include "core/brush/EraserBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/ItemData.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h" // For AppSettings
#include "editor_logic/commands/RecordModifyTileContentsCommand.h"

#include <QDebug> // For qWarning, qDebug
#include <memory>   // For std::unique_ptr, std::move
#include <vector>
#include <algorithm> // For std::remove_if for cleaning up items_to_remove_from_tile

// Placeholder for an eraser sprite ID, replace with actual if available
const int EDITOR_SPRITE_ERASER_LOOK_ID = 0; // Or some defined constant from resources

namespace RME {
namespace core {

EraserBrush::EraserBrush() {
    // Constructor
}

QString EraserBrush::getName() const {
    return QStringLiteral("Eraser Brush");
}

int EraserBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    // Return a look ID for an eraser icon, if available in ItemDatabase or a constant
    return EDITOR_SPRITE_ERASER_LOOK_ID;
}

bool EraserBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    // Eraser can generally be applied to any valid tile that might have content.
    return true;
}

void EraserBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    Map* map = controller->getMap();
    if (!canApply(map, pos, settings)) { // Basic check, though canApply is currently permissive
        qWarning("EraserBrush::apply: Cannot apply at this position.");
        return;
    }

    Tile* tile = map->getTileForEditing(pos);
    if (!tile) {
        qWarning("EraserBrush::apply: Failed to get tile for editing at %s").arg(qUtf8Printable(pos.toString()));
        return;
    }

    const AppSettings& appSettings = controller->getAppSettings();
    // Default to true if setting not found, meaning unique items are preserved by default.
    bool leaveUniqueItems = appSettings.getBool("ERASER_LEAVE_UNIQUE_ITEMS", true);
    // bool leaveBorderItems = appSettings.getBool("ERASER_LEAVE_BORDER_ITEMS", leaveUniqueItems); // Optional: separate setting for borders

    bool isAggressive = settings.isEraseMode; // isEraseMode for aggressive, normal click for normal erase

    std::unique_ptr<Item> capturedOldGround = nullptr;
    std::vector<std::unique_ptr<Item>> capturedClearedItems;
    std::unique_ptr<Spawn> capturedOldSpawn = nullptr;
    std::unique_ptr<Creature> capturedOldCreature = nullptr;

    bool groundWasCleared = false;
    bool spawnWasCleared = false;
    bool creatureWasCleared = false;

    const ItemDatabase& itemDb = controller->getAssetManager()->getItemDatabase();

    // --- Perform actual clearing on the tile and capture what was cleared ---

    if (isAggressive) {
        // Aggressive Erase
        if (tile->getGround()) {
            const ItemData& groundData = itemDb.getItemData(tile->getGround()->getID());
            if (!leaveUniqueItems || !groundData.isComplex()) { // Assumes ItemData has isComplex()
                capturedOldGround = tile->popGround(); // popGround returns unique_ptr
                if(capturedOldGround) groundWasCleared = true;
            }
        }

        // Collect all items that are not (unique AND to be left)
        std::vector<Item*> items_to_remove_from_tile_ptr; // pointers to items on tile
        for (const auto& item_ptr : tile->getItems()) {
            const ItemData& itemData = itemDb.getItemData(item_ptr->getID());
            if (!leaveUniqueItems || !itemData.isComplex()) {
                items_to_remove_from_tile_ptr.push_back(item_ptr.get());
            }
        }
        for (Item* item_raw_ptr : items_to_remove_from_tile_ptr) {
             std::unique_ptr<Item> removed_item = tile->removeItem(item_raw_ptr, false); // false: don't destroy, just detach
             if(removed_item) capturedClearedItems.push_back(std::move(removed_item));
        }

        if (tile->getSpawn()) {
            capturedOldSpawn = tile->popSpawn(); // popSpawn returns unique_ptr
            if(capturedOldSpawn) spawnWasCleared = true;
        }
        if (tile->getCreature()) {
            capturedOldCreature = tile->popCreature(); // popCreature returns unique_ptr
             if(capturedOldCreature) creatureWasCleared = true;
        }
    } else {
        // Normal Erase (does not touch ground, spawn, creature as per wx `draw` behavior)
        std::vector<Item*> items_to_remove_from_tile_ptr;
        for (const auto& item_ptr : tile->getItems()) {
            const ItemData& itemData = itemDb.getItemData(item_ptr->getID());
            // Original wx logic: keep if ( (isComplex || isBorder) && leaveUniqueItems )
            // Translating: remove if NOT ( (isComplex || isBorder) && leaveUniqueItems )
            // Which is: remove if ( !(isComplex || isBorder) || !leaveUniqueItems )
            bool isComplexOrBorder = itemData.isComplex() || itemData.isBorder(); // Assumes ItemData has isBorder()
            if (! (isComplexOrBorder && leaveUniqueItems) ) {
                items_to_remove_from_tile_ptr.push_back(item_ptr.get());
            }
        }
        for (Item* item_raw_ptr : items_to_remove_from_tile_ptr) {
             std::unique_ptr<Item> removed_item = tile->removeItem(item_raw_ptr, false);
             if(removed_item) capturedClearedItems.push_back(std::move(removed_item));
        }
    }

    // Only create a command if something was actually changed
    if (groundWasCleared || !capturedClearedItems.empty() || spawnWasCleared || creatureWasCleared) {
        auto cmd = std::make_unique<RME::core::actions::RecordModifyTileContentsCommand>(
            tile, controller,
            std::move(capturedOldGround),
            std::move(capturedClearedItems),
            std::move(capturedOldSpawn),
            std::move(capturedOldCreature)
            // The command constructor infers cleared flags from whether unique_ptrs/vector are populated.
        );
        controller->pushCommand(std::move(cmd));
    } else {
        qDebug("EraserBrush::apply: No elements were cleared, no command pushed.");
    }
}

} // namespace core
} // namespace RME
