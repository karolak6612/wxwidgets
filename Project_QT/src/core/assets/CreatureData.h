#ifndef RME_CREATURE_DATA_H
#define RME_CREATURE_DATA_H

#include "core/creatures/Outfit.h"
#include <QString>
#include <QList>    // For list of outfits if a creature has multiple
#include <QVariantMap> // For generic attributes
#include <QFlags>     // For Q_DECLARE_FLAGS

namespace RME {
namespace core {
namespace assets {

enum class CreatureTypeFlag {
    NONE = 0,
    IS_NPC = 1 << 0,
    IS_PASSABLE = 1 << 1,
    IS_SUMMON = 1 << 2,
    IS_HOSTILE = 1 << 3,
    IS_CONVINCIBLE = 1 << 4,
    // Add other flags if RME's CreatureType had them
};
Q_DECLARE_FLAGS(CreatureTypeFlags, CreatureTypeFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CreatureTypeFlags)

struct CreatureData {
    QString name;
    QString scriptName; // Often the filename or a unique identifier for scripts
    RME::core::creatures::Outfit defaultOutfit; // Renamed to match Creature.cpp expectations
    // QList<RME::core::creatures::Outfit> alternativeOutfits; // If creatures can have multiple outfits selectable

    CreatureTypeFlags flags = CreatureTypeFlags(CreatureTypeFlag::NONE);

    // Common stats (might be loaded from XML or have defaults)
    int healthMax = 100;
    int manaMax = 50;
    int speed = 100;
    
    // Convenience properties for backward compatibility
    bool isNpc = false;        // Added for Creature.cpp compatibility
    bool isPassable = true;    // Added for Creature.cpp compatibility

    QVariantMap genericAttributes; // For any other properties from XML

    CreatureData() = default;
    
    // Constructor to set flags and convenience bools consistently
    CreatureData(const QString& creatureName) : name(creatureName) {
        updateConvenienceFlags();
    }
    
    // Update convenience flags based on main flags
    void updateConvenienceFlags() {
        isNpc = flags.testFlag(CreatureTypeFlag::IS_NPC);
        isPassable = flags.testFlag(CreatureTypeFlag::IS_PASSABLE);
    }
    
    // Set convenience flags and update main flags
    void setIsNpc(bool npc) {
        isNpc = npc;
        if (npc) {
            flags |= CreatureTypeFlag::IS_NPC;
        } else {
            flags &= ~CreatureTypeFlag::IS_NPC;
        }
    }
    
    void setIsPassable(bool passable) {
        isPassable = passable;
        if (passable) {
            flags |= CreatureTypeFlag::IS_PASSABLE;
        } else {
            flags &= ~CreatureTypeFlag::IS_PASSABLE;
        }
    }
};

} // namespace assets
} // namespace core

} // namespace RME

#endif // RME_CREATURE_DATA_H
