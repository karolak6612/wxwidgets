#include "core/brush/GroundBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/assets/AssetManager.h"
#include "core/brush/BrushEnums.h"

#include <QRandomGenerator>
#include <QDebug>
#include <array> // For std::array

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
    // ... (apply implementation as before) ...
    // Get map and tile for editing
    RME::core::map::Map* map = controller->getMap();
    if (!map) {
        qWarning() << "GroundBrush::apply: Null map from controller.";
        return;
    }
     // Ensure materialData is valid before proceeding
    if (!m_materialData) {
        qWarning() << "GroundBrush::apply: No material set.";
        return;
    }
    const auto* materialSpecifics = getCurrentGroundSpecifics();
    if (!materialSpecifics || (!settings.isEraseMode && materialSpecifics->items.empty())) {
        qWarning() << "GroundBrush::apply: Material has no ground items defined for drawing.";
        // Allow erase even if items list is empty, as it might just be clearing.
        if (!settings.isEraseMode) return;
    }


    RME::core::Tile* tile = controller->getTileForEditing(pos);
    if (!tile) {
        qWarning() << "GroundBrush::apply: Failed to get tile for editing at" << pos.x << pos.y << pos.z;
        return;
    }

    uint16_t oldGroundId = 0;
    const Item* currentGround = tile->getGround();
    if (currentGround) {
        oldGroundId = currentGround->getID();
    }

    // Determine the material of the current ground item on the tile for accurate `doAutoBorders` later
    // This is important because `doAutoBorders` for neighbors relies on knowing the *new* state of this tile.
    // const assets::MaterialData* newTileMaterialForBordering = nullptr; // This variable is not used later

    if (settings.isEraseMode) {
        if (oldGroundId != 0) {
            // controller->recordSetGroundItem(pos, 0, oldGroundId); // Action: remove ground
            // The controller action should effectively do: tile->setGround(nullptr);
            // For simulation here if controller doesn't update tile immediately:
            // tile->setGround(nullptr);
            qDebug() << "GroundBrush: Erasing ground at" << pos.x << pos.y << pos.z << "(was " << oldGroundId << ")";
        }
        // newTileMaterialForBordering = nullptr; // Tile becomes empty // Not used
    } else {
        int totalChance = 0;
        if (materialSpecifics){ // Check if materialSpecifics is not null
            for (const auto& itemEntry : materialSpecifics->items) {
                totalChance += itemEntry.chance;
            }
            if (totalChance == 0 && !materialSpecifics->items.empty()) { // if items exist but total chance is 0, use first.
                 totalChance = materialSpecifics->items.first().chance > 0 ? materialSpecifics->items.first().chance : 1;
            }
        }

        if (totalChance == 0) { // Still 0 means no items or all items have 0 chance.
             qWarning() << "GroundBrush::apply: Total chance for ground items is 0 and no items to pick.";
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

        if (oldGroundId != selectedItemId) {
            // controller->recordSetGroundItem(pos, selectedItemId, oldGroundId); // Action: set/replace ground
            // The controller action should effectively do: tile->setGround(Item::Create(selectedItemId));
            // For simulation:
            // tile->setGround(Item::Create(selectedItemId, controller->getAssetManager())); // Assuming Item::Create needs AssetManager
            qDebug() << "GroundBrush: Drawing ground item" << selectedItemId << "at" << pos.x << pos.y << pos.z;
        } else {
            qDebug() << "GroundBrush: Ground item" << selectedItemId << "already present at" << pos.x << pos.y << pos.z;
        }
        // newTileMaterialForBordering = m_materialData; // New state is this brush's material // Not used
    }

    // Auto-Bordering Phase
    // Pass the 'newTileMaterialForBordering' to doAutoBorders if its logic needs to know the *intended* state
    // rather than re-querying the tile which might not yet reflect the change if controller actions are deferred.
    // However, doAutoBorders typically reads the map state as it is.
    // For now, doAutoBorders will read from the map.
    doAutoBorders(controller, pos, settings); // Borders for the current tile

    static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1}; // NW, N, NE, W, E, SW, S, SE
    static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1}; // Matched to typical neighbor iteration order

    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            doAutoBorders(controller, neighborPos, settings); // Borders for each neighbor
        }
    }

    // Notifications are now assumed to be handled by the controller actions
    // For example, if recordSetGroundItem or recordSetBorderItems also call notifyTileChanged.
    // If not, explicit notification is needed. The original code had this:
    controller->notifyTileChanged(pos);
    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            controller->notifyTileChanged(neighborPos);
        }
    }
}


// Helper to get MaterialData from a tile's ground item
const assets::MaterialData* getMaterialFromTile(const Tile* tile, [[maybe_unused]] const assets::AssetManager* assetManager) { // assetManager marked maybe_unused
    if (!tile || !tile->getGround() /*|| !assetManager*/) { // assetManager check commented out
        return nullptr;
    }
    // This assumes Item has a way to get its brush name or that AssetManager
    // can map item ID to MaterialData. Let's assume Item stores its source brush name/ID.
    // Or, more realistically, that MaterialManager itself is the source.
    // This part is complex: mapping ground item ID back to a MaterialData.
    // For now, placeholder:
    // return assetManager->getMaterialManager()->getMaterialForItem(tile->getGround()->getID());
    // A simpler way for now: if Item itself stored a pointer or ID to its MaterialData.
    // Or if ItemType in ItemDatabase had a link to material_id.
    // This is a known gap from wxwidgets where ItemType had ->brush pointer.
    // Let's assume for the sake of structure:
    // return tile->getGround()->getMaterialData(); // If Item could provide this
    qWarning() << "getMaterialFromTile: Critical lookup function not fully implemented. Returning nullptr.";
    return nullptr; // Placeholder - this is a critical lookup.
}


// --- doAutoBorders Implementation ---
void GroundBrush::doAutoBorders(RME::core::editor::EditorControllerInterface* controller,
                                  const RME::core::Position& targetPos,
                                  const RME::core::BrushSettings& /*settings*/) { // settings might be used for custom border mode later
    if (!s_staticDataInitialized) {
        qCritical("GroundBrush::doAutoBorders: static data not initialized!");
        initializeStaticData();
    }

    RME::core::map::Map* map = controller->getMap();
    // RME::core::assets::AssetManager* assetManager = controller->getAssetManager(); // Assuming controller provides this
    // For now, getMaterialFromTile is stubbed and doesn't use assetManager.
    if (!map) {
        qWarning("GroundBrush::doAutoBorders: Map not available from controller.");
        return;
    }

    const Tile* currentTile = map->getTile(targetPos);
    if (!currentTile) {
        qWarning("GroundBrush::doAutoBorders: Target tile not found at" << targetPos.x << targetPos.y << targetPos.z);
        return;
    }

    // Material of the current tile (the one whose borders are being calculated)
    // This material's 'borders' rules will be used.
    const assets::MaterialData* currentTileMaterial = getMaterialFromTile(currentTile, nullptr /*assetManager*/); // Pass nullptr for now
    const assets::MaterialGroundSpecifics* currentTileSpecifics = nullptr;
    if (currentTileMaterial && currentTileMaterial->isGround()) {
        currentTileSpecifics = std::get_if<assets::MaterialGroundSpecifics>(&currentTileMaterial->specificData);
    }

    if (!currentTileSpecifics) {
        // If the current tile has no ground or its ground isn't a known material,
        // it might still need its borders cleared if it previously had some.
        // controller->recordClearBorderItems(targetPos); // Action to remove all non-ground items
        qDebug() << "GroundBrush::doAutoBorders: No ground material on target tile" << targetPos.x << targetPos.y << targetPos.z << "or material has no ground specifics. Clearing borders (conceptually).";
        // Actual border clearing action would be: controller->recordClearBorderItems(targetPos);
        return;
    }

    // 1. Analyze 8 neighbors and build tiledata bitmask
    uint8_t tiledata = 0;
    std::array<const assets::MaterialData*, 8> neighborMaterials;

    static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
        {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
    }}; // NW, N, NE, W, E, SW, S, SE


    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(targetPos.x + neighborOffsets[i].first,
                                targetPos.y + neighborOffsets[i].second,
                                targetPos.z);
        const Tile* neighborTile = map->getTile(neighborPos); // Read-only get
        neighborMaterials[i] = getMaterialFromTile(neighborTile, nullptr /*assetManager*/); // Pass nullptr for now

        bool isDifferent = false;
        if (!neighborMaterials[i]) {
            isDifferent = true;
        } else if (neighborMaterials[i]->id != currentTileMaterial->id) {
            bool areFriends = false;
            if(currentTileSpecifics) { // Check currentTileSpecifics is not null
                 areFriends = currentTileSpecifics->friends.contains(neighborMaterials[i]->id);
            }
            // Also check if neighbor considers current tile a friend
            const auto* neighborSpecifics = std::get_if<assets::MaterialGroundSpecifics>(&neighborMaterials[i]->specificData);
            if(neighborSpecifics && !areFriends){ // check neighborSpecifics is not null
                areFriends = neighborSpecifics->friends.contains(currentTileMaterial->id);
            }

            if (!areFriends) {
                isDifferent = true;
            }
        }
        // TODO: Add settings check for WALLS_REPEL_BORDERS from AppSettings via controller
        // RME::AppSettings* appSettings = controller->getAppSettings();
        // if (appSettings && appSettings->isWallsRepelBordersEnabled() && neighborTile && neighborTile->hasWall()) isDifferent = false;

        if (isDifferent) {
            tiledata |= (1 << i);
        }
    }

    // 2. Use s_border_types[tiledata] to get packed BorderType enums
    uint32_t packedComputedBorderTypes = s_border_types[tiledata];
    if (packedComputedBorderTypes == 0 && tiledata != 0) {
         qWarning() << "GroundBrush::doAutoBorders: s_border_types lookup for tiledata" << Qt::bin << tiledata << "resulted in NO BORDERS. Table might be empty or this configuration yields no borders.";
    }

    // Action: Clear existing non-ground items (borders) before adding new ones
    // controller->recordClearBorderItems(targetPos);
    qDebug() << "GroundBrush::doAutoBorders: Conceptually clearing existing borders on tile" << targetPos.x << targetPos.y << targetPos.z;


    QList<uint16_t> newBorderItemIds;

    for (int pieceNum = 0; pieceNum < 4; ++pieceNum) {
        BorderType computedPiece = unpackBorderType(packedComputedBorderTypes, pieceNum);
        if (computedPiece == BorderType::NONE) {
            continue;
        }

        qDebug() << "GroundBrush::doAutoBorders: Computed border piece" << static_cast<int>(computedPiece) << "for tiledata" << Qt::bin << tiledata;

        QString alignStr = "unknown_align_placeholder";
        QString toBrushId = "unknown_neighbor_material_placeholder";

        // --- Placeholder for complex translation logic: computedPiece + neighbor data -> alignStr, toBrushId ---
        // This requires mapping BorderType enum (which might be WX_NORTH_HORIZONTAL etc.)
        // to an "align" string like "outer", "inner", "outer_n", "corner_nw", etc.
        // AND identifying which neighbor(s) this border piece is "against" to get 'toBrushId'.
        // This is a large piece of logic from the original RME.
        qWarning() << "GroundBrush::doAutoBorders: CRITICAL - Translation from BorderType to align/toBrushName not implemented.";


        bool ruleFound = false;
        if(currentTileSpecifics){ // Check currentTileSpecifics is not null
            for (const auto& rule : currentTileSpecifics->borders) {
                // TODO: Refine matching for 'alignStr'.
                bool alignMatch = (rule.align == alignStr); // This will likely always fail with placeholder alignStr
                // A temporary, very loose match for testing structure:
                // if (computedPiece != BorderType::NONE) alignMatch = true;


                if (alignMatch && (rule.toBrushName == toBrushId || rule.toBrushName == "all")) {
                    // TODO: Handle 'super' rules (priority)
                    newBorderItemIds.append(rule.borderItemId);
                    ruleFound = true;
                    qDebug() << "GroundBrush::doAutoBorders: Applying rule for align" << rule.align << "to" << toBrushId << "item" << rule.borderItemId;
                    break;
                }
            }
        }
        if (!ruleFound && computedPiece != BorderType::NONE) {
             qDebug() << "GroundBrush::doAutoBorders: No specific MaterialBorderRule found for piece" << static_cast<int>(computedPiece) << "against" << toBrushId << "with align" << alignStr;
        }
    }

    // 5. Apply new borders (after clearing old ones)
    if (!newBorderItemIds.isEmpty()) {
        // controller->recordSetBorderItems(targetPos, newBorderItemIds); // New controller action needed
        qDebug() << "GroundBrush::doAutoBorders: Would apply new border items to" << targetPos.x << targetPos.y << targetPos.z << ":" << newBorderItemIds;
    } else if (tiledata != 0 && packedComputedBorderTypes != 0) { // If there was a border config, but no rules matched
        qDebug() << "GroundBrush::doAutoBorders: Border configuration found (tiledata" << Qt::bin << tiledata << ", packed" << packedComputedBorderTypes << ") but no matching rules led to new border items for" << targetPos.x << targetPos.y << targetPos.z;
    } else {
         // This case is normal if tiledata was 0 (no borders needed) or s_border_types[tiledata] was 0.
    }

    // TODO: Implement "specific cases" logic if required.
}


} // namespace core
} // namespace RME
