#include "core/creatures/Creature.h"
#include "core/assets/CreatureData.h" // For accessing m_type properties
#include <QDebug> // For assertions or warnings

namespace RME {
namespace core {
namespace creatures {

/**
 * @brief Constructs a Creature instance.
 * @param type Pointer to the CreatureData defining the creature's type. Must not be null.
 * @param pos The initial position of the creature.
 */
Creature::Creature(const RME::core::assets::CreatureData* type, const Position& pos) :
    m_type(type),
    m_position(pos),
    m_outfit(), // Default construct, then copy
    m_flags(CreatureFlagValue::NONE), // Default flags
    m_instanceName()
{
    Q_ASSERT(m_type != nullptr); // Ensure type is provided
    if (m_type) {
        m_outfit = m_type->defaultOutfit;
        m_instanceName = m_type->name; // Default instance name to type name
        // m_flags = m_type->flags; // If CreatureData had flags to copy
    } else {
        // Fallback if type is null, though this should ideally not happen
        qWarning("Creature created with null CreatureData type!");
        m_instanceName = "Unknown Creature";
    }
}

/**
 * @brief Creates a deep copy of this creature instance.
 * @return A unique_ptr to the newly created Creature copy.
 */
std::unique_ptr<Creature> Creature::deepCopy() const {
    // Using new directly and wrapping in unique_ptr.
    // std::make_unique cannot be used easily if constructor is private for factory pattern,
    // but here constructor is public.
    auto copy = std::make_unique<Creature>(m_type, m_position);
    copy->m_outfit = this->m_outfit;
    copy->m_flags = this->m_flags;
    copy->m_instanceName = this->m_instanceName;
    return copy;
}

// --- Delegated Getters (from CreatureData) ---

/**
 * @brief Gets the static name of the creature type (e.g., "Dragon").
 * @return QString The name from CreatureData, or "Unknown" if type is null.
 */
QString Creature::getStaticName() const {
    if (m_type) {
        return m_type->name;
    }
    return "Unknown"; // Fallback
}

/**
 * @brief Checks if the creature type is an NPC.
 * Delegates to CreatureData's flags or properties.
 * @return True if the creature is an NPC type, false otherwise.
 */
bool Creature::isNpc() const {
    if (m_type) {
        // Assuming CreatureData has a flag or method like isNpc()
        // For example: return m_type->hasFlag(CreatureTypeFlag::IS_NPC);
        // Or if CreatureData stores our CreatureFlags:
        // return RME::core::creatures::CreatureFlags(static_cast<quint32>(m_type->creatureFlags)).testFlag(CreatureFlagValue::IS_NPC);
        // For now, let's assume a simple boolean in CreatureData:
        return m_type->isNpc;
    }
    return false;
}

// Other getters and setters are inline in the header or trivial.
// If any setter had complex logic, it would be implemented here.
// For example, setLookAddons might need more if addons are not simple bitwise:
// void Creature::setLookAddons(quint8 addons) {
//    // If addons had specific validation or structure
//    m_outfit.lookAddons = addons;
// }

} // namespace creatures
} // namespace core
} // namespace RME
