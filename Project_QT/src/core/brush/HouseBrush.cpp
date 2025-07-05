#include "core/brush/HouseBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/houses/House.h" // For RME::core::houses::House
#include "core/houses/Houses.h" // For EditorControllerInterface::getHousesManager()
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h" // For future settings like remove items/door IDs
#include "editor_logic/commands/SetHouseTileCommand.h"

#include <QDebug> // For qWarning, qDebug
#include <memory>   // For std::make_unique if commands were owned here

// Placeholder for a house brush sprite ID, replace with actual if available
const int EDITOR_SPRITE_HOUSE_BRUSH_LOOK_ID = 0; // Or some defined constant

namespace RME {
namespace core {
namespace brush {

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
        qDebug("HouseBrush::apply: Preconditions not met at %s.", qUtf8Printable(pos.toString()));
        return;
    }

    Tile* tile = map->getTileForEditing(pos);
    if (!tile) {
        qWarning("HouseBrush::apply: Tile not found at position %s, though canApply passed (should check getTile).").arg(pos.toString());
        return;
    }

    bool assignToHouse = !settings.isEraseMode;
    RME::core::houses::Houses* housesManager = controller->getHousesManager();

    if (assignToHouse) {
        if (m_currentHouseId == 0) {
            qDebug("HouseBrush::apply (assign): No current house ID selected. Cannot assign 'no house'.");
            return;
        }
        RME::core::houses::House* houseToAssign = housesManager->getHouse(m_currentHouseId);
        if (!houseToAssign) {
            qWarning("HouseBrush::apply (assign): House with ID %u not found in HousesManager.").arg(m_currentHouseId);
            return;
        }

        if (tile->getHouseId() != 0 && tile->getHouseId() != m_currentHouseId) {
            qInfo("HouseBrush: Tile at %s (belonging to house %u) will be reassigned to house %u.")
                .arg(pos.toString()).arg(tile->getHouseId()).arg(m_currentHouseId);
        } else if (tile->getHouseId() == m_currentHouseId) {
            qDebug("HouseBrush::apply (assign): Tile at %s already belongs to house %u. No change.").arg(pos.toString()).arg(m_currentHouseId);
            return;
        }

        // QUndoStack takes ownership of raw pointer.
        // SetHouseTileCommand's constructor was: House*, Tile*, bool, Controller*, Parent*
        controller->pushCommand(new RME_COMMANDS::SetHouseTileCommand(houseToAssign, tile, true, controller));

    } else { // Erase mode
        quint32 currentTileHouseId = tile->getHouseId();
        if (currentTileHouseId == 0) {
            qDebug("HouseBrush::apply (erase): Tile at %s has no house assignment. No action.").arg(pos.toString());
            return;
        }

        RME::core::houses::House* houseOfTile = housesManager->getHouse(currentTileHouseId);
        if (!houseOfTile) {
            qWarning("HouseBrush::apply (erase): Tile at %s has house ID %u, but house not found in manager. Cannot create command to update house's tile list correctly.")
                .arg(pos.toString()).arg(currentTileHouseId);
            // Not pushing command because SetHouseTileCommand requires a valid House* to update its tile list.
            return;
        }

        if (m_currentHouseId == 0) { // Generic erase: remove any house assignment
            controller->pushCommand(new RME_COMMANDS::SetHouseTileCommand(houseOfTile, tile, false, controller));
        } else { // Specific erase: only remove if tile belongs to m_currentHouseId
            if (currentTileHouseId == m_currentHouseId) {
                // houseOfTile is the same as housesManager->getHouse(m_currentHouseId) here.
                controller->pushCommand(new RME_COMMANDS::SetHouseTileCommand(houseOfTile, tile, false, controller));
            } else {
                qDebug("HouseBrush::apply (specific erase): Tile at %s (house %u) does not match brush's target house ID %u. No action.")
                    .arg(pos.toString()).arg(currentTileHouseId).arg(m_currentHouseId);
            }
        }
    }
}

} // namespace brush
} // namespace core
} // namespace RME
