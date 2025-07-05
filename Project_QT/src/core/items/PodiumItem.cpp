#include "core/items/PodiumItem.h"
#include "core/io/BinaryNode.h"
#include "core/io/NodeFileWriteHandle.h"
#include "core/assets/AssetManager.h"
#include "core/io/otbm_constants.h" // For OTBM_ATTR_PODIUM_OUTFIT etc.

namespace RME {

PodiumItem::PodiumItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype)
    : Item(id, provider, subtype), m_direction(0), m_showOutfit(true), m_showMount(true), m_showPlatform(true) {
    // m_outfit is default constructed
}

std::unique_ptr<Item> PodiumItem::deepCopy() const {
    auto newPodium = std::make_unique<PodiumItem>(getID(), getTypeProvider(), getSubtype());
    copyBaseMembersTo(*newPodium);
    copyDerivedMembersTo(*newPodium);
    return newPodium;
}

void PodiumItem::copyDerivedMembersTo(PodiumItem& target) const {
    target.m_outfit = this->m_outfit;
    target.m_direction = this->m_direction;
    target.m_showOutfit = this->m_showOutfit;
    target.m_showMount = this->m_showMount;
    target.m_showPlatform = this->m_showPlatform;
}

// OTBM Attribute Handling
bool PodiumItem::deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) {
    switch (attributeId) {
        case OTBM_ATTR_PODIUM_DIRECTION: {
            uint8_t direction;
            if (node->getU8(direction)) {
                m_direction = static_cast<Direction>(direction);
                return true;
            }
            return false;
        }
        case OTBM_ATTR_PODIUM_FLAGS: {
            uint8_t flags;
            if (node->getU8(flags)) {
                m_showFlags = flags;
                return true;
            }
            return false;
        }
        // TODO: Implement OTBM_ATTR_PODIUM_OUTFIT when outfit serialization format is defined
        default:
            return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
    }
}

void PodiumItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    // Serialize podium direction
    if (m_direction != Direction::SOUTH) { // Only serialize if not default
        writer.addU8(OTBM_ATTR_PODIUM_DIRECTION);
        writer.addU8(static_cast<uint8_t>(m_direction));
    }
    
    // Serialize podium display flags
    if (m_showFlags != 0) {
        writer.addU8(OTBM_ATTR_PODIUM_FLAGS);
        writer.addU8(m_showFlags);
    }
    
    // TODO: Implement OTBM_ATTR_PODIUM_OUTFIT when outfit serialization format is defined
}

} // namespace RME
