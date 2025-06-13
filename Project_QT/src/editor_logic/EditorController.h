#ifndef RME_EDITOR_CONTROLLER_H
#define RME_EDITOR_CONTROLLER_H

#include "core/editor/EditorControllerInterface.h" // Base interface
#include "core/Position.h"
#include <QList>
#include <QString>
#include <memory> // For std::unique_ptr
#include <QtGlobal> // For Qt::KeyboardModifiers

// Forward declarations
class QUndoStack;
class QUndoCommand;

namespace RME {
namespace core {
    class Map;
    class BrushSettings;
    // class Item; // Not directly used in method signatures here
    namespace brush { class BrushManager; }
    namespace selection { class SelectionManager; }
    namespace settings { class AppSettings; }
    namespace assets { class AssetManager; }
    class Tile; // For getTileForEditing
    class SpawnData; // For recordAddSpawn etc.
    struct CreatureData; // For recordAddCreature etc. (often a struct in assets namespace)
    namespace actions { class AppUndoCommand; } // For recordAction
}
}

namespace RME {
namespace editor_logic {

class EditorController : public RME::core::editor::EditorControllerInterface {
public:
    EditorController(
        RME::core::Map* map,
        QUndoStack* undoStack,
        RME::core::selection::SelectionManager* selectionManager,
        RME::core::brush::BrushManager* brushManager,
        RME::core::settings::AppSettings* appSettings,
        RME::core::assets::AssetManager* assetManager
    );
    ~EditorController() override = default;

    // --- Core editing operations defined in this class ---
    void applyBrushStroke(const QList<RME::core::Position>& positions, const RME::core::BrushSettings& settings);
    void deleteSelection();
    void clearCurrentSelection();
    void performBoundingBoxSelection(const RME::core::Position& p1, const RME::core::Position& p2, Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& currentBrushSettings);

    // --- Implementation of EditorControllerInterface ---
    // Basic accessors
    RME::core::Map* getMap() override;
    const RME::core::Map* getMap() const override;
    QUndoStack* getUndoStack() override;
    RME::core::assets::AssetManager* getAssetManager() override;
    const RME::core::assets::AssetManager* getAssetManager() const override;
    RME::core::settings::AppSettings& getAppSettings() override; // Returns reference
    const RME::core::settings::AppSettings& getAppSettings() const override; // Returns const reference

    // Command handling
    void pushCommand(std::unique_ptr<QUndoCommand> command) override;

    // Tile/Notification related methods (likely from interface, used by brushes/commands)
    void notifyTileChanged(const RME::core::Position& pos) override;
    RME::core::Tile* getTileForEditing(const RME::core::Position& pos) override;

    // Record methods from EditorControllerInterface
    // These should be implemented, possibly by creating specific commands or a generic one.
    // For now, they are just declarations matching a plausible interface.
    void recordAction(std::unique_ptr<RME::core::actions::AppUndoCommand> command) override;
    void recordTileChange(const RME::core::Position& pos,
                          std::unique_ptr<RME::core::Tile> oldTileState,
                          std::unique_ptr<RME::core::Tile> newTileState) override;
    void recordAddCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) override;
    void recordRemoveCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) override;
    void recordAddSpawn(const RME::core::SpawnData& spawnData) override;
    void recordRemoveSpawn(const RME::core::Position& spawnCenterPos) override;
    void recordUpdateSpawn(const RME::core::Position& spawnCenterPos,
                           const RME::core::SpawnData& oldSpawnData,
                           const RME::core::SpawnData& newSpawnData) override;
    void recordSetGroundItem(const RME::core::Position& pos, uint16_t newGroundItemId, uint16_t oldGroundItemId) override;
    void recordSetBorderItems(const RME::core::Position& pos,
                              const QList<uint16_t>& newBorderItemIds,
                              const QList<uint16_t>& oldBorderItemIds) override;
    void recordAddItem(const RME::core::Position& pos, uint16_t itemId) override;
    void recordRemoveItem(const RME::core::Position& pos, uint16_t itemId) override;

private:
    RME::core::Map* m_map;
    QUndoStack* m_undoStack;
    RME::core::selection::SelectionManager* m_selectionManager;
    RME::core::brush::BrushManager* m_brushManager;
    RME::core::settings::AppSettings* m_appSettings;
    RME::core::assets::AssetManager* m_assetManager;
};

} // namespace editor_logic
} // namespace RME

#endif // RME_EDITOR_CONTROLLER_H
