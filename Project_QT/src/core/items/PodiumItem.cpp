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
    // TODO: Implement actual deserialization for PodiumItem (outfit, direction, flags)
    // Example: if (attributeId == OTBM_ATTR_PODIUM_OUTFIT) { /* read outfit from node */ return true; }
    return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
}

void PodiumItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    // TODO: Implement actual serialization for PodiumItem (outfit, direction, flags)
    // Example: writer.writeU8(OTBM_ATTR_PODIUM_OUTFIT); /* write m_outfit */
}

} // namespace RME
