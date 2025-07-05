#include "ItemTypeProvider.h"
#include "assets/ItemDatabase.h"
#include "assets/ItemData.h"
#include <QDebug>

namespace RME {
namespace core {

ItemTypeProvider::ItemTypeProvider(const assets::ItemDatabase* itemDatabase)
    : m_itemDatabase(itemDatabase)
{
    if (!m_itemDatabase) {
        qWarning() << "ItemTypeProvider: ItemDatabase is null";
    }
}

bool ItemTypeProvider::isWalkable(uint16_t itemId) const {
    auto it = m_walkableCache.find(itemId);
    if (it != m_walkableCache.end()) {
        return it.value();
    }
    
    const assets::ItemData* itemData = getItemData(itemId);
    bool walkable = itemData ? !itemData->isBlocking : true; // Default to walkable if no data
    
    cacheBoolProperty(m_walkableCache, itemId, walkable);
    return walkable;
}

bool ItemTypeProvider::isBlocking(uint16_t itemId) const {
    auto it = m_blockingCache.find(itemId);
    if (it != m_blockingCache.end()) {
        return it.value();
    }
    
    const assets::ItemData* itemData = getItemData(itemId);
    bool blocking = itemData ? itemData->isBlocking : false;
    
    cacheBoolProperty(m_blockingCache, itemId, blocking);
    return blocking;
}

bool ItemTypeProvider::isContainer(uint16_t itemId) const {
    auto it = m_containerCache.find(itemId);
    if (it != m_containerCache.end()) {
        return it.value();
    }
    
    const assets::ItemData* itemData = getItemData(itemId);
    bool container = itemData ? itemData->isContainer : false;
    
    cacheBoolProperty(m_containerCache, itemId, container);
    return container;
}

bool ItemTypeProvider::isDoor(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isDoor : false;
}

bool ItemTypeProvider::isTeleport(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isTeleport : false;
}

bool ItemTypeProvider::isBed(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isBed : false;
}

bool ItemTypeProvider::isDepot(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isDepot : false;
}

bool ItemTypeProvider::isSplash(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isSplash : false;
}

bool ItemTypeProvider::isFluidContainer(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isFluidContainer : false;
}

bool ItemTypeProvider::hasLight(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? (itemData->lightLevel > 0) : false;
}

bool ItemTypeProvider::isStackable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isStackable : false;
}

bool ItemTypeProvider::isMoveable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isMoveable : false;
}

bool ItemTypeProvider::isPickupable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isPickupable : false;
}

bool ItemTypeProvider::isRotatable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isRotatable : false;
}

bool ItemTypeProvider::isHangable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isHangable : false;
}

bool ItemTypeProvider::isVertical(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isVertical : false;
}

bool ItemTypeProvider::isHorizontal(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isHorizontal : false;
}

bool ItemTypeProvider::isReadable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isReadable : false;
}

bool ItemTypeProvider::isWriteable(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isWriteable : false;
}

bool ItemTypeProvider::isDecoration(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->isDecoration : false;
}

uint8_t ItemTypeProvider::getSpeed(uint16_t itemId) const {
    auto it = m_speedCache.find(itemId);
    if (it != m_speedCache.end()) {
        return it.value();
    }
    
    const assets::ItemData* itemData = getItemData(itemId);
    uint8_t speed = itemData ? itemData->speed : 0;
    
    cacheNumericProperty(m_speedCache, itemId, speed);
    return speed;
}

uint8_t ItemTypeProvider::getLightLevel(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->lightLevel : 0;
}

uint8_t ItemTypeProvider::getLightColor(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->lightColor : 0;
}

uint16_t ItemTypeProvider::getMaxReadWriteChars(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->maxReadWriteChars : 0;
}

uint16_t ItemTypeProvider::getMaxItems(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->maxItems : 0;
}

uint16_t ItemTypeProvider::getWeight(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->weight : 0;
}

QString ItemTypeProvider::getName(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->name : QString("Unknown Item %1").arg(itemId);
}

QString ItemTypeProvider::getDescription(uint16_t itemId) const {
    const assets::ItemData* itemData = getItemData(itemId);
    return itemData ? itemData->description : QString();
}

void ItemTypeProvider::clearCache() {
    m_walkableCache.clear();
    m_blockingCache.clear();
    m_containerCache.clear();
    m_speedCache.clear();
    
    qDebug() << "ItemTypeProvider: Cache cleared";
}

bool ItemTypeProvider::isValid() const {
    return m_itemDatabase != nullptr;
}

const assets::ItemData* ItemTypeProvider::getItemData(uint16_t itemId) const {
    if (!m_itemDatabase) {
        return nullptr;
    }
    
    return m_itemDatabase->getItemData(itemId);
}

void ItemTypeProvider::cacheBoolProperty(QHash<uint16_t, bool>& cache, uint16_t itemId, bool value) const {
    cache[itemId] = value;
}

void ItemTypeProvider::cacheNumericProperty(QHash<uint16_t, uint8_t>& cache, uint16_t itemId, uint8_t value) const {
    cache[itemId] = value;
}

} // namespace core
} // namespace RME