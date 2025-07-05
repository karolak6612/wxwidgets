#ifndef RME_EDITOR_CONTROLLER_INTERFACE_H
#define RME_EDITOR_CONTROLLER_INTERFACE_H

#include <memory> // For std::unique_ptr
#include <QList> // For QList

// Forward declarations for types used by controller methods
namespace RME {
    namespace core {
        class Tile;
        struct Position;
        class AppSettings; // For accessing global settings
        namespace map { class Map; }
        namespace assets {
            struct CreatureData;
            class CreatureDatabase;
            class AssetManager; // General asset provider
        }
        namespace actions { class AppUndoCommand; } // Base for undo commands
        class SpawnData; // For spawn management
    }
}


namespace RME {
namespace core {
namespace editor { // Ensured namespace consistency

class EditorControllerInterface {
public:
    virtual ~EditorControllerInterface() = default;

    // --- Map Access ---
    virtual RME::core::map::Map* getMap() = 0;
    virtual const RME::core::map::Map* getMap() const = 0;
    virtual RME::core::Tile* getTileForEditing(const RME::core::Position& pos) = 0; // Gets or creates if necessary, marks for update

    // --- Settings & Asset Access ---
    virtual RME::core::AppSettings* getAppSettings() = 0;
    virtual RME::core::assets::AssetManager* getAssetManager() = 0; // MODIFIED


    // --- Undoable Action Recording ---
    // General way to record any command
    virtual void recordAction(std::unique_ptr<RME::core::actions::AppUndoCommand> command) = 0;

    // Specific convenience methods for common actions (might internally create and record commands)
    // For Tile changes (generic)
    virtual void recordTileChange(const RME::core::Position& pos,
                                  std::unique_ptr<RME::core::Tile> oldTileState,
                                  std::unique_ptr<RME::core::Tile> newTileState) = 0;

    // For Creature specific changes
    virtual void recordAddCreature(const RME::core::Position& tilePos,
                                   const RME::core::assets::CreatureData* creatureType) = 0;
    virtual void recordRemoveCreature(const RME::core::Position& tilePos,
                                      const RME::core::assets::CreatureData* creatureType) = 0;
    // Note: Spawn time is part of SpawnData, not the creature instance here.

    // For SpawnData changes
    virtual void recordAddSpawn(const RME::core::spawns::Spawn& spawn) = 0;
    virtual void recordRemoveSpawn(const RME::core::Position& spawnCenterPos) = 0;
    virtual void recordUpdateSpawn(const RME::core::Position& spawnCenterPos,
                                   const RME::core::spawns::Spawn& oldSpawn,
                                   const RME::core::spawns::Spawn& newSpawn) = 0;

    // --- Tile Content Specific Actions ---
    virtual void recordSetGroundItem(const RME::core::Position& pos, uint16_t newGroundItemId, uint16_t oldGroundItemId) = 0;
    virtual void recordSetBorderItems(const RME::core::Position& pos, const QList<uint16_t>& newBorderItemIds, const QList<uint16_t>& oldBorderItemIds) = 0;

    // --- Generic Item Manipulation Actions ---
    // Adds an item to the top of the stack at the given position.
    virtual void recordAddItem(const RME::core::Position& pos, uint16_t itemId) = 0; // NEW

    // Removes the topmost item matching the given ID at the position.
    virtual void recordRemoveItem(const RME::core::Position& pos, uint16_t itemId) = 0; // NEW

    // --- Notifications ---
    virtual void notifyTileChanged(const RME::core::Position& pos) = 0; // If map needs explicit notification
};

} // namespace editor
} // namespace core
} // namespace RME

#endif // RME_EDITOR_CONTROLLER_INTERFACE_H
