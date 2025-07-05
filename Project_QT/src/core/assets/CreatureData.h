#ifndef RME_CREATURE_DATA_H
#define RME_CREATURE_DATA_H

#include "Outfit.h"
#include <QString>
#include <QList>    // For list of outfits if a creature has multiple
#include <QVariantMap> // For generic attributes
#include <QFlags>     // For Q_DECLARE_FLAGS

namespace RME {

enum class CreatureTypeFlag {
    NONE = 0,
    IS_NPC = 1 << 0,
    // Add other flags if RME's CreatureType had them (e.g. summonable, hostiles, etc.)
};
Q_DECLARE_FLAGS(CreatureTypeFlags, CreatureTypeFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CreatureTypeFlags)


struct CreatureData {
    QString name;
    QString scriptName; // Often the filename or a unique identifier for scripts
    Outfit outfit;      // Default outfit
    // QList<Outfit> alternativeOutfits; // If creatures can have multiple outfits selectable

    CreatureTypeFlags flags = CreatureTypeFlags(CreatureTypeFlag::NONE); // Initialize correctly

    // Common stats (might be loaded from XML or have defaults)
    int healthMax = 100;
    int manaMax = 50;
    // Add other stats as needed: speed, resistances, immunities, spells, inventory items...
    // These are often more detailed in server-side XMLs than editor creature lists.

    QVariantMap genericAttributes; // For any other properties from XML

    CreatureData() = default;
};

} // namespace RME

#endif // RME_CREATURE_DATA_H
