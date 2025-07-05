#include "core/items/TeleportItem.h"
#include "core/io/BinaryNode.h"
#include "core/io/NodeFileWriteHandle.h"
#include "core/assets/AssetManager.h"
#include "core/io/otbm_constants.h" // For OTBM_ATTR_TELE_DEST etc.

namespace RME {

TeleportItem::TeleportItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype)
    : Item(id, provider, subtype), m_destination(0,0,0) { // Default to no destination
}

std::unique_ptr<Item> TeleportItem::deepCopy() const {
    auto newTele = std::make_unique<TeleportItem>(getID(), getTypeProvider(), getSubtype());
    copyBaseMembersTo(*newTele);
    copyDerivedMembersTo(*newTele);
    return newTele;
}

void TeleportItem::copyDerivedMembersTo(TeleportItem& target) const {
    target.m_destination = this->m_destination;
}

// OTBM Attribute Handling
bool TeleportItem::deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) {
    // TODO: Implement actual deserialization for TeleportItem (e.g., destination)
    // Example: if (attributeId == OTBM_ATTR_TELE_DEST) { /* read Position from node */ return true; }
    return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
}

void TeleportItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    // TODO: Implement actual serialization for TeleportItem (e.g., destination)
    // Example: if (hasDestination()) { writer.writeU8(OTBM_ATTR_TELE_DEST); /* write m_destination */ }
}

} // namespace RME
