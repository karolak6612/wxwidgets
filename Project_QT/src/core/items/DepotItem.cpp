#include "core/items/DepotItem.h"
#include "core/io/BinaryNode.h"
#include "core/io/NodeFileWriteHandle.h"
#include "core/assets/AssetManager.h"
#include "core/io/otbm_constants.h" // For OTBM_ATTR_DEPOT_ID etc.

namespace RME {

DepotItem::DepotItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype)
    : Item(id, provider, subtype), m_depotId(0) {
}

std::unique_ptr<Item> DepotItem::deepCopy() const {
    auto newDepot = std::make_unique<DepotItem>(getID(), getTypeProvider(), getSubtype());
    copyBaseMembersTo(*newDepot);
    copyDerivedMembersTo(*newDepot);
    return newDepot;
}

void DepotItem::copyDerivedMembersTo(DepotItem& target) const {
    target.m_depotId = this->m_depotId;
}

// OTBM Attribute Handling
bool DepotItem::deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) {
    // TODO: Implement actual deserialization for DepotItem (e.g., m_depotId)
    // Example: if (attributeId == OTBM_ATTR_DEPOT_ID) { /* set m_depotId from node */ return true; }
    return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
}

void DepotItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    // TODO: Implement actual serialization for DepotItem (e.g., m_depotId)
    // Example: if (m_depotId != 0) { writer.writeU8(OTBM_ATTR_DEPOT_ID); writer.writeU8(m_depotId); }
}

} // namespace RME
