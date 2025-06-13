#include "editor_logic/EditorController.h"
#include "core/map/Map.h"
#include "core/brush/Brush.h"
#include "core/brush/BrushManager.h"
#include "core/selection/SelectionManager.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h"
#include "core/assets/AssetManager.h"
#include "editor_logic/commands/DeleteSelectionCommand.h" // Required for deleteSelection
#include "core/Tile.h" // For getTileForEditing, notifyTileChanged
#include "core/SpawnData.h" // For record methods
#include "core/assets/CreatureData.h" // For record methods, ensure this is the correct header for CreatureData struct/class
#include "core/actions/AppUndoCommand.h" // For recordAction

#include <QUndoStack>
#include <QUndoCommand> // For pushCommand std::unique_ptr
#include <QList>
#include <QDebug>

namespace RME {
namespace editor_logic {

EditorController::EditorController(
    RME::core::Map* map,
    QUndoStack* undoStack,
    RME::core::selection::SelectionManager* selectionManager,
    RME::core::brush::BrushManager* brushManager,
    RME::core::settings::AppSettings* appSettings,
    RME::core::assets::AssetManager* assetManager
) : m_map(map),
    m_undoStack(undoStack),
    m_selectionManager(selectionManager),
    m_brushManager(brushManager),
    m_appSettings(appSettings),
    m_assetManager(assetManager)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_undoStack);
    Q_ASSERT(m_selectionManager);
    Q_ASSERT(m_brushManager);
    Q_ASSERT(m_appSettings);
    Q_ASSERT(m_assetManager);
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

void EditorController::recordAddCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) {
    qWarning("EditorController::recordAddCreature: Not implemented. Pos: (%d,%d,%d) Creature: %s",
        tilePos.x, tilePos.y, tilePos.z, creatureType ? qUtf8Printable(creatureType->name) : "null");
}

void EditorController::recordRemoveCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) {
    qWarning("EditorController::recordRemoveCreature: Not implemented. Pos: (%d,%d,%d) Creature: %s",
        tilePos.x, tilePos.y, tilePos.z, creatureType ? qUtf8Printable(creatureType->name) : "null");
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

} // namespace editor_logic
} // namespace RME
