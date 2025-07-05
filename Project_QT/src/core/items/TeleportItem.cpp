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
    switch (attributeId) {
        case OTBM_ATTR_TELE_DEST_X: {
            uint16_t x;
            if (node->getU16(x)) {
                m_destination.setX(x);
                return true;
            }
            return false;
        }
        case OTBM_ATTR_TELE_DEST_Y: {
            uint16_t y;
            if (node->getU16(y)) {
                m_destination.setY(y);
                return true;
            }
            return false;
        }
        case OTBM_ATTR_TELE_DEST_Z: {
            uint8_t z;
            if (node->getU8(z)) {
                m_destination.setZ(z);
                return true;
            }
            return false;
        }
        default:
            return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
    }
}

void TeleportItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager);
    if (m_destination.isValid()) {
        // Serialize teleport destination coordinates
        writer.addU8(OTBM_ATTR_TELE_DEST_X);
        writer.addU16(static_cast<uint16_t>(m_destination.x()));
        
        writer.addU8(OTBM_ATTR_TELE_DEST_Y);
        writer.addU16(static_cast<uint16_t>(m_destination.y()));
        
        writer.addU8(OTBM_ATTR_TELE_DEST_Z);
        writer.addU8(static_cast<uint8_t>(m_destination.z()));
    }
}

} // namespace RME
