#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject> // For Q_OBJECT if signals/slots are needed later
#include "ClipboardData.h" // For ClipboardContent etc.

// Forward declarations
class QUndoStack;
namespace RME {
class SelectionManager;
class Map;
class Tile; // For iterating selection
class Item;
namespace core {
    namespace creatures {
        class Creature;
    }
    namespace spawns {
        class SpawnData;
    }
}
}

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    explicit ClipboardManager(QObject *parent = nullptr);

    // Copies the current selection from SelectionManager to the system clipboard
    void copySelection(const RME::SelectionManager& selectionManager, const RME::Map& map);

    // Cuts the current selection: copies then creates a delete command
    void cutSelection(RME::SelectionManager& selectionManager, RME::Map& map, QUndoStack* undoStack);

    // Pastes data from the system clipboard to the map at targetPosition
    void paste(RME::Map& map, const RME::Position& targetPosition, QUndoStack* undoStack);

    bool canPaste() const;
    
    // Advanced clipboard operations
    QString getClipboardStatistics() const;
    bool validateClipboardData() const;
    void compressClipboardData();
    
    // Clipboard analysis
    struct ClipboardStats {
        int totalTiles;
        int totalItems;
        int totalCreatures;
        int totalSpawns;
        int uniqueItemTypes;
        int uniqueCreatureTypes;
        QSize boundingBox;
        QString formatVersion;
    };
    ClipboardStats analyzeClipboardData() const;

private:
    // Helper to retrieve and deserialize data for pasting
    // Returns empty ClipboardContent if paste is not possible or data is invalid.
    RME::ClipboardContent getPasteData() const;
    
    // Helper methods for creating clipboard data
    RME::ClipboardItemData createItemClipboardData(const RME::Item* item) const;
    RME::ClipboardCreatureData createCreatureClipboardData(const RME::core::creatures::Creature* creature) const;
    RME::ClipboardSpawnData createSpawnClipboardData(const RME::Tile* tile) const;
};

#endif // CLIPBOARDMANAGER_H
