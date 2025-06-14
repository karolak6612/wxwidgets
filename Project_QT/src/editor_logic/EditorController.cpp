#include "editor_logic/EditorController.h"
#include "core/map/Map.h"
#include "core/brush/Brush.h"
#include "core/brush/BrushManager.h"
#include "core/selection/SelectionManager.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h"
#include "core/assets/AssetManager.h"
#include "core/selection/SelectionManager.h" // Included for method implementations
#include "core/settings/AppSettings.h"     // Included for method implementations
#include "core/map/Map.h"                  // Already included, but for context
#include "core/Tile.h"                     // Already included, for context
#include "editor_logic/commands/DeleteSelectionCommand.h"
#include "editor_logic/commands/ClearSelectionCommand.h"    // Added
#include "editor_logic/commands/BoundingBoxSelectCommand.h" // Added
#include "core/spawns/SpawnData.h" // Corrected path for SpawnData.h
#include "core/assets/CreatureData.h" // For record methods, ensure this is the correct header for CreatureData struct/class
#include "core/actions/AppUndoCommand.h" // For recordAction
#include "editor_logic/commands/AddCreatureCommand.h"   // Added
#include "editor_logic/commands/RemoveCreatureCommand.h" // Added
#include "core/houses/Houses.h"        // Added
#include "core/houses/House.h"           // Added
#include "editor_logic/commands/SetHouseExitCommand.h" // Added

#include <QUndoStack>
#include <QUndoCommand> // For pushCommand std::unique_ptr
#include <QList>
#include <memory>      // For std::make_unique
#include <QDebug>
#include <algorithm> // For std::min, std::max
#include "core/map_constants.h" // For MAP_MAX_Z_VALUE, GROUND_LAYER

namespace RME {
namespace editor_logic {

EditorController::EditorController(
    RME::core::Map* map,
    QUndoStack* undoStack,
    RME::core::selection::SelectionManager* selectionManager,
    RME::core::brush::BrushManager* brushManager,
    RME::core::settings::AppSettings* appSettings,
    RME::core::assets::AssetManager* assetManager,
    RME::core::houses::Houses* housesManager // Added
) : m_map(map),
    m_undoStack(undoStack),
    m_selectionManager(selectionManager),
    m_brushManager(brushManager),
    m_appSettings(appSettings),
    m_assetManager(assetManager),
    m_housesManager(housesManager) // Added
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_undoStack);
    Q_ASSERT(m_selectionManager);
    Q_ASSERT(m_brushManager);
    Q_ASSERT(m_appSettings);
    Q_ASSERT(m_assetManager);
    Q_ASSERT(m_housesManager); // Added
}

// --- Core editing operations ---
void EditorController::applyBrushStroke(const QList<RME::core::Position>& positions, const RME::core::BrushSettings& settings) {
    if (!m_brushManager) {
        qWarning("EditorController::applyBrushStroke: BrushManager is null.");
        return;
    }

    RME::core::brush::Brush* activeBrush = m_brushManager->getActiveBrush();
    if (!activeBrush) {
        qWarning("EditorController::applyBrushStroke: No active brush found.");
        return;
    }

    if (positions.isEmpty()) {
        return;
    }

    QString macroText = QString("%1 Stroke").arg(activeBrush->getName());
    // Assuming BrushSettings has isEraseMode and Brush has canBeErasingTool
    if(settings.isEraseMode && activeBrush->canBeErasingTool()) {
         macroText = QString("Erase Stroke (%1)").arg(activeBrush->getName());
    }

    m_undoStack->beginMacro(macroText);
    for (const RME::core::Position& pos : positions) {
        activeBrush->apply(this, pos, settings);
    }
    m_undoStack->endMacro();
}

void EditorController::deleteSelection() {
    if (!m_selectionManager) {
        qWarning("EditorController::deleteSelection: SelectionManager is null.");
        return;
    }
    if (m_selectionManager->isEmpty()) {
        qDebug("EditorController::deleteSelection: Selection is empty.");
    }
    pushCommand(std::make_unique<RME_COMMANDS::DeleteSelectionCommand>(m_map, m_selectionManager, this));
    // Note: DeleteSelectionCommand's constructor was changed in a previous task to take QList<Position>.
    // The implementation above uses the old signature.
    // The new handleDeleteSelection below should use the new signature.
}

void EditorController::handleDeleteSelection() {
    if (!m_selectionManager) {
        qWarning("EditorController::handleDeleteSelection: SelectionManager is null.");
        return;
    }
    if (m_selectionManager->isEmpty()) {
        qDebug("EditorController::handleDeleteSelection: Selection is empty, no action taken.");
        return;
    }
    // Get selected positions from SelectionManager.
    // Assumes SelectionManager provides a method to get these, e.g. getCurrentSelectedTilesList()
    // which returns QList<Tile*>, then we extract positions. Or a direct position list getter.
    // For this implementation, using getCurrentSelectedTilesList() then extracting positions.
    QList<RME::core::Tile*> selectedTiles = m_selectionManager->getCurrentSelectedTilesList();
    QList<RME::core::Position> selectedPositions;
    for (RME::core::Tile* tile : selectedTiles) {
        if (tile) {
            selectedPositions.append(tile->getPosition());
        }
    }

    if (selectedPositions.isEmpty()) {
         qDebug("EditorController::handleDeleteSelection: No valid tile positions in selection.");
        return;
    }

    pushCommand(std::make_unique<RME_COMMANDS::DeleteCommand>(m_map, selectedPositions, this));
}

void EditorController::clearCurrentSelection() {
    if (m_selectionManager && !m_selectionManager->isEmpty()) { // Check if there's something to clear
        // ClearSelectionCommand's constructor takes SelectionManager*.
        // It will query the current selection itself in its redo().
        pushCommand(std::make_unique<RME_COMMANDS::ClearSelectionCommand>(m_selectionManager));
    } else {
        qDebug("EditorController::clearCurrentSelection: SelectionManager is null or selection is already empty.");
        // Optionally, push a command that does nothing but indicates "Clear Selection (nothing selected)"
        // if that's desired UX for undo stack. For now, only push if something might be cleared.
    }
}

void EditorController::performBoundingBoxSelection(
    const RME::core::Position& p1,
    const RME::core::Position& p2,
    Qt::KeyboardModifiers modifiers,
    const RME::core::BrushSettings& currentBrushSettings
) {
    if (!m_map || !m_selectionManager || !m_appSettings) {
        qWarning("EditorController::performBoundingBoxSelection: Crucial managers missing.");
        return;
    }

    int min_x = std::min(p1.x, p2.x);
    int max_x = std::max(p1.x, p2.x);
    int min_y = std::min(p1.y, p2.y);
    int max_y = std::max(p1.y, p2.y);
    int current_floor = currentBrushSettings.getActiveZ(); // Assuming BrushSettings has getActiveZ()

    // Use getValue with Config::Key for AppSettings
    QString selectionTypeSetting = m_appSettings->getValue(RME::core::settings::Config::Key::SELECTION_TYPE, "CurrentFloor").toString();
    bool compensatedSelect = m_appSettings->getValue(RME::core::settings::Config::Key::COMPENSATED_SELECT, true).toBool();

    QList<RME::core::Tile*> tilesToPotentiallySelect;
    QList<int> z_levels_to_scan;

    if (selectionTypeSetting == "CurrentFloor") {
        z_levels_to_scan.append(current_floor);
    } else if (selectionTypeSetting == "AllFloors") {
        // Corrected Z-level iteration for "AllFloors"
        for (int z = RME::core::MAP_MAX_FLOOR; z >= RME::core::MAP_MIN_FLOOR; --z) {
            z_levels_to_scan.append(z);
        }
    } else if (selectionTypeSetting == "VisibleFloors") {
        int start_z = RME::core::GROUND_LAYER;
        if (current_floor < RME::core::GROUND_LAYER) {
            start_z = std::min(RME::core::GROUND_LAYER, current_floor + 2);
        }
        for (int z = start_z; z >= current_floor; --z) {
             if (z < 0) break;
             z_levels_to_scan.append(z);
        }
    } else {
        z_levels_to_scan.append(current_floor);
    }
    if (z_levels_to_scan.isEmpty()) z_levels_to_scan.append(current_floor);

    for (int z : z_levels_to_scan) {
        int compensated_min_x = min_x;
        int compensated_max_x = max_x;
        int compensated_min_y = min_y;
        int compensated_max_y = max_y;

        if (compensatedSelect && z != current_floor) {
            int dz = current_floor - z; // dz > 0 for floors below current, dz < 0 for floors above.
            compensated_min_x = min_x - dz;
            compensated_max_x = max_x - dz;
            compensated_min_y = min_y - dz;
            compensated_max_y = max_y - dz;
        }

        for (int y_scan = compensated_min_y; y_scan <= compensated_max_y; ++y_scan) {
            for (int x_scan = compensated_min_x; x_scan <= compensated_max_x; ++x_scan) {
                if(m_map->isPositionValid(RME::core::Position(x_scan,y_scan,z))){
                    RME::core::Tile* tile = m_map->getTile(RME::core::Position(x_scan, y_scan, z));
                    if (tile) { // getTile can return nullptr if no tile allocated and not creating
                        tilesToPotentiallySelect.append(tile);
                    }
                }
            }
        }
    }

    bool isAdditive = (modifiers & Qt::ControlModifier);
    QList<RME::core::Tile*> selectionBefore = m_selectionManager->getCurrentSelectedTilesList();

    // Only push command if there are tiles in the box or if it's a non-additive selection that would clear previous selection.
    if (!tilesToPotentiallySelect.isEmpty() || (!isAdditive && !selectionBefore.isEmpty())) {
        pushCommand(std::make_unique<RME_COMMANDS::BoundingBoxSelectCommand>(
            m_selectionManager,
            tilesToPotentiallySelect,
            isAdditive,
            selectionBefore
        ));
    } else {
        qDebug("BoundingBoxSelection: No tiles to select and no existing selection to clear non-additively.");
    }
}

// --- Implementation of EditorControllerInterface ---
RME::core::Map* EditorController::getMap() {
    return m_map;
}

const RME::core::Map* EditorController::getMap() const {
    return m_map;
}

QUndoStack* EditorController::getUndoStack() {
    return m_undoStack;
}

RME::core::assets::AssetManager* EditorController::getAssetManager() {
    return m_assetManager;
}

const RME::core::assets::AssetManager* EditorController::getAssetManager() const {
    return m_assetManager;
}

RME::core::settings::AppSettings& EditorController::getAppSettings() {
    Q_ASSERT(m_appSettings);
    return *m_appSettings;
}

const RME::core::settings::AppSettings& EditorController::getAppSettings() const {
    Q_ASSERT(m_appSettings);
    return *m_appSettings;
}

RME::core::houses::Houses* EditorController::getHousesManager() { // Added
    return m_housesManager;
}

const RME::core::houses::Houses* EditorController::getHousesManager() const { // Added
    return m_housesManager;
}

void EditorController::pushCommand(std::unique_ptr<QUndoCommand> command) {
    if (command) {
        m_undoStack->push(command.release());
    }
}

// Tile/Notification related methods
void EditorController::notifyTileChanged(const RME::core::Position& pos) {
    if (m_map) {
        m_map->notifyTileChanged(pos);
    }
}

RME::core::Tile* EditorController::getTileForEditing(const RME::core::Position& pos) {
    if (m_map) {
        return m_map->getTileForEditing(pos);
    }
    qWarning("EditorController::getTileForEditing: Map is null. Position: (%d,%d,%d)", pos.x, pos.y, pos.z);
    return nullptr;
}

// --- Record methods (placeholders or to be implemented with generic commands) ---
void EditorController::recordAction(std::unique_ptr<RME::core::actions::AppUndoCommand> command) {
    // If AppUndoCommand inherits QUndoCommand, this is valid:
    pushCommand(std::unique_ptr<QUndoCommand>(static_cast<QUndoCommand*>(command.release())));
    // If not, a wrapper or different push mechanism is needed.
    // qWarning("EditorController::recordAction: Generic AppUndoCommand recording might need specific handling if not QUndoCommand.");
}

void EditorController::recordTileChange(const RME::core::Position& pos,
                                      std::unique_ptr<RME::core::Tile> /*oldTileState*/,
                                      std::unique_ptr<RME::core::Tile> /*newTileState*/) {
    qWarning("EditorController::recordTileChange: Not implemented. Tile Pos: (%d,%d,%d)", pos.x, pos.y, pos.z);
}

void EditorController::recordAddCreature(const RME::core::Position& tilePos, const RME::core::assets::CreatureData* creatureData) {
    if (!creatureData) {
        qWarning("EditorController::recordAddCreature: creatureData is null. Pos: (%d,%d,%d)",
            tilePos.x, tilePos.y, tilePos.z);
        return;
    }
    RME::core::Tile* tile = getTileForEditing(tilePos);
    if (!tile) {
        qWarning("EditorController::recordAddCreature: Tile not found or not creatable at Pos: (%d,%d,%d) for Creature: %s",
            tilePos.x, tilePos.y, tilePos.z, qUtf8Printable(creatureData->name));
        return;
    }

    // Optional: Check if adding this creature is valid (e.g., tile already has a creature of different type?)
    // For now, AddCreatureCommand will handle replacing an existing creature.

    pushCommand(std::make_unique<RME::editor_logic::commands::AddCreatureCommand>(tile, creatureData, this));
}

void EditorController::recordRemoveCreature(const RME::core::Position& tilePos, const RME::core::assets::CreatureData* creatureData) {
    // creatureData parameter is not strictly used by RemoveCreatureCommand as it removes whatever creature is present.
    // It's kept for interface consistency for now. It could be used for validation if needed.
    RME::core::Tile* tile = getTileForEditing(tilePos);
    if (!tile) {
        qWarning("EditorController::recordRemoveCreature: Tile not found or not creatable at Pos: (%d,%d,%d)",
            tilePos.x, tilePos.y, tilePos.z);
        return;
    }

    if (!tile->hasCreature()) {
        // No creature to remove, so don't push a command that will do nothing.
        qDebug("EditorController::recordRemoveCreature: No creature on tile Pos: (%d,%d,%d) to remove.",
            tilePos.x, tilePos.y, tilePos.z);
        return;
    }

    // If creatureData is provided, we could add a check:
    // if (tile->getCreature() && tile->getCreature()->getType() != creatureData) {
    //     qWarning("EditorController::recordRemoveCreature: Creature on tile is not of the expected type. Removing anyway.");
    // }

    pushCommand(std::make_unique<RME::editor_logic::commands::RemoveCreatureCommand>(tile, this));
}

void EditorController::recordAddSpawn(const RME::core::SpawnData& spawnData) {
    qWarning("EditorController::recordAddSpawn: Not implemented. Pos: (%d,%d,%d)",
        spawnData.getCenter().x, spawnData.getCenter().y, spawnData.getCenter().z);
}

void EditorController::recordRemoveSpawn(const RME::core::Position& spawnCenterPos) {
    qWarning("EditorController::recordRemoveSpawn: Not implemented. Pos: (%d,%d,%d)",
        spawnCenterPos.x, spawnCenterPos.y, spawnCenterPos.z);
}

void EditorController::recordUpdateSpawn(const RME::core::Position& spawnCenterPos,
                                       const RME::core::SpawnData& /*oldSpawnData*/,
                                       const RME::core::SpawnData& /*newSpawnData*/) {
    qWarning("EditorController::recordUpdateSpawn: Not implemented. Pos: (%d,%d,%d)",
        spawnCenterPos.x, spawnCenterPos.y, spawnCenterPos.z);
}

void EditorController::recordSetGroundItem(const RME::core::Position& pos, uint16_t /*newGroundItemId*/, uint16_t /*oldGroundItemId*/) {
     qWarning("EditorController::recordSetGroundItem: Not implemented. Pos: (%d,%d,%d)", pos.x, pos.y, pos.z);
}

void EditorController::recordSetBorderItems(const RME::core::Position& pos,
                                          const QList<uint16_t>& /*newBorderItemIds*/,
                                          const QList<uint16_t>& /*oldBorderItemIds*/) {
     qWarning("EditorController::recordSetBorderItems: Not implemented. Pos: (%d,%d,%d)", pos.x, pos.y, pos.z);
}

void EditorController::recordAddItem(const RME::core::Position& pos, uint16_t itemId) {
     qWarning("EditorController::recordAddItem: Not implemented. Pos: (%d,%d,%d), ItemID: %d", pos.x, pos.y, pos.z, itemId);
}

void EditorController::recordRemoveItem(const RME::core::Position& pos, uint16_t itemId) {
     qWarning("EditorController::recordRemoveItem: Not implemented. Pos: (%d,%d,%d), ItemID: %d", pos.x, pos.y, pos.z, itemId);
}

// --- House-specific operations ---
void EditorController::setHouseExit(quint32 houseId, const RME::core::Position& exitPos) {
    if (!m_housesManager) {
        qWarning("EditorController::setHouseExit: HousesManager is null.");
        return;
    }
    RME::core::houses::House* house = m_housesManager->getHouse(houseId);
    if (!house) {
        qWarning("EditorController::setHouseExit: House with ID %u not found.", houseId);
        return;
    }
    if (!m_map) {
        qWarning("EditorController::setHouseExit: Map instance is null.");
        return;
    }

    RME::core::Position currentExit = house->getExitPos();

    if (currentExit == exitPos) {
        qDebug("EditorController::setHouseExit: New exit position is the same as current for house ID %u.", houseId);
        return;
    }

    // If new exitPos is valid, it must be a valid location.
    // If new exitPos is invalid, it means we are clearing the exit.
    if (exitPos.isValid()) {
        if (!m_map->isValidHouseExitLocation(exitPos)) {
            qWarning("EditorController::setHouseExit: Attempted to set invalid new exit location (%s) for house ID %u.",
                     qUtf8Printable(exitPos.toString()), houseId);
            return;
        }
    } else { // We are trying to clear the exit (newExitPos is invalid)
        if (!currentExit.isValid()) { // And there's no current exit to clear
            qDebug("EditorController::setHouseExit: Attempting to clear an already non-existent exit for house ID %u.", houseId);
            return;
        }
    }

    // Proceed to create and push the command
    // Using SetHouseExitCommand directly as it's not in RME_COMMANDS namespace per previous task.
    pushCommand(std::make_unique<SetHouseExitCommand>(house, exitPos, m_map));
}

} // namespace editor_logic
} // namespace RME
