#include "core/brush/TableBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/AppSettings.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/ItemData.h"
#include "core/assets/MaterialManager.h" // For getMaterialFromTile in helpers, if used

#include <QRandomGenerator>
#include <QDebug>
#include <array>     // For neighbor offset array
#include <algorithm> // For std::sort

// Static member definitions
uint32_t RME::core::TableBrush::s_table_types[256];
bool RME::core::TableBrush::s_staticDataInitialized = false;

namespace RME {
namespace core {

TableBrush::TableBrush() : m_materialData(nullptr) {
    initializeStaticData();
}

void TableBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isTable()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "TableBrush::setMaterial: Material is null or not a table type.";
    }
}

const RME::core::assets::MaterialData* TableBrush::getMaterial() const {
    return m_materialData;
}

const RME::core::assets::MaterialTableSpecifics* TableBrush::getCurrentTableSpecifics() const {
    if (m_materialData && m_materialData->isTable()) {
        return std::get_if<RME::core::assets::MaterialTableSpecifics>(&m_materialData->specificData);
    }
    return nullptr;
}

QString TableBrush::getName() const {
    if (m_materialData) {
        return m_materialData->id;
    }
    return QStringLiteral("Table Brush");
}

int TableBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) {
        return 0;
    }

    if (m_materialData->lookId != 0) { // lookId is assumed to be client ID
        return m_materialData->lookId;
    }

    const auto* specifics = getCurrentTableSpecifics();
    if (specifics) {
        QString defaultAlignStr = tableSegmentTypeToAlignString(RME::BorderType::TABLE_ALONE);
        uint16_t serverItemId = getRandomItemIdForAlignString(defaultAlignStr, specifics);
        if (serverItemId != 0) {
            qWarning() << "TableBrush 'getLookID': Material" << m_materialData->id << "has no client lookId. Attempting to use server ID" << serverItemId << "from" << defaultAlignStr << "segment. THIS REQUIRES CONVERSION by MaterialManager or caller.";
            return 0;
        }
    }

    if (m_materialData->serverLookId != 0) {
         qWarning() << "TableBrush 'getLookID': Material" << m_materialData->id << "has serverLookId" << m_materialData->serverLookId << "but no client lookId. THIS REQUIRES CONVERSION by MaterialManager or caller.";
        return 0;
    }

    qWarning() << "TableBrush 'getLookID': Material" << m_materialData->id << "has no lookId, serverLookId, or default items to derive a look from.";
    return 0;
}

bool TableBrush::canApply(const RME::core::map::Map* map,
                            const RME::core::Position& pos,
                            const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) return false;
    const auto* specifics = getCurrentTableSpecifics();
    if (!specifics || specifics->parts.empty()) {
        qWarning() << "TableBrush::canApply: No table parts defined for material" << m_materialData->id;
        return false;
    }
    if (!map || !map->isPositionValid(pos)) return false;
    return true;
}

void TableBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }

    // Use specific namespace qualifiers for better code clarity
    using RME::core::TILE_TABLE_NORTH;
    using RME::core::TILE_TABLE_SOUTH;
    using RME::core::TILE_TABLE_EAST;
    using RME::core::TILE_TABLE_WEST;
    using BT = RME::BorderType;

    for (int i = 0; i < 256; ++i) {
        s_table_types[i] = static_cast<uint32_t>(BT::TABLE_ALONE);
    }

    s_table_types[0] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_N] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_W] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_E] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_N] = static_cast<uint32_t>(BT::TABLE_VERTICAL);
    s_table_types[TILE_S | TILE_W] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_E] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);

    s_table_types[TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_NE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_N] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SW | TILE_W] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SW | TILE_E] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_NW] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_NE] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_NORTH_END);
    s_table_types[TILE_S | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_W | TILE_NE | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_N] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_S | TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SW | TILE_WEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SW | TILE_EAST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_SE | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_WEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_WEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_EAST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_EAST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTHEAST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTHEAST | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_WEST | TILE_NORTHEAST | TILE_NORTH | TILE_NORTHWEST] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_EAST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_SE | TILE_SOUTHWEST | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_N] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_S | TILE_SE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_NE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_W] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_EAST] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_N] = static_cast<uint32_t>(BT::TABLE_SOUTH_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_ALONE);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_EAST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_WEST_END);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);
    s_table_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::TABLE_HORIZONTAL);


    qInfo("TableBrush::s_table_types table has been initialized by porting static assignments from original wxWidgets brush_tables.cpp.");
    s_staticDataInitialized = true;
}

QString TableBrush::tableSegmentTypeToAlignString(RME::BorderType segmentType) const {
    switch (segmentType) {
        case RME::BorderType::TABLE_ALONE: return QStringLiteral("alone");
        case RME::BorderType::TABLE_VERTICAL: return QStringLiteral("vertical");
        case RME::BorderType::TABLE_HORIZONTAL: return QStringLiteral("horizontal");
        case RME::BorderType::TABLE_SOUTH_END: return QStringLiteral("south");
        case RME::BorderType::TABLE_EAST_END: return QStringLiteral("east");
        case RME::BorderType::TABLE_NORTH_END: return QStringLiteral("north");
        case RME::BorderType::TABLE_WEST_END: return QStringLiteral("west");
        default:
            qWarning() << "TableBrush::tableSegmentTypeToAlignString: Unknown table segment type" << static_cast<int>(segmentType);
            return QStringLiteral("alone");
    }
}

uint16_t TableBrush::getRandomItemIdForAlignString(const QString& alignStr,
                                                 const RME::core::assets::MaterialTableSpecifics* specifics) const {
    if (!specifics) return 0;

    for (const auto& part : specifics->parts) {
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
            return part.items.isEmpty() ? 0 : part.items.first().itemId;
        }
    }

    if (alignStr.compare(QStringLiteral("alone"), Qt::CaseInsensitive) != 0) {
        qDebug("TableBrush::getRandomItemIdForAlignString: Alignment '%s' not found, trying 'alone'.", qUtf8Printable(alignStr));
        return getRandomItemIdForAlignString(QStringLiteral("alone"), specifics);
    }

    return 0;
}

void TableBrush::updateTableAppearance(RME::core::editor::EditorControllerInterface* controller, const RME::core::Position& pos) {
    RME::core::map::Map* map = controller->getMap();
    RME::core::assets::AssetManager* assetManager = controller->getAssetManager();
    if (!map || !assetManager || !m_materialData) return;

    const RME::core::assets::ItemDatabase* itemDb = assetManager->getItemDatabase();
    if(!itemDb) return;

    const RME::core::Tile* currentTile = map->getTile(pos);
    if (!currentTile) return;

    const auto* tableSpecifics = getCurrentTableSpecifics();
    if (!tableSpecifics) return;

    QList<RME::core::Item*> itemsToUpdate;
    for(const auto& itemPtr : currentTile->getItems()){
        if(itemPtr){
            const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
            if(itemData && itemData->materialId == m_materialData->id && m_materialData->isTable()){
                 itemsToUpdate.append(itemPtr.get());
            }
        }
    }

    if(itemsToUpdate.isEmpty()) return;


    uint8_t tiledata = 0;
    static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
        {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
    }};

    for (int i = 0; i < 8; ++i) {
        RME::core::Position neighborPos(pos.x + neighborOffsets[i].first, pos.y + neighborOffsets[i].second, pos.z);
        const RME::core::Tile* neighborTile = map->getTile(neighborPos);
        if (neighborTile) {
            for (const auto& itemPtr : neighborTile->getItems()) {
                if (itemPtr) {
                    const RME::core::assets::ItemData* neighborItemData = itemDb->getItemData(itemPtr->getID());
                    if (neighborItemData && neighborItemData->materialId == m_materialData->id) {
                        tiledata |= (1 << i);
                        break;
                    }
                }
            }
        }
    }

    RME::BorderType segmentType = static_cast<RME::BorderType>(s_table_types[tiledata]);
    QString alignStr = tableSegmentTypeToAlignString(segmentType);
    uint16_t newItemId = getRandomItemIdForAlignString(alignStr, tableSpecifics);

    if (newItemId == 0) {
        qWarning() << "TableBrush::updateTableAppearance: No item ID found for align" << alignStr << "(tiledata 0x" << QString::number(tiledata, 16) << ") for material" << m_materialData->id << "on tile" << pos.toString();
        return;
    }

    for(RME::core::Item* itemToUpdate : itemsToUpdate){
        if (itemToUpdate->getID() != newItemId) {
            qDebug("TableBrush::updateTableAppearance: Tile %s, table item %u changing to %u (align: %s, tiledata: 0x%X)",
                     qUtf8Printable(pos.toString()), itemToUpdate->getID(), newItemId, qUtf8Printable(alignStr), tiledata);
            uint16_t oldId = itemToUpdate->getID();
            controller->recordRemoveItem(pos, oldId);
            controller->recordAddItem(pos, newItemId);
            break;
        }
    }
}


void TableBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                         const RME::core::Position& pos,
                         const RME::core::BrushSettings& settings) {
    if (!controller || !canApply(controller->getMap(), pos, settings)) {
        return;
    }
    RME::core::map::Map* map = controller->getMap();

    const auto* tableSpecifics = getCurrentTableSpecifics();
    if (!tableSpecifics) return;

    const RME::core::assets::ItemDatabase* itemDb = controller->getAssetManager() ? controller->getAssetManager()->getItemDatabase() : nullptr;
    if(!itemDb) {
        qWarning() << "TableBrush::apply: ItemDatabase not available.";
        return;
    }


    if (settings.isEraseMode) {
        const RME::core::Tile* tile = map->getTile(pos);
        if (tile) {
            QList<uint16_t> idsToRemove;
            for (const auto& itemPtr : tile->getItems()) {
                if (itemPtr) {
                     const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
                     if(itemData && itemData->materialId == m_materialData->id && m_materialData->isTable()){
                        idsToRemove.append(itemPtr->getID());
                     }
                }
            }
            for(uint16_t id_to_remove : idsToRemove){
                controller->recordRemoveItem(pos, id_to_remove);
                qDebug("TableBrush::apply: Erasing table item %u at %s", id_to_remove, qUtf8Printable(pos.toString()));
            }
        }
    } else {
        const RME::core::Tile* tile = map->getTile(pos);
        if (tile) {
            QList<uint16_t> idsToRemove;
             for (const auto& itemPtr : tile->getItems()) {
                if (itemPtr) {
                     const RME::core::assets::ItemData* itemData = itemDb->getItemData(itemPtr->getID());
                     if(itemData && itemData->materialId == m_materialData->id && m_materialData->isTable()){
                        idsToRemove.append(itemPtr->getID());
                     }
                }
            }
            for(uint16_t id_to_remove : idsToRemove){
                controller->recordRemoveItem(pos, id_to_remove);
            }
        }

        QString defaultAlignStr = tableSegmentTypeToAlignString(RME::BorderType::TABLE_ALONE);
        uint16_t initialItemId = getRandomItemIdForAlignString(defaultAlignStr, tableSpecifics);

        if (initialItemId != 0) {
            controller->recordAddItem(pos, initialItemId);
            qDebug("TableBrush::apply: Drawing initial table item %u (align: %s) at %s", initialItemId, qUtf8Printable(defaultAlignStr), qUtf8Printable(pos.toString()));
        } else {
            qWarning() << "TableBrush::apply: No item ID found for default alignment" << defaultAlignStr << "for material" << m_materialData->id;
            return;
        }
    }

    updateTableAppearance(controller, pos);

    static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    for (int i = 0; i < 8; ++i) {
        RME::core::Position neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            updateTableAppearance(controller, neighborPos);
        }
    }

    controller->notifyTileChanged(pos);
    for (int i = 0; i < 8; ++i) {
        RME::core::Position neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
        if (map->isPositionValid(neighborPos)) {
            controller->notifyTileChanged(neighborPos);
        }
    }
}

} // namespace core
} // namespace RME
