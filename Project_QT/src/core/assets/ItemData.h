#ifndef RME_ITEM_DATA_H
#define RME_ITEM_DATA_H

#include <QString>
#include <QList>
#include <QVariantMap> // For generic attributes
#include "core/creatures/Outfit.h" // For ItemType equivalent of default outfit/look

// Forward declaration for ClientProfile if it's complex and only used by pointer/reference
// namespace RME { namespace core { namespace assets { class ClientProfile; }}}

// Define ItemFlag enum for item properties
// Based on usage in ItemDatabase.cpp and AssetManager.cpp
// The values are examples and might need adjustment if specific bit values are required.
enum class ItemFlag : quint32 {
    NONE             = 0,
    BLOCK_SOLID      = 1 << 0,
    BLOCK_PROJECTILE = 1 << 1,
    BLOCK_PATHFIND   = 1 << 2,
    HAS_HEIGHT       = 1 << 3,
    PICKUPABLE       = 1 << 4,
    STACKABLE        = 1 << 5,
    MOVEABLE         = 1 << 6,
    ALWAYSONTOP      = 1 << 7,
    READABLE         = 1 << 8,
    ROTATABLE        = 1 << 9,
    HANGABLE         = 1 << 10,
    VERTICAL         = 1 << 11,
    HORIZONTAL       = 1 << 12,
    ANIMATION        = 1 << 13,
    ALLOWDISTREAD    = 1 << 14,
    LOOKTHROUGH      = 1 << 15,
    WALKSTACK        = 1 << 16, // Also known as fullground
    WALL             = 1 << 17
    // Add other flags as needed from OTB/XML parsing in ItemDatabase.cpp
};

inline ItemFlag operator|(ItemFlag a, ItemFlag b) {
    return static_cast<ItemFlag>(static_cast<quint32>(a) | static_cast<quint32>(b));
}

inline ItemFlag& operator|=(ItemFlag& a, ItemFlag b) {
    a = a | b;
    return a;
}

namespace RME {
namespace core {
namespace assets {

// Corresponds roughly to RME's ItemType in wxwidgets
// This structure holds the static properties of an item type.
struct ItemData {
    QString name;
    uint16_t serverID = 0; // The ID used by the server (OTB ID)
    uint16_t clientID = 0; // The ID used by the client (sprite ID)

    // Common Flags (can be expanded based on ItemType flags from wx)
    bool isGround = false;
    bool isTopOrder1 = false; // e.g. hangable
    bool isTopOrder2 = false; // e.g. splash, fluid container content
    bool isTopOrder3 = false; // e.g. projectile
    bool isContainer = false;
    bool isStackable = false;
    bool isUseable = false;
    bool isReadable = false;
    bool isWriteable = false;
    bool isFluidContainer = false;
    bool isSplash = false;
    bool isBlocking = false;        // Item blocks movement
    bool isMoveable = false;
    bool isPickupable = false;
    bool hasHeight = false;         // e.g. chairs, some wall pieces
    bool isRotatable = false;
    bool hasLight = false;
    bool isDontHide = false;        // For items like magic fields that shouldn't be hidden by "hide items"
    bool isTranslucent = false;
    bool isShift = false;           // If item has x/y offsets when drawn
    bool isFullGround = false;      // e.g. grates, some grounds that cover whole tile
    bool isIgnoreLook = false;
    bool isWrapable = false;
    bool isUnwrapable = false;
    bool isTopEffect = false;       // e.g. some magic effects, corpse smoke
    bool isAnimation = false;       // If item is an animation
    bool isBorder = false;          // If item is specifically a border item (auto-bordering context)

    // New field for linking to a material definition
    QString materialId;             // ID of the material this item primarily belongs to (e.g., for grounds)

    // Attributes
    int lightLevel = 0;
    int lightColor = 0;
    int minimapColor = 0;
    int aniamtionTicks = 0; // Typo in original RME? "animTicks", should be animTicks or animationTicks
    int frameCount = 0;     // For animated items
    int groundSpeed = 0;    // Speed modifier if it's a ground tile

    // Dimensions / Offsets (if applicable, might be client-profile specific)
    // int spriteXOffset = 0;
    // int spriteYOffset = 0;
    // int spriteWidth = 1;  // In tiles
    // int spriteHeight = 1; // In tiles

    // For containers
    // int maxItems = 0;

    // For complex items like doors, beds, podiums - specific data might be needed
    // Or handled by specialized Item subclasses that use this ItemData.

    // Fields required by IItemTypeProvider and used in ItemDatabase/AssetManager
    QString description;
    quint32 flags = 0; // Combined bitmask of item properties, corresponds to ItemFlag enum

    QVariantMap genericAttributes; // For any other properties from XML/OTB

    ItemData() = default; // Ensure materialId is default constructed (empty QString)

    // Method to check flags, required by IItemTypeProvider implementation in AssetManager
    bool hasFlag(ItemFlag flag_value) const {
        return (flags & static_cast<quint32>(flag_value)) != 0;
    }
};

// Helper structure to group item types by category for palette display
struct ItemGroup {
    QString name;
    QList<uint16_t> itemServerIDs; // List of server IDs belonging to this group
};


} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_ITEM_DATA_H
