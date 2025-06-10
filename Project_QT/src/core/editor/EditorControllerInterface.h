#ifndef RME_EDITOR_CONTROLLER_INTERFACE_H
#define RME_EDITOR_CONTROLLER_INTERFACE_H

// Forward declarations for types used by controller methods
namespace RME {
    class Map;
    class Tile;
    struct Position;
    // Potentially Item, Creature, etc.
}

namespace RME {

// Placeholder interface for editor operations that brushes might need to invoke.
// This allows decoupling brushes from concrete editor implementation.
// Methods will be added as specific brush logic requires them.
class EditorControllerInterface {
public:
    virtual ~EditorControllerInterface() = default;

    // Example methods brushes might need (to be defined by actual controller later):
    // virtual RME::Map* getCurrentMap() = 0;
    // virtual const RME::Map* getCurrentMap() const = 0;
    // virtual RME::Tile* getTile(const Position& pos) = 0; // Get or create tile
    // virtual bool canPlaceItem(const Position& pos, const ItemType* itemType) = 0;
    // virtual void addItemToTile(const Position& pos, std::unique_ptr<Item> item) = 0;
    // virtual void recordAction(std::unique_ptr<Action> action) = 0; // For undo/redo
    // virtual AssetManager* getAssetManager() = 0; // For brushes to get item/creature data
    // virtual AppSettings* getSettings() = 0; // For brush behavior based on settings
};

} // namespace RME

#endif // RME_EDITOR_CONTROLLER_INTERFACE_H
