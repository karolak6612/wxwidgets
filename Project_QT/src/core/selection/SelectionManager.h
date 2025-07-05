#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject> // For Q_OBJECT macro if signals/slots are needed later
#include <QSet>
#include <QString>

// Forward declarations (reduce header dependencies)
namespace RME { namespace core { class Tile; }} // More explicit
class Item; // Assuming RME::core::Item
class Creature;
class Spawn;
class QUndoStack; // For later integration
class QUndoCommand; // For later integration
class Map;        // Assuming Map class exists and is needed for context

namespace RME {

class SelectionCommand; // Forward declaration

class SelectionManager : public QObject {
    Q_OBJECT
    friend class SelectionCommand; // Allow SelectionCommand to access private members like m_selectedTiles

signals: // Added signals section
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
    void addTile(Tile* tile);
    void removeTile(Tile* tile);
    void addItem(Tile* tile, Item* item);
    void removeItem(Tile* tile, Item* item);
    void addCreature(Tile* tile, Creature* creature);
    void removeCreature(Tile* tile, Creature* creature);
    void addSpawn(Tile* tile, Spawn* spawn);
    void removeSpawn(Tile* tile, Spawn* spawn);

    void toggleTileSelection(Tile* tile);
    void toggleItemSelection(Tile* tile, Item* item);
    void toggleCreatureSelection(Tile* tile, Creature* creature);
    void toggleSpawnSelection(Tile* tile, Spawn* spawn);

    void clear(); // Clears all current selection

    // Methods to query selection state
    bool isSelected(const Tile* tile) const;
    bool isSelected(const Tile* tile, const Item* item) const;
    bool isSelected(const Tile* tile, const Creature* creature) const;
    bool isSelected(const Tile* tile, const Spawn* spawn) const;

    const QSet<Tile*>& getSelectedTiles() const;
    // Later, might add methods to get selected items, creatures, spawns directly

    // Utility to check if a selection change session is active
    bool isSelectionChangeActive() const;

    // Internal methods for direct state manipulation by commands
    void clearSelectionInternal();
    void addTilesToSelectionInternal(const QList<RME::core::Tile*>& tilesToSelect);
    void removeTilesFromSelectionInternal(const QList<RME::core::Tile*>& tilesToDeselect);
    void setSelectedTilesInternal(const QList<RME::core::Tile*>& tilesToSelect);

    // Getter for current selection state (primarily tiles considered selected)
    QList<RME::core::Tile*> getCurrentSelectedTilesList() const;

private:
    // Internal helper methods
    void recordTileSelectionChange(Tile* tile, bool select); // select = true to select, false to deselect
    void recordItemSelectionChange(Tile* tile, Item* item, bool select);
    void recordCreatureSelectionChange(Tile* tile, Creature* creature, bool select);
    void recordSpawnSelectionChange(Tile* tile, Spawn* spawn, bool select);

    // Data members
    Map* m_map; // Non-owning pointer to the map
    QUndoStack* m_undoStack; // Non-owning pointer to the undo stack

    QSet<Tile*> m_selectedTiles; // Tiles that contain at least one selected element or are fully selected

    // Temporary storage for changes during a "selection change session"
    // These will be used to create the SelectionCommand
    struct SelectionChange {
        Tile* tile;
        // Pointers to specific items/creatures/spawns can be null if the whole tile is affected
        Item* item;
        Creature* creature;
        Spawn* spawn;
        bool previousState; // Was it selected before this change?
        bool currentState;  // Is it selected now after this change?

        enum class TargetType { TILE, ITEM, CREATURE, SPAWN };
        TargetType type;

        SelectionChange(Tile* t, bool prev, bool curr) : tile(t), item(nullptr), creature(nullptr), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::TILE) {}
        SelectionChange(Tile* t, Item* i, bool prev, bool curr) : tile(t), item(i), creature(nullptr), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::ITEM) {}
        SelectionChange(Tile* t, Creature* c, bool prev, bool curr) : tile(t), item(nullptr), creature(c), spawn(nullptr), previousState(prev), currentState(curr), type(TargetType::CREATURE) {}
        SelectionChange(Tile* t, Spawn* s, bool prev, bool curr) : tile(t), item(nullptr), creature(nullptr), spawn(s), previousState(prev), currentState(curr), type(TargetType::SPAWN) {}
    };
    QList<SelectionChange> m_pendingChanges;
    bool m_selectionChangeActive;

    // Placeholder for the actual SelectionCommand - this will be properly defined in Step 2
    // For now, SelectionManager will accumulate changes in m_pendingChanges.
    // When SelectionCommand is ready, finishSelectionChange will use m_pendingChanges to initialize it.
};

} // namespace RME

#endif // SELECTIONMANAGER_H
