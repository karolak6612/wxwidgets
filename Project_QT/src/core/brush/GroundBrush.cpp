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
#include "core/assets/MaterialManager.h" // For AssetManager::getMaterialManager()

#include <QRandomGenerator>
#include <QDebug>
#include <array> // For std::array
#include <algorithm> // For std::sort


namespace { // Anonymous namespace for helpers

const RME::core::assets::MaterialData* getMaterialFromTile(
    const RME::core::Tile* tile,
    const RME::core::assets::AssetManager* assetManager) {

    if (!tile || !tile->getGround() || !assetManager) {
        return nullptr;
    }

    const RME::core::Item* groundItem = tile->getGround();
    uint16_t groundItemId = groundItem->getID();

    const RME::core::assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
    const RME::core::assets::MaterialManager* materialMgr = assetManager->getMaterialManager();

    if (!itemDb || !materialMgr) { // Also check materialMgr
        qWarning("getMaterialFromTile: ItemDatabase or MaterialManager not available via AssetManager.");
        return nullptr;
    }

    const RME::core::assets::ItemData* itemData = itemDb->getItemData(groundItemId);
    if (!itemData) {
        // qWarning("getMaterialFromTile: ItemData not found for ground item ID %u.", groundItemId); // Less verbose
        return nullptr;
    }

    if (!itemData->materialId.isEmpty()) {
        const RME::core::assets::MaterialData* foundMaterial = materialMgr->getMaterial(itemData->materialId);
        // if (!foundMaterial) { // Less verbose
        //     qDebug("getMaterialFromTile: Material ID '%s' from ItemData (item %u) not found in MaterialManager.",
        //            qUtf8Printable(itemData->materialId), groundItemId);
        // }
        return foundMaterial;
    } else {
        // qDebug("getMaterialFromTile: Item ID %u (name: '%s') has no associated materialId in its ItemData.",
        //        groundItemId, qUtf8Printable(itemData->name)); // Less verbose
        return nullptr;
    }
}

QString determineAlignString(
    RME::BorderType pieceType,
    uint8_t /*tiledata*/, // tiledata might be used for more complex inner/outer distinctions later
    const std::array<const RME::core::assets::MaterialData*, 8>& /*neighborMaterials*/,
    const RME::core::assets::MaterialData* /*currentTileMaterial*/
) {
    switch (pieceType) {
        case RME::BorderType::WX_NORTH_HORIZONTAL:
        case RME::BorderType::WX_EAST_HORIZONTAL:
        case RME::BorderType::WX_SOUTH_HORIZONTAL:
        case RME::BorderType::WX_WEST_HORIZONTAL:
        case RME::BorderType::WX_NORTHWEST_CORNER:
        case RME::BorderType::WX_NORTHEAST_CORNER:
        case RME::BorderType::WX_SOUTHWEST_CORNER:
        case RME::BorderType::WX_SOUTHEAST_CORNER:
        case RME::BorderType::WX_NORTHWEST_DIAGONAL:
        case RME::BorderType::WX_NORTHEAST_DIAGONAL:
        case RME::BorderType::WX_SOUTHWEST_DIAGONAL:
        case RME::BorderType::WX_SOUTHEAST_DIAGONAL:
            return QStringLiteral("outer"); // Most ground borders are outer borders

        case RME::BorderType::NONE:
        default:
            if (pieceType == RME::BorderType::NONE) return QStringLiteral("none");
            qWarning("determineAlignString: Unhandled BorderType %d, defaulting align to 'outer'.", static_cast<int>(pieceType));
            return QStringLiteral("outer");
    }
}

QString determineToBrushName(
    RME::BorderType pieceType,
    uint8_t tiledata, // Used to know *which* neighbors are different
    const std::array<const RME::core::assets::MaterialData*, 8>& neighborMaterials,
    const RME::core::assets::MaterialData* currentTileMaterial // The material of the tile we are placing borders on
) {
    // Helper to check if a neighbor at index 'idx' is different and not a friend
    auto isNeighborDifferentNonFriend = [&](int idx) {
        if (idx < 0 || idx >= 8) return false;

        bool tileIsDifferentBit = (tiledata & (1 << idx));
        if (!tileIsDifferentBit) return false; // This neighbor was not marked as 'different' by tiledata calculation

        if (neighborMaterials[idx]) {
            return true; // It's different and was included in tiledata (implies not friend or different ID)
        } else {
            return true; // It's void, thus different
        }
    };

    // Indices for neighbors: NW(0), N(1), NE(2), W(3), E(4), SW(5), S(6), SE(7)
    switch (pieceType) {
        case RME::BorderType::WX_NORTH_HORIZONTAL: // Border against North
            if (isNeighborDifferentNonFriend(1)) return neighborMaterials[1] ? neighborMaterials[1]->id : QStringLiteral("none");
            break;
        case RME::BorderType::WX_EAST_HORIZONTAL:  // Border against East
            if (isNeighborDifferentNonFriend(4)) return neighborMaterials[4] ? neighborMaterials[4]->id : QStringLiteral("none");
            break;
        case RME::BorderType::WX_SOUTH_HORIZONTAL: // Border against South
            if (isNeighborDifferentNonFriend(6)) return neighborMaterials[6] ? neighborMaterials[6]->id : QStringLiteral("none");
            break;
        case RME::BorderType::WX_WEST_HORIZONTAL:  // Border against West
            if (isNeighborDifferentNonFriend(3)) return neighborMaterials[3] ? neighborMaterials[3]->id : QStringLiteral("none");
            break;

        case RME::BorderType::WX_NORTHWEST_CORNER: // Against N or W or NW
            if (isNeighborDifferentNonFriend(1)) return neighborMaterials[1] ? neighborMaterials[1]->id : QStringLiteral("none"); // Prioritize N
            if (isNeighborDifferentNonFriend(3)) return neighborMaterials[3] ? neighborMaterials[3]->id : QStringLiteral("none"); // Then W
            if (isNeighborDifferentNonFriend(0)) return neighborMaterials[0] ? neighborMaterials[0]->id : QStringLiteral("none"); // Then NW
            break;
        case RME::BorderType::WX_NORTHEAST_CORNER: // Against N or E or NE
            if (isNeighborDifferentNonFriend(1)) return neighborMaterials[1] ? neighborMaterials[1]->id : QStringLiteral("none"); // N
            if (isNeighborDifferentNonFriend(4)) return neighborMaterials[4] ? neighborMaterials[4]->id : QStringLiteral("none"); // E
            if (isNeighborDifferentNonFriend(2)) return neighborMaterials[2] ? neighborMaterials[2]->id : QStringLiteral("none"); // NE
            break;
        case RME::BorderType::WX_SOUTHWEST_CORNER: // Against S or W or SW
            if (isNeighborDifferentNonFriend(6)) return neighborMaterials[6] ? neighborMaterials[6]->id : QStringLiteral("none"); // S
            if (isNeighborDifferentNonFriend(3)) return neighborMaterials[3] ? neighborMaterials[3]->id : QStringLiteral("none"); // W
            if (isNeighborDifferentNonFriend(5)) return neighborMaterials[5] ? neighborMaterials[5]->id : QStringLiteral("none"); // SW
            break;
        case RME::BorderType::WX_SOUTHEAST_CORNER: // Against S or E or SE
            if (isNeighborDifferentNonFriend(6)) return neighborMaterials[6] ? neighborMaterials[6]->id : QStringLiteral("none"); // S
            if (isNeighborDifferentNonFriend(4)) return neighborMaterials[4] ? neighborMaterials[4]->id : QStringLiteral("none"); // E
            if (isNeighborDifferentNonFriend(7)) return neighborMaterials[7] ? neighborMaterials[7]->id : QStringLiteral("none"); // SE
            break;
        case RME::BorderType::WX_NORTHWEST_DIAGONAL: // N and W are different
            if (isNeighborDifferentNonFriend(1)) return neighborMaterials[1] ? neighborMaterials[1]->id : QStringLiteral("none"); // Check N
            if (isNeighborDifferentNonFriend(3)) return neighborMaterials[3] ? neighborMaterials[3]->id : QStringLiteral("none"); // Check W
            break;
        case RME::BorderType::WX_NORTHEAST_DIAGONAL: // N and E are different
            if (isNeighborDifferentNonFriend(1)) return neighborMaterials[1] ? neighborMaterials[1]->id : QStringLiteral("none");
            if (isNeighborDifferentNonFriend(4)) return neighborMaterials[4] ? neighborMaterials[4]->id : QStringLiteral("none");
            break;
        case RME::BorderType::WX_SOUTHWEST_DIAGONAL: // S and W are different
            if (isNeighborDifferentNonFriend(6)) return neighborMaterials[6] ? neighborMaterials[6]->id : QStringLiteral("none");
            if (isNeighborDifferentNonFriend(3)) return neighborMaterials[3] ? neighborMaterials[3]->id : QStringLiteral("none");
            break;
        case RME::BorderType::WX_SOUTHEAST_DIAGONAL: // S and E are different
            if (isNeighborDifferentNonFriend(6)) return neighborMaterials[6] ? neighborMaterials[6]->id : QStringLiteral("none");
            if (isNeighborDifferentNonFriend(4)) return neighborMaterials[4] ? neighborMaterials[4]->id : QStringLiteral("none");
            break;

        case RME::BorderType::NONE:
            return QStringLiteral("none");
        default:
            break;
    }
    qWarning("determineToBrushName: Could not determine a single target brush for BorderType %d with tiledata %u. Defaulting to 'none'. Rule matching might need 'all'.",
             static_cast<int>(pieceType), tiledata);
    return QStringLiteral("none");
}

static QString borderTypeToEdgeKey(RME::BorderType pieceType) {
    switch (pieceType) {
        case RME::BorderType::WX_NORTH_HORIZONTAL: return QStringLiteral("n");
        case RME::BorderType::WX_EAST_HORIZONTAL:  return QStringLiteral("e");
        case RME::BorderType::WX_SOUTH_HORIZONTAL: return QStringLiteral("s");
        case RME::BorderType::WX_WEST_HORIZONTAL:  return QStringLiteral("w");
        case RME::BorderType::WX_NORTHWEST_CORNER: return QStringLiteral("cnw");
        case RME::BorderType::WX_NORTHEAST_CORNER: return QStringLiteral("cne");
        case RME::BorderType::WX_SOUTHWEST_CORNER: return QStringLiteral("csw");
        case RME::BorderType::WX_SOUTHEAST_CORNER: return QStringLiteral("cse");
        case RME::BorderType::WX_NORTHWEST_DIAGONAL: return QStringLiteral("dnw");
        case RME::BorderType::WX_NORTHEAST_DIAGONAL: return QStringLiteral("dne");
        case RME::BorderType::WX_SOUTHWEST_DIAGONAL: return QStringLiteral("dsw");
        case RME::BorderType::WX_SOUTHEAST_DIAGONAL: return QStringLiteral("dse");
        default: return QString(); // Invalid or NONE
    }
}

// Placeholder for evaluating specific conditions
static bool evaluateSpecificConditions(
    const QList<RME::core::assets::SpecificCondition>& conditions,
    const RME::core::Tile* targetTile,
    const RME::core::assets::AssetManager* assetManager,
    const std::array<const RME::core::assets::MaterialData*, 8>& /*neighborMaterials*/, // For future use
    const QList<uint16_t>& oldBorderItemIds) {

    if (!targetTile || !assetManager) return false;

    for (const auto& cond : conditions) {
        bool currentConditionMet = false;
        switch (cond.type) {
            case RME::core::assets::SpecificConditionType::MATCH_BORDER: {
                bool ok = false;
                uint16_t matchItemId = cond.targetId.toUShort(&ok);
                if (ok) {
                    if (oldBorderItemIds.contains(matchItemId)) {
                        currentConditionMet = true;
                    }
                    if (!cond.edge.isEmpty()) {
                        qWarning("GroundBrush::evaluateSpecificConditions: MATCH_BORDER condition for item %u. Edge '%s' specific matching is NOT YET IMPLEMENTED. Condition currently checks for item presence anywhere on tile.", matchItemId, qUtf8Printable(cond.edge));
                    }
                } else {
                    qWarning("GroundBrush::evaluateSpecificConditions: Invalid item ID '%s' in MATCH_BORDER condition.", qUtf8Printable(cond.targetId));
                }
                break;
            }
            case RME::core::assets::SpecificConditionType::MATCH_GROUND: {
                const RME::core::assets::MaterialData* groundMaterial = ::getMaterialFromTile(targetTile, assetManager);
                if (groundMaterial && groundMaterial->id == cond.targetId) {
                    currentConditionMet = true;
                } else if (!groundMaterial && cond.targetId.compare("none", Qt::CaseInsensitive) == 0) { // Matching void ground
                    currentConditionMet = true;
                }
                break;
            }
            case RME::core::assets::SpecificConditionType::UNKNOWN:
            default:
                qWarning("GroundBrush::evaluateSpecificConditions: Unknown or UNKNOWN condition type encountered.");
                break; // Fails this condition
        }
        if (!currentConditionMet) return false; // All conditions must be met
    }
    return true; // All conditions met, or no conditions to meet
}

// Placeholder for applying specific actions
static void applySpecificActions(
    QList<uint16_t>& itemsForPiece, // Items determined so far for this specific geometric piece
    const QList<RME::core::assets::SpecificAction>& actions,
    uint16_t baseItemIdForThisPiece, // The item ID derived from ruleTargetId or BorderSet for this piece
    bool keepBaseBorderFromSpecificCase) {

    if (!keepBaseBorderFromSpecificCase && baseItemIdForThisPiece != 0) {
        itemsForPiece.removeAll(baseItemIdForThisPiece);
    }

    for (const auto& act : actions) {
        switch (act.type) {
            case RME::core::assets::SpecificActionType::REPLACE_BORDER: {
                bool ok = false;
                uint16_t originalItemId = act.targetId.toUShort(&ok); // Item to be replaced on this edge

                qWarning("GroundBrush::applySpecificActions: REPLACE_BORDER action. Edge '%s' specific replacement for item %u with %u is NOT YET FULLY IMPLEMENTED. Action defaults to replacing any instance of item %u or adding %u if original not present.",
                         qUtf8Printable(act.edge), originalItemId, act.withItemId, originalItemId, act.withItemId );

                if (ok && originalItemId != 0) {
                    itemsForPiece.removeAll(originalItemId);
                }
                if (act.withItemId != 0 && !itemsForPiece.contains(act.withItemId)) {
                    itemsForPiece.append(act.withItemId);
                }
                break;
            }
            case RME::core::assets::SpecificActionType::ADD_ITEM: {
                if (act.affectedItemId != 0 && !itemsForPiece.contains(act.affectedItemId)) {
                    itemsForPiece.append(act.affectedItemId);
                }
                break;
            }
            case RME::core::assets::SpecificActionType::UNKNOWN:
            default:
                qWarning("GroundBrush::applySpecificActions: Unknown or UNKNOWN action type encountered.");
                break;
        }
    }

    if (keepBaseBorderFromSpecificCase && baseItemIdForThisPiece != 0) {
        if (!itemsForPiece.contains(baseItemIdForThisPiece)) {
            itemsForPiece.append(baseItemIdForThisPiece);
        }
    }
    // Ensure uniqueness by converting to set and back, if QList operations weren't careful
    // QSet<uint16_t> uniqueItems(itemsForPiece.begin(), itemsForPiece.end());
    // itemsForPiece = QList<uint16_t>(uniqueItems.begin(), uniqueItems.end());
    // For now, rely on .contains() checks.
}


} // end anonymous namespace


// Static member definitions
uint32_t RME::core::GroundBrush::s_border_types[256];
bool RME::core::GroundBrush::s_staticDataInitialized = false;

namespace RME {
namespace core {

void GroundBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }

    using namespace RME; // For TILE_... constants
    using BT = RME::BorderType; // Alias for RME::BorderType

    // Initialize all to BORDER_NONE first, as a base.
    for (int i = 0; i < 256; ++i) {
        s_border_types[i] = packBorderTypes(BT::NONE);
    }

    // --- Ported data from wxwidgets/brush_tables.cpp GroundBrush::init() ---
    // GroundBrush::border_types in wxWidgets stores packed uint32_t.
    // Each component is a BorderType enum value.
    // packBorderTypes(p1, p2, p3, p4) creates this uint32_t.

    s_border_types[0] = packBorderTypes(BT::NONE);
    s_border_types[TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_NE] = packBorderTypes(BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_NE | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_W | TILE_NE] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_E] = packBorderTypes(BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_N] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_E | TILE_NE] = packBorderTypes(BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_E | TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER);
    s_border_types[TILE_SW | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SW | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_SW | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SW | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_SW | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_SW | TILE_W | TILE_NE] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_SW | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_SW | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL);
    s_border_types[TILE_SW | TILE_E] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SW | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SW | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SW | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SW | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SW | TILE_E | TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_W] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL);
    s_border_types[TILE_S | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL);
    s_border_types[TILE_S | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL);
    s_border_types[TILE_S | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL);
    s_border_types[TILE_S | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SW | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SW | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SW | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_W] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_EAST_HORIZONTAL); // Strict port
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_EAST_HORIZONTAL); // Strict port
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE] = packBorderTypes(BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_NE] = packBorderTypes(BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W | TILE_NE] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_WEST_HORIZONTAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_E] = packBorderTypes(BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SE | TILE_E | TILE_N] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SE | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SE | TILE_E | TILE_NE] = packBorderTypes(BT::WX_EAST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_SE | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SE | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHEAST_DIAGONAL);
    s_border_types[TILE_SE | TILE_E | TILE_W] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_CORNER, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_W] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_NORTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER);
    s_border_types[TILE_SE | TILE_SW | TILE_E] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_W] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_E] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTHEAST_CORNER, BT::WX_NORTHWEST_CORNER);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHWEST_DIAGONAL, BT::WX_NORTHEAST_CORNER, BT::WX_SOUTHEAST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHEAST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTHEAST_DIAGONAL, BT::WX_SOUTHWEST_CORNER, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_SOUTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL);
    s_border_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = packBorderTypes(BT::WX_SOUTH_HORIZONTAL, BT::WX_EAST_HORIZONTAL, BT::WX_WEST_HORIZONTAL, BT::WX_NORTH_HORIZONTAL, BT::WX_NORTHWEST_CORNER); // Strict port

    qInfo("GroundBrush::s_border_types table has been fully initialized by porting static assignments from wxwidgets/brush_tables.cpp.");
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

    // --- Auto-Bordering Phase ---
    doAutoBorders(controller, pos); // For the primary tile

    static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1}; // NW, N, NE, W, E, SW, S, SE
    static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int i = 0; i < 8; ++i) {
        RME::core::Position neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) { // Check validity before calling
            doAutoBorders(controller, neighborPos);
        }
    }

    // --- Notify Changes ---
    controller->notifyTileChanged(pos); // Notify primary tile

    for (int i = 0; i < 8; ++i) { // dx, dy are from the block above
        RME::core::Position neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            controller->notifyTileChanged(neighborPos);
        }
    }
}

// Legacy compatibility methods for direct map manipulation
void GroundBrush::draw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) {
    if (!map || !tile || !settings) {
        qWarning() << "GroundBrush::draw: Invalid parameters (map, tile, or settings is null)";
        return;
    }
    
    if (!m_materialData) {
        qWarning() << "GroundBrush::draw: No material set";
        return;
    }
    
    const auto* materialSpecifics = getCurrentGroundSpecifics();
    if (!materialSpecifics || materialSpecifics->items.empty()) {
        qWarning() << "GroundBrush::draw: Material has no ground items defined";
        return;
    }
    
    // Select random ground item based on chances
    int totalChance = 0;
    for (const auto& itemEntry : materialSpecifics->items) {
        totalChance += itemEntry.chance;
    }
    
    if (totalChance == 0) {
        totalChance = 1; // Fallback to ensure we can select something
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
    
    // Create and set the ground item directly on the tile
    auto groundItem = std::make_unique<RME::core::Item>(selectedItemId);
    tile->setGround(std::move(groundItem));
    
    qDebug() << "GroundBrush::draw: Set ground item" << selectedItemId << "on tile";
}

void GroundBrush::undraw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) {
    Q_UNUSED(map)
    Q_UNUSED(settings)
    
    if (!tile) {
        qWarning() << "GroundBrush::undraw: Invalid tile parameter";
        return;
    }
    
    // Remove ground item from tile
    tile->setGround(nullptr);
    qDebug() << "GroundBrush::undraw: Removed ground item from tile";
}

// ... (apply method and other GroundBrush methods as before) ...
} // namespace core
} // namespace RME

void RME::core::GroundBrush::doAutoBorders(RME::core::editor::EditorControllerInterface* controller,
                                           const RME::core::Position& targetPos) {
    if (!s_staticDataInitialized) {
        qCritical("GroundBrush::doAutoBorders: s_border_types not initialized!");
        // initializeStaticData(); // Constructor should have handled this.
        if (!s_staticDataInitialized) {
            qCritical("GroundBrush::doAutoBorders: Initialization failed even after explicit call. Aborting.");
            return;
        }
    }

    RME::core::map::Map* map = controller->getMap();
    RME::core::assets::AssetManager* assetManager = controller->getAssetManager();

    if (!map || !assetManager) {
        qWarning("GroundBrush::doAutoBorders: Map or AssetManager not available from controller for pos %s.", qUtf8Printable(targetPos.toString()));
        return;
    }

    const RME::core::assets::MaterialManager* materialManager = assetManager->getMaterialManager();
    if (!materialManager) {
        qWarning("GroundBrush::doAutoBorders: MaterialManager not available from AssetManager for pos %s.", qUtf8Printable(targetPos.toString()));
        return;
    }

    const RME::core::assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
    if (!itemDb) {
        qWarning("GroundBrush::doAutoBorders: ItemDatabase not available for pos %s.", qUtf8Printable(targetPos.toString()));
        return;
    }

    const RME::core::Tile* targetTile = map->getTile(targetPos);
    if (!targetTile) {
        qDebug("GroundBrush::doAutoBorders: Target tile not found at %s. No borders will be applied.", qUtf8Printable(targetPos.toString()));
        return;
    }

    // 1. Collect old border item IDs
    QList<uint16_t> oldBorderItemIds;
    for (const auto& itemPtr : targetTile->getItems()) {
        if (itemPtr) {
            const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
            if (itemData) {
                 oldBorderItemIds.append(itemPtr->getID());
            }
        }
    }
    std::sort(oldBorderItemIds.begin(), oldBorderItemIds.end());

    // 2. Get current tile's material
    const RME::core::assets::MaterialData* currentTileMaterial = ::getMaterialFromTile(targetTile, assetManager);
    const RME::core::assets::MaterialGroundSpecifics* currentTileSpecifics = nullptr;
    if (currentTileMaterial && currentTileMaterial->isGround()) {
        currentTileSpecifics = std::get_if<RME::core::assets::MaterialGroundSpecifics>(&currentTileMaterial->specificData);
    }

    QList<uint16_t> newBorderItemIds;

    if (currentTileMaterial && currentTileSpecifics) {
        // 4. Calculate tiledata
        uint8_t tiledata = 0;
        std::array<const RME::core::assets::MaterialData*, 8> neighborMaterials;
        static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
            {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
        }};

        for (int i = 0; i < 8; ++i) {
            RME::core::Position neighborPos(targetPos.x + neighborOffsets[i].first,
                                            targetPos.y + neighborOffsets[i].second,
                                            targetPos.z);
            const RME::core::Tile* neighborTile = map->getTile(neighborPos);
            neighborMaterials[i] = ::getMaterialFromTile(neighborTile, assetManager);

            bool isDifferent = false;
            if (!neighborMaterials[i]) {
                isDifferent = true;
            } else if (neighborMaterials[i]->id != currentTileMaterial->id) {
                bool areFriends = currentTileSpecifics->friends.contains(neighborMaterials[i]->id);
                if (!areFriends) {
                    isDifferent = true;
                }
            }

            if (isDifferent) {
                tiledata |= (1 << i);
            }
        }

        // 5. Lookup and Unpack Border Types
        uint32_t packedComputedBorderTypes = s_border_types[tiledata];

        // Helper lambda to resolve a rule and piece to an item ID (defined earlier, now just for context)
        auto resolveItemForRuleAndPiece =
            [&materialManager](const RME::core::assets::MaterialBorderRule* rule, RME::BorderType pieceType) -> uint16_t {
            if (!rule) return 0;

            bool conversionOk = false;
            uint16_t directItemId = rule->ruleTargetId.toUShort(&conversionOk);

            if (conversionOk && directItemId != 0) {
                return directItemId;
            } else {
                const RME::core::assets::BorderSetData* borderSet = materialManager->getBorderSet(rule->ruleTargetId);
                if (borderSet) {
                    QString edgeKey = ::borderTypeToEdgeKey(pieceType);
                    if (!edgeKey.isEmpty()) {
                        return borderSet->edgeItems.value(edgeKey, 0);
                    } else {
                        qWarning("GroundBrush::doAutoBorders: Could not map BorderType %d to an edge key for set '%s'.",
                                 static_cast<int>(pieceType), qUtf8Printable(rule->ruleTargetId));
                    }
                } else {
                    if (!conversionOk || directItemId == 0) {
                         qWarning("GroundBrush::doAutoBorders: ruleTargetId '%s' for align '%s' to '%s' is not a valid direct item ID and not a found BorderSet.",
                                 qUtf8Printable(rule->ruleTargetId), qUtf8Printable(rule->align), qUtf8Printable(rule->toBrushName));
                    }
                }
            }
            return 0;
        };

        for (int pieceNum = 0; pieceNum < 4; ++pieceNum) {
            RME::BorderType piece = RME::unpackBorderType(packedComputedBorderTypes, pieceNum);
            if (piece == RME::BorderType::NONE) continue;

            QList<uint16_t> itemsForThisPiece; // Holds items specifically for this geometric piece

            QString alignStr = ::determineAlignString(piece, tiledata, neighborMaterials, currentTileMaterial);
            QString toBrushIdStr = ::determineToBrushName(piece, tiledata, neighborMaterials, currentTileMaterial);

            if (alignStr == QLatin1String("none") && piece != RME::BorderType::NONE) {
                 qDebug("GroundBrush::doAutoBorders: alignStr is 'none' for a valid pieceType %d for tile %s. Skipping.", static_cast<int>(piece), qUtf8Printable(targetPos.toString()));
                 continue;
            }

            QList<const RME::core::assets::MaterialBorderRule*> pieceMatchedSuperRules;
            QList<const RME::core::assets::MaterialBorderRule*> pieceMatchedNormalRules;

            for (const auto& rule : currentTileSpecifics->borders) {
                bool alignMatch = (rule.align.compare(alignStr, Qt::CaseInsensitive) == 0 ||
                                   rule.align.compare(QStringLiteral("any"), Qt::CaseInsensitive) == 0);
                bool toBrushMatch = (rule.toBrushName.compare(toBrushIdStr, Qt::CaseInsensitive) == 0 ||
                                     rule.toBrushName.compare(QStringLiteral("all"), Qt::CaseInsensitive) == 0 ||
                                     (toBrushIdStr == QLatin1String("none") && rule.toBrushName.compare(QStringLiteral("none"), Qt::CaseInsensitive) == 0));
                if (alignMatch && toBrushMatch) {
                    if (rule.isSuper) pieceMatchedSuperRules.append(&rule);
                    else pieceMatchedNormalRules.append(&rule);
                }
            }

            bool superRuleGeneratedItemsForThisPiece = false;
            for (const auto* rule : pieceMatchedSuperRules) {
                uint16_t baseItemId = resolveItemForRuleAndPiece(rule, piece);
                QList<uint16_t> resolvedItemsForRule; // Items from this rule after specific cases

                if (rule->specificRuleCases.isEmpty()) {
                    if (baseItemId != 0) resolvedItemsForRule.append(baseItemId);
                } else {
                    bool specificCaseApplied = false;
                    for (const auto& specificCase : rule->specificRuleCases) {
                        if (::evaluateSpecificConditions(specificCase.conditions, targetTile, assetManager, neighborMaterials, oldBorderItemIds)) {
                            QList<uint16_t> tempItemsForAction = resolvedItemsForRule; // Or start fresh?
                            if (specificCase.keepBaseBorder && baseItemId != 0 && !tempItemsForAction.contains(baseItemId)) {
                                tempItemsForAction.append(baseItemId);
                            }
                            ::applySpecificActions(tempItemsForAction, specificCase.actions, baseItemId, specificCase.keepBaseBorder);
                            resolvedItemsForRule = tempItemsForAction; // Update with items after actions
                            specificCaseApplied = true;
                            break;
                        }
                    }
                    if (!specificCaseApplied && baseItemId != 0) { // No specific case met, use base
                        resolvedItemsForRule.append(baseItemId);
                    }
                }
                for (uint16_t item : resolvedItemsForRule) {
                    if (item != 0 && !itemsForThisPiece.contains(item)) itemsForThisPiece.append(item);
                }
            }
            if (!itemsForThisPiece.isEmpty()) {
                superRuleGeneratedItemsForThisPiece = true;
            }

            if (!superRuleGeneratedItemsForThisPiece) {
                for (const auto* rule : pieceMatchedNormalRules) {
                    uint16_t baseItemId = resolveItemForRuleAndPiece(rule, piece);
                    QList<uint16_t> resolvedItemsForRule;
                    if (rule->specificRuleCases.isEmpty()) {
                        if (baseItemId != 0) resolvedItemsForRule.append(baseItemId);
                    } else {
                        bool specificCaseApplied = false;
                        for (const auto& specificCase : rule->specificRuleCases) {
                            if (::evaluateSpecificConditions(specificCase.conditions, targetTile, assetManager, neighborMaterials, oldBorderItemIds)) {
                                QList<uint16_t> tempItemsForAction = resolvedItemsForRule;
                                if (specificCase.keepBaseBorder && baseItemId != 0 && !tempItemsForAction.contains(baseItemId)) {
                                     tempItemsForAction.append(baseItemId);
                                }
                                ::applySpecificActions(tempItemsForAction, specificCase.actions, baseItemId, specificCase.keepBaseBorder);
                                resolvedItemsForRule = tempItemsForAction;
                                specificCaseApplied = true;
                                break;
                            }
                        }
                        if (!specificCaseApplied && baseItemId != 0) {
                             resolvedItemsForRule.append(baseItemId);
                        }
                    }
                    for (uint16_t item : resolvedItemsForRule) {
                        if (item != 0 && !itemsForThisPiece.contains(item)) itemsForThisPiece.append(item);
                    }
                }
            }
            // Add all unique items for this piece to the main list
            for (uint16_t item : itemsForThisPiece) {
                if (item != 0 && !newBorderItemIds.contains(item)) {
                    newBorderItemIds.append(item);
                }
            }
        }
    }

    // 7. Apply Changes
    std::sort(newBorderItemIds.begin(), newBorderItemIds.end());
    if (oldBorderItemIds != newBorderItemIds) {
        qDebug() << "GroundBrush::doAutoBorders: Tile" << targetPos.toString() << "borders changing. Old:" << oldBorderItemIds << "New:" << newBorderItemIds;
        controller->recordSetBorderItems(targetPos, newBorderItemIds, oldBorderItemIds);
    } else {
        // qDebug() << "GroundBrush::doAutoBorders: Tile" << targetPos.toString() << "borders did not change.";
    }
}
