#ifndef RME_ITEM_H
#define RME_ITEM_H

#include "IItemTypeProvider.h" // Interface for item properties
#include <cstdint>
#include <memory>      // For std::unique_ptr
#include <QMap>
#include <QString>
#include <QVariant>

// Forward declarations for derived item types can be added here if needed later
// namespace RME { class ContainerItem; }

namespace RME {

namespace core {
    namespace io {
        class BinaryNode;
        class NodeFileWriteHandle;
    } // namespace io
    namespace assets {
        class AssetManager;
    } // namespace assets
} // namespace core

class Item {
public:
    // Attributes map type
    using AttributeMap = QMap<QString, QVariant>;

protected:
    uint16_t id;
    uint16_t subtype; // Used for count, charges, fluid type, etc.
    AttributeMap attributes;
    IItemTypeProvider* itemTypeProvider; // Non-owning pointer, set externally

public:
    // Constructor
    Item(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0); // subtype 0 is often default
    virtual ~Item() = default;

    // Factory method
    static std::unique_ptr<Item> create(uint16_t id, IItemTypeProvider* provider, uint16_t subtype = 0);
    // Add overloads if subtype is not always needed or provider is globally accessible (though passed provider is better)
    // static std::unique_ptr<Item> create(uint16_t id, IItemTypeProvider* provider);


    // Virtual deep copy
    virtual std::unique_ptr<Item> deepCopy() const;

    // ID and Subtype
    uint16_t getID() const { return id; }
    uint16_t getSubtype() const { return subtype; }
    void setSubtype(uint16_t newSubtype) { subtype = newSubtype; }
    bool hasSubtype() const; // e.g. if subtype has a special meaning like count > 1

    // Attributes (UID, AID, text, description, tier, etc.)
    void setAttribute(const QString& key, const QVariant& value);
    QVariant getAttribute(const QString& key) const;
    bool hasAttribute(const QString& key) const;
    void clearAttribute(const QString& key);
    const AttributeMap& getAllAttributes() const { return attributes; }
    void setAllAttributes(const AttributeMap& newAttributes) { attributes = newAttributes; }


    // Common attribute accessors (convenience)
    void setUniqueID(uint16_t uid); // Often "uid"
    uint16_t getUniqueID() const;
    void setActionID(uint16_t aid);   // Often "aid"
    uint16_t getActionID() const;
    void setText(const QString& text); // Often "text"
    QString getText() const;
    // Add more for description, tier etc. as needed

    // Item properties (delegated to itemTypeProvider)
    // These methods assume itemTypeProvider is valid. Add checks if necessary.
    QString getName() const;
    QString getDescription() const;
    double getWeight() const; // Considers stackable items and subtype

    bool isBlocking() const;
    bool isProjectileBlocking() const;
    bool isPathBlocking() const;
    bool isWalkable() const;
    bool isStackable() const;
    bool isGround() const;
    bool isAlwaysOnTop() const;
    bool isReadable() const;
    bool isWriteable() const;
    bool isFluidContainer() const;
    bool isSplash() const;
    bool isMoveable() const;
    bool hasHeight() const;
    bool isContainer() const;
    bool isTeleport() const;
    bool isDoor() const;
    bool isPodium() const;
    bool isDepot() const;

    // Selection state (if managed by Item itself, though often by editor/tile)
    // bool isSelected() const;
    // void setSelected(bool selected);

    // Provide access to the type provider if needed by other systems
    IItemTypeProvider* getTypeProvider() const { return itemTypeProvider; }

    /**
     * @brief Estimates the memory usage of this Item object.
     *
     * This method should account for the size of the Item object itself,
     * its attributes, and any other dynamically allocated memory or
     * complex members it owns.
     *
     * @return size_t Estimated memory usage in bytes.
     */
    virtual size_t estimateMemoryUsage() const;

    // OTBM Attribute Handling
    virtual bool deserializeOtbmAttribute(uint8_t attributeId, RME::core::io::BinaryNode* node, RME::core::assets::AssetManager* assetManager);
    virtual void serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& writer, RME::core::assets::AssetManager* assetManager) const;

protected:
    // Helper for deep copy of base members
    void copyBaseMembersTo(Item& targetItem) const;
};

// Placeholder derived classes (can be expanded in future tasks)
/*
class ContainerItem : public Item { ... };
class TeleportItem : public Item { ... };
class DoorItem : public Item { ... };
class PodiumItem : public Item { ... };
*/

} // namespace RME

#endif // RME_ITEM_H
