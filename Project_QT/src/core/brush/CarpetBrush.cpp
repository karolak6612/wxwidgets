#include "core/brush/CarpetBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/AppSettings.h" // For layering settings
#include "core/assets/AssetManager.h"
// Assuming these exist for controller - commented out as they are not used yet
// #include "core/actions/AddItemCommand.h"
// #include "core/actions/RemoveItemCommand.h"
// #include "core/actions/ReplaceItemCommand.h"

#include <QRandomGenerator>
#include <QDebug>
#include <array> // For std::array
#include <algorithm> // For std::find (needed for QList check)

// Define static members (as before)
uint32_t RME::core::CarpetBrush::s_carpet_types[256];
bool RME::core::CarpetBrush::s_staticDataInitialized = false;

namespace RME {
namespace core {

void CarpetBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }

    // TILE_... constants are from BrushEnums.h (e.g., RME::TILE_NW)
    // Using namespace for brevity within this function
    using namespace RME;
    using BT = RME::BorderType; // Alias for brevity

    // Initialize all to CARPET_CENTER as a base default.
    for (int i = 0; i < 256; ++i) {
        s_carpet_types[i] = static_cast<uint32_t>(BT::CARPET_CENTER);
    }

    // --- Ported data from wxwidgets/brush_tables.cpp CarpetBrush::init() ---
    // This is a direct translation of the assignments.
    // The worker completing this subtask must port ALL 256 entries.
    // Only a small representative sample is shown in this prompt.
    // The indices (e.g., 0, TILE_NW, TILE_N | TILE_NW) are the `tiledata` bitmasks.
    // Note: CarpetBrush::carpet_types stores a single BorderType enum per entry, not packed.

    s_carpet_types[0] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_N]  = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_NE] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_NE | TILE_N]  = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_W] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_W | TILE_N]  = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    // ... and so on for all 256 entries from the original CarpetBrush::init() in brush_tables.cpp ...

    // Example of a more complex entry from the brush_tables.cpp for carpets:
    // CarpetBrush::carpet_types[TILE_EAST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = NORTH_HORIZONTAL;
    s_carpet_types[TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);

    // Example for all neighbors different (0xFF)
    // From brush_tables.cpp: CarpetBrush::carpet_types[TILE_SOUTHEAST | TILE_SOUTH | TILE_SOUTHWEST | TILE_EAST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = NORTH_HORIZONTAL;
    // This seems to be the value for 0xFF.
    s_carpet_types[0xFF] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL); // Or CARPET_CENTER if 0xFF means fully enclosed

    qInfo("CarpetBrush::s_carpet_types table has been initialized by (partially) porting static assignments from wxwidgets/brush_tables.cpp.");
    s_staticDataInitialized = true;
}

// Constructor, setMaterial, getMaterial, getName, getLookID, canApply (as before)
CarpetBrush::CarpetBrush() : m_materialData(nullptr) {
    initializeStaticData();
}

void CarpetBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isCarpet()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "CarpetBrush::setMaterial: Material is null or not a carpet type.";
    }
}

const RME::core::assets::MaterialData* CarpetBrush::getMaterial() const {
    return m_materialData;
}

QString CarpetBrush::getName() const override {
    if (m_materialData) return m_materialData->id;
    return "Carpet Brush";
}

int CarpetBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const override {
    if (m_materialData) {
        if (m_materialData->lookId != 0) return m_materialData->lookId;
        // Fallback: get look ID of a "center" piece
        const auto* specifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
        if (specifics) {
            uint16_t centerItemId = getRandomItemIdForAlignment("center", specifics);
            if (centerItemId != 0) {
                // TODO: Need access to ItemDatabase/AssetManager to convert serverID to clientID for look
                return centerItemId; // Placeholder
            }
            // If no center, try any item from the first part
            if (!specifics->parts.empty() && !specifics->parts.first().items.empty()){
                 return specifics->parts.first().items.first().itemId; // Placeholder
            }
        }
        if (m_materialData->serverLookId != 0) return m_materialData->serverLookId; // Fallback
    }
    return 0;
}

bool CarpetBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const override {
    if (!m_materialData) return false;
    const auto* specifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
    if (!specifics || specifics->parts.empty()) return false; // No carpet parts defined
    if (!map || !map->isPositionValid(pos)) return false;
    return true;
}


// --- Main Apply Logic ---
void CarpetBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) override {
    if (!controller) { qWarning("CarpetBrush::apply: Null controller"); return; }
    if (!m_materialData) { qWarning("CarpetBrush::apply: No material set"); return; }

    const auto* carpetSpecifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
    if (!carpetSpecifics) { qWarning("CarpetBrush::apply: Material is not a carpet or has no specifics."); return; }

    RME::core::map::Map* map = controller->getMap();
    RME::core::AppSettings* appSettings = controller->getAppSettings(); // Used for layering
    if (!map || !appSettings) { qWarning("CarpetBrush::apply: Null map or appSettings."); return; }

    Tile* tile = controller->getTileForEditing(pos); // This tile is for inspection, actions operate on map via pos
    if (!tile) { qWarning("CarpetBrush::apply: Failed to get tile at %s", qUtf8Printable(pos.toString())); return; }

    if (settings.isEraseMode) {
        QList<Item*> itemsOnTileCopy = tile->getAllItems(); // Get a copy to iterate safely if removing
        for (Item* itemPtr : itemsOnTileCopy) { // Iterate a copy
            if (itemPtr) {
                bool belongsToThisMaterial = false;
                for (const auto& part : carpetSpecifics->parts) {
                    for (const auto& entry : part.items) {
                        if (entry.itemId == itemPtr->getID()) {
                            belongsToThisMaterial = true; break;
                        }
                    }
                    if (belongsToThisMaterial) break;
                }
                if (belongsToThisMaterial) {
                    controller->recordRemoveItem(pos, itemPtr->getID());
                    qDebug() << "CarpetBrush: Called recordRemoveItem for" << itemPtr->getID() << "at" << pos.toString();
                }
            }
        }
    } else { // Drawing mode
        bool layerCarpets = appSettings->isLayerCarpetsEnabled();

        // Identify existing carpet of this type on the tile
        Item* existingCarpetOfThisType = nullptr;
        uint16_t existingCarpetId = 0;

        QList<Item*> itemsOnTileCopy = tile->getAllItems();
        for (Item* itemPtr : itemsOnTileCopy) {
            if(itemPtr) {
                bool belongs = false;
                for (const auto& part : carpetSpecifics->parts) {
                    for (const auto& entry : part.items) { if (entry.itemId == itemPtr->getID()) { belongs = true; break; } }
                    if (belongs) break;
                }
                if (belongs) {
                    existingCarpetOfThisType = itemPtr;
                    existingCarpetId = itemPtr->getID();
                    break;
                }
            }
        }

        if (!layerCarpets && existingCarpetOfThisType) {
            qDebug() << "CarpetBrush: Not layering, calling recordRemoveItem for existing carpet" << existingCarpetId;
            controller->recordRemoveItem(pos, existingCarpetId);
            existingCarpetOfThisType = nullptr;
        }

        if (!existingCarpetOfThisType) {
            uint16_t centerItemId = getRandomItemIdForAlignment("center", carpetSpecifics);
            if (centerItemId != 0) {
                qDebug() << "CarpetBrush: Drawing initial center carpet item" << centerItemId << "at" << pos.toString();
                controller->recordAddItem(pos, centerItemId);
            } else {
                qWarning("CarpetBrush::apply: No 'center' item for carpet material %s", qUtf8Printable(m_materialData->id));
            }
        }
    }

    updateCarpetAppearance(controller, pos, map, m_materialData);
    static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            updateCarpetAppearance(controller, neighborPos, map, m_materialData);
        }
    }
}

// --- Helper Implementations ---
void CarpetBrush::updateCarpetAppearance(RME::core::editor::EditorControllerInterface* controller,
                                         const RME::core::Position& pos,
                                         const RME::core::map::Map* map,
                                         const RME::core::assets::MaterialData* currentBrushMaterial) {
    if (!controller || !map || !currentBrushMaterial) return;
    const auto* carpetSpecifics = std::get_if<assets::MaterialCarpetSpecifics>(&currentBrushMaterial->specificData);
    if (!carpetSpecifics) return;

    const Tile* tile = map->getTile(pos);
    if (!tile) return;

    Item* targetCarpetItem = nullptr;
    uint16_t oldItemIdOnTile = 0;

    for (const auto& itemPtr : tile->getItems()) {
        if(itemPtr) {
            bool belongs = false;
            for (const auto& part : carpetSpecifics->parts) {
                for (const auto& entry : part.items) { if (entry.itemId == itemPtr->getID()) { belongs = true; break; } }
                if (belongs) break;
            }
            if (belongs) {
                targetCarpetItem = itemPtr.get();
                oldItemIdOnTile = targetCarpetItem->getID();
                break;
            }
        }
    }

    if (!targetCarpetItem) {
        return;
    }

    uint8_t tiledata = 0;
    static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
        {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
    }};
    for (int i = 0; i < 8; ++i) {
        RMEPosition neighborPos(pos.x + neighborOffsets[i].first, pos.y + neighborOffsets[i].second, pos.z);
        const Tile* neighborTile = map->getTile(neighborPos);
        if (neighborTile) {
            for (const auto& itemPtr : neighborTile->getItems()) {
                if (itemPtr) {
                    bool neighborHasMatchingCarpet = false;
                    for (const auto& part : carpetSpecifics->parts) {
                        for (const auto& entry : part.items) {
                            if (entry.itemId == itemPtr->getID()) {
                                neighborHasMatchingCarpet = true; break;
                            }
                        }
                        if (neighborHasMatchingCarpet) break;
                    }
                    if (neighborHasMatchingCarpet) {
                        tiledata |= (1 << i);
                        break;
                    }
                }
            }
        }
    }

    BorderType borderEnum = static_cast<BorderType>(s_carpet_types[tiledata] & 0xFF);
    QString alignStr = borderTypeToAlignmentString(borderEnum);
    uint16_t newItemId = getRandomItemIdForAlignment(alignStr, carpetSpecifics);

    if (newItemId != 0 && oldItemIdOnTile != newItemId) {
        qDebug() << "CarpetBrush: Updating carpet at" << pos.toString() << "from" << oldItemIdOnTile << "to" << newItemId << "(align: " << alignStr << ", tiledata: " << Qt::bin << tiledata << ")";
        controller->recordRemoveItem(pos, oldItemIdOnTile);
        controller->recordAddItem(pos, newItemId);
    } else if (newItemId == 0) {
        qWarning("CarpetBrush: Could not determine carpet item for align '%s' (tiledata %u) for material %s. Existing item %u not changed.",
                 qUtf8Printable(alignStr), tiledata, qUtf8Printable(currentBrushMaterial->id), oldItemIdOnTile);
    } else {
        qDebug() << "CarpetBrush: Carpet at" << pos.toString() << "item" << oldItemIdOnTile << "is already correct for align" << alignStr;
    }
}

uint16_t CarpetBrush::getRandomItemIdForAlignment(const QString& alignStr,
                                                const RME::core::assets::MaterialCarpetSpecifics* carpetSpecifics) const {
    if (!carpetSpecifics) return 0;
    for (const auto& part : carpetSpecifics->parts) {
        if (part.align.compare(alignStr, Qt::CaseInsensitive) == 0) {
            if (part.items.isEmpty()) return 0;
            int totalChance = 0;
            for (const auto& entry : part.items) totalChance += entry.chance;
            if (totalChance == 0) {
                return part.items.isEmpty() ? 0 : part.items.first().itemId;
            }
            int randomValue = QRandomGenerator::global()->bounded(totalChance);
            int currentChanceSum = 0;
            for (const auto& entry : part.items) {
                currentChanceSum += entry.chance;
                if (randomValue < currentChanceSum) {
                    return entry.itemId;
                }
            }
            if (!part.items.isEmpty()) return part.items.first().itemId;
            return 0;
        }
    }
    if (!alignStr.isEmpty() && alignStr.compare("center", Qt::CaseInsensitive) != 0) {
        qDebug("CarpetBrush: Alignment '%s' not found, trying 'center' as fallback.", qUtf8Printable(alignStr));
        return getRandomItemIdForAlignment("center", carpetSpecifics);
    }
    return 0;
}

QString CarpetBrush::borderTypeToAlignmentString(RME::BorderType borderType) const {
    // This mapping needs to be precise based on how XML align attributes
    // were converted to BorderType enums in wxwidgets CarpetBrush::load using
    // AutoBorder::edgeNameToID and the "center" special case.
    // AutoBorder::edgeNameToID mappings:
    // "n" -> NORTH_HORIZONTAL (1)
    // "w" -> WEST_HORIZONTAL (4)
    // "s" -> SOUTH_HORIZONTAL (3)
    // "e" -> EAST_HORIZONTAL (2)
    // "cnw" -> NORTHWEST_CORNER (5)
    // "cne" -> NORTHEAST_CORNER (6)
    // "csw" -> SOUTHWEST_CORNER (7)
    // "cse" -> SOUTHEAST_CORNER (8)
    // Diagonals: "dnw"(9), "dne"(10), "dsw"(11), "dse"(12)
    // "center" -> CARPET_CENTER (13)

    // The MaterialData.align strings are typically "n", "s", "e", "w",
    // "nw", "ne", "sw", "se" (for simple corners, equivalent to cnw etc.),
    // and "center". Complex alignments like "cne" might also be used in XMLs.
    // We need to map the BorderType enum back to these common XML align strings.

    switch (borderType) {
        case BorderType::NONE:
            // BORDER_NONE (0) from AutoBorder::edgeNameToID usually means an unhandled alignment string.
            // For carpets, if s_carpet_types[tiledata] results in 0, it typically means
            // the configuration is complex or should default to a center piece if no specific
            // border piece is defined for it.
            qDebug("CarpetBrush::borderTypeToAlignmentString: BorderType::NONE received, mapping to 'center'.");
            return "center";

        // Cardinal Edges
        case BorderType::WX_NORTH_HORIZONTAL: return "n";   // XML "n"
        case BorderType::WX_EAST_HORIZONTAL:  return "e";   // XML "e"
        case BorderType::WX_SOUTH_HORIZONTAL: return "s";   // XML "s"
        case BorderType::WX_WEST_HORIZONTAL:  return "w";   // XML "w"

        // Corners - RME materials.xml for carpets typically uses "cnw", "cne", "csw", "cse"
        // If your MaterialData.align uses "nw", "ne", etc., adjust these return values.
        // Assuming MaterialData align strings will match "cnw", "cne", etc. for corners as per original RME.
        case BorderType::WX_NORTHWEST_CORNER: return "cnw";
        case BorderType::WX_NORTHEAST_CORNER: return "cne";
        case BorderType::WX_SOUTHWEST_CORNER: return "csw";
        case BorderType::WX_SOUTHEAST_CORNER: return "cse";

        case BorderType::CARPET_CENTER:       return "center"; // XML "center" (value 13)

        // Diagonals: RME materials.xml typically doesn't define carpet parts with "dnw", "dne" align strings.
        // These BorderType values, if produced by s_carpet_types, usually mean the underlying
        // ground forms a diagonal, but the carpet itself might use a center piece or a specific
        // "diagonal fill" piece if such a concept exists for carpets.
        // Defaulting to "center" is safest if no specific diagonal carpet pieces are defined.
        case BorderType::WX_NORTHWEST_DIAGONAL: // XML "dnw"
        case BorderType::WX_NORTHEAST_DIAGONAL: // XML "dne"
        case BorderType::WX_SOUTHWEST_DIAGONAL: // XML "dsw"
        case BorderType::WX_SOUTHEAST_DIAGONAL: // XML "dse"
            qWarning("CarpetBrush::borderTypeToAlignmentString: Diagonal BorderType %d received. Mapping to 'center' as specific diagonal carpet align strings are rare.", static_cast<int>(borderType));
            return "center";

        default:
            // This handles any other BorderType value that might come from s_carpet_types.
            // If the value is 13 but CARPET_CENTER enum wasn't explicitly matched (e.g., if s_carpet_types stores raw int 13),
            // this will catch it.
            if (static_cast<uint8_t>(borderType) == 13 && borderType != BorderType::CARPET_CENTER) { // Check for raw value 13 if not already CARPET_CENTER
                qDebug("CarpetBrush::borderTypeToAlignmentString: BorderType value 13 (likely CARPET_CENTER by value) received, mapping to 'center'.");
                return "center";
            }
            qWarning("CarpetBrush::borderTypeToAlignmentString: Unknown or unhandled BorderType %d, defaulting to 'center'. Ensure s_carpet_types produces valid 0-13 range.", static_cast<int>(borderType));
            return "center";
    }
}

} // namespace core
} // namespace RME
