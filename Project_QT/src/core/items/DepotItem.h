#ifndef RME_DEPOTITEM_H
#define RME_DEPOTITEM_H

#include "core/Item.h"
#include <cstdint> // For uint8_t

namespace RME {

class DepotItem : public Item {
public:
    DepotItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);

    std::unique_ptr<Item> deepCopy() const override;

    uint8_t getDepotId() const { return m_depotId; }
    void setDepotId(uint8_t depotId) { m_depotId = depotId; }

protected:
    void copyDerivedMembersTo(DepotItem& target) const;
    uint8_t m_depotId = 0; // Default depot ID

public:
    // OTBM Attribute Handling
    bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) override;
    void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const override;
};

} // namespace RME
#endif // RME_DEPOTITEM_H
