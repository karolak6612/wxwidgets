#include "core/brush/RawBrush.h"
#include "core/map/Map.h" // For canApply, and for AssetManager access via Map
#include "core/assets/AssetManager.h"
#include "core/assets/ItemData.h"     // For ItemType struct
#include "core/assets/IItemTypeProvider.h" // For ItemTypeProvider interface
// Tile.h is not directly used by RawBrush::apply, controller handles tiles
// #include "core/Tile.h"
#include "core/Item.h"                // For Item::create
#include "core/brush/BrushSettings.h"
#include "core/editor_logic/EditorControllerInterface.h" // For controller in apply
#include "core/Position.h" // Required for Position struct
#include <QString>
#include <QDebug> // For placeholder warnings

namespace RME {
namespace core {
namespace brush {

RawBrush::RawBrush() : Brush() {
    // m_itemId is initialized to 0 by default member initializer
}

void RawBrush::setItemId(uint16_t id) {
    m_itemId = id;
}

const RME::core::assets::ItemType* RawBrush::getItemTypeData(const BrushSettings& settings) const {
    const IItemTypeProvider* provider = settings.getItemTypeProvider();
    if (!provider) {
        qWarning("RawBrush::getItemTypeData: ItemTypeProvider not available in BrushSettings.");
        return nullptr;
    }
    return provider->getItemType(m_itemId);
}

QString RawBrush::getName() const {
    // This ideally should use getItemTypeData, but that needs BrushSettings.
    // getName is const and doesn't take settings. This is a known design issue.
    // For now, a simple ID-based name. BrushManagerService could override/set a better name.
    if (m_itemId == 0) return QObject::tr("RAW Brush (None)"); // Use QObject::tr for potential translation
    return QObject::tr("RAW Brush (ID: %1)").arg(m_itemId);
}

int RawBrush::getLookID(const BrushSettings& settings) const {
    const assets::ItemType* it = getItemTypeData(settings);
    if (it) return it->clientID; // clientID is uint16_t, fits in int.
    return 0;
}

bool RawBrush::canApply(const RME::core::map::Map* map, const Position& pos, const BrushSettings& /*settings*/) const {
    if (m_itemId == 0) return false;
    if (!map) return false;
    // Basic check: is position valid on map?
    // More complex checks (e.g. based on itemType properties or existing tile content) can be added.
    // const assets::ItemType* itemType = getItemTypeData(settings);
    // if (!itemType) return false;
    return map->isPositionValid(pos);
}

void RawBrush::apply(RME::core::editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) {
    if (!controller || m_itemId == 0) {
        return;
    }

    const IItemTypeProvider* provider = settings.getItemTypeProvider();
    if (!provider) {
        qWarning("RawBrush::apply: ItemTypeProvider not available in BrushSettings.");
        return;
    }

    const assets::ItemType* itemType = provider->getItemType(m_itemId);
    if (!itemType) {
         qWarning("RawBrush::apply: Could not find ItemType for ID %d.", m_itemId);
         return;
    }

    if (settings.isEraseMode()) {
        // Conceptual calls to the controller.
        // The controller will decide based on item properties if it's ground/stacked etc.
        // For a raw brush, we might try to remove any item with this ID.
        // If it's ground, it will call a ground removal. If stackable, top item removal.
        // This is a simplified view; controller methods might be more specific.
        qWarning("RawBrush::apply - Erase mode for item ID %d at (%d,%d,%d) - Controller call placeholder",
                 m_itemId, pos.x, pos.y, pos.z);
        // Example: controller->removeItemFromTile(pos, m_itemId, itemType->isGroundTile());
        // For now, these are placeholders for what will be implemented with EditorController.
        // The controller might need to know if it's ground or not.
        if(itemType->isGroundTile()){
            controller->setTileGround(pos, nullptr); // Request to clear ground
        } else {
            controller->removeStackedItemFromTile(pos, m_itemId); // Request to remove a specific item type
        }

    } else {
        std::unique_ptr<Item> newItem = Item::create(m_itemId, const_cast<IItemTypeProvider*>(provider), settings.getItemSubtype());

        if (newItem) {
            if (settings.isActionIdEnabled()) {
                newItem->setAttribute("aid", settings.getActionId());
            }
            // TODO: Logic for RAW_LIKE_SIMONE (removing items with same top order) - this is controller's job.

            if (itemType->isGroundTile()) {
                qWarning("RawBrush::apply - Draw mode (ground) for item ID %d at (%d,%d,%d) - Controller call placeholder",
                         m_itemId, pos.x, pos.y, pos.z);
                controller->setTileGround(pos, std::move(newItem));
            } else {
                qWarning("RawBrush::apply - Draw mode (item) for item ID %d at (%d,%d,%d) - Controller call placeholder",
                         m_itemId, pos.x, pos.y, pos.z);
                controller->addStackedItemToTile(pos, std::move(newItem));
            }
        } else {
            qWarning("RawBrush::apply: Failed to create item with ID %d for drawing.", m_itemId);
        }
    }
}

} // namespace brush
} // namespace core
} // namespace RME
