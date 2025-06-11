#ifndef RME_CREATURES_CREATURE_H
#define RME_CREATURES_CREATURE_H

#include "core/Position.h"
#include "core/assets/Outfit.h" // Assumes Outfit.h is in core/assets/
#include <QString>
#include <memory> // For std::unique_ptr
#include <QFlags>
#include <QtGlobal> // For quint types

// Forward declaration for CreatureData
namespace RME {
namespace core {
namespace assets {
    class CreatureData;
} // namespace assets
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace creatures {

/**
 * @brief Represents flags that define creature properties and behaviors.
 */
enum class CreatureFlagValue : quint32 {
    NONE                    = 0x0000, ///< No special flags.
    UNPASSABLE              = 0x0001, ///< Creature blocks pathing. (Typically derived from type)
    CAN_SUMMON              = 0x0002, ///< Creature can summon others.
    IS_NPC                  = 0x0004, ///< Creature is an NPC. (Typically derived from type)
    IS_HOSTILE              = 0x0008, ///< Creature is hostile.
    PUSHABLE                = 0x0010, ///< Creature can be pushed. (Typically derived from type)
    SPEAKS_UNKNOWN_LANGUAGE = 0x0020, ///< Creature speaks in an unknown language.
    LIGHT_EMITTING          = 0x0040, ///< Creature emits light.
    NO_CORPSE               = 0x0080  ///< Creature leaves no corpse.
    // Add more flags as they are identified from OT sources or editor needs
};
Q_DECLARE_FLAGS(CreatureFlags, CreatureFlagValue)
Q_DECLARE_OPERATORS_FOR_FLAGS(CreatureFlags)

/**
 * @brief Represents an instance of a creature on the map.
 *
 * A Creature instance holds its specific state, such as its current position,
 * outfit, and any instance-specific flags or name overrides. It refers to
 * a CreatureData object for its base type information (static name, default outfit, etc.).
 */
class Creature {
public:
    /**
     * @brief Constructs a Creature instance.
     * @param type Pointer to the CreatureData defining the creature's type. Must not be null.
     * @param pos The initial position of the creature.
     */
    explicit Creature(const RME::core::assets::CreatureData* type, const Position& pos);

    /**
     * @brief Default virtual destructor.
     */
    virtual ~Creature() = default;

    // Prevent copying, allow moving if necessary (though unique_ptr is typical)
    Creature(const Creature&) = delete;
    Creature& operator=(const Creature&) = delete;
    Creature(Creature&&) = default; // Or delete if unique_ptr is strictly enforced
    Creature& operator=(Creature&&) = default; // Or delete

    /**
     * @brief Creates a deep copy of this creature instance.
     * @return A unique_ptr to the newly created Creature copy.
     */
    std::unique_ptr<Creature> deepCopy() const;

    // --- Getters ---
    /** @brief Gets the creature's current map position. */
    const RME::Position& getPosition() const { return m_position; }
    /** @brief Gets the creature's current outfit. */
    const RME::Outfit& getOutfit() const { return m_outfit; }
    /** @brief Gets all flags set for this creature instance. */
    CreatureFlags getFlags() const { return m_flags; }
    /** @brief Gets a pointer to the creature's base type data. */
    const RME::core::assets::CreatureData* getType() const { return m_type; }
    /** @brief Gets the instance-specific name of the creature (if set, otherwise may fall back to type name). */
    const QString& getInstanceName() const { return m_instanceName; }

    // --- Setters ---
    /** @brief Sets the creature's map position. */
    void setPosition(const Position& pos) { m_position = pos; }
    /** @brief Sets the creature's entire outfit. */
    void setOutfit(const RME::Outfit& outfit) { m_outfit = outfit; }
    /** @brief Sets the look type (itemID or creature type ID) for the outfit. */
    void setLookType(quint16 lookType) { m_outfit.lookType = lookType; }
    /** @brief Sets the head color/style for the outfit. */
    void setLookHead(quint8 head) { m_outfit.lookHead = head; }
    /** @brief Sets the body color/style for the outfit. */
    void setLookBody(quint8 body) { m_outfit.lookBody = body; }
    /** @brief Sets the legs color/style for the outfit. */
    void setLookLegs(quint8 legs) { m_outfit.lookLegs = legs; }
    /** @brief Sets the feet color/style for the outfit. */
    void setLookFeet(quint8 feet) { m_outfit.lookFeet = feet; }
    /** @brief Sets the addons for the outfit (bitwise). */
    void setLookAddons(quint8 addons) { m_outfit.lookAddons = addons; }
    /** @brief Sets the look type of the creature's mount. */
    void setLookMount(quint16 mountLookType) { m_outfit.lookMount = mountLookType; }
    /** @brief Sets all flags for this creature instance. */
    void setFlags(CreatureFlags flags) { m_flags = flags; }
    /** @brief Adds a specific flag to the creature instance. */
    void addFlag(CreatureFlagValue flag) { m_flags |= flag; }
    /** @brief Removes a specific flag from the creature instance. */
    void removeFlag(CreatureFlagValue flag) { m_flags &= ~flag; }
    /** @brief Checks if a specific flag is set for this creature. */
    bool hasFlag(CreatureFlagValue flag) const { return m_flags.testFlag(flag); }
    /** @brief Sets an instance-specific name for the creature. */
    void setInstanceName(const QString& name) { m_instanceName = name; }

    // --- Delegated Getters (from CreatureData) ---
    /** @brief Gets the static name of the creature type (e.g., "Dragon"). */
    QString getStaticName() const;
    /** @brief Checks if the creature type is an NPC. */
    bool isNpc() const;
    // Add more delegated getters as needed (e.g., isPushable, canSummon from CreatureData flags)

private:
    const RME::core::assets::CreatureData* m_type; ///< Non-owning pointer to the creature type definition.
    RME::Position m_position;                      ///< Current position on the map.
    RME::Outfit m_outfit;                          ///< Current outfit of this instance.
    CreatureFlags m_flags;                         ///< Instance-specific flags.
    QString m_instanceName;                        ///< Instance-specific name (override).
};

} // namespace creatures
} // namespace core
} // namespace RME

// Make CreatureFlags usable with QVariant
Q_DECLARE_METATYPE(RME::core::creatures::CreatureFlags)

#endif // RME_CREATURES_CREATURE_H
