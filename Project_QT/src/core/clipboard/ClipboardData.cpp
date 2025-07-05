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
    out << data.name << data.lookType << data.head << data.body << data.legs 
        << data.feet << data.addons << data.mount << data.direction << data.isNpc << data.attributes;
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardCreatureData& data) {
    in >> data.name >> data.lookType >> data.head >> data.body >> data.legs 
       >> data.feet >> data.addons >> data.mount >> data.direction >> data.isNpc >> data.attributes;
    return in;
}

// --- ClipboardSpawnData ---
QDataStream& operator<<(QDataStream& out, const ClipboardSpawnData& data) {
    out << data.radius << data.creatureNames << data.spawnTime << data.despawnRange 
        << data.despawnRadius << data.attributes;
    
    // Serialize creature spawn entries
    out << static_cast<quint32>(data.creatures.size());
    for (const auto& creature : data.creatures) {
        out << creature.name << creature.chance << creature.max;
    }
    
    return out;
}
QDataStream& operator>>(QDataStream& in, ClipboardSpawnData& data) {
    in >> data.radius >> data.creatureNames >> data.spawnTime >> data.despawnRange 
       >> data.despawnRadius >> data.attributes;
    
    // Deserialize creature spawn entries
    quint32 creatureCount;
    in >> creatureCount;
    data.creatures.clear();
    data.creatures.reserve(creatureCount);
    
    for (quint32 i = 0; i < creatureCount; ++i) {
        ClipboardSpawnData::CreatureSpawnEntry entry;
        in >> entry.name >> entry.chance >> entry.max;
        data.creatures.append(entry);
    }
    
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
