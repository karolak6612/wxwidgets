#ifndef RME_PODIUMITEM_H
#define RME_PODIUMITEM_H

#include "core/Item.h"
#include "core/creatures/Outfit.h" // For RME::Outfit
#include <cstdint> // For uint8_t

namespace RME {

class PodiumItem : public Item {
public:
    PodiumItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);

    std::unique_ptr<Item> deepCopy() const override;

    const Outfit& getOutfit() const { return m_outfit; }
    void setOutfit(const Outfit& outfit) { m_outfit = outfit; }

    uint8_t getDirection() const { return m_direction; }
    void setDirection(uint8_t direction) { m_direction = direction; }

    bool getShowOutfit() const { return m_showOutfit; }
    void setShowOutfit(bool show) { m_showOutfit = show; }

    bool getShowMount() const { return m_showMount; }
    void setShowMount(bool show) { m_showMount = show; }

    bool getShowPlatform() const { return m_showPlatform; }
    void setShowPlatform(bool show) { m_showPlatform = show; }

    // bool hasShowOutfit() const { return m_showOutfit && m_outfit.lookType != 0; } // Original logic
    // bool hasShowMount() const { return m_showMount && m_outfit.lookMount != 0; } // Original logic

protected:
    void copyDerivedMembersTo(PodiumItem& target) const;
    Outfit m_outfit;
    uint8_t m_direction = 0; // Default direction
    bool m_showOutfit = true;
    bool m_showMount = true;
    bool m_showPlatform = true;

public:
    // OTBM Attribute Handling
    bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) override;
    void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const override;
};

} // namespace RME
#endif // RME_PODIUMITEM_H
