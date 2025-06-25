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
#include "core/clipboard/ClipboardManager.h"
#include "core/clipboard/ClipboardData.h"
#include "core/actions/PasteCommand.h"
#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include "editor_logic/commands/AddWaypointCommand.h"
#include "editor_logic/commands/MoveWaypointCommand.h"
#include "editor_logic/commands/RemoveWaypointCommand.h"
#include "editor_logic/commands/RenameWaypointCommand.h"
#include "editor_logic/commands/SetHouseTileCommand.h"
#include "core/houses/Houses.h"
#include "core/houses/HouseData.h"
#include "editor_logic/commands/ClearSelectionCommand.h"    // Added
#include "editor_logic/commands/BoundingBoxSelectCommand.h" // Added
#include "core/spawns/SpawnData.h" // Corrected path for SpawnData.h
#include "core/assets/CreatureData.h" // For record methods, ensure this is the correct header for CreatureData struct/class
#include "core/actions/AppUndoCommand.h" // For recordAction
#include "editor_logic/commands/AddCreatureCommand.h"   // Added
#include "editor_logic/commands/RemoveCreatureCommand.h" // Added
#include "core/houses/Houses.h"        // Added
#include "core/houses/HouseData.h"      // Added for HouseData
#include "editor_logic/commands/SetHouseExitCommand.h" // Added
#include "editor_logic/commands/CreateHouseCommand.h" // Added for LOGIC-05
#include "editor_logic/commands/RemoveHouseCommand.h" // Added for LOGIC-05
#include "editor_logic/commands/ModifyHousePropertiesCommand.h" // Added for LOGIC-05
#include "editor_logic/commands/AddSpawnCommand.h" // Added for LOGIC-07
#include "editor_logic/commands/RemoveSpawnCommand.h" // Added for LOGIC-07
#include "editor_logic/commands/MapWideOperationCommand.h" // Added for LOGIC-09

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
    RME::core::IBrushStateService* brushStateService,
    RME::core::IEditorStateService* editorStateService,
    RME::core::IClientDataService* clientDataService,
    RME::core::IApplicationSettingsService* settingsService,
    QObject* parent
) : QObject(parent),
    m_map(map),
    m_brushStateService(brushStateService),
    m_editorStateService(editorStateService),
    m_clientDataService(clientDataService),
    m_settingsService(settingsService)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_editorStateService);
    Q_ASSERT(m_clientDataService);
    Q_ASSERT(m_settingsService);
    
    // Initialize direct dependencies from services
    initializeDependencies();
}

void EditorController::initializeDependencies()
{
    // Get dependencies from services
    m_assetManager = m_clientDataService->getAssetManager();
    m_appSettings = nullptr; // TODO: Get from settings service
    
    // Create or get other dependencies
    m_undoStack = new QUndoStack(this);
    m_selectionManager = new RME::core::selection::SelectionManager(this);
    m_brushManager = nullptr; // TODO: Get from brush service
    m_housesManager = nullptr; // TODO: Get from map or create
    m_clipboardManager = new RME::core::clipboard::ClipboardManager(this);
    m_waypointManager = new RME::core::waypoints::WaypointManager(this);
    
    qDebug() << "EditorController: Dependencies initialized from services";
}

// --- Core editing operations ---
void EditorController::applyBrushStroke(const QList<RME::core::Position>& positions, const RME::core::BrushSettings& settings) {
    if (!m_brushStateService) {
        qWarning("EditorController::applyBrushStroke: BrushStateService is null.");
        return;
    }

    RME::core::brush::Brush* activeBrush = m_brushStateService->getActiveBrush();
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
    pushCommand(std::make_unique<RME::core::actions::DeleteSelectionCommand>(m_map, m_selectionManager, this));
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

    pushCommand(std::make_unique<RME::core::actions::DeleteCommand>(m_map, selectedPositions, this));
}

void EditorController::clearCurrentSelection() {
    if (m_selectionManager && !m_selectionManager->isEmpty()) { // Check if there's something to clear
        // ClearSelectionCommand's constructor takes SelectionManager*.
        // It will query the current selection itself in its redo().
        pushCommand(std::make_unique<RME::core::actions::ClearSelectionCommand>(m_selectionManager));
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
        pushCommand(std::make_unique<RME::core::actions::BoundingBoxSelectCommand>(
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

RME::core::waypoints::WaypointManager* EditorController::getWaypointManager() { // Added for LOGIC-04
    return m_waypointManager;
}

const RME::core::waypoints::WaypointManager* EditorController::getWaypointManager() const { // Added for LOGIC-04
    return m_waypointManager;
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

void EditorController::recordAddSpawn(const RME::core::spawns::SpawnData& spawnData) {
    if (!m_map || !m_spawnManager) {
        qWarning("EditorController::recordAddSpawn: Invalid state - map or spawn manager is null");
        return;
    }
    
    // Create AddSpawnCommand for proper undo support
    auto addSpawnCommand = std::make_unique<RME::core::actions::AddSpawnCommand>(
        spawnData, m_spawnManager, this);
    
    pushCommand(std::move(addSpawnCommand));
    
    qDebug() << "EditorController::recordAddSpawn: Added spawn at" << spawnData.position.toString()
             << "with" << spawnData.creatures.size() << "creature types";
}

void EditorController::recordRemoveSpawn(const RME::core::Position& pos) {
    if (!m_map || !m_spawnManager) {
        qWarning("EditorController::recordRemoveSpawn: Invalid state - map or spawn manager is null");
        return;
    }
    
    // Verify spawn exists before creating command
    const RME::core::spawns::SpawnData* existingSpawn = m_spawnManager->getSpawn(pos);
    if (!existingSpawn) {
        qWarning("EditorController::recordRemoveSpawn: No spawn found at position (%d, %d, %d)",
                 pos.x, pos.y, pos.z);
        return;
    }
    
    // Create RemoveSpawnCommand for proper undo support
    auto removeSpawnCommand = std::make_unique<RME::core::actions::RemoveSpawnCommand>(
        pos, m_spawnManager, this);
    
    pushCommand(std::move(removeSpawnCommand));
    
    qDebug() << "EditorController::recordRemoveSpawn: Removed spawn at" << pos.toString();
}

void EditorController::recordUpdateSpawn(const RME::core::Position& pos, const RME::core::spawns::SpawnData& oldState, const RME::core::spawns::SpawnData& newState) {
    if (!m_map || !m_spawnManager) {
        qWarning("EditorController::recordUpdateSpawn: Invalid state - map or spawn manager is null");
        return;
    }
    
    // For now, implement as remove + add for simplicity
    // TODO: Create a dedicated UpdateSpawnCommand for better efficiency
    recordRemoveSpawn(pos);
    recordAddSpawn(newState);
    
    qDebug() << "EditorController::recordUpdateSpawn: Updated spawn at" << pos.toString();
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
    RME::core::houses::HouseData* house = m_housesManager->getHouse(houseId);
    if (!house) {
        qWarning("EditorController::setHouseExit: House with ID %u not found.", houseId);
        return;
    }
    if (!m_map) {
        qWarning("EditorController::setHouseExit: Map instance is null.");
        return;
    }

    RME::core::Position currentExit = house->entryPoint;

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
    // Using SetHouseExitCommand with updated constructor signature
    pushCommand(std::make_unique<SetHouseExitCommand>(houseId, exitPos, m_housesManager, m_map));
}

// --- Clipboard operations (LOGIC-03) ---
void EditorController::copySelection() {
    if (!m_selectionManager || !m_clipboardManager) {
        qWarning("EditorController::copySelection: SelectionManager or ClipboardManager is null");
        return;
    }
    
    auto selectedTiles = m_selectionManager->getCurrentSelectedTilesList();
    if (selectedTiles.isEmpty()) {
        qDebug("EditorController::copySelection: No tiles selected");
        return;
    }
    
    // Convert selected tiles to clipboard content
    RME::core::clipboard::ClipboardContent content = convertTilesToClipboardContent(selectedTiles);
    
    // Store in both internal and system clipboard
    m_clipboardManager->setInternalClipboard(content);
    m_clipboardManager->copyToClipboard(content);
    
    qDebug() << "EditorController::copySelection: Copied" << selectedTiles.size() << "tiles to clipboard";
}

void EditorController::cutSelection() {
    if (!m_selectionManager || !m_clipboardManager) {
        qWarning("EditorController::cutSelection: SelectionManager or ClipboardManager is null");
        return;
    }
    
    auto selectedTiles = m_selectionManager->getCurrentSelectedTilesList();
    if (selectedTiles.isEmpty()) {
        qDebug("EditorController::cutSelection: No tiles selected");
        return;
    }
    
    // First copy the selection
    copySelection();
    
    // Then delete the selection
    deleteSelection();
    
    qDebug() << "EditorController::cutSelection: Cut" << selectedTiles.size() << "tiles";
}

void EditorController::pasteAtPosition(const RME::core::Position& pos) {
    if (!m_clipboardManager || !m_map) {
        qWarning("EditorController::pasteAtPosition: ClipboardManager or Map is null");
        return;
    }
    
    if (!m_clipboardManager->hasClipboardData()) {
        qDebug("EditorController::pasteAtPosition: No clipboard data available");
        return;
    }
    
    // Create and execute paste command
    auto pasteCommand = std::make_unique<RME::core::actions::PasteCommand>(
        m_map, m_clipboardManager, pos);
    
    pushCommand(std::move(pasteCommand));
    
    qDebug() << "EditorController::pasteAtPosition: Pasted at position" << pos.x << pos.y << pos.z;
}

void EditorController::moveSelection(const RME::core::Position& offset) {
    if (!m_selectionManager || !m_clipboardManager) {
        qWarning("EditorController::moveSelection: SelectionManager or ClipboardManager is null");
        return;
    }
    
    auto selectedTiles = m_selectionManager->getCurrentSelectedTilesList();
    if (selectedTiles.isEmpty()) {
        qDebug("EditorController::moveSelection: No tiles selected");
        return;
    }
    
    // Calculate the target position (use first selected tile as reference)
    if (!selectedTiles.isEmpty()) {
        RME::core::Position firstTilePos = selectedTiles.first()->getPosition();
        RME::core::Position targetPos(
            firstTilePos.x + offset.x,
            firstTilePos.y + offset.y,
            firstTilePos.z + offset.z
        );
        
        // Cut the current selection
        cutSelection();
        
        // Paste at the new position
        pasteAtPosition(targetPos);
        
        qDebug() << "EditorController::moveSelection: Moved" << selectedTiles.size() 
                 << "tiles by offset" << offset.x << offset.y << offset.z;
    }
}

// Helper method to convert tiles to clipboard content
RME::core::clipboard::ClipboardContent EditorController::convertTilesToClipboardContent(const QList<RME::core::Tile*>& tiles) const {
    RME::core::clipboard::ClipboardContent content;
    
    if (tiles.isEmpty()) {
        return content;
    }
    
    // Find the top-left position to use as reference for relative positioning
    RME::core::Position topLeft = tiles.first()->getPosition();
    for (const auto* tile : tiles) {
        if (tile) {
            RME::core::Position pos = tile->getPosition();
            if (pos.x < topLeft.x) topLeft.x = pos.x;
            if (pos.y < topLeft.y) topLeft.y = pos.y;
            if (pos.z < topLeft.z) topLeft.z = pos.z;
        }
    }
    
    // Convert each tile to clipboard format
    for (const auto* tile : tiles) {
        if (!tile) continue;
        
        RME::core::clipboard::ClipboardTileData tileData;
        RME::core::Position tilePos = tile->getPosition();
        
        // Set relative position
        tileData.relativePosition = RME::core::Position(
            tilePos.x - topLeft.x,
            tilePos.y - topLeft.y,
            tilePos.z - topLeft.z
        );
        
        // Copy ground item
        if (tile->getGround()) {
            tileData.hasGround = true;
            tileData.groundItemID = tile->getGround()->getID();
        } else {
            tileData.hasGround = false;
            tileData.groundItemID = 0;
        }
        
        // Copy house ID and tile flags
        tileData.houseId = tile->getHouseId();
        tileData.tileFlags = tile->getFlags();
        
        // Copy items
        const auto& items = tile->getItems();
        for (const auto& item : items) {
            if (item) {
                RME::core::clipboard::ClipboardItemData itemData;
                itemData.itemID = item->getID();
                itemData.count = item->getCount();
                itemData.actionID = item->getActionID();
                itemData.uniqueID = item->getUniqueID();
                itemData.text = item->getText();
                itemData.description = item->getDescription();
                tileData.items.append(itemData);
            }
        }
        
        // Copy creature
        if (tile->getCreature()) {
            const auto* creature = tile->getCreature();
            tileData.creature.hasCreature = true;
            tileData.creature.creatureID = creature->getID();
            tileData.creature.name = creature->getName();
            tileData.creature.direction = static_cast<int>(creature->getDirection());
        } else {
            tileData.creature.hasCreature = false;
        }
        
        // Copy spawn (if tile has spawn)
        if (tile->hasSpawn()) {
            tileData.spawn.hasSpawn = true;
            // Copy spawn data - implementation depends on spawn system
        } else {
            tileData.spawn.hasSpawn = false;
        }
        
        content.tiles.append(tileData);
    }
    
    qDebug() << "EditorController::convertTilesToClipboardContent: Converted" 
             << tiles.size() << "tiles to clipboard content";
    
    return content;
}

// --- Waypoint operations (LOGIC-04) ---
void EditorController::placeOrMoveWaypoint(const QString& name, const RME::core::Position& targetPos) {
    if (!m_waypointManager) {
        qWarning("EditorController::placeOrMoveWaypoint: WaypointManager is null");
        return;
    }
    
    if (name.isEmpty()) {
        qWarning("EditorController::placeOrMoveWaypoint: Waypoint name is empty");
        return;
    }
    
    // Check if waypoint already exists
    RME::core::waypoints::Waypoint* existing = m_waypointManager->getWaypoint(name);
    if (existing) {
        // Move existing waypoint
        RME::core::Position oldPos = existing->getPosition();
        if (oldPos != targetPos) {
            auto moveCommand = std::make_unique<RME::core::actions::MoveWaypointCommand>(
                m_waypointManager, name, oldPos, targetPos);
            pushCommand(std::move(moveCommand));
            qDebug() << "EditorController::placeOrMoveWaypoint: Moving waypoint" << name 
                     << "from" << oldPos.x << oldPos.y << oldPos.z 
                     << "to" << targetPos.x << targetPos.y << targetPos.z;
        } else {
            qDebug() << "EditorController::placeOrMoveWaypoint: Waypoint" << name << "already at target position";
        }
    } else {
        // Add new waypoint
        auto addCommand = std::make_unique<RME::core::actions::AddWaypointCommand>(
            m_waypointManager, name, targetPos);
        pushCommand(std::move(addCommand));
        qDebug() << "EditorController::placeOrMoveWaypoint: Adding new waypoint" << name 
                 << "at" << targetPos.x << targetPos.y << targetPos.z;
    }
}

void EditorController::removeWaypoint(const QString& name) {
    if (!m_waypointManager) {
        qWarning("EditorController::removeWaypoint: WaypointManager is null");
        return;
    }
    
    if (name.isEmpty()) {
        qWarning("EditorController::removeWaypoint: Waypoint name is empty");
        return;
    }
    
    // Check if waypoint exists
    RME::core::waypoints::Waypoint* existing = m_waypointManager->getWaypoint(name);
    if (!existing) {
        qDebug() << "EditorController::removeWaypoint: Waypoint" << name << "does not exist";
        return;
    }
    
    // Create and execute remove command for proper undo support
    auto removeCommand = std::make_unique<RME::core::actions::RemoveWaypointCommand>(
        m_waypointManager, name);
    pushCommand(std::move(removeCommand));
    
    qDebug() << "EditorController::removeWaypoint: Removing waypoint" << name << "with undo support";
}

void EditorController::renameWaypoint(const QString& oldName, const QString& newName) {
    if (!m_waypointManager) {
        qWarning("EditorController::renameWaypoint: WaypointManager is null");
        return;
    }
    
    if (oldName.isEmpty() || newName.isEmpty()) {
        qWarning("EditorController::renameWaypoint: Old or new name is empty");
        return;
    }
    
    if (oldName == newName) {
        qDebug("EditorController::renameWaypoint: Old and new names are the same");
        return;
    }
    
    // Check if old waypoint exists
    RME::core::waypoints::Waypoint* existing = m_waypointManager->getWaypoint(oldName);
    if (!existing) {
        qWarning() << "EditorController::renameWaypoint: Waypoint" << oldName << "does not exist";
        return;
    }
    
    // Check if new name already exists
    if (m_waypointManager->getWaypoint(newName)) {
        qWarning() << "EditorController::renameWaypoint: Waypoint with name" << newName << "already exists";
        return;
    }
    
    // Create and execute rename command for proper undo support
    auto renameCommand = std::make_unique<RME::core::actions::RenameWaypointCommand>(
        m_waypointManager, oldName, newName);
    pushCommand(std::move(renameCommand));
    
    qDebug() << "EditorController::renameWaypoint: Renaming waypoint from" << oldName << "to" << newName << "with undo support";
}

void EditorController::navigateToWaypoint(const QString& name) {
    if (!m_waypointManager) {
        qWarning("EditorController::navigateToWaypoint: WaypointManager is null");
        return;
    }
    
    if (name.isEmpty()) {
        qWarning("EditorController::navigateToWaypoint: Waypoint name is empty");
        return;
    }
    
    // Check if waypoint exists
    RME::core::waypoints::Waypoint* waypoint = m_waypointManager->getWaypoint(name);
    if (!waypoint) {
        qWarning() << "EditorController::navigateToWaypoint: Waypoint" << name << "does not exist";
        return;
    }
    
    RME::core::Position pos = waypoint->getPosition();
    qDebug() << "EditorController::navigateToWaypoint: Navigating to waypoint" << name 
             << "at position" << pos.x << pos.y << pos.z;
    
    // TODO: Implement actual navigation - this would typically:
    // 1. Update the map view to center on the waypoint position
    // 2. Set the current floor to the waypoint's floor
    // 3. Possibly highlight the waypoint temporarily
    // 4. Update cursor position
    
    // For now, just emit a signal or call a method to update the view
    // This would require integration with the MapView or similar component
    
    qWarning("EditorController::navigateToWaypoint: Navigation not yet implemented - needs MapView integration");
}

// --- House operations (LOGIC-05) ---
quint32 EditorController::createHouse(const QString& name, const RME::core::Position& entryPoint) {
    return createHouse(name, entryPoint, 0, 0, false);
}

quint32 EditorController::createHouse(const QString& name, const RME::core::Position& entryPoint, 
                                     quint32 townId, quint32 rent, bool isGuildhall) {
    if (!m_housesManager) {
        qWarning("EditorController::createHouse: Houses manager is null");
        return 0;
    }
    
    if (name.isEmpty()) {
        qWarning("EditorController::createHouse: House name is empty");
        return 0;
    }
    
    // Create CreateHouseCommand for proper undo support
    auto createCommand = std::make_unique<RME::core::actions::CreateHouseCommand>(
        name, entryPoint, townId, rent, isGuildhall, m_housesManager, this);
    
    // Get the house ID before pushing the command
    quint32 houseId = 0;
    RME::core::actions::CreateHouseCommand* cmdPtr = createCommand.get();
    
    pushCommand(std::move(createCommand));
    
    // Get the created house ID
    houseId = cmdPtr->getCreatedHouseId();
    
    qDebug() << "EditorController::createHouse: Created house" << houseId 
             << "with name" << name << "at" << entryPoint.x << entryPoint.y << entryPoint.z;
    
    return houseId;
}

void EditorController::assignTileToHouse(const RME::core::Position& pos, quint32 houseId) {
    if (!m_map || !m_housesManager) {
        qWarning("EditorController::assignTileToHouse: Map or Houses manager is null");
        return;
    }
    
    if (houseId == 0) {
        qWarning("EditorController::assignTileToHouse: Invalid house ID (0)");
        return;
    }
    
    // Verify house exists
    RME::core::houses::HouseData* house = m_housesManager->getHouse(houseId);
    if (!house) {
        qWarning() << "EditorController::assignTileToHouse: House with ID" << houseId << "not found";
        return;
    }
    
    // Get or create tile at position
    RME::core::Tile* tile = m_map->getTile(pos);
    if (!tile) {
        // Create tile if it doesn't exist
        bool tileCreated = false;
        tile = m_map->getOrCreateTile(pos, tileCreated);
        if (!tile) {
            qWarning() << "EditorController::assignTileToHouse: Failed to get or create tile at" 
                       << pos.x << pos.y << pos.z;
            return;
        }
    }
    
    // Create and execute command for tile assignment
    auto assignCommand = std::make_unique<RME::core::actions::SetHouseTileCommand>(
        houseId, tile, true, this);
    pushCommand(std::move(assignCommand));
    
    qDebug() << "EditorController::assignTileToHouse: Assigned tile at" 
             << pos.x << pos.y << pos.z << "to house" << houseId << "(" << house->name << ")";
}

void EditorController::removeHouseAssignment(const RME::core::Position& pos) {
    if (!m_map) {
        qWarning("EditorController::removeHouseAssignment: Map is null");
        return;
    }
    
    // Get tile at position
    RME::core::Tile* tile = m_map->getTile(pos);
    if (!tile) {
        qDebug() << "EditorController::removeHouseAssignment: No tile at" 
                 << pos.x << pos.y << pos.z;
        return;
    }
    
    quint32 currentHouseId = tile->getHouseId();
    if (currentHouseId == 0) {
        qDebug() << "EditorController::removeHouseAssignment: Tile at" 
                 << pos.x << pos.y << pos.z << "is not assigned to any house";
        return;
    }
    
    // Create and execute command for tile unassignment
    auto unassignCommand = std::make_unique<RME::core::actions::SetHouseTileCommand>(
        currentHouseId, tile, false, this);
    pushCommand(std::move(unassignCommand));
    
    qDebug() << "EditorController::removeHouseAssignment: Removed house assignment from tile at" 
             << pos.x << pos.y << pos.z << "(was assigned to house" << currentHouseId << ")";
}

void EditorController::setHouseProperty(quint32 houseId, const QString& property, const QVariant& value) {
    QHash<QString, QVariant> properties;
    properties[property] = value;
    setHouseProperties(houseId, properties);
}

void EditorController::setHouseProperties(quint32 houseId, const QHash<QString, QVariant>& properties) {
    if (!m_housesManager) {
        qWarning("EditorController::setHouseProperties: Houses manager is null");
        return;
    }
    
    if (houseId == 0) {
        qWarning("EditorController::setHouseProperties: Invalid house ID (0)");
        return;
    }
    
    if (properties.isEmpty()) {
        qWarning("EditorController::setHouseProperties: No properties to set");
        return;
    }
    
    // Get house to verify it exists
    RME::core::houses::HouseData* house = m_housesManager->getHouse(houseId);
    if (!house) {
        qWarning() << "EditorController::setHouseProperties: House with ID" << houseId << "not found";
        return;
    }
    
    // Create ModifyHousePropertiesCommand for proper undo support
    auto modifyCommand = std::make_unique<RME::core::actions::ModifyHousePropertiesCommand>(
        houseId, properties, m_housesManager, this);
    
    pushCommand(std::move(modifyCommand));
    
    qDebug() << "EditorController::setHouseProperties: Modified" << properties.size() 
             << "properties for house" << houseId << "(" << house->name << ")";
}

void EditorController::removeHouse(quint32 houseId) {
    if (!m_housesManager) {
        qWarning("EditorController::removeHouse: Houses manager is null");
        return;
    }
    
    if (houseId == 0) {
        qWarning("EditorController::removeHouse: Invalid house ID (0)");
        return;
    }
    
    // Verify house exists
    RME::core::houses::HouseData* house = m_housesManager->getHouse(houseId);
    if (!house) {
        qWarning() << "EditorController::removeHouse: House with ID" << houseId << "not found";
        return;
    }
    
    QString houseName = house->name;
    
    // Create RemoveHouseCommand for proper undo support
    auto removeCommand = std::make_unique<RME::core::actions::RemoveHouseCommand>(
        houseId, m_housesManager, this);
    
    pushCommand(std::move(removeCommand));
    
    qDebug() << "EditorController::removeHouse: Removed house" << houseId << "(" << houseName << ")";
}

// --- Map interaction handling (LOGIC-06) ---
void EditorController::handleMapClick(const RME::core::Position& pos, Qt::MouseButton button, 
                                     Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings) {
    if (!m_map) {
        qWarning("EditorController::handleMapClick: Map is null");
        return;
    }
    
    // Handle different tool modes
    switch (m_currentToolMode) {
        case ToolMode::Brush:
            // Normal brush operation
            if (button == Qt::LeftButton) {
                if (!m_brushManager) {
                    qWarning("EditorController::handleMapClick: BrushManager is null");
                    return;
                }
                
                RME::core::brush::Brush* activeBrush = m_brushManager->getActiveBrush();
                if (activeBrush) {
                    activeBrush->apply(this, pos, settings);
                }
            }
            break;
            
        case ToolMode::HouseExit:
            // House exit placement
            if (button == Qt::LeftButton && m_currentHouseForTools != 0) {
                setHouseExit(m_currentHouseForTools, pos);
            }
            break;
            
        case ToolMode::Waypoint:
            // Waypoint placement/movement
            if (button == Qt::LeftButton && !m_currentWaypointForTools.isEmpty()) {
                placeOrMoveWaypoint(m_currentWaypointForTools, pos);
            }
            break;
    }
}

void EditorController::handleMapDrag(const QList<RME::core::Position>& positions, Qt::MouseButton button, 
                                    Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings) {
    if (!m_map) {
        qWarning("EditorController::handleMapDrag: Map is null");
        return;
    }
    
    // Only brush mode supports dragging
    if (m_currentToolMode == ToolMode::Brush && button == Qt::LeftButton) {
        applyBrushStroke(positions, settings);
    }
}

void EditorController::handleMapRelease(const RME::core::Position& pos, Qt::MouseButton button, 
                                       Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings) {
    // Currently no special handling needed for mouse release
    // This method is provided for future extensibility
    Q_UNUSED(pos)
    Q_UNUSED(button)
    Q_UNUSED(modifiers)
    Q_UNUSED(settings)
}

// --- Tool mode management (LOGIC-06) ---
void EditorController::setToolMode(ToolMode mode) {
    if (m_currentToolMode != mode) {
        m_currentToolMode = mode;
        qDebug() << "EditorController::setToolMode: Changed to mode" << static_cast<int>(mode);
    }
}

EditorController::ToolMode EditorController::getToolMode() const {
    return m_currentToolMode;
}

void EditorController::setCurrentHouseForTools(quint32 houseId) {
    if (m_currentHouseForTools != houseId) {
        m_currentHouseForTools = houseId;
        qDebug() << "EditorController::setCurrentHouseForTools: Set to house ID" << houseId;
        
        // Auto-switch to house exit mode if a house is selected
        if (houseId != 0 && m_currentToolMode != ToolMode::HouseExit) {
            setToolMode(ToolMode::HouseExit);
        }
    }
}

void EditorController::setCurrentWaypointForTools(const QString& waypointName) {
    if (m_currentWaypointForTools != waypointName) {
        m_currentWaypointForTools = waypointName;
        qDebug() << "EditorController::setCurrentWaypointForTools: Set to waypoint" << waypointName;
        
        // Auto-switch to waypoint mode if a waypoint is selected
        if (!waypointName.isEmpty() && m_currentToolMode != ToolMode::Waypoint) {
            setToolMode(ToolMode::Waypoint);
        }
    }
}

// --- Map-wide operations (LOGIC-09) ---
void EditorController::borderizeMap(bool showProgressDialog) {
    if (!m_map) {
        qWarning("EditorController::borderizeMap: Map is null");
        return;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["showProgressDialog"] = showProgressDialog;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::BorderizeMap,
        m_map, this, parameters);
    
    if (showProgressDialog) {
        // TODO: Show progress dialog and connect to command
        command->setProgressCallback([](int progress, const QString& message) {
            qDebug() << "Borderize progress:" << progress << "%" << message;
        });
    }
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::borderizeMap: Started map borderization";
}

void EditorController::randomizeMap(bool showProgressDialog) {
    if (!m_map) {
        qWarning("EditorController::randomizeMap: Map is null");
        return;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["showProgressDialog"] = showProgressDialog;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::RandomizeMap,
        m_map, this, parameters);
    
    if (showProgressDialog) {
        command->setProgressCallback([](int progress, const QString& message) {
            qDebug() << "Randomize progress:" << progress << "%" << message;
        });
    }
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::randomizeMap: Started map randomization";
}

void EditorController::clearInvalidHouseTiles(bool showProgressDialog) {
    if (!m_map || !m_housesManager) {
        qWarning("EditorController::clearInvalidHouseTiles: Map or houses manager is null");
        return;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["showProgressDialog"] = showProgressDialog;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::ClearInvalidHouseTiles,
        m_map, this, parameters);
    
    if (showProgressDialog) {
        command->setProgressCallback([](int progress, const QString& message) {
            qDebug() << "Clear invalid house tiles progress:" << progress << "%" << message;
        });
    }
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::clearInvalidHouseTiles: Started clearing invalid house tiles";
}

void EditorController::clearModifiedTileState(bool showProgressDialog) {
    if (!m_map) {
        qWarning("EditorController::clearModifiedTileState: Map is null");
        return;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["showProgressDialog"] = showProgressDialog;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::ClearModifiedTileState,
        m_map, this, parameters);
    
    if (showProgressDialog) {
        command->setProgressCallback([](int progress, const QString& message) {
            qDebug() << "Clear modified state progress:" << progress << "%" << message;
        });
    }
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::clearModifiedTileState: Started clearing modified tile state";
}

quint32 EditorController::validateGrounds(bool validateStack, bool generateEmpty, bool removeDuplicates) {
    if (!m_map) {
        qWarning("EditorController::validateGrounds: Map is null");
        return 0;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["validateStack"] = validateStack;
    parameters["generateEmpty"] = generateEmpty;
    parameters["removeDuplicates"] = removeDuplicates;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::ValidateGrounds,
        m_map, this, parameters);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Validate grounds progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    // TODO: Return actual count from command
    quint32 modifiedCount = 0; // command->getModifiedTileCount();
    
    qDebug() << "EditorController::validateGrounds: Validated grounds, modified" << modifiedCount << "tiles";
    return modifiedCount;
}

// --- Selection-based operations (LOGIC-09) ---
void EditorController::borderizeSelection() {
    if (!m_map || !m_selectionManager) {
        qWarning("EditorController::borderizeSelection: Map or selection manager is null");
        return;
    }
    
    if (!m_selectionManager->hasSelection()) {
        qWarning("EditorController::borderizeSelection: No selection to borderize");
        return;
    }
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::BorderizeSelection,
        m_map, this);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Borderize selection progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::borderizeSelection: Started selection borderization";
}

void EditorController::randomizeSelection() {
    if (!m_map || !m_selectionManager) {
        qWarning("EditorController::randomizeSelection: Map or selection manager is null");
        return;
    }
    
    if (!m_selectionManager->hasSelection()) {
        qWarning("EditorController::randomizeSelection: No selection to randomize");
        return;
    }
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::RandomizeSelection,
        m_map, this);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Randomize selection progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    qDebug() << "EditorController::randomizeSelection: Started selection randomization";
}

void EditorController::moveSelection(const RME::core::Position& offset) {
    if (!m_map || !m_selectionManager) {
        qWarning("EditorController::moveSelection: Map or selection manager is null");
        return;
    }
    
    if (!m_selectionManager->hasSelection()) {
        qWarning("EditorController::moveSelection: No selection to move");
        return;
    }
    
    if (offset.x == 0 && offset.y == 0 && offset.z == 0) {
        qDebug("EditorController::moveSelection: Zero offset, no movement needed");
        return;
    }
    
    // TODO: Implement MoveSelectionCommand
    qWarning("EditorController::moveSelection: Not yet implemented - offset (%d, %d, %d)",
             offset.x, offset.y, offset.z);
}

void EditorController::destroySelection() {
    if (!m_map || !m_selectionManager) {
        qWarning("EditorController::destroySelection: Map or selection manager is null");
        return;
    }
    
    if (!m_selectionManager->hasSelection()) {
        qWarning("EditorController::destroySelection: No selection to destroy");
        return;
    }
    
    // Use existing deleteSelection method which should handle this
    deleteSelection();
    
    qDebug() << "EditorController::destroySelection: Destroyed selection";
}

// --- Import/Export operations (LOGIC-09) ---
bool EditorController::importMap(const QString& filename, const RME::core::Position& offset) {
    if (!m_map) {
        qWarning("EditorController::importMap: Map is null");
        return false;
    }
    
    if (filename.isEmpty()) {
        qWarning("EditorController::importMap: Filename is empty");
        return false;
    }
    
    // TODO: Implement ImportMapCommand
    qWarning("EditorController::importMap: Not yet implemented - file: %s, offset: (%d, %d, %d)",
             qUtf8Printable(filename), offset.x, offset.y, offset.z);
    
    return false;
}

bool EditorController::exportMiniMap(const QString& filename, int floor, bool showDialog) {
    if (!m_map) {
        qWarning("EditorController::exportMiniMap: Map is null");
        return false;
    }
    
    if (filename.isEmpty()) {
        qWarning("EditorController::exportMiniMap: Filename is empty");
        return false;
    }
    
    // TODO: Implement minimap export functionality
    qWarning("EditorController::exportMiniMap: Not yet implemented - file: %s, floor: %d, dialog: %s",
             qUtf8Printable(filename), floor, showDialog ? "true" : "false");
    
    return false;
}

bool EditorController::exportSelectionAsMiniMap(const QString& filename) {
    if (!m_map || !m_selectionManager) {
        qWarning("EditorController::exportSelectionAsMiniMap: Map or selection manager is null");
        return false;
    }
    
    if (!m_selectionManager->hasSelection()) {
        qWarning("EditorController::exportSelectionAsMiniMap: No selection to export");
        return false;
    }
    
    if (filename.isEmpty()) {
        qWarning("EditorController::exportSelectionAsMiniMap: Filename is empty");
        return false;
    }
    
    // TODO: Implement selection minimap export functionality
    qWarning("EditorController::exportSelectionAsMiniMap: Not yet implemented - file: %s",
             qUtf8Printable(filename));
    
    return false;
}

// --- Ground validation operations (LOGIC-09) ---
quint32 EditorController::validateGroundStacks() {
    if (!m_map) {
        qWarning("EditorController::validateGroundStacks: Map is null");
        return 0;
    }
    
    QHash<QString, QVariant> parameters;
    parameters["validateStack"] = true;
    parameters["generateEmpty"] = false;
    parameters["removeDuplicates"] = false;
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::ValidateGrounds,
        m_map, this, parameters);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Validate ground stacks progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    // TODO: Return actual count from command
    quint32 modifiedCount = 0; // command->getModifiedTileCount();
    
    qDebug() << "EditorController::validateGroundStacks: Validated ground stacks, modified" << modifiedCount << "tiles";
    return modifiedCount;
}

quint32 EditorController::generateEmptySurroundedGrounds() {
    if (!m_map) {
        qWarning("EditorController::generateEmptySurroundedGrounds: Map is null");
        return 0;
    }
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::GenerateEmptyGrounds,
        m_map, this);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Generate empty grounds progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    // TODO: Return actual count from command
    quint32 generatedCount = 0; // command->getModifiedTileCount();
    
    qDebug() << "EditorController::generateEmptySurroundedGrounds: Generated" << generatedCount << "empty grounds";
    return generatedCount;
}

quint32 EditorController::removeDuplicateGrounds() {
    if (!m_map) {
        qWarning("EditorController::removeDuplicateGrounds: Map is null");
        return 0;
    }
    
    auto command = std::make_unique<RME::core::actions::MapWideOperationCommand>(
        RME::core::actions::MapWideOperationCommand::OperationType::RemoveDuplicateGrounds,
        m_map, this);
    
    command->setProgressCallback([](int progress, const QString& message) {
        qDebug() << "Remove duplicate grounds progress:" << progress << "%" << message;
    });
    
    pushCommand(std::move(command));
    
    // TODO: Return actual count from command
    quint32 removedCount = 0; // command->getModifiedTileCount();
    
    qDebug() << "EditorController::removeDuplicateGrounds: Removed" << removedCount << "duplicate grounds";
    return removedCount;
}

} // namespace editor_logic
} // namespace RME
