#ifndef CLIPBOARDDATA_H
#define CLIPBOARDDATA_H

#include "core/Position.h" // Assuming Position is defined here or accessible
#include <QString>
#include <QList>
#include <QDataStream>
#include <QVariantMap> // For flexible attribute storage

// Forward declarations if necessary (e.g., for ItemID types)
// class ItemType; // If ItemType is a class

namespace RME {

// Basic data for an item on the clipboard
struct ClipboardItemData {
    uint16_t id; // Server Item ID
    uint8_t subType; // For fluids, splash, count, etc.
    QVariantMap attributes; // For other specific item attributes (actionID, text, etc.)
    // Consider stack order if relevant for paste

    ClipboardItemData() : id(0), subType(1) {}
};
QDataStream& operator<<(QDataStream& out, const ClipboardItemData& data);
QDataStream& operator>>(QDataStream& in, ClipboardItemData& data);

// Basic data for a creature on the clipboard
struct ClipboardCreatureData {
    QString name;
    
    // Outfit data
    uint16_t lookType;
    uint8_t head;
    uint8_t body;
    uint8_t legs;
    uint8_t feet;
    uint8_t addons;
    uint16_t mount;
    
    // Direction and other properties
    uint8_t direction; // 0=North, 1=East, 2=South, 3=West
    bool isNpc;
    
    // Additional creature attributes
    QVariantMap attributes; // For custom properties, scripts, etc.

    ClipboardCreatureData() : lookType(0), head(0), body(0), legs(0), feet(0), addons(0), mount(0), direction(2), isNpc(false) {}
};
QDataStream& operator<<(QDataStream& out, const ClipboardCreatureData& data);
QDataStream& operator>>(QDataStream& in, ClipboardCreatureData& data);

// Forward declaration for unified spawn class
namespace core { namespace spawns { class Spawn; } }

// Basic data for a spawn on the clipboard
struct ClipboardSpawnData {
    uint16_t radius;
    QList<QString> creatureNames; // List of creature names in this spawn
    
    // Spawn timing and behavior
    uint32_t spawnTime; // Respawn time in seconds
    uint32_t despawnRange; // Despawn range
    uint32_t despawnRadius; // Despawn radius
    
    // Creature spawn data with chances
    struct CreatureSpawnEntry {
        QString name;
        uint32_t chance; // Spawn chance (0-100000)
        uint32_t max; // Maximum number of this creature type
        
        CreatureSpawnEntry() : chance(100), max(1) {}
        CreatureSpawnEntry(const QString& n, uint32_t c = 100, uint32_t m = 1) : name(n), chance(c), max(m) {}
    };
    QList<CreatureSpawnEntry> creatures;
    
    // Additional spawn attributes
    QVariantMap attributes; // For custom properties, scripts, etc.

    ClipboardSpawnData() : radius(1), spawnTime(60), despawnRange(2), despawnRadius(1) {}
};
QDataStream& operator<<(QDataStream& out, const ClipboardSpawnData& data);
QDataStream& operator>>(QDataStream& in, ClipboardSpawnData& data);

// Represents a single tile's content for the clipboard
struct ClipboardTileData {
    Position relativePosition; // Position relative to the top-left of the copied selection

    // Ground tile properties (if ground was selected/part of the tile)
    bool hasGround;
    uint16_t groundItemID; // If ground is represented by an item ID (e.g. in some OT formats)
                           // Alternatively, material ID if using a material system.
    uint32_t houseId;
    uint32_t tileFlags; // From Tile::getMapFlags() in wxWidgets
    // Zone IDs might be complex, store as needed by your zone system.

    QList<ClipboardItemData> items;
    ClipboardCreatureData creature; // Assuming at most one creature per tile
    bool hasCreature;
    ClipboardSpawnData spawn;       // Assuming at most one spawn per tile
    bool hasSpawn;


    ClipboardTileData() : hasGround(false), groundItemID(0), houseId(0), tileFlags(0), hasCreature(false), hasSpawn(false) {}
};
QDataStream& operator<<(QDataStream& out, const ClipboardTileData& data);
QDataStream& operator>>(QDataStream& in, ClipboardTileData& data);

// Container for all clipboard data (list of tiles and original top-left pos)
struct ClipboardContent {
    QList<ClipboardTileData> tiles;
    // Position originalTopLeft; // Could be useful for some paste modes, but relative positions handle most cases.
                                // The wxWidgets version stored copyPos in the CopyBuffer class, not in the data itself.
                                // We will calculate copyPos at copy time and use it to make positions relative.
    ClipboardContent() = default;
};
QDataStream& operator<<(QDataStream& out, const ClipboardContent& content);
QDataStream& operator>>(QDataStream& in, ClipboardContent& content);


// Define your custom MIME type
const QString RME_CLIPBOARD_MIME_TYPE = "application/x-rme-map-fragment";

} // namespace RME

#endif // CLIPBOARDDATA_H
