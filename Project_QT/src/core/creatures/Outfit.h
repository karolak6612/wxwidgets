#ifndef RME_OUTFIT_H
#define RME_OUTFIT_H

#include <cstdint>
#include <Objects/Outfit.h> // Assuming this is a typo and should be removed or is a future planned header
                           // For now, I will remove it as it's not standard and not defined in this task.
                           // If it refers to a specific Qt object type, it needs clarification.

namespace RME {
namespace core {
namespace creatures {

// Forward declaration if CreatureData uses Outfit and Outfit needs to know about it (not typical for Outfit itself)
// namespace assets {
//     struct CreatureData;
// }

struct Outfit {
    uint16_t lookType = 0;
    uint16_t lookItem = 0;    // Used if lookType is an item that provides the appearance
    uint16_t lookMount = 0;   // Creature type ID for the mount
    uint8_t lookHead = 0;
    uint8_t lookBody = 0;
    uint8_t lookLegs = 0;
    uint8_t lookFeet = 0;
    uint8_t lookAddons = 0;  // Bitmask for addons

    // Default constructor
    Outfit() = default;

    // Full initializer constructor
    Outfit(uint16_t type, uint8_t head, uint8_t body, uint8_t legs, uint8_t feet, uint8_t addons,
           uint16_t mount = 0, uint16_t item = 0)
        : lookType(type), lookItem(item), lookMount(mount),
          lookHead(head), lookBody(body), lookLegs(legs), lookFeet(feet), lookAddons(addons)
    {}

    // Copy constructor (implicitly defined is fine, but can be explicit)
    Outfit(const Outfit& other) = default;

    // Move constructor (implicitly defined is fine)
    Outfit(Outfit&& other) = default;

    // Copy assignment (implicitly defined is fine)
    Outfit& operator=(const Outfit& other) = default;

    // Move assignment (implicitly defined is fine)
    Outfit& operator=(Outfit&& other) = default;

    // Comparison operators
    bool operator==(const Outfit& other) const {
        return lookType == other.lookType &&
               lookItem == other.lookItem &&
               lookMount == other.lookMount &&
               lookHead == other.lookHead &&
               lookBody == other.lookBody &&
               lookLegs == other.lookLegs &&
               lookFeet == other.lookFeet &&
               lookAddons == other.lookAddons;
    }

    bool operator!=(const Outfit& other) const {
        return !(*this == other);
    }

    // Helper for addons, if needed (example)
    bool hasAddon(uint8_t addonBit) const { // addonBit should be e.g., 1, 2, 4
        if (addonBit == 1) return (lookAddons & 0x01) != 0; // Addon 1 (bit 0)
        if (addonBit == 2) return (lookAddons & 0x02) != 0; // Addon 2 (bit 1)
        if (addonBit == 3) return (lookAddons & 0x03) == 0x03; // Both Addons 1 and 2
        // For specific addons, usually check bits: (lookAddons & addon_flag_for_1), (lookAddons & addon_flag_for_2)
        // The parameter should ideally be the specific addon flag (1, 2, 4, etc.)
        return (lookAddons & addonBit) != 0;
    }

    void setAddon(uint8_t addonBit, bool enabled) { // addonBit e.g. 1, 2, 4
        if (enabled) {
            lookAddons |= addonBit;
        } else {
            lookAddons &= ~addonBit;
        }
    }
};

} // namespace creatures
} // namespace core
} // namespace RME

#endif // RME_OUTFIT_H
