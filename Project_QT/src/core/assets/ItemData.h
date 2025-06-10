#ifndef RME_ITEM_DATA_H
#define RME_ITEM_DATA_H

#include <QString>
#include <QList>
#include <QVariantMap> // For flexible attributes from XML/OTB
#include <QFlags>     // For Q_DECLARE_FLAGS

// Enums from original items.h (adapted for C++11 enum class if possible)
namespace RME {

enum class ItemGroup : quint8 {
    NONE = 0,
    GROUND,
    CONTAINER,
    WEAPON,
    AMMUNITION,
    ARMOR,
    RUNE,
    TELEPORT,
    MAGICFIELD,
    WRITEABLE,
    KEY,
    SPLASH,
    FLUID,
    DOOR,
    DEPRECATED, // Was "TRASH",
    PODIUM,     // Added for podiums
    LAST
};

enum class ItemType : quint8 { // More specific classification
    TYPE_NONE = 0,
    TYPE_NORMAL,       // Normal item
    TYPE_CONTAINER,    // Container
    TYPE_FLUID,        // Fluids like water, blood, etc
    TYPE_SPLASH,       // Splashes
    TYPE_DEPRECATED,   // Was "USELESS"
    TYPE_DEPOT,        // Depot item
    TYPE_MAILBOX,
    TYPE_TRASHHOLDER,
    TYPE_TELEPORT,     // Teleport (magic rope etc)
    TYPE_MAGICFIELD,   // Magic field
    TYPE_DOOR,         // Door
    TYPE_BED,          // Bed Item
    TYPE_KEY,
    TYPE_RUNE,
    TYPE_PODIUM,
    LAST
};

enum OtbAttribute : quint8 {
    OTB_ATTR_DESCRIPTION = 1,
    OTB_ATTR_EXT_FILE = 2, // Not used in current RME from quick glance, but part of OTB
    OTB_ATTR_TILE_FLAGS = 3,
    OTB_ATTR_ACTION_ID = 4,
    OTB_ATTR_UNIQUE_ID = 5,
    OTB_ATTR_TEXT = 6,
    OTB_ATTR_DESC = 7, // Seems like a duplicate of 1? Original RME has it.
    OTB_ATTR_TELE_DEST = 8,
    OTB_ATTR_ITEM = 9, // For containers
    OTB_ATTR_DEPOT_ID = 10,
    // OTB_ATTR_EXT_SPAWN_FILE = 11, // Not in current RME
    // OTB_ATTR_EXT_HOUSE_FILE = 12, // Not in current RME
    OTB_ATTR_HOUSE_DOOR_ID = 13,
    OTB_ATTR_NAME = 14, // Not standard OTB, RME specific addition?
    OTB_ATTR_PLURALNAME = 15, // Not standard OTB
    OTB_ATTR_ATTACK = 16, // Not standard OTB
    OTB_ATTR_EXTRAATTACK = 17, // Not standard OTB
    OTB_ATTR_DEFENSE = 18, // Not standard OTB
    OTB_ATTR_EXTRADEFENSE = 19, // Not standard OTB
    OTB_ATTR_ARMOR = 20, // Not standard OTB
    OTB_ATTR_ATTACKSPEED = 21, // Not standard OTB
    OTB_ATTR_HITCHANCE = 22, // Not standard OTB
    OTB_ATTR_SHOOTRANGE = 23, // Not standard OTB
    OTB_ATTR_ARTICLE = 24, // Not standard OTB
    OTB_ATTR_SCRIPTPROTECTED = 25, // Not standard OTB
    OTB_ATTR_DUALWIELD = 26, // Not standard OTB
    OTB_ATTR_ATTRIBUTE_MAP = 128 // From The Forgotten Server OTB extension
};


// Item flags (server-side flags, mostly from OTB_ATTR_TILE_FLAGS)
// These correspond to Tibia.dat item flags.
enum class ItemFlag : quint32 {
    BLOCK_SOLID             = 1 << 0,  // Otb::BlockSolid
    BLOCK_PROJECTILE        = 1 << 1,  // Otb::BlockProjectile
    BLOCK_PATHFIND          = 1 << 2,  // Otb::BlockPathfind
    HAS_HEIGHT              = 1 << 3,  // Otb::HasHeight
    USEABLE                 = 1 << 4,  // Otb::Useable (DEPRECATED) -> IsUseable
    PICKUPABLE              = 1 << 5,  // Otb::Pickupable -> IsPickupable
    MOVEABLE                = 1 << 6,  // Otb::Moveable -> IsMoveable
    STACKABLE               = 1 << 7,  // Otb::Stackable -> IsStackable
    FLOORCHANGEDOWN         = 1 << 8,  // Otb::FloorchangeDown
    FLOORCHANGENORTH        = 1 << 9,  // Otb::FloorchangeNorth
    FLOORCHANGEEAST         = 1 << 10, // Otb::FloorchangeEast
    FLOORCHANGESOUTH        = 1 << 11, // Otb::FloorchangeSouth
    FLOORCHANGEWEST         = 1 << 12, // Otb::FloorchangeWest
    ALWAYSONTOP             = 1 << 13, // Otb::AlwaysOnTop -> IsAlwaysOnTop
    READABLE                = 1 << 14, // Otb::Readable -> IsReadable
    ROTATABLE               = 1 << 15, // Otb::Rotatable -> IsRotatable
    HANGABLE                = 1 << 16, // Otb::Hangable -> IsHangable
    VERTICAL                = 1 << 17, // Otb::Vertical -> IsVertical
    HORIZONTAL              = 1 << 18, // Otb::Horizontal -> IsHorizontal
    CANNOTDECAY             = 1 << 19, // Otb::CannotDecay (DEPRECATED)
    ALLOWDISTREAD           = 1 << 20, // Otb::AllowDistRead
    UNUSED                  = 1 << 21, // Otb::Unused (DEPRECATED)
    CLIENTCHARGES           = 1 << 22, // Otb::ClientCharges -> HasCharges / IsCorpse
    LOOKTHROUGH             = 1 << 23, // Otb::LookThrough -> IsLookThrough
    ANIMATION               = 1 << 24, // Otb::Animation -> HasAnimation
    WALKSTACK               = 1 << 25, // Otb::FullGround -> IsWalkStack
    WALL                    = 1 << 26, // Otb::Wall
    LAST_FLAG               = 1 << 27  // Not an actual flag, marker
};
Q_DECLARE_FLAGS(ItemFlags, ItemFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ItemFlags)


struct ItemData {
    quint16 serverID = 0; // Server Item ID
    quint16 clientID = 0; // Client Item ID (for appearance)
    ItemGroup group = ItemGroup::NONE;
    ItemType type = ItemType::TYPE_NONE; // More specific type for RME logic

    QString name;
    QString description; // From OTB_ATTR_DESCRIPTION or OTB_ATTR_DESC
    QString article;     // "a", "an", ""
    QString pluralName;

    ItemFlags flags = ItemFlags(ItemFlag::NO_FLAGS); // Server-side flags, initialize correctly

    // Visual properties (can be part of sprite data too)
    quint8 layers = 0;
    quint8 numPatternX = 0, numPatternY = 0, numPatternZ = 0;
    quint8 animationPhases = 0;

    // Light properties
    quint16 lightLevel = 0;
    quint16 lightColor = 0;

    // Writeable/Readable
    quint16 maxTextLen = 0;
    quint16 maxReadWriteChars = 0; // From original ItemType::writablechars

    // Other attributes parsed from OTB or XML
    quint16 attack = 0;
    quint16 extraAttack = 0; // defense + attack = extra attack value?
    quint16 defense = 0;
    quint16 extraDefense = 0;
    quint16 armor = 0;
    quint16 attackSpeed = 0;
    quint16 hitChance = 0;
    quint16 shootRange = 0;
    double weight = 0.0; // Kilograms
    quint16 slotPosition = 0; // Bitmask of equipable slots
    quint16 maxCharges = 0;   // For items with charges (runes, some tools)
    quint16 speed = 0;        // For boots, etc.
    quint16 decayTo = 0;      // Item ID it decays to
    quint16 corpseType = 0;   // For containers that are corpses

    // For internal editor use / brushes
    QString groundBrushName;
    QString wallBrushName;
    QString carpetBrushName;
    QString tableBrushName;

    // Generic attribute map for any other properties from XML or OTB AttributeMap
    QVariantMap genericAttributes;

    ItemData() = default;

    // Helper to check flags
    bool hasFlag(ItemFlag flag) const { return flags.testFlag(flag); }
};

} // namespace RME

#endif // RME_ITEM_DATA_H
