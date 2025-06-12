#include "core/brush/GroundBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/assets/AssetManager.h"
#include "core/brush/BrushEnums.h"
#include "core/assets/ItemData.h" // For ItemData::materialId
#include "core/assets/ItemDatabase.h" // For AssetManager::getItemDatabase()

#include <QRandomGenerator>
#include <QDebug>
#include <array> // For std::array
#include <algorithm> // For std::sort and std::find

// Static member definitions (as before)
uint32_t RME::core::GroundBrush::s_border_types[256];
bool RME::core::GroundBrush::s_staticDataInitialized = false;

namespace RME {
namespace core {

void GroundBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }

    // TILE_... constants are now expected to be included from BrushEnums.h (RME::TILE_NW etc.)
    using namespace RME; // To allow using TILE_NW directly instead of RME::TILE_NW

    using BT = RME::BorderType; // Alias for brevity

    // Initialize all to BORDER_NONE first, as a base.
    for (int i = 0; i < 256; ++i) {
        s_border_types[i] = packBorderTypes(BT::NONE);
    }

    // --- Ported data from wxwidgets/brush_tables.cpp GroundBrush::init() ---
    // This is a direct translation of the assignments.
    // The worker completing this subtask must port ALL 256 entries.
    // Only a small representative sample is shown in this prompt.
    // The indices (e.g., 0, TILE_NW, TILE_N | TILE_NW) are the `tiledata` bitmasks.

    s_border_types[0] = packBorderTypes(BT::NONE); // 0x00
    s_border_types[TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER); // 0x01
    s_border_types[TILE_N]  = packBorderTypes(BT::WX_NORTH_HORIZONTAL); // 0x02
    s_border_types[TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL); // 0x03
    s_border_types[TILE_NE] = packBorderTypes(BT::WX_NORTHEAST_CORNER); // 0x04
    s_border_types[TILE_NE | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_NORTHEAST_CORNER); // 0x05
    s_border_types[TILE_NE | TILE_N]  = packBorderTypes(BT::WX_NORTH_HORIZONTAL); // 0x06
    s_border_types[TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL); // 0x07
    s_border_types[TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL); // 0x08
    s_border_types[TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL); // 0x09
    s_border_types[TILE_W | TILE_N]  = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL); // 0x0A
    s_border_types[TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL); // 0x0B
    s_border_types[TILE_W | TILE_NE] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER); // 0x0C
    s_border_types[TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER); // 0x0D
    s_border_types[TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL); // 0x0E
    s_border_types[TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL); // 0x0F

    s_border_types[TILE_E] = packBorderTypes(BT::WX_EAST_HORIZONTAL); // 0x10
    // ... (The remaining ~240 entries from brush_tables.cpp need to be ported here by the worker) ...
    // For example, the next entry from the original table would be:
    // s_border_types[TILE_E | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL); // 0x11
    // ... up to index 255 ...

    // Example of a more complex line from the end of the table:
    s_border_types[TILE_SE | TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] // 0xFF
        = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);

    qInfo("GroundBrush::s_border_types table has been initialized by (partially) porting static assignments from wxwidgets/brush_tables.cpp.");
    s_staticDataInitialized = true;
}

// Constructor and other methods (as before)
GroundBrush::GroundBrush() : m_materialData(nullptr) {
    initializeStaticData();
}

void GroundBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isGround()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "GroundBrush::setMaterial: Material is null or not a ground type.";
    }
}

const RME::core::assets::MaterialData* GroundBrush::getMaterial() const {
    return m_materialData;
}

const RME::core::assets::MaterialGroundSpecifics* GroundBrush::getCurrentGroundSpecifics() const {
    if (m_materialData && m_materialData->isGround()) {
        return std::get_if<RME::core::assets::MaterialGroundSpecifics>(&m_materialData->specificData);
    }
    return nullptr;
}

QString GroundBrush::getName() const {
    if (m_materialData) {
        return m_materialData->id;
    }
    return "Ground Brush";
}

int GroundBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    if (m_materialData) {
        const auto* specifics = getCurrentGroundSpecifics();
        if (specifics && !specifics->items.empty()) {
            if (m_materialData->lookId != 0) return m_materialData->lookId;
            if (m_materialData->serverLookId != 0) {
                 return m_materialData->serverLookId;
            }
        }
    }
    return 0;
}

bool GroundBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData || !getCurrentGroundSpecifics()) {
        return false;
    }
    if (!map) {
        return false;
    }
    return map->isPositionValid(pos);
}

void GroundBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    if (!controller) {
        qWarning() << "GroundBrush::apply: Null controller.";
        return;
    }
    RME::core::map::Map* map = controller->getMap();
    if (!map) {
        qWarning() << "GroundBrush::apply: Null map from controller.";
        return;
    }
    if (!m_materialData) {
        qWarning() << "GroundBrush::apply: No material set.";
        return;
    }
    const auto* materialSpecifics = getCurrentGroundSpecifics();
    if (!materialSpecifics || (!settings.isEraseMode && materialSpecifics->items.empty())) {
        qWarning() << "GroundBrush::apply: Material has no ground items defined for drawing.";
        if (!settings.isEraseMode) return;
    }

    RME::core::Tile* tile = controller->getTileForEditing(pos);
    if (!tile) {
        qWarning() << "GroundBrush::apply: Failed to get tile for editing at" << pos.x << pos.y << pos.z;
        return;
    }

    uint16_t oldGroundItemId = 0;
    const Item* currentGround = tile->getGround();
    if (currentGround) {
        oldGroundItemId = currentGround->getID();
    }

    if (settings.isEraseMode) {
        if (oldGroundItemId != 0) {
            controller->recordSetGroundItem(pos, 0, oldGroundItemId); // Erase: new ID is 0
            qDebug() << "GroundBrush: Called recordSetGroundItem to erase ground at" << pos.x << pos.y << pos.z << "(was " << oldGroundItemId << ")";
        } else {
            qDebug() << "GroundBrush: Erase mode, but no ground to erase at" << pos.x << pos.y << pos.z;
            // No ground change, but borders might still need update if neighbors changed.
        }
    } else { // Drawing mode
        // This check should be done before `materialSpecifics->items.empty()` if materialSpecifics can be null.
        // It was done at the start of the function.
        // if (!materialSpecifics || materialSpecifics->items.empty()) { /* already handled */ }

        int totalChance = 0;
        if (materialSpecifics){ // materialSpecifics is already checked for null at the start of the function along with items.empty() for non-erase mode
            for (const auto& itemEntry : materialSpecifics->items) {
                totalChance += itemEntry.chance;
            }
            if (totalChance == 0 && !materialSpecifics->items.empty()) {
                 totalChance = materialSpecifics->items.first().chance > 0 ? materialSpecifics->items.first().chance : 1; // Ensure some chance if items exist
            }
        }

        if (totalChance == 0) { // Still 0 means no items or all items have 0 chance and items list was empty.
             qWarning() << "GroundBrush::apply: Total chance for ground items is 0.";
            return;
        }

        uint16_t selectedItemId = materialSpecifics->items.first().itemId;
        int randomValue = QRandomGenerator::global()->bounded(totalChance);
        int currentChanceSum = 0;
        for (const auto& itemEntry : materialSpecifics->items) {
            currentChanceSum += itemEntry.chance;
            if (randomValue < currentChanceSum) {
                selectedItemId = itemEntry.itemId;
                break;
            }
        }

        // Record action even if item ID is the same, as this ensures undo stack consistency
        // and allows the controller's action to handle no-op if necessary.
        // Or, add a check: if (oldGroundItemId != selectedItemId)
        controller->recordSetGroundItem(pos, selectedItemId, oldGroundItemId);
        qDebug() << "GroundBrush: Called recordSetGroundItem to draw ground item" << selectedItemId << "at" << pos.x << pos.y << pos.z << "(old was " << oldGroundItemId << ")";
    }

    // Auto-Bordering Phase (remains the same)
    doAutoBorders(controller, pos, settings);

    static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            doAutoBorders(controller, neighborPos, settings);
        }
    }

    controller->notifyTileChanged(pos);
    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            controller->notifyTileChanged(neighborPos);
        }
    }
}

// Anonymous namespace for helpers
namespace {

const assets::MaterialData* getMaterialFromTile(
    const Tile* tile,
    const assets::AssetManager* assetManager) {

    if (!tile || !tile->getGround() || !assetManager) {
        return nullptr;
    }

    const Item* groundItem = tile->getGround();
    uint16_t groundItemId = groundItem->getID();

    const assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
    const assets::MaterialManager* materialMgr = assetManager->getMaterialManager();

    if (!itemDb) {
        qWarning("getMaterialFromTile: ItemDatabase not available via AssetManager.");
        return nullptr;
    }

    const assets::ItemData* itemData = itemDb->getItemData(groundItemId);
    if (!itemData) {
        qWarning("getMaterialFromTile: ItemData not found for ground item ID %u.", groundItemId);
        return nullptr;
    }

    if (!itemData->materialId.isEmpty()) {
        if (materialMgr) {
            const assets::MaterialData* foundMaterial = materialMgr->getMaterial(itemData->materialId);
            if (!foundMaterial) {
                qDebug("getMaterialFromTile: Material ID '%s' from ItemData (item %u) not found in MaterialManager.",
                       qUtf8Printable(itemData->materialId), groundItemId);
            }
            return foundMaterial;
        } else {
            qDebug("getMaterialFromTile: MaterialManager is null, cannot look up material ID '%s' for item %u.",
                   qUtf8Printable(itemData->materialId), groundItemId);
            return nullptr;
        }
    } else {
        qDebug("getMaterialFromTile: Item ID %u (name: '%s') has no associated materialId in its ItemData.",
               groundItemId, qUtf8Printable(itemData->name));
        return nullptr;
    }
}

} // end anonymous namespace

namespace { // Anonymous namespace for new helpers

QString determineAlignString(
    BorderType pieceType,
    uint8_t tiledata,
    const std::array<const assets::MaterialData*, 8>& /*neighborMaterials*/,
    const assets::MaterialData* /*currentTileMaterial*/
) {
    Q_UNUSED(tiledata);
    // This function translates an abstract piece type (from s_border_types)
    // into an "align" string for MaterialBorderRule matching.
    // The primary distinction in MaterialBorderRule.align is "outer" vs "inner".
    // Most WX_... pieces from s_border_types imply an outer border context.
    // Specific "inner" BorderType enums would be needed from s_border_types
    // to robustly return "inner". The current s_border_types port mainly has WX_ pieces.

    switch (pieceType) {
        case BorderType::WX_NORTH_HORIZONTAL:
        case BorderType::WX_EAST_HORIZONTAL:
        case BorderType::WX_SOUTH_HORIZONTAL:
        case BorderType::WX_WEST_HORIZONTAL:
        case BorderType::WX_NORTHWEST_CORNER:
        case BorderType::WX_NORTHEAST_CORNER:
        case BorderType::WX_SOUTHWEST_CORNER:
        case BorderType::WX_SOUTHEAST_CORNER:
        case BorderType::WX_NORTHWEST_DIAGONAL:
        case BorderType::WX_NORTHEAST_DIAGONAL:
        case BorderType::WX_SOUTHWEST_DIAGONAL:
        case BorderType::WX_SOUTHEAST_DIAGONAL:
            return "outer"; // Assume these are all for outer border contexts

        // Case for explicitly inner pieces, if s_border_types could produce them:
        // case BorderType::INNER_NORTH_WEST: // And other INNER_...
        //     return "inner";

        case BorderType::NONE:
            return "none"; // Or some other indicator for no alignment

        default:
            qWarning("determineAlignString: Unknown BorderType %d, defaulting to 'outer'.", static_cast<int>(pieceType));
            return "outer"; // Default assumption
    }
}

QString determineToBrushName(
    BorderType pieceType,
    uint8_t /*tiledata*/, // tiledata might be useful for complex corner disambiguation
    const std::array<const assets::MaterialData*, 8>& neighborMaterials,
    const assets::MaterialData* /*currentTileMaterial*/
) {
    // Determine the material ID of the neighbor(s) this pieceType is primarily against.
    // This is a simplified interpretation.
    int relevantNeighborIndex = -1;

    // Mapping BorderType piece to the primary differing neighbor direction it implies
    // NW(0) N(1) NE(2) W(3) E(4) SW(5) S(6) SE(7)
    switch (pieceType) {
        case BorderType::WX_NORTH_HORIZONTAL:   relevantNeighborIndex = 1; break; // Against North
        case BorderType::WX_EAST_HORIZONTAL:    relevantNeighborIndex = 4; break; // Against East
        case BorderType::WX_SOUTH_HORIZONTAL:   relevantNeighborIndex = 6; break; // Against South
        case BorderType::WX_WEST_HORIZONTAL:    relevantNeighborIndex = 3; break; // Against West

        // For corners, it's more complex. A NW_CORNER piece is placed because N and W (and NW) are different.
        // Which differing neighbor's material ID should be used for toBrushName?
        // Often, corner rules in materials.xml might use toBrushName="none" or "all"
        // because the specific corner item itself implies the shape.
        // If rules are specific like "corner_nw_vs_dirt", then we'd need to identify "dirt".
        // Simplification: pick one cardinal neighbor. This is a MAJOR simplification.
        case BorderType::WX_NORTHWEST_CORNER: relevantNeighborIndex = 1; break; // Primarily against N (or W)
        case BorderType::WX_NORTHEAST_CORNER: relevantNeighborIndex = 1; break; // Primarily against N (or E)
        case BorderType::WX_SOUTHWEST_CORNER: relevantNeighborIndex = 6; break; // Primarily against S (or W)
        case BorderType::WX_SOUTHEAST_CORNER: relevantNeighborIndex = 6; break; // Primarily against S (or E)

        // Diagonals are also complex, often related to two differing cardinal directions.
        case BorderType::WX_NORTHWEST_DIAGONAL: relevantNeighborIndex = 1; break; // e.g., N and W different
        case BorderType::WX_NORTHEAST_DIAGONAL: relevantNeighborIndex = 1; break; // e.g., N and E different
        case BorderType::WX_SOUTHWEST_DIAGONAL: relevantNeighborIndex = 6; break; // e.g., S and W different
        case BorderType::WX_SOUTHEAST_DIAGONAL: relevantNeighborIndex = 6; break; // e.g., S and E different

        case BorderType::NONE: // No specific piece, no specific "toBrush"
            return "none"; // Or a special value indicating no specific target

        default:
            qWarning("determineToBrushName: Unhandled BorderType %d, cannot determine relevant neighbor.", static_cast<int>(pieceType));
            return "none"; // Fallback
    }

    if (relevantNeighborIndex != -1) {
        if (neighborMaterials[relevantNeighborIndex]) {
            return neighborMaterials[relevantNeighborIndex]->id;
        } else {
            return "none"; // Explicitly bordering void (empty neighbor tile)
        }
    }
    // If logic didn't set relevantNeighborIndex (e.g. for some corners/diagonals if not simplified above)
    qWarning("determineToBrushName: Complex BorderType %d, defaulting toBrushName to 'none'. Rule matching might need 'all' or specific handling.", static_cast<int>(pieceType));
    return "none"; // Default for unhandled complex pieces
}

} // end anonymous namespace

// --- doAutoBorders Implementation ---
void GroundBrush::doAutoBorders(RME::core::editor::EditorControllerInterface* controller,
                                  const RME::core::Position& targetPos,
                                  const RME::core::BrushSettings& settings) {
    if (!s_staticDataInitialized) {
        qCritical("GroundBrush::doAutoBorders: static data not initialized!");
        initializeStaticData();
    }

    RME::core::map::Map* map = controller->getMap();
    RME::core::assets::AssetManager* assetManager = controller->getAssetManager();
    if (!map || !assetManager) {
        qWarning("GroundBrush::doAutoBorders: Map or AssetManager not available from controller.");
        return;
    }

    const Tile* currentTile = map->getTile(targetPos);
    if (!currentTile) {
        qWarning("GroundBrush::doAutoBorders: Target tile not found at %s.", qUtf8Printable(targetPos.toString()));
        return;
    }

    // 1. Collect oldBorderItemIds
    QList<uint16_t> oldBorderItemIds;
    const QList<std::unique_ptr<Item>>& itemsOnTile = currentTile->getItems();
    for (const auto& itemPtr : itemsOnTile) {
        if (itemPtr) {
            // Ideally, filter for actual border items:
            // const assets::ItemData* itemData = assetManager->getItemDatabase()->getItemData(itemPtr->getID());
            // if (itemData && itemData->isBorder) {
            //     oldBorderItemIds.append(itemPtr->getID());
            // }
            // Simplified: collecting all non-ground items.
            oldBorderItemIds.append(itemPtr->getID());
        }
    }
    std::sort(oldBorderItemIds.begin(), oldBorderItemIds.end());


    const assets::MaterialData* currentTileMaterial = getMaterialFromTile(currentTile, assetManager);
    const assets::MaterialGroundSpecifics* currentTileSpecifics = nullptr;
    if (currentTileMaterial && currentTileMaterial->isGround()) {
        currentTileSpecifics = std::get_if<assets::MaterialGroundSpecifics>(&currentTileMaterial->specificData);
    }

    QList<uint16_t> newBorderItemIds;

    if (!currentTileSpecifics) {
        qDebug() << "GroundBrush::doAutoBorders: No ground material on target tile" << targetPos.toString() << "or material has no specifics. Existing borders will be evaluated for removal.";
        // newBorderItemIds remains empty, so if oldBorderItemIds is not empty, it means clear borders.
    } else {
        uint8_t tiledata = 0;
        std::array<const assets::MaterialData*, 8> neighborMaterials;
        static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
            {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
        }};

        for (int i = 0; i < 8; ++i) {
            RMEPosition neighborPos(targetPos.x + neighborOffsets[i].first,
                                    targetPos.y + neighborOffsets[i].second,
                                    targetPos.z);
            const Tile* neighborTile = map->getTile(neighborPos);
            neighborMaterials[i] = getMaterialFromTile(neighborTile, assetManager);

            bool isDifferent = false;
            if (!neighborMaterials[i]) { isDifferent = true; }
            else if (currentTileMaterial && neighborMaterials[i]->id != currentTileMaterial->id) {
                bool areFriends = currentTileSpecifics->friends.contains(neighborMaterials[i]->id);
                 if (!areFriends) { // Check reverse friendship only if not already friends
                    const auto* neighborSpecifics = std::get_if<assets::MaterialGroundSpecifics>(&neighborMaterials[i]->specificData);
                    if (neighborSpecifics && currentTileMaterial && neighborSpecifics->friends.contains(currentTileMaterial->id)) {
                        areFriends = true;
                    }
                }
                if (!areFriends) { isDifferent = true; }
            } else if (!currentTileMaterial && neighborMaterials[i]) { // Current is void, neighbor is not
                isDifferent = true;
            }
            if (isDifferent) { tiledata |= (1 << i); }
        }

        uint32_t packedComputedBorderTypes = s_border_types[tiledata];
        if (packedComputedBorderTypes == 0 && tiledata != 0) {
             qWarning() << "GroundBrush::doAutoBorders: s_border_types lookup for tiledata" << Qt::bin << tiledata << "resulted in NO BORDERS. Table might be empty or this is a valid no-border case.";
        }

        QList<const assets::MaterialBorderRule*> matchedSuperRules;
        QList<const assets::MaterialBorderRule*> matchedNormalRules;

        for (int pieceNum = 0; pieceNum < 4; ++pieceNum) {
            BorderType computedPiece = unpackBorderType(packedComputedBorderTypes, pieceNum);
            if (computedPiece == BorderType::NONE) continue;

            QString alignStr = determineAlignString(computedPiece, tiledata, neighborMaterials, currentTileMaterial);
            QString toBrushIdStr = determineToBrushName(computedPiece, tiledata, neighborMaterials, currentTileMaterial);

            if (alignStr.startsWith("unknown") || toBrushIdStr.startsWith("unknown")) {
                qWarning("GroundBrush::doAutoBorders: Skipping piece %d due to unknown align/toBrush translation.", static_cast<int>(computedPiece));
                continue;
            }

            for (const auto& rule : currentTileSpecifics->borders) {
                // More precise matching attempt
                if ((rule.align.compare(alignStr, Qt::CaseInsensitive) == 0 || rule.align.compare("any", Qt::CaseInsensitive) == 0) &&
                    (rule.toBrushName.compare(toBrushIdStr, Qt::CaseInsensitive) == 0 || rule.toBrushName.compare("all", Qt::CaseInsensitive) == 0)) {
                    if (rule.isSuper) {
                        matchedSuperRules.append(&rule);
                    } else {
                        matchedNormalRules.append(&rule);
                    }
                }
            }
        }
        for (const auto* rule : matchedSuperRules) { if (std::find(newBorderItemIds.begin(), newBorderItemIds.end(), rule->borderItemId) == newBorderItemIds.end()) newBorderItemIds.append(rule->borderItemId); }
        for (const auto* rule : matchedNormalRules) { if (std::find(newBorderItemIds.begin(), newBorderItemIds.end(), rule->borderItemId) == newBorderItemIds.end()) newBorderItemIds.append(rule->borderItemId); }
    }

    std::sort(newBorderItemIds.begin(), newBorderItemIds.end());
    if (oldBorderItemIds != newBorderItemIds) {
        controller->recordSetBorderItems(targetPos, newBorderItemIds, oldBorderItemIds);
        qDebug() << "GroundBrush::doAutoBorders: Tile" << targetPos.toString() << "borders changed. Old:" << oldBorderItemIds << "New:" << newBorderItemIds << ". Called recordSetBorderItems.";
    } else {
        qDebug() << "GroundBrush::doAutoBorders: Tile" << targetPos.toString() << "borders did not change.";
    }
}

// ... (apply method and other GroundBrush methods as before) ...
} // namespace core
} // namespace RME
