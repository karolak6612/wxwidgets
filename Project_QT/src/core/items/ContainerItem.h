#ifndef RME_CONTAINERITEM_H
#define RME_CONTAINERITEM_H

#include "core/Item.h" // Base class RME::Item
#include <QList>
#include <memory> // For std::unique_ptr

namespace RME {

class ContainerItem : public Item {
public:
    ContainerItem(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);
    ~ContainerItem() override;

    std::unique_ptr<Item> deepCopy() const override;

    // Container specific methods
    void addItem(std::unique_ptr<Item> item);
    Item* getItem(int index) const;
    const QList<std::unique_ptr<Item>>& getContents() const { return m_contents; }
    QList<std::unique_ptr<Item>>& getContents() { return m_contents; } // Non-const access for modification
    int getItemCount() const { return m_contents.size(); }
    // virtual double getWeight() const override; // TODO: Calculate total weight
    // uint32_t getVolume() const; // Volume of the container itself from ItemData

    size_t estimateMemoryUsage() const override;

    // OTBM Attribute Handling
    bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager) override;
    void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const override;

protected:
    void copyDerivedMembersTo(ContainerItem& target) const;
    QList<std::unique_ptr<Item>> m_contents;
};

} // namespace RME
#endif // RME_CONTAINERITEM_H
