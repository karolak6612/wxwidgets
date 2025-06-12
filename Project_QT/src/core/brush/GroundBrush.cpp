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

// initializeStaticData() implementation (as before)
void GroundBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }
    qCritical("GroundBrush::initializeStaticData(): CRITICAL - Attempting to initialize s_border_types table.");
    qCritical("The actual lookup values for auto-bordering logic MUST be ported from the original RME's");
    qCritical("GroundBrush::border_types array (or its initialization routine). Without this, auto-bordering will NOT work correctly.");
    qCritical("The table is currently being filled with BorderType::NONE.");
    for (int i = 0; i < 256; ++i) {
        s_border_types[i] = packBorderTypes(BorderType::NONE);
    }
    qDebug("GroundBrush::s_border_types table has been initialized with placeholder (NONE) values.");
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

// CRITICAL STUB: This function needs to translate an abstract border piece type
// and its context (neighbor configuration) into an "align" string
// that matches MaterialBorderRule.align (e.g., "outer", "inner",
// or potentially more specific like "outer_edge_n", "inner_corner_nw").
QString determineAlignString(
    BorderType pieceType,
    uint8_t tiledata, // Full 8-neighbor configuration
    const std::array<const assets::MaterialData*, 8>& /*neighborMaterials*/, // Materials of neighbors
    const assets::MaterialData* /*currentTileMaterial*/
) {
    Q_UNUSED(tiledata);
    // This is highly dependent on how s_border_types is structured and how
    // MaterialBorderRule.align is defined in the XMLs.
    // Example: if pieceType indicates a primary edge (N,E,S,W) it's likely "outer".
    // If it's a corner type, it could be "outer_corner" or "inner_corner".
    // The original RME used BorderBlock->outer (bool) to distinguish.
    // For now, a very crude placeholder:
    if (pieceType >= BorderType::WX_NORTH_HORIZONTAL && pieceType <= BorderType::WX_WEST_HORIZONTAL) return "outer";
    if (pieceType >= BorderType::WX_NORTHWEST_CORNER && pieceType <= BorderType::WX_SOUTHWEST_CORNER) return "outer_corner"; // Or just "outer"
    if (pieceType >= BorderType::INNER_NORTH_WEST && pieceType <= BorderType::INNER_SOUTH_WEST) return "inner_corner"; // Or just "inner"

    qWarning("determineAlignString: Placeholder cannot determine align for BorderType %d", static_cast<int>(pieceType));
    return "unknown_align_from_stub";
}

// CRITICAL STUB: This function needs to determine the relevant "toBrushName"
// for a given border piece type by looking at the implicated neighbor(s).
QString determineToBrushName(
    BorderType pieceType,
    uint8_t tiledata, // Full 8-neighbor configuration
    const std::array<const assets::MaterialData*, 8>& neighborMaterials,
    const assets::MaterialData* currentTileMaterial
) {
    Q_UNUSED(tiledata);
    Q_UNUSED(currentTileMaterial);
    // This needs to map the 'pieceType' to the specific neighbor(s) it's interacting with.
    // E.g., WX_NORTH_HORIZONTAL interacts with neighbor at index 1 (North).
    // WX_NORTHWEST_CORNER interacts with NW, N, W (indices 0, 1, 3).
    // If interacting with multiple different neighbors, the rule is complex.
    // For simplicity, RME rules often border "none" or a specific type.
    // Placeholder logic:
    int relevantNeighborIndex = -1;
    if (pieceType == BorderType::WX_NORTH_HORIZONTAL) relevantNeighborIndex = 1; // N
    else if (pieceType == BorderType::WX_EAST_HORIZONTAL) relevantNeighborIndex = 4; // E
    else if (pieceType == BorderType::WX_SOUTH_HORIZONTAL) relevantNeighborIndex = 6; // S
    else if (pieceType == BorderType::WX_WEST_HORIZONTAL) relevantNeighborIndex = 3; // W
    // Simplified: corners and other types just check one primary neighbor for now or default to "none"
    else if (pieceType == BorderType::WX_NORTHWEST_CORNER) relevantNeighborIndex = 0; // NW (or N or W)
    else if (pieceType == BorderType::WX_NORTHEAST_CORNER) relevantNeighborIndex = 2; // NE (or N or E)
    else if (pieceType == BorderType::WX_SOUTHEAST_CORNER) relevantNeighborIndex = 7; // SE (or S or E)
    else if (pieceType == BorderType::WX_SOUTHWEST_CORNER) relevantNeighborIndex = 5; // SW (or S or W)

    if (relevantNeighborIndex != -1) {
        if (neighborMaterials[relevantNeighborIndex]) {
            return neighborMaterials[relevantNeighborIndex]->id;
        } else {
            return "none"; // Explicitly bordering void
        }
    }
    qWarning("determineToBrushName: Placeholder cannot determine toBrushName for BorderType %d", static_cast<int>(pieceType));
    return "unknown_tobrush_from_stub";
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
