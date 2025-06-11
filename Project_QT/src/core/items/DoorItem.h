#ifndef RME_DOORITEM_H
#define RME_DOORITEM_H

#include "core/Item.h"
#include <cstdint> // For uint8_t

namespace RME {

class DoorItem : public Item {
public:
    DoorItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);

    std::unique_ptr<Item> deepCopy() const override;

    uint8_t getDoorId() const { return m_doorId; }
    void setDoorId(uint8_t doorId) { m_doorId = doorId; }

    // bool isRealDoor() const; // This might depend on ItemData from provider
    // DoorType getDoorType() const; // This might depend on ItemData from provider

protected:
    void copyDerivedMembersTo(DoorItem& target) const;
    uint8_t m_doorId = 0; // Default door ID

public:
    // OTBM Attribute Handling
    bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) override;
    void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const override;
};

} // namespace RME
#endif // RME_DOORITEM_H
