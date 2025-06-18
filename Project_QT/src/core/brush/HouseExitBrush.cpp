#include "core/brush/HouseExitBrush.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "editor_logic/commands/SetHouseExitCommand.h"

#include <QDebug> // For qWarning, qDebug

// Placeholder for a house exit brush sprite ID, replace with actual if available
const int EDITOR_SPRITE_HOUSE_EXIT_BRUSH_LOOK_ID = 0; // Or some defined constant

namespace RME {
namespace core {
namespace brush {

HouseExitBrush::HouseExitBrush()
    : m_currentHouseId(0) // Default to no house selected
{
}

void HouseExitBrush::setCurrentHouseId(quint32 houseId) {
    m_currentHouseId = houseId;
}

quint32 HouseExitBrush::getCurrentHouseId() const {
    return m_currentHouseId;
}

QString HouseExitBrush::getName() const {
    if (m_currentHouseId == 0) {
        return QStringLiteral("House Exit Brush (No House Selected)");
    }
    return QStringLiteral("House Exit Brush (House ID: %1)").arg(m_currentHouseId);
}

int HouseExitBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    return EDITOR_SPRITE_HOUSE_EXIT_BRUSH_LOOK_ID;
}

bool HouseExitBrush::canApply(const RME::core::map::Map* map,
                              const RME::core::Position& pos,
                              const RME::core::BrushSettings& /*settings*/) const {
    if (!map || !map->isPositionValid(pos)) {
        return false;
    }
    
    // A house must be selected to set an exit
    if (m_currentHouseId == 0) {
        return false;
    }
    
    // Get tile at position
    const Tile* tile = map->getTile(pos);
    if (!tile) {
        return false; // Tile must exist
    }
    
    // Tile must have ground
    if (!tile->hasGround()) {
        return false;
    }
    
    // Tile must not be blocking
    if (tile->isBlocking()) {
        return false;
    }
    
    // Tile should not be a house tile itself (exits are typically outside houses)
    if (tile->getHouseId() != 0) {
        return false;
    }
    
    return true;
}

void HouseExitBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                           const RME::core::Position& pos,
                           const RME::core::BrushSettings& settings) {
    
    if (!controller || !controller->getMap() || !controller->getHousesManager()) {
        qWarning("HouseExitBrush::apply: Controller, Map, or HousesManager is null.");
        return;
    }
    
    if (m_currentHouseId == 0) {
        qWarning("HouseExitBrush::apply: No house selected for exit setting.");
        return;
    }
    
    Map* map = controller->getMap();
    
    // Re-check canApply with live map
    if (!canApply(map, pos, settings)) {
        qDebug("HouseExitBrush::apply: Preconditions not met at %s.", qUtf8Printable(pos.toString()));
        return;
    }
    
    // Verify house exists
    RME::core::houses::Houses* housesManager = controller->getHousesManager();
    RME::core::houses::HouseData* house = housesManager->getHouse(m_currentHouseId);
    if (!house) {
        qWarning("HouseExitBrush::apply: House with ID %u not found.", m_currentHouseId);
        return;
    }
    
    // Check if this position is already the house exit
    if (house->entryPoint == pos) {
        qDebug("HouseExitBrush::apply: Position %s is already the exit for house %u.", 
               qUtf8Printable(pos.toString()), m_currentHouseId);
        return;
    }
    
    // Create and push the SetHouseExitCommand
    // This will handle the actual exit setting with proper undo/redo support
    controller->pushCommand(new SetHouseExitCommand(m_currentHouseId, pos, housesManager, map));
    
    qDebug("HouseExitBrush::apply: Set exit for house %u (%s) to position %s.", 
           m_currentHouseId, qUtf8Printable(house->name), qUtf8Printable(pos.toString()));
}

} // namespace brush
} // namespace core
} // namespace RME