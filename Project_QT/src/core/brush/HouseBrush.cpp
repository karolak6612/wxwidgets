#include "core/brush/HouseBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/houses/Houses.h" // For RME::core::houses::HouseData
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h" // For settings like remove items/door IDs
#include "core/items/DoorItem.h" // For door ID management
#include "editor_logic/commands/SetHouseTileCommand.h"

#include <QDebug> // For qWarning, qDebug
#include <memory>   // For std::make_unique if commands were owned here

// Placeholder for a house brush sprite ID, replace with actual if available
const int EDITOR_SPRITE_HOUSE_BRUSH_LOOK_ID = 0; // Or some defined constant

namespace RME {
namespace core {

HouseBrush::HouseBrush()
    : m_currentHouseId(0) // Default to no house selected
{
}

void HouseBrush::setCurrentHouseId(quint32 houseId) {
    m_currentHouseId = houseId;
}

quint32 HouseBrush::getCurrentHouseId() const {
    return m_currentHouseId;
}

QString HouseBrush::getName() const {
    if (m_currentHouseId == 0) {
        return QStringLiteral("House Brush (Generic Erase / No House Selected)");
    }
    return QStringLiteral("House Brush (ID: %1)").arg(m_currentHouseId);
}

int HouseBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    return EDITOR_SPRITE_HOUSE_BRUSH_LOOK_ID;
}

bool HouseBrush::canApply(const RME::core::map::Map* map,
                            const RME::core::Position& pos,
                            const RME::core::BrushSettings& settings) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    // If not erasing, a house ID must be selected.
    if (!settings.isEraseMode && m_currentHouseId == 0) {
        // This check is more for UI enabling/disabling the brush tool.
        // The apply() method will also check this and return if no ID is set for drawing.
        return false;
    }
    // Tile must exist to apply house brush (cannot create tiles with house brush)
    if (!map->getTile(pos)) { // Use const getTile
        return false;
    }
    return true;
}

void HouseBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                         const RME::core::Position& pos,
                         const RME::core::BrushSettings& settings) {

    if (!controller || !controller->getMap() || !controller->getHousesManager()) {
        qWarning("HouseBrush::apply: Controller, Map, or HousesManager is null.");
        return;
    }

    Map* map = controller->getMap();
    // Re-check canApply with live map, although it's mostly position validation here
    if (!canApply(map, pos, settings)) {
        qDebug() << "HouseBrush::apply: Preconditions not met at" << pos.toString();
        return;
    }

    Tile* tile = map->getTileForEditing(pos);
    if (!tile) {
        qWarning() << "HouseBrush::apply: Tile not found at position" << pos.toString() << ", though canApply passed (should check getTile).";
        return;
    }

    bool assignToHouse = !settings.isEraseMode;
    RME::core::houses::Houses* housesManager = controller->getHousesManager();

    if (assignToHouse) {
        if (m_currentHouseId == 0) {
            qDebug() << "HouseBrush::apply (assign): No current house ID selected. Cannot assign 'no house'.";
            return;
        }
        RME::core::houses::HouseData* houseToAssign = housesManager->getHouse(m_currentHouseId);
        if (!houseToAssign) {
            qWarning() << "HouseBrush::apply (assign): House with ID" << m_currentHouseId << "not found in HousesManager.";
            return;
        }

        if (tile->getHouseId() != 0 && tile->getHouseId() != m_currentHouseId) {
            qInfo() << "HouseBrush: Tile at" << pos.toString() << "(belonging to house" << tile->getHouseId() << ") will be reassigned to house" << m_currentHouseId;
        } else if (tile->getHouseId() == m_currentHouseId) {
            qDebug() << "HouseBrush::apply (assign): Tile at" << pos.toString() << "already belongs to house" << m_currentHouseId << ". No change.";
            return;
        }

        // QUndoStack takes ownership of raw pointer.
        // SetHouseTileCommand's constructor now takes: houseId, Tile*, bool, Controller*, Parent*
        controller->pushCommand(new RME::core::actions::SetHouseTileCommand(m_currentHouseId, tile, true, controller));

    } else { // Erase mode
        quint32 currentTileHouseId = tile->getHouseId();
        if (currentTileHouseId == 0) {
            qDebug() << "HouseBrush::apply (erase): Tile at" << pos.toString() << "has no house assignment. No action.";
            return;
        }

        RME::core::houses::HouseData* houseOfTile = housesManager->getHouse(currentTileHouseId);
        if (!houseOfTile) {
            qWarning() << "HouseBrush::apply (erase): Tile at" << pos.toString() << "has house ID" << currentTileHouseId << ", but house not found in manager. Cannot create command to update house's tile list correctly.";
            // Not pushing command because SetHouseTileCommand requires a valid house ID to update its tile list.
            return;
        }

        if (m_currentHouseId == 0) { // Generic erase: remove any house assignment
            controller->pushCommand(new RME::core::actions::SetHouseTileCommand(currentTileHouseId, tile, false, controller));
        } else { // Specific erase: only remove if tile belongs to m_currentHouseId
            if (currentTileHouseId == m_currentHouseId) {
                // Use the current house ID for the command
                controller->pushCommand(new RME::core::actions::SetHouseTileCommand(m_currentHouseId, tile, false, controller));
            } else {
                qDebug() << "HouseBrush::apply (specific erase): Tile at" << pos.toString() << "(house" << currentTileHouseId << ") does not match brush's target house ID" << m_currentHouseId << ". No action.";
            }
        }
    }
}

} // namespace core
} // namespace RME
