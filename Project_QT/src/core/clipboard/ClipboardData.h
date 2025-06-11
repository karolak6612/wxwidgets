#ifndef CLIPBOARDDATA_H
#define CLIPBOARDDATA_H

#include "Project_QT/src/core/Position.h" // Assuming Position is defined here or accessible
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
    // Add other relevant creature properties: outfit, flags, etc.
    // For simplicity, starting with name.
    // int lookType;
    // int lookHead, lookBody, lookLegs, lookFeet, lookAddons;

    ClipboardCreatureData() = default;
};
QDataStream& operator<<(QDataStream& out, const ClipboardCreatureData& data);
QDataStream& operator>>(QDataStream& in, ClipboardCreatureData& data);

// Basic data for a spawn on the clipboard
struct ClipboardSpawnData {
    uint16_t radius;
    QList<QString> creatureNames; // List of creature names in this spawn
    // Add other spawn properties

    ClipboardSpawnData() : radius(1) {}
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
