#include "core/items/DoorItem.h"
#include "core/io/BinaryNode.h"
#include "core/io/NodeFileWriteHandle.h"
#include "core/assets/AssetManager.h"
#include "core/io/otbm_constants.h" // For OTBM_ATTR_DOOR_ID etc.

namespace RME {

DoorItem::DoorItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype)
    : Item(id, provider, subtype), m_doorId(0) {
}

std::unique_ptr<Item> DoorItem::deepCopy() const {
    auto newDoor = std::make_unique<DoorItem>(getID(), getTypeProvider(), getSubtype());
    copyBaseMembersTo(*newDoor);
    copyDerivedMembersTo(*newDoor);
    return newDoor;
}

void DoorItem::copyDerivedMembersTo(DoorItem& target) const {
    target.m_doorId = this->m_doorId;
}

// OTBM Attribute Handling
bool DoorItem::deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) {
    if (attributeId == OTBM_ATTR_DOOR_ID) {
        uint8_t doorId;
        if (node->getU8(doorId)) {
            m_doorId = doorId;
            return true;
        }
        return false;
    }
    return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
}

void DoorItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    if (m_doorId != 0) {
        writer.addU8(OTBM_ATTR_DOOR_ID);
        writer.addU8(m_doorId);
    }
}

} // namespace RME
