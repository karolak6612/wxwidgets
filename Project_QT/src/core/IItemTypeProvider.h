#ifndef RME_I_ITEM_TYPE_PROVIDER_H
#define RME_I_ITEM_TYPE_PROVIDER_H

#include <cstdint>
#include <QString> // For item name, description etc.

// Forward declarations
namespace RME { class Item; }
namespace RME { namespace core { namespace assets { struct ItemData; } } }

namespace RME {

// Interface for providing item type information (metadata)
// This replaces the global g_items functionality for property lookups.
class IItemTypeProvider {
public:
    virtual ~IItemTypeProvider() = default;

    // Basic properties
    virtual QString getName(uint16_t id) const = 0;
    virtual QString getDescription(uint16_t id) const = 0;
    // virtual Sprite getSprite(uint16_t id, uint16_t subtype) const = 0; // Sprite class TBD
    virtual uint32_t getFlags(uint16_t id) const = 0; // For raw flags from OTB/XML
    virtual double getWeight(uint16_t id, uint16_t subtype) const = 0; // Subtype for stackable items

    // Common boolean properties (derived from flags or specific attributes)
    virtual bool isBlocking(uint16_t id) const = 0;
    virtual bool isProjectileBlocking(uint16_t id) const = 0;
    virtual bool isPathBlocking(uint16_t id) const = 0;
    virtual bool isWalkable(uint16_t id) const = 0; // Opposite of pathblocking in some contexts
    virtual bool isStackable(uint16_t id) const = 0;
    virtual bool isGround(uint16_t id) const = 0; // e.g. terrain
    virtual bool isAlwaysOnTop(uint16_t id) const = 0;
    virtual bool isReadable(uint16_t id) const = 0;
    virtual bool isWriteable(uint16_t id) const = 0;
    virtual bool isFluidContainer(uint16_t id) const = 0;
    virtual bool isSplash(uint16_t id) const = 0;
    virtual bool isMoveable(uint16_t id) const = 0;
    virtual bool hasHeight(uint16_t id) const = 0; // e.g. items that elevate player
    virtual bool isContainer(uint16_t id) const = 0;
    virtual bool isTeleport(uint16_t id) const = 0;
    virtual bool isDoor(uint16_t id) const = 0;
    virtual bool isPodium(uint16_t id) const = 0;
    virtual bool isDepot(uint16_t id) const = 0;

    // Lighting properties
    virtual bool hasLight(uint16_t id) const = 0;
    virtual uint8_t getLightIntensity(uint16_t id) const = 0;
    virtual uint8_t getLightColor(uint16_t id) const = 0;

    // Get the complete item data structure for the given item ID
    virtual const RME::core::assets::ItemData* getItemData(uint16_t id) const = 0;

    // Add more property queries as needed from the original g_items or Item class.
    // For example, light properties, specific attributes like 'charges', etc.
    // virtual LightProperties getLight(uint16_t id) const = 0;
};

} // namespace RME

#endif // RME_I_ITEM_TYPE_PROVIDER_H
