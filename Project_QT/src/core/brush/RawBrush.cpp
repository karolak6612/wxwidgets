#include "core/brush/RawBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/ItemData.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "editor_logic/commands/RecordSetGroundCommand.h"
#include "editor_logic/commands/RecordAddRemoveItemCommand.h"

#include <QDebug> // For qWarning, qDebug
#include <memory>   // For std::unique_ptr, std::move

namespace RME {
namespace core {
namespace brush {

RawBrush::RawBrush(uint16_t itemId) : m_itemId(itemId) {
    // Constructor
}

void RawBrush::setItemId(uint16_t itemId) {
    m_itemId = itemId;
}

uint16_t RawBrush::getItemId() const {
    return m_itemId;
}

QString RawBrush::getName() const {
    if (m_itemId == 0) {
        return QStringLiteral("RAW Brush (No item selected)");
    }
    // Optionally, could fetch item name from ItemDatabase via AssetManager if available here
    // For now, keeping it simple as AssetManager might not be easily accessible in const getName().
    return QStringLiteral("RAW Brush (ID: %1)").arg(m_itemId);
}

int RawBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    // For now, returns the raw server item ID.
    // UI might need to map this to a client ID or sprite if AssetManager is accessible
    // and we want to display the actual item's appearance.
    // A more advanced getLookID might involve:
    // const auto* itemData = settings.getAssetManager() ? &settings.getAssetManager()->getItemDatabase().getItemData(m_itemId) : nullptr;
    // return itemData ? itemData->look_id_or_equivalent : m_itemId;
    return m_itemId;
}

bool RawBrush::canApply(const RME::core::map::Map* map,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) const {
    if (m_itemId == 0) { // No item selected for the brush
        return false;
    }
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    const Tile* tile = map->getTile(pos);
    if (!tile || !tile->getGround()) { // Require ground to place anything
        // Exception: if the RAW item itself is ground and we are not erasing.
        if (settings.isEraseMode) return false; // Cannot erase from a tile without ground

        // Allow placing a ground item if no ground exists yet.
        // This requires knowing if m_itemId is a ground item.
        // This check might be better done in apply() with full AssetManager access.
        // For now, let's assume if there's no ground, you can only place a ground item.
        // This simplified canApply might need refinement based on ItemData access here.
    }

    // Check if item ID exists in database (requires AssetManager access in BrushSettings or globally)
    // For now, this check is deferred to apply() to simplify canApply dependencies.
    // if (settings.getAssetManager() && !settings.getAssetManager()->getItemDatabase().isValidId(m_itemId)) {
    //     return false;
    // }

    return true;
}

void RawBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                       const RME::core::Position& pos,
                       const RME::core::BrushSettings& settings) {
    if (m_itemId == 0) {
        qDebug("RawBrush::apply: No item ID selected for the brush.");
        return;
    }

    Map* map = controller->getMap();
    if (!map || !map->isPositionValid(pos)) {
        qWarning("RawBrush::apply: Invalid map or position.");
        return;
    }

    AssetManager* assetManager = controller->getAssetManager();
    if (!assetManager) {
        qWarning("RawBrush::apply: AssetManager not available via controller.");
        return;
    }
    const ItemDatabase& itemDb = assetManager->getItemDatabase();
    if (!itemDb.isValidId(m_itemId)) {
        qWarning("RawBrush::apply: Item ID %1 is invalid or does not exist.").arg(m_itemId);
        return;
    }
    const ItemData& itemData = itemDb.getItemData(m_itemId);

    Tile* tile = map->getTileForEditing(pos);
    if (!tile) {
        qWarning("RawBrush::apply: Failed to get tile for editing at %s").arg(qUtf8Printable(pos.toString()));
        return;
    }

    // Ensure there's ground if we are not placing a ground item itself on an empty tile
    if (!tile->getGround() && !itemData.isGround && !settings.isEraseMode) {
        qDebug("RawBrush::apply: Cannot place non-ground item on a tile without ground.");
        return;
    }

    if (settings.isEraseMode) {
        if (itemData.isGround) {
            // Try to erase if it's the ground item
            if (tile->getGround() && tile->getGround()->getID() == m_itemId) {
                std::unique_ptr<Item> oldGround = tile->popGround();
                auto cmd = std::make_unique<RME_COMMANDS::RecordSetGroundCommand>(
                    tile, nullptr, std::move(oldGround), controller);
                controller->pushCommand(std::move(cmd));
            } else {
                qDebug("RawBrush::apply (erase): Ground item ID %1 not found as ground on tile.").arg(m_itemId);
            }
        } else {
            // Erase non-ground item: find topmost item with this ID
            Item* itemToRemove = nullptr;
            // Iterate in reverse to find the topmost item
            const auto& items = tile->getItems();
            for (auto it = items.rbegin(); it != items.rend(); ++it) {
                if ((*it)->getID() == m_itemId) {
                    itemToRemove = it->get(); // Get raw pointer from unique_ptr
                    break;
                }
            }

            if (itemToRemove) {
                // itemToRemove is a raw pointer to an item managed by tile->getItems()
                // RecordAddRemoveItemCommand for remove takes Item*
                auto cmd = std::make_unique<RME_COMMANDS::RecordAddRemoveItemCommand>(
                    tile, itemToRemove, controller);
                controller->pushCommand(std::move(cmd));
            } else {
                qDebug("RawBrush::apply (erase): Item ID %1 not found on tile.").arg(m_itemId);
            }
        }
    } else { // Drawing mode
        std::unique_ptr<Item> newItem = Item::create(m_itemId); // Assumes Item::create uses its own means to get ItemData if needed beyond ID
        if (!newItem) {
            qWarning("RawBrush::apply: Failed to create item with ID %1.").arg(m_itemId);
            return;
        }

        if (itemData.isGround) {
            std::unique_ptr<Item> oldGround = tile->popGround();
            auto cmd = std::make_unique<RME_COMMANDS::RecordSetGroundCommand>(
                tile, std::move(newItem), std::move(oldGround), controller);
            controller->pushCommand(std::move(cmd));
        } else {
            // Before adding a non-ground item, ensure there is ground.
            if (!tile->getGround()) {
                qDebug("RawBrush::apply (draw): Cannot place non-ground item ID %1 as there is no ground.").arg(m_itemId);
                return; // Or handle by placing a default ground first, if that's desired behavior.
            }
            auto cmd = std::make_unique<RME_COMMANDS::RecordAddRemoveItemCommand>(
                tile, std::move(newItem), controller);
            controller->pushCommand(std::move(cmd));
        }
    }
}

} // namespace brush
} // namespace core
} // namespace RME
