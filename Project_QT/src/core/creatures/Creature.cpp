#include "core/creatures/Creature.h"
#include "core/assets/CreatureData.h" // For CreatureData definition

// For QDebug, if used for warnings/errors, though not strictly necessary for this class's logic
// #include <QDebug>

namespace RME {
namespace core {
namespace creatures {

Creature::Creature(const RME::core::assets::CreatureData* type, const Position& pos)
    : m_type(type), m_position(pos), m_flags(CreatureFlag::NONE), 
      m_direction(Direction::SOUTH), m_spawnTime(0), m_saved(false), m_selected(false)
{
    if (m_type) {
        m_outfit = m_type->defaultOutfit; // Initialize instance outfit from type's default
        // Set default flags based on type
        if (m_type->isNpc) {
             m_flags |= CreatureFlag::NPC;
        }
        if (!m_type->isPassable) {
            m_flags |= CreatureFlag::UNPASSABLE;
        }
    } else {
        // Handle null type - this is an error condition.
        // qWarning() << "Creature created with null CreatureData type!";
    }
}

// Copy constructor
Creature::Creature(const Creature& other)
    : m_type(other.m_type), // Copy pointer/reference, non-owning
      m_position(other.m_position),
      m_outfit(other.m_outfit),
      m_flags(other.m_flags),
      m_direction(other.m_direction),
      m_spawnTime(other.m_spawnTime),
      m_saved(other.m_saved),
      m_selected(other.m_selected)
{}

// Copy assignment operator
Creature& Creature::operator=(const Creature& other) {
    if (this == &other) {
        return *this;
    }
    m_type = other.m_type; // Copy pointer/reference
    m_position = other.m_position;
    m_outfit = other.m_outfit;
    m_flags = other.m_flags;
    m_direction = other.m_direction;
    m_spawnTime = other.m_spawnTime;
    m_saved = other.m_saved;
    m_selected = other.m_selected;
    return *this;
}

// Move constructor
Creature::Creature(Creature&& other) noexcept
    : m_type(other.m_type),
      m_position(std::move(other.m_position)),
      m_outfit(std::move(other.m_outfit)),
      m_flags(other.m_flags),
      m_direction(other.m_direction),
      m_spawnTime(other.m_spawnTime),
      m_saved(other.m_saved),
      m_selected(other.m_selected)
{
    // Since m_type is non-owning, just copying is fine. No need to null out other.m_type.
}

// Move assignment operator
Creature& Creature::operator=(Creature&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    m_type = other.m_type;
    m_position = std::move(other.m_position);
    m_outfit = std::move(other.m_outfit);
    m_flags = other.m_flags;
    m_direction = other.m_direction;
    m_spawnTime = other.m_spawnTime;
    m_saved = other.m_saved;
    m_selected = other.m_selected;
    return *this;
}

std::unique_ptr<Creature> Creature::deepCopy() const {
    // Create a new Creature using the copy constructor implicitly or explicitly.
    // The constructor will copy m_type (pointer), m_position, m_outfit, and m_flags.
    // Since m_type is a non-owning pointer to static data, this is fine.
    return std::make_unique<Creature>(*this);
}

// Type-related getters (delegated to m_type)
QString Creature::getName() const {
    if (m_type) {
        return m_type->name;
    }
    return QStringLiteral("Unknown Creature"); // Fallback if type is null
}

bool Creature::isNpc() const {
    // Could be a flag on the instance, or a property of the type.
    // The constructor sets CreatureFlag::NPC based on m_type->isNpc.
    // So, checking the flag is consistent if that's the design.
    // Alternatively, always delegate to type if instance cannot change its NPC status:
    // if (m_type) return m_type->isNpc;
    // return false; // Default if no type
    return hasFlag(CreatureFlag::NPC);
}

// Static direction conversion methods (from original wxWidgets implementation)
QString Creature::directionIdToName(uint16_t id) {
    return DirectionUtils::directionToName(static_cast<Direction>(id));
}

uint16_t Creature::directionNameToId(const QString& name) {
    return static_cast<uint16_t>(DirectionUtils::nameToDirection(name));
}

} // namespace creatures
} // namespace core
} // namespace RME
