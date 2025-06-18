#include "core/items/ContainerItem.h"
#include "core/io/BinaryNode.h"
#include "core/io/NodeFileWriteHandle.h"
#include "core/assets/AssetManager.h"
#include "core/io/otbm_constants.h" // For OTBM_ATTR_CONTAINER_ITEMS etc.

namespace RME {

ContainerItem::ContainerItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype)
    : Item(id, provider, subtype) {
}

ContainerItem::~ContainerItem() {
    // m_contents unique_ptrs will auto-delete
}

std::unique_ptr<Item> ContainerItem::deepCopy() const {
    auto newCont = std::make_unique<ContainerItem>(getID(), getTypeProvider(), getSubtype());
    copyBaseMembersTo(*newCont);
    copyDerivedMembersTo(*newCont);
    return newCont;
}

void ContainerItem::copyDerivedMembersTo(ContainerItem& target) const {
    for (const auto& item : m_contents) {
        if (item) {
            target.addItem(item->deepCopy());
        }
    }
}

void ContainerItem::addItem(std::unique_ptr<Item> item) {
    if (item) {
        m_contents.append(std::move(item));
    }
}

Item* ContainerItem::getItem(int index) const {
    if (index >= 0 && index < m_contents.size()) {
        return m_contents.at(index).get();
    }
    return nullptr;
}

size_t ContainerItem::estimateMemoryUsage() const {
     size_t memory = Item::estimateMemoryUsage();
     memory += sizeof(m_contents); // Size of QList itself
     for(const auto& itemPtr : m_contents){
         if(itemPtr){
             memory += itemPtr->estimateMemoryUsage();
         }
     }
     return memory;
}

// OTBM Attribute Handling
bool ContainerItem::deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) {
    if (attributeId == OTBM_ATTR_CONTAINER_ITEMS) {
        // Container items are stored as nested node structure
        // Each item in the container is represented as a child node
        // This is handled by the OTBM I/O system during tile loading
        // The actual item deserialization happens in the main OTBM parsing loop
        // We just need to acknowledge that we handle this attribute
        return true;
    }
    return Item::deserializeOtbmAttribute(attributeId, node, assetManager);
}

void ContainerItem::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const {
    Item::serializeOtbmAttributes(writer, assetManager); // Call base if it might do something
    // Serialize container contents if not empty
    if (!m_contents.isEmpty()) {
        // Container items are serialized as child nodes in OTBM format
        // Each item becomes a separate OTBM_NODE_ITEM child node
        // This is handled by the OTBM I/O system during tile saving
        
        // Mark that this container has items (for OTBM format compatibility)
        writer.addU8(OTBM_ATTR_CONTAINER_ITEMS);
        writer.addU16(static_cast<uint16_t>(m_contents.size())); // Item count
        
        // Note: The actual item serialization is handled by the OTBM I/O system
        // which will call serializeOtbmAttributes on each contained item
        // and create proper OTBM_NODE_ITEM child nodes
    }
}

} // namespace RME
