#include "core/data_transfer/TileData.h"
#include "core/Tile.h" // Full definition of Tile
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include <QDebug> // For Q_ASSERT

namespace RME {
namespace core {
namespace data_transfer {

// Deep copy constructor
TileData::TileData(const TileData& other)
    : position(other.position),
      waypointCount(other.waypointCount),
      houseId(other.houseId),             // Added
      isHouseExit(other.isHouseExit),       // Added
      isProtectionZone(other.isProtectionZone) // Added
{
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item_ptr : other.items) {
        if (item_ptr) items.append(item_ptr->deepCopy());
    }
    if (other.spawn) spawn = other.spawn->deepCopy();
    if (other.creature) creature = other.creature->deepCopy();
    // waypointCount is handled by initializer list
}

// Deep copy assignment operator
TileData& TileData::operator=(const TileData& other) {
    if (this == &other) return *this;

    position = other.position;
    waypointCount = other.waypointCount;
    houseId = other.houseId;                   // Added
    isHouseExit = other.isHouseExit;             // Added
    isProtectionZone = other.isProtectionZone;   // Added
    ground.reset();
    if (other.ground) ground = other.ground->deepCopy();

    items.clear();
    for (const auto& item_ptr : other.items) {
        if (item_ptr) items.append(item_ptr->deepCopy());
    }

    spawn.reset();
    if (other.spawn) spawn = other.spawn->deepCopy();

    creature.reset();
    if (other.creature) creature = other.creature->deepCopy();

    return *this;
}

TileData TileData::fromTile(const RME::core::Tile* tile) {
    Q_ASSERT(tile != nullptr);
    TileData data(tile->getPosition());

    if (tile->getGround()) {
        data.ground = tile->getGround()->deepCopy();
    }
    for (const auto& item_ptr : tile->getItems()) {
        if (item_ptr) {
            data.items.append(item_ptr->deepCopy());
        }
    }
    if (tile->getSpawn()) {
        data.spawn = tile->getSpawn()->deepCopy();
    }
    if (tile->getCreature()) {
        data.creature = tile->getCreature()->deepCopy();
    }
    data.waypointCount = tile->getWaypointCount();
    data.houseId = tile->getHouseId();                   // Added
    data.isHouseExit = tile->isHouseExit();             // Added
    data.isProtectionZone = tile->isProtectionZone();   // Added
    return data;
}

void TileData::applyToTile(RME::core::Tile* targetTile) const {
    Q_ASSERT(targetTile != nullptr);

    // Clear existing contents of the target tile first
    targetTile->setGround(nullptr);
    targetTile->clearItems();
    targetTile->setSpawn(nullptr);
    targetTile->setCreature(nullptr);
    // Assuming direct access to m_waypointCount or a setter like setWaypointCount(0) for reset
    // For now, using direct access as per TileData.cpp's current m_waypointCount logic.
    // If Tile::m_waypointCount is private, this needs a public setter on Tile.
    targetTile->m_waypointCount = 0; // Reset before applying new count.

    // Apply new settings from TileData
    targetTile->setHouseId(this->houseId);                // Added
    targetTile->setIsHouseExit(this->isHouseExit);          // Added
    targetTile->setIsProtectionZone(this->isProtectionZone); // Added
    // Note: If setIsProtectionZone should also manage TileMapFlag, that logic is in Tile setter.

    // Apply new items from TileData
    if (this->ground) {
        targetTile->setGround(this->ground->deepCopy());
    }
    for (const auto& item_ptr : this->items) {
        if (item_ptr) {
            targetTile->addItem(item_ptr->deepCopy());
        }
    }
    if (this->spawn) {
        targetTile->setSpawn(this->spawn->deepCopy());
    }
    if (this->creature) {
        targetTile->setCreature(this->creature->deepCopy());
    }
    // Apply waypoint count - assuming direct access or a public setter on Tile
    targetTile->m_waypointCount = this->waypointCount;
}

} // namespace data_transfer
} // namespace core
} // namespace RME
