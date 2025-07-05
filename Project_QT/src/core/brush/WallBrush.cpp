#include "core/brush/WallBrush.h"
#include "core/assets/MaterialData.h"
#include "core/assets/MaterialManager.h" // For friend brush lookup
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"
#include "core/assets/ItemDatabase.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h" // For place door/window modes
#include "core/assets/AssetManager.h"   // To get ItemDatabase, MaterialManager

#include <QRandomGenerator>
#include <QDebug>
#include <array>
#include <algorithm> // For std::sort

// Static member definitions
uint32_t RME::core::WallBrush::s_full_wall_types[16];
uint32_t RME::core::WallBrush::s_half_wall_types[16];
bool RME::core::WallBrush::s_staticDataInitialized = false;

// Define wall tiledata bits if not globally available from BrushEnums.h
// These must match the order N, W, E, S for the s_..._wall_types arrays.
static constexpr uint8_t LOCAL_WALL_N_BIT = (1 << 0);
static constexpr uint8_t LOCAL_WALL_W_BIT = (1 << 1);
static constexpr uint8_t LOCAL_WALL_E_BIT = (1 << 2);
static constexpr uint8_t LOCAL_WALL_S_BIT = (1 << 3);


namespace RME {
namespace core {

// Anonymous namespace for local helpers if needed
namespace {
    bool hasMatchingWallMaterialAtTile(
        const RME::core::Tile* tile,
        const RME::core::assets::MaterialData* currentWallMaterial,
        const RME::core::assets::AssetManager* assetManager) {
        if (!tile || !currentWallMaterial || !assetManager) {
            return false;
        }
        const RME::core::assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
        const RME::core::assets::MaterialManager* materialMgr = assetManager->getMaterialManager();
        if(!itemDb || !materialMgr) return false;

        for (const auto& itemPtr : tile->getItems()) {
            if (itemPtr) {
                const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
                if (itemData && itemData->isWall) {
                    if (itemData->materialId == currentWallMaterial->id) {
                        return true;
                    }
                    const RME::core::assets::MaterialData* neighborWallMaterial = materialMgr->getMaterial(itemData->materialId);
                    if (neighborWallMaterial) {
                        const auto* currentSpecifics = std::get_if<RME::core::assets::MaterialWallSpecifics>(&currentWallMaterial->specificData);
                        if (currentSpecifics && currentSpecifics->friends.contains(neighborWallMaterial->id)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
}


WallBrush::WallBrush() : m_materialData(nullptr) {
    initializeStaticData();
}

void WallBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isWall()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "WallBrush::setMaterial: Material is null or not a wall type.";
    }
}

const RME::core::assets::MaterialData* WallBrush::getMaterial() const {
    return m_materialData;
}

const RME::core::assets::MaterialWallSpecifics* WallBrush::getCurrentWallSpecifics() const {
    if (m_materialData && m_materialData->isWall()) {
        return std::get_if<RME::core::assets::MaterialWallSpecifics>(&m_materialData->specificData);
    }
    return nullptr;
}

QString WallBrush::getName() const {
    if (m_materialData) {
        return m_materialData->id;
    }
    return QStringLiteral("Wall Brush");
}

int WallBrush::getLookID(const RME::core::BrushSettings& settings) const {
     if (!m_materialData) return 0;
    if (m_materialData->lookId != 0) return m_materialData->lookId;

    const auto* wallSpecifics = getCurrentWallSpecifics();
    if (wallSpecifics) {
        uint16_t serverItemId = getItemIdForSegment(RME::BorderType::WALL_POLE, settings, wallSpecifics);
        if (serverItemId == 0) {
            serverItemId = getItemIdForSegment(RME::BorderType::WALL_HORIZONTAL, settings, wallSpecifics);
        }
        if (serverItemId != 0) {
             qWarning("WallBrush 'getLookID': Material %s has no client lookId. Attempting to use server ID %u. THIS REQUIRES CONVERSION.",
                      qUtf8Printable(m_materialData->id), serverItemId);
            return 0;
        }
    }
    if (m_materialData->serverLookId != 0) {
        qWarning("WallBrush 'getLookID': Material %s has serverLookId %u but no client lookId. THIS REQUIRES CONVERSION.",
                 qUtf8Printable(m_materialData->id), m_materialData->serverLookId);
        return 0;
    }
    qWarning("WallBrush 'getLookID': Material %s has no lookId, serverLookId, or default items to derive a look from.", qUtf8Printable(m_materialData->id));
    return 0;
}

bool WallBrush::canApply(const RME::core::map::Map* map,
                           const RME::core::Position& pos,
                           const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) return false;
    const auto* specifics = getCurrentWallSpecifics();
    if (!specifics || specifics->parts.empty()) {
         qWarning("WallBrush::canApply: No wall parts defined for material %s", qUtf8Printable(m_materialData->id));
        return false;
    }
    if (!map || !map->isPositionValid(pos)) return false;
    return true;
}

void WallBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }
    using namespace RME;
    using BT = RME::BorderType;

    for (int i = 0; i < 16; ++i) {
        s_full_wall_types[i] = static_cast<uint32_t>(BT::WALL_POLE);
        s_half_wall_types[i] = static_cast<uint32_t>(BT::WALL_POLE);
    }

    s_full_wall_types[0] = static_cast<uint32_t>(BT::WALL_POLE);
    s_full_wall_types[LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTH_END);
    s_full_wall_types[LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_EAST_END);
    s_full_wall_types[LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHEAST_DIAGONAL);
    s_full_wall_types[LOCAL_WALL_E_BIT] = static_cast<uint32_t>(BT::WALL_WEST_END);
    s_full_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHWEST_DIAGONAL);
    s_full_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_HORIZONTAL);
    s_full_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTH_T);
    s_full_wall_types[LOCAL_WALL_S_BIT] = static_cast<uint32_t>(BT::WALL_NORTH_END);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_VERTICAL);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_NORTHEAST_DIAGONAL);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_EAST_T);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT] = static_cast<uint32_t>(BT::WALL_NORTHWEST_DIAGONAL);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_WEST_T);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_NORTH_T);
    s_full_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_INTERSECTION);

    s_half_wall_types[0] = static_cast<uint32_t>(BT::WALL_POLE);
    s_half_wall_types[LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_VERTICAL);
    s_half_wall_types[LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_HORIZONTAL);
    s_half_wall_types[LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHEAST_DIAGONAL);
    s_half_wall_types[LOCAL_WALL_E_BIT] = static_cast<uint32_t>(BT::WALL_POLE);
    s_half_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_VERTICAL);
    s_half_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_HORIZONTAL);
    s_half_wall_types[LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHEAST_DIAGONAL);
    s_half_wall_types[LOCAL_WALL_S_BIT] = static_cast<uint32_t>(BT::WALL_POLE);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_VERTICAL);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_HORIZONTAL);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHEAST_DIAGONAL);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT] = static_cast<uint32_t>(BT::WALL_POLE);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_VERTICAL);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT] = static_cast<uint32_t>(BT::WALL_HORIZONTAL);
    s_half_wall_types[LOCAL_WALL_S_BIT | LOCAL_WALL_E_BIT | LOCAL_WALL_W_BIT | LOCAL_WALL_N_BIT] = static_cast<uint32_t>(BT::WALL_SOUTHEAST_DIAGONAL);

    qInfo("WallBrush: Static wall type tables initialized.");
    s_staticDataInitialized = true;
}


QString WallBrush::wallSegmentTypeToOrientationString(RME::BorderType segmentType) const {
    switch (segmentType) {
        case RME::BorderType::WALL_POLE: return QStringLiteral("pole");
        case RME::BorderType::WALL_VERTICAL: return QStringLiteral("vertical");
        case RME::BorderType::WALL_HORIZONTAL: return QStringLiteral("horizontal");
        case RME::BorderType::WALL_SOUTH_END: return QStringLiteral("south_end");
        case RME::BorderType::WALL_EAST_END: return QStringLiteral("east_end");
        case RME::BorderType::WALL_NORTH_END: return QStringLiteral("north_end");
        case RME::BorderType::WALL_WEST_END: return QStringLiteral("west_end");
        case RME::BorderType::WALL_SOUTH_T: return QStringLiteral("south_t");
        case RME::BorderType::WALL_EAST_T: return QStringLiteral("east_t");
        case RME::BorderType::WALL_NORTH_T: return QStringLiteral("north_t");
        case RME::BorderType::WALL_WEST_T: return QStringLiteral("west_t");
        case RME::BorderType::WALL_INTERSECTION: return QStringLiteral("intersection");
        case RME::BorderType::WALL_NORTHWEST_DIAGONAL: return QStringLiteral("northwest_diagonal");
        case RME::BorderType::WALL_NORTHEAST_DIAGONAL: return QStringLiteral("northeast_diagonal");
        case RME::BorderType::WALL_SOUTHWEST_DIAGONAL: return QStringLiteral("southwest_diagonal");
        case RME::BorderType::WALL_SOUTHEAST_DIAGONAL: return QStringLiteral("southeast_diagonal");
        case RME::BorderType::WALL_UNTOUCHABLE: return QStringLiteral("untouchable");
        default:
            qWarning("WallBrush::wallSegmentTypeToOrientationString: Unknown segment type %d", static_cast<int>(segmentType));
            return QStringLiteral("pole");
    }
}

uint16_t WallBrush::getItemIdForSegment(RME::BorderType segmentType,
                                        const RME::core::BrushSettings& settings,
                                        const RME::core::assets::MaterialWallSpecifics* specifics) const {
    if (!specifics) return 0;

    QString orientationStr = wallSegmentTypeToOrientationString(segmentType);
    const RME::core::assets::MaterialWallPart* foundPart = nullptr;
    for (const auto& part : specifics->parts) {
        if (part.orientationType.compare(orientationStr, Qt::CaseInsensitive) == 0) {
            foundPart = &part;
            break;
        }
    }

    if (!foundPart) {
        if (orientationStr.contains(QStringLiteral("diagonal"))) {
            orientationStr = QStringLiteral("corner"); // Try generic "corner" for diagonals
            for (const auto& part : specifics->parts) {
                if (part.orientationType.compare(orientationStr, Qt::CaseInsensitive) == 0) {
                    foundPart = &part;
                    break;
                }
            }
        }
        if (!foundPart) {
            qDebug("WallBrush::getItemIdForSegment: No wall part found for orientation '%s' in material %s",
                     qUtf8Printable(orientationStr), qUtf8Printable(m_materialData->id));
            return 0;
        }
    }

    bool placeDoor = settings.getGenericBrushParameter("placeDoor").toBool();
    bool placeWindow = settings.getGenericBrushParameter("placeWindow").toBool();
    QString doorTypeStr = settings.getGenericBrushParameter("doorType").toString();

    if (placeDoor && !foundPart->doors.isEmpty()) {
        for (const auto& doorDef : foundPart->doors) {
            if (doorTypeStr.isEmpty() || doorDef.doorType.contains(doorTypeStr, Qt::CaseInsensitive)) {
                return doorDef.id;
            }
        }
        qDebug("WallBrush: Place door mode, but no matching door type '%s' found for orientation '%s'. Trying first door.",
                 qUtf8Printable(doorTypeStr), qUtf8Printable(orientationStr));
        if(!foundPart->doors.isEmpty()) return foundPart->doors.first().id;
    } else if (placeWindow && !foundPart->doors.isEmpty()) {
         for (const auto& doorDef : foundPart->doors) {
            if (doorDef.doorType.contains(QStringLiteral("window"), Qt::CaseInsensitive)) {
                return doorDef.id;
            }
        }
        qDebug("WallBrush: Place window mode, but no window type found for orientation '%s'. Trying first door/window.",
                qUtf8Printable(orientationStr));
        if(!foundPart->doors.isEmpty()) return foundPart->doors.first().id;
    }

    if (foundPart->items.isEmpty()) {
        qDebug("WallBrush::getItemIdForSegment: Wall part for orientation '%s' has no solid items.", qUtf8Printable(orientationStr));
        return 0;
    }

    int totalChance = 0;
    for (const auto& entry : foundPart->items) totalChance += entry.chance;
    if (totalChance == 0) return foundPart->items.first().itemId;

    int randomValue = QRandomGenerator::global()->bounded(totalChance);
    int currentChanceSum = 0;
    for (const auto& entry : foundPart->items) {
        currentChanceSum += entry.chance;
        if (randomValue < currentChanceSum) {
            return entry.itemId;
        }
    }
    return foundPart->items.first().itemId;
}


void WallBrush::updateWallAppearance(RME::core::editor::EditorControllerInterface* controller, const RME::core::Position& pos) {
    RME::core::map::Map* map = controller->getMap();
    RME::core::assets::AssetManager* assetManager = controller->getAssetManager();
    const RME::core::BrushSettings& brushSettings = controller->getBrushSettings();

    if (!map || !assetManager || !m_materialData) return;
    const auto* wallSpecifics = getCurrentWallSpecifics();
    if (!wallSpecifics) return;

    const RME::core::Tile* currentTile = map->getTile(pos);
    if (!currentTile) return;

    const RME::core::assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
    if(!itemDb) return;

    RME::core::Item* currentWallItem = nullptr;
    uint16_t oldWallItemId = 0;
    for (const auto& itemPtr : currentTile->getItems()) {
        if (itemPtr) {
            const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
            if (itemData && itemData->isWall && itemData->materialId == m_materialData->id) {
                currentWallItem = itemPtr.get();
                oldWallItemId = currentWallItem->getID();
                break;
            }
        }
    }
    if (!currentWallItem) return;

    uint8_t tiledata = 0;
    // These must match the bit order used in initializeStaticData: N, W, E, S
    static const RME::core::Position cardinalOffsets[] = { {0,-1,0}, {-1,0,0}, {1,0,0}, {0,1,0} };
    static const uint8_t wallBits[] = { LOCAL_WALL_N_BIT, LOCAL_WALL_W_BIT, LOCAL_WALL_E_BIT, LOCAL_WALL_S_BIT };

    for (int i = 0; i < 4; ++i) {
        const RME::core::Tile* neighborTile = map->getTile(pos.translated(cardinalOffsets[i]));
        if (::hasMatchingWallMaterialAtTile(neighborTile, m_materialData, assetManager)) {
            tiledata |= wallBits[i];
        }
    }

    RME::BorderType segmentType = static_cast<RME::BorderType>(s_full_wall_types[tiledata]);
    uint16_t newItemId = getItemIdForSegment(segmentType, brushSettings, wallSpecifics);

    if (newItemId == 0 && segmentType != RME::BorderType::WALL_POLE) {
        segmentType = static_cast<RME::BorderType>(s_half_wall_types[tiledata]);
        newItemId = getItemIdForSegment(segmentType, brushSettings, wallSpecifics);
    }

    if (newItemId != 0 && oldWallItemId != newItemId) {
        qDebug("WallBrush::updateWallAppearance: Tile %s, wall item %u changing to %u (segment: %d, tiledata: 0x%X)",
                 qUtf8Printable(pos.toString()), oldWallItemId, newItemId, static_cast<int>(segmentType), tiledata);
        controller->recordRemoveItem(pos, oldWallItemId);
        controller->recordAddItem(pos, newItemId);
    }
}

void WallBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                        const RME::core::Position& pos,
                        const RME::core::BrushSettings& settings) {
    if (!controller || !canApply(controller->getMap(), pos, settings)) {
        return;
    }
    RME::core::map::Map* map = controller->getMap();
    const auto* wallSpecifics = getCurrentWallSpecifics();
    if(!wallSpecifics) return;

    const RME::core::assets::ItemDatabase* itemDb = controller->getAssetManager() ? controller->getAssetManager()->getItemDatabase() : nullptr;
    if(!itemDb) { qWarning("WallBrush::apply: ItemDatabase not available."); return; }


    if (settings.isEraseMode) {
        const RME::core::Tile* tile = map->getTile(pos);
        if (tile) {
            QList<uint16_t> idsToRemove;
            for (const auto& itemPtr : tile->getItems()) {
                if (itemPtr) {
                     const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
                     if(itemData && itemData->isWall && itemData->materialId == m_materialData->id){
                       idsToRemove.append(itemPtr->getID());
                     }
                }
            }
            for(uint16_t id_to_remove : idsToRemove){
                controller->recordRemoveItem(pos, id_to_remove);
                qDebug("WallBrush::apply: Erasing wall item %u at %s", id_to_remove, qUtf8Printable(pos.toString()));
            }
        }
    } else {
        const RME::core::Tile* tile = map->getTile(pos);
        if (tile) {
             QList<uint16_t> idsToRemove;
             for (const auto& itemPtr : tile->getItems()) {
                if (itemPtr) {
                    const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
                     if(itemData && itemData->isWall && itemData->materialId == m_materialData->id){
                        idsToRemove.append(itemPtr->getID());
                     }
                }
            }
            for(uint16_t id_to_remove : idsToRemove){
                controller->recordRemoveItem(pos, id_to_remove);
            }
        }

        uint16_t initialItemId = getItemIdForSegment(RME::BorderType::WALL_POLE, settings, wallSpecifics);
        if (initialItemId != 0) {
            controller->recordAddItem(pos, initialItemId);
        } else {
            qWarning("WallBrush::apply: No item ID found for default WALL_POLE for material %s.", qUtf8Printable(m_materialData->id));
        }
    }

    updateWallAppearance(controller, pos);
    static const RME::core::Position cardinalOffsets[] = { {0,-1,0}, {-1,0,0}, {1,0,0}, {0,1,0} }; // N, W, E, S
    for (const auto& offset : cardinalOffsets) {
        RME::core::Position neighborPos = pos.translated(offset);
        if (map->isPositionValid(neighborPos)) {
            updateWallAppearance(controller, neighborPos);
        }
    }

    controller->notifyTileChanged(pos);
     for (const auto& offset : cardinalOffsets) {
        RME::core::Position neighborPos = pos.translated(offset);
        if (map->isPositionValid(neighborPos)) {
            controller->notifyTileChanged(neighborPos);
        }
    }
}

} // namespace core
} // namespace RME
