#ifndef RME_TELEPORTITEM_H
#define RME_TELEPORTITEM_H

#include "core/Item.h"
#include "core/Position.h" // For RME::Position

namespace RME {

class TeleportItem : public Item {
public:
    TeleportItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);

    std::unique_ptr<Item> deepCopy() const override;

    const Position& getDestination() const { return m_destination; }
    void setDestination(const Position& dest) { m_destination = dest; }
    bool hasDestination() const { return m_destination != Position(0,0,0); } // Assuming 0,0,0 is invalid/no dest

protected:
    void copyDerivedMembersTo(TeleportItem& target) const;
    Position m_destination;

public:
    // OTBM Attribute Handling
    bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) override;
    void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const override;
};

} // namespace RME
#endif // RME_TELEPORTITEM_H
