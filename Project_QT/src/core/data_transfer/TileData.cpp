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
    : position(other.position)
{
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item_ptr : other.items) {
        if (item_ptr) items.append(item_ptr->deepCopy());
    }
    if (other.spawn) spawn = other.spawn->deepCopy();
    if (other.creature) creature = other.creature->deepCopy();
}

// Deep copy assignment operator
TileData& TileData::operator=(const TileData& other) {
    if (this == &other) return *this;

    position = other.position;
    ground.reset();
    if (other.ground) ground = other.ground->deepCopy();

    items.clear(); // QList<unique_ptr> needs careful clear or item-by-item reset then clear
                   // The default QList::clear() will call destructors of unique_ptr, which is correct.
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
    for (const auto& item_ptr : tile->getItems()) { // Assuming getItems() returns const QList<std::unique_ptr<Item>>&
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
    return data;
}

void TileData::applyToTile(RME::core::Tile* targetTile) const {
    Q_ASSERT(targetTile != nullptr);
    // Optional: Q_ASSERT(targetTile->getPosition() == this->position);
    // This assert might be too strict if TileData is used for applying to a *different* position (template/stamp)

    // Clear existing contents of the target tile first
    targetTile->setGround(nullptr); // This should release old ground if Tile uses unique_ptr
    targetTile->clearItems(); // Assumes Tile::clearItems() exists and properly deletes old items
    targetTile->setSpawn(nullptr);    // Releases old spawn
    targetTile->setCreature(nullptr); // Releases old creature

    // Apply new contents from TileData (deep copies to ensure TileData still owns its copies)
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
}

} // namespace data_transfer
} // namespace core
} // namespace RME
