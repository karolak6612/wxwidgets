#ifndef RME_CREATURE_H
#define RME_CREATURE_H

#include "core/Position.h"
#include "core/creatures/Outfit.h"
#include "core/assets/CreatureData.h" // Include full definition for type->defaultOutfit access
#include <QString>
#include <memory> // For std::unique_ptr

namespace RME {
namespace core {
// Forward declaration from assets needed if CreatureData.h isn't fully included above
// namespace assets {
//     struct CreatureData;
// }

namespace creatures {

// Based on wxwidgets/creature.h CreatureFlags
enum class CreatureFlag : uint32_t {
    NONE = 0,
    UNPASSABLE = 1 << 0,       // Creature blocks movement
    SUMMON = 1 << 1,           // Creature is a summon
    NPC = 1 << 2,              // Creature is an NPC (not a monster)
    PERSISTENT = 1 << 3,       // Creature persists on map save/load (often for NPCs or special monsters)
    // Add other flags as identified or needed, e.g., HOSTILE, CONVINCIBLE, etc.
};

// Enable bitwise operations for CreatureFlag enum class
inline CreatureFlag operator|(CreatureFlag a, CreatureFlag b) {
    return static_cast<CreatureFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline CreatureFlag& operator|=(CreatureFlag& a, CreatureFlag b) {
    a = a | b;
    return a;
}
inline CreatureFlag operator&(CreatureFlag a, CreatureFlag b) {
    return static_cast<CreatureFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline CreatureFlag& operator&=(CreatureFlag& a, CreatureFlag b) {
    a = a & b;
    return a;
}
inline CreatureFlag operator~(CreatureFlag a) {
    return static_cast<CreatureFlag>(~static_cast<uint32_t>(a));
}


class Creature {
public:
    Creature(const RME::core::assets::CreatureData* type, const Position& pos);
    ~Creature() = default; // Default destructor is fine with unique_ptr members if any were used for owned data

    // Copying and Moving
    Creature(const Creature& other);
    Creature& operator=(const Creature& other);
    Creature(Creature&& other) noexcept;
    Creature& operator=(Creature&& other) noexcept;

    std::unique_ptr<Creature> deepCopy() const;

    // Type-related getters (delegated to m_type)
    QString getName() const;
    bool isNpc() const; // Example, actual check might be (m_flags & CreatureFlag::NPC) or from m_type
    // Add more as needed: e.g., getHealth, getSpeed if these are part of CreatureData

    // Position
    const Position& getPosition() const { return m_position; }
    void setPosition(const Position& pos) { m_position = pos; }
    
    // Direction (from original wxWidgets implementation)
    Direction getDirection() const { return m_direction; }
    void setDirection(Direction dir) { m_direction = dir; }
    
    // Spawn time (from original wxWidgets implementation)
    int getSpawnTime() const { return m_spawnTime; }
    void setSpawnTime(int spawnTime) { m_spawnTime = spawnTime; }
    
    // Save/selection state (from original wxWidgets implementation)
    bool isSaved() const { return m_saved; }
    void save() { m_saved = true; }
    void reset() { m_saved = false; }
    
    bool isSelected() const { return m_selected; }
    void select() { m_selected = true; }
    void deselect() { m_selected = false; }
    
    // Static direction conversion methods (from original wxWidgets)
    static QString directionIdToName(uint16_t id);
    static uint16_t directionNameToId(const QString& name);

    // Outfit
    const Outfit& getOutfit() const { return m_outfit; }
    void setOutfit(const Outfit& outfit) { m_outfit = outfit; }
    // Convenience setters for outfit parts
    void setLookType(uint16_t lookType) { m_outfit.lookType = lookType; }
    void setLookItem(uint16_t lookItem) { m_outfit.lookItem = lookItem; }
    void setLookMount(uint16_t lookMount) { m_outfit.lookMount = lookMount; }
    void setLookHead(uint8_t head) { m_outfit.lookHead = head; }
    void setLookBody(uint8_t body) { m_outfit.lookBody = body; }
    void setLookLegs(uint8_t legs) { m_outfit.lookLegs = legs; }
    void setLookFeet(uint8_t feet) { m_outfit.lookFeet = feet; }
    void setLookAddons(uint8_t addons) { m_outfit.lookAddons = addons; }
    void setAddonFlag(uint8_t addonBit, bool enabled) { m_outfit.setAddon(addonBit, enabled); }

    // Flags
    CreatureFlag getFlags() const { return m_flags; }
    void setFlags(CreatureFlag flags) { m_flags = flags; }
    bool hasFlag(CreatureFlag flag) const { return (m_flags & flag) != CreatureFlag::NONE; }
    void addFlag(CreatureFlag flag) { m_flags |= flag; }
    void removeFlag(CreatureFlag flag) { m_flags &= ~flag; }

    // Access to the static type data
    const RME::core::assets::CreatureData* getType() const { return m_type; }

private:
    const RME::core::assets::CreatureData* m_type; // Non-owning pointer to static creature data
    Position m_position;
    Outfit m_outfit;         // Instance-specific outfit
    CreatureFlag m_flags = CreatureFlag::NONE;
    Direction m_direction = Direction::SOUTH; // Default direction from original wxWidgets
    int m_spawnTime = 0;     // Spawn time in seconds
    bool m_saved = false;    // Save state tracking
    bool m_selected = false; // Selection state tracking
};

} // namespace creatures
} // namespace core
} // namespace RME

#endif // RME_CREATURE_H
