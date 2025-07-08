#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject> // For Q_OBJECT macro if signals/slots are needed later
#include <QSet>
#include <QString>
#include "core/Position.h" // For Position class

// Forward declarations (reduce header dependencies)
namespace RME { 
    class Tile;
    class Item;
    namespace core {
        namespace creatures {
            class Creature;
        }
        namespace spawns {
            class Spawn;
        }
    }
}
class QUndoStack; // For later integration
class QUndoCommand; // For later integration
class Map;        // Assuming Map class exists and is needed for context

namespace RME {

class SelectionCommand; // Forward declaration

class SelectionManager : public QObject {
    Q_OBJECT
    friend class SelectionCommand; // Allow SelectionCommand to access private members like m_selectedTiles

signals:
    void selectionChanged();

public:
    explicit SelectionManager(Map* map, QUndoStack* undoStack, QObject *parent = nullptr);
    ~SelectionManager();

    // Session management for grouping selection changes into one undo command
    void startSelectionChange();
    // If commandText is empty, a default one will be generated.
    void finishSelectionChange(const QString& commandText = "");

    // Methods to modify selection
    // These will internally record changes to be put into a SelectionCommand
    void addTile(RME::Tile* tile);
    void removeTile(RME::Tile* tile);
    void addItem(RME::Tile* tile, RME::Item* item);
    void removeItem(RME::Tile* tile, RME::Item* item);
    void addCreature(RME::Tile* tile, RME::core::creatures::Creature* creature);
    void removeCreature(RME::Tile* tile, RME::core::creatures::Creature* creature);
    void addSpawn(RME::Tile* tile, RME::core::spawns::Spawn* spawn);
    void removeSpawn(RME::Tile* tile, RME::core::spawns::Spawn* spawn);

    void toggleTileSelection(RME::Tile* tile);
    void toggleItemSelection(RME::Tile* tile, RME::Item* item);
    void toggleCreatureSelection(RME::Tile* tile, RME::core::creatures::Creature* creature);
    void toggleSpawnSelection(RME::Tile* tile, RME::core::spawns::Spawn* spawn);

    void clear(); // Clears all current selection

    // Methods to query selection state
    bool isSelected(const RME::Tile* tile) const;
    bool isSelected(const RME::Tile* tile, const RME::Item* item) const;
    bool isSelected(const RME::Tile* tile, const RME::core::creatures::Creature* creature) const;
    bool isSelected(const RME::Tile* tile, const RME::core::spawns::Spawn* spawn) const;
    
    // Convenience methods for clipboard integration
    bool isItemSelected(const RME::Tile* tile, const RME::Item* item) const;
    bool isCreatureSelected(const RME::Tile* tile, const RME::core::creatures::Creature* creature) const;
    bool isSpawnSelected(const RME::Tile* tile, const RME::core::spawns::Spawn* spawn) const;

    const QSet<RME::Tile*>& getSelectedTiles() const;
    // Later, might add methods to get selected items, creatures, spawns directly

    // Utility to check if a selection change session is active
    bool isSelectionChangeActive() const;

    // Internal methods for direct state manipulation by commands
    void clearSelectionInternal();
    void addTilesToSelectionInternal(const QList<RME::Tile*>& tilesToSelect);
    void removeTilesFromSelectionInternal(const QList<RME::Tile*>& tilesToDeselect);
    void setSelectedTilesInternal(const QList<RME::Tile*>& tilesToSelect);

    // Getter for current selection state (primarily tiles considered selected)
    QList<RME::Tile*> getCurrentSelectedTilesList() const;
    
    // Position-based selection methods for easier integration
    QList<RME::core::Position> getSelectedPositions() const;
    void setSelectedPositions(const QList<RME::core::Position>& positions);
    void clearSelection();
    bool hasSelection() const;

private:
    // Internal helper methods
    void recordTileSelectionChange(RME::Tile* tile, bool select); // select = true to select, false to deselect
    void recordItemSelectionChange(RME::Tile* tile, RME::Item* item, bool select);
    void recordCreatureSelectionChange(RME::Tile* tile, RME::core::creatures::Creature* creature, bool select);
    void recordSpawnSelectionChange(RME::Tile* tile, RME::core::spawns::Spawn* spawn, bool select);

    // Data members
    Map* m_map; // Non-owning pointer to the map
    QUndoStack* m_undoStack; // Non-owning pointer to the undo stack

    QSet<RME::Tile*> m_selectedTiles; // Tiles that contain at least one selected element or are fully selected

    // Temporary storage for changes during a "selection change session"
    // These will be used to create the SelectionCommand
    struct SelectionChange {
        RME::Tile* tile;
        // Pointers to specific items/creatures/spawns can be null if the whole tile is affected
        RME::Item* item;
        RME::core::creatures::Creature* creature;
        RME::core::spawns::Spawn* spawn;
        bool previousState; // Was it selected before this change?
        bool currentState;  // Is it selected now after this change?

        enum class TargetType { TILE, ITEM, CREATURE, SPAWN };
        TargetType type;

        SelectionChange(RME::Tile* t, bool prev, bool curr) : tile(t), item(nullptr), creature(nullptr), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::TILE) {}
        SelectionChange(RME::Tile* t, RME::Item* i, bool prev, bool curr) : tile(t), item(i), creature(nullptr), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::ITEM) {}
        SelectionChange(RME::Tile* t, RME::core::creatures::Creature* c, bool prev, bool curr) : tile(t), item(nullptr), creature(c), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::CREATURE) {}
        SelectionChange(RME::Tile* t, RME::core::spawns::Spawn* s, bool prev, bool curr) : tile(t), item(nullptr), creature(nullptr), spawn(s), previousState(prev), currentState(curr), type(TargetType::SPAWN) {}
    };
    QList<SelectionChange> m_pendingChanges;
    bool m_selectionChangeActive;

    // Placeholder for the actual SelectionCommand - this will be properly defined in Step 2
    // For now, SelectionManager will accumulate changes in m_pendingChanges.
    // When SelectionCommand is ready, finishSelectionChange will use m_pendingChanges to initialize it.
};

} // namespace RME

#endif // SELECTIONMANAGER_H
