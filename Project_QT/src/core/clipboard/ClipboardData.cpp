#include "ClipboardData.h"

namespace RME {

// --- ClipboardItemData ---
QDataStream& operator<<(QDataStream& out, const ClipboardItemData& data) {
    out << data.id << data.subType << data.attributes;
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardItemData& data) {
    in >> data.id >> data.subType >> data.attributes;
    return in;
}

// --- ClipboardCreatureData ---
QDataStream& operator<<(QDataStream& out, const ClipboardCreatureData& data) {
    out << data.name; // Add other fields
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardCreatureData& data) {
    in >> data.name; // Add other fields
    return in;
}

// --- ClipboardSpawnData ---
QDataStream& operator<<(QDataStream& out, const ClipboardSpawnData& data) {
    out << data.radius << data.creatureNames; // Add other fields
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardSpawnData& data) {
    in >> data.radius >> data.creatureNames; // Add other fields
    return in;
}

// --- ClipboardTileData ---
QDataStream& operator<<(QDataStream& out, const ClipboardTileData& data) {
    out << data.relativePosition.x << data.relativePosition.y << data.relativePosition.z
        << data.hasGround << data.groundItemID << data.houseId << data.tileFlags
        << data.items
        << data.hasCreature;
    if (data.hasCreature) {
        out << data.creature;
    }
    out << data.hasSpawn;
    if (data.hasSpawn) {
        out << data.spawn;
    }
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardTileData& data) {
    in >> data.relativePosition.x >> data.relativePosition.y >> data.relativePosition.z
       >> data.hasGround >> data.groundItemID >> data.houseId >> data.tileFlags
       >> data.items
       >> data.hasCreature;
    if (data.hasCreature) {
        in >> data.creature;
    }
    in >> data.hasSpawn;
    if (data.hasSpawn) {
        in >> data.spawn;
    }
    return in;
}

// --- ClipboardContent ---
QDataStream& operator<<(QDataStream& out, const ClipboardContent& content) {
    out << content.tiles;
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardContent& content) {
    in >> content.tiles;
    return in;
}

} // namespace RME
