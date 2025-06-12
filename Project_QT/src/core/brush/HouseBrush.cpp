#include "core/brush/HouseBrush.h"
#include "core/map/Map.h"
#include "core/brush/BrushSettings.h"
#include "core/editor_logic/EditorControllerInterface.h"
#include "core/Tile.h" // For RME::TileMapFlag
#include "core/Position.h"
#include <QString>
#include <QDebug>
#include <QObject> // For QObject::tr

namespace RME {
namespace core {
namespace brush {

HouseBrush::HouseBrush() : Brush(), m_currentHouseId(0) {
}

void HouseBrush::setCurrentHouseId(uint32_t houseId) {
    m_currentHouseId = houseId;
}

QString HouseBrush::getName() const {
    if (m_currentHouseId == 0) {
        return QObject::tr("House Brush (Eraser)");
    }
    return QObject::tr("House Brush (ID: %1)").arg(m_currentHouseId);
}

int HouseBrush::getLookID(const BrushSettings& /*settings*/) const {
    return EDITOR_SPRITE_HOUSE_BRUSH_LOOKID;
}

bool HouseBrush::canApply(const map::Map* map, const Position& pos, const BrushSettings& /*settings*/) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    return true;
}

void HouseBrush::apply(editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) {
    if (!controller) {
        qWarning("HouseBrush::apply: EditorControllerInterface is null for position (%d,%d,%d).", pos.x, pos.y, pos.z);
        return;
    }

    // TODO: Get these from AppSettings via REFACTOR-01 (passed through settings or controller context)
    bool setting_autoAssignDoorId = false;
    bool setting_houseBrushRemoveItems = false;
    // qInfo("HouseBrush::apply: 'AUTO_ASSIGN_DOORID' is currently hardcoded to %s for pos (%d,%d,%d).",
    //       setting_autoAssignDoorId ? "true" : "false", pos.x, pos.y, pos.z);
    // qInfo("HouseBrush::apply: 'HOUSE_BRUSH_REMOVE_ITEMS' is currently hardcoded to %s for pos (%d,%d,%d).",
    //       setting_houseBrushRemoveItems ? "true" : "false", pos.x, pos.y, pos.z);

    uint32_t currentTileHouseId = controller->getTileHouseId(pos); // Conceptual: Get current house ID of the tile

    if (settings.isEraseMode()) { // Erasing house assignment
        qInfo("HouseBrush::apply (Erase Mode) for tile at (%d,%d,%d)", pos.x, pos.y, pos.z);
        if (currentTileHouseId != 0) { // Only act if the tile actually belongs to a house
            controller->setTileHouseId(pos, 0); // Conceptual
            controller->setTileMapFlag(pos, RME::TileMapFlag::PROTECTION_ZONE, false); // Clear PZ (RME::TileMapFlag)

            if (setting_autoAssignDoorId) {
                // controller->clearDoorIdsOnTile(pos); // Conceptual
                qWarning("HouseBrush::apply (Erase): AUTO_ASSIGN_DOORID logic is TODO for tile (%d,%d,%d).", pos.x, pos.y, pos.z);
            }
            controller->removeTilePositionFromHouse(currentTileHouseId, pos); // Conceptual
        }
    } else { // Assigning house
        if (m_currentHouseId == 0) {
            qWarning("HouseBrush::apply (Draw Mode): No current house ID set (or trying to assign ID 0) for tile (%d,%d,%d). Brush should be configured with a valid house ID.", pos.x, pos.y, pos.z);
            return;
        }
        qInfo("HouseBrush::apply (Draw Mode) house ID %d to tile at (%d,%d,%d)", m_currentHouseId, pos.x, pos.y, pos.z);

        // If tile already belongs to a different house, remove it from old house first
        if (currentTileHouseId != 0 && currentTileHouseId != m_currentHouseId) {
            controller->removeTilePositionFromHouse(currentTileHouseId, pos); // Conceptual
        }

        // Assign to new house if not already assigned to this house
        if (currentTileHouseId != m_currentHouseId) {
            controller->setTileHouseId(pos, m_currentHouseId); // Conceptual
            controller->addTilePositionToHouse(m_currentHouseId, pos); // Conceptual
        }
        controller->setTileMapFlag(pos, RME::TileMapFlag::PROTECTION_ZONE, true); // Set PZ (RME::TileMapFlag)

        if (setting_houseBrushRemoveItems) {
            // controller->removeMovablesFromTile(pos); // Conceptual
            qWarning("HouseBrush::apply (Draw): HOUSE_BRUSH_REMOVE_ITEMS logic is TODO for tile (%d,%d,%d).", pos.x, pos.y, pos.z);
        }
        if (setting_autoAssignDoorId) {
            // controller->assignHouseDoorIdToTileDoors(pos, m_currentHouseId, currentTileHouseId); // Conceptual
            qWarning("HouseBrush::apply (Draw): AUTO_ASSIGN_DOORID logic is TODO for tile (%d,%d,%d).", pos.x, pos.y, pos.z);
        }
    }
    // Border updates are handled by the EditorController if brush->needsBorders() is true.
    // HouseBrush::needsBorders() currently returns false (default from base Brush).
}

} // namespace brush
} // namespace core
} // namespace RME
