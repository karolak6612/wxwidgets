#ifndef RME_ITEM_TYPE_PROVIDER_H
#define RME_ITEM_TYPE_PROVIDER_H

#include "IItemTypeProvider.h"
#include <QHash>
#include <memory>

namespace RME {
namespace core {
namespace assets {
    class ItemDatabase;
}

/**
 * @brief Concrete implementation of IItemTypeProvider using ItemDatabase
 * 
 * This class provides item type information by querying the ItemDatabase
 * and caching results for performance. It implements all the interface
 * methods required by the Tile class and other components.
 */
class ItemTypeProvider : public IItemTypeProvider {
public:
    /**
     * @brief Construct ItemTypeProvider with ItemDatabase
     * @param itemDatabase Pointer to the item database (not owned)
     */
    explicit ItemTypeProvider(const assets::ItemDatabase* itemDatabase);
    
    /**
     * @brief Destructor
     */
    ~ItemTypeProvider() override = default;

    // IItemTypeProvider interface implementation
    bool isWalkable(uint16_t itemId) const override;
    bool isBlocking(uint16_t itemId) const override;
    bool isContainer(uint16_t itemId) const override;
    bool isDoor(uint16_t itemId) const override;
    bool isTeleport(uint16_t itemId) const override;
    bool isBed(uint16_t itemId) const override;
    bool isDepot(uint16_t itemId) const override;
    bool isSplash(uint16_t itemId) const override;
    bool isFluidContainer(uint16_t itemId) const override;
    bool hasLight(uint16_t itemId) const override;
    bool isStackable(uint16_t itemId) const override;
    bool isMoveable(uint16_t itemId) const override;
    bool isPickupable(uint16_t itemId) const override;
    bool isRotatable(uint16_t itemId) const override;
    bool isHangable(uint16_t itemId) const override;
    bool isVertical(uint16_t itemId) const override;
    bool isHorizontal(uint16_t itemId) const override;
    bool isReadable(uint16_t itemId) const override;
    bool isWriteable(uint16_t itemId) const override;
    bool isDecoration(uint16_t itemId) const override;
    
    uint8_t getSpeed(uint16_t itemId) const override;
    uint8_t getLightLevel(uint16_t itemId) const override;
    uint8_t getLightColor(uint16_t itemId) const override;
    uint16_t getMaxReadWriteChars(uint16_t itemId) const override;
    uint16_t getMaxItems(uint16_t itemId) const override;
    uint16_t getWeight(uint16_t itemId) const override;
    
    QString getName(uint16_t itemId) const override;
    QString getDescription(uint16_t itemId) const override;
    
    /**
     * @brief Clear the internal cache
     * 
     * Call this when the ItemDatabase is reloaded to ensure
     * cached data is refreshed.
     */
    void clearCache();
    
    /**
     * @brief Check if the provider is valid (has a valid ItemDatabase)
     * @return true if the provider can be used
     */
    bool isValid() const;

private:
    const assets::ItemDatabase* m_itemDatabase;
    
    // Cache for frequently accessed properties
    mutable QHash<uint16_t, bool> m_walkableCache;
    mutable QHash<uint16_t, bool> m_blockingCache;
    mutable QHash<uint16_t, bool> m_containerCache;
    mutable QHash<uint16_t, uint8_t> m_speedCache;
    
    /**
     * @brief Get item data from database with null checking
     * @param itemId The item ID to look up
     * @return Pointer to item data or nullptr if not found
     */
    const assets::ItemData* getItemData(uint16_t itemId) const;
    
    /**
     * @brief Cache a boolean property
     * @param cache The cache to store in
     * @param itemId The item ID
     * @param value The value to cache
     */
    void cacheBoolProperty(QHash<uint16_t, bool>& cache, uint16_t itemId, bool value) const;
    
    /**
     * @brief Cache a numeric property
     * @param cache The cache to store in
     * @param itemId The item ID
     * @param value The value to cache
     */
    void cacheNumericProperty(QHash<uint16_t, uint8_t>& cache, uint16_t itemId, uint8_t value) const;
};

} // namespace core
} // namespace RME

#endif // RME_ITEM_TYPE_PROVIDER_H