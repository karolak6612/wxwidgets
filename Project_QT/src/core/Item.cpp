#include "Item.h"
#include "items/ContainerItem.h"
#include "items/TeleportItem.h"
#include "items/DoorItem.h"
#include "items/DepotItem.h"
#include "items/PodiumItem.h"
#include <stdexcept> // For potential errors if provider is null
#include <QDebug>    // For qWarning() logging

namespace RME {

Item::Item(uint16_t item_id, IItemTypeProvider* provider, uint16_t item_subtype)
    : id(item_id), subtype(item_subtype), itemTypeProvider(provider) {
    if (!itemTypeProvider) {
        qWarning() << "Item created with null IItemTypeProvider for item ID" << item_id;
        // Note: We allow null provider but warn about it for debugging
        // Consider a global default provider or a null object provider pattern if needed.
    }
}

std::unique_ptr<Item> Item::create(uint16_t id, IItemTypeProvider* provider, uint16_t subtype) {
    if (!provider) {
        qWarning() << "Item::create called with null provider for item ID" << id 
                   << "- creating base Item without specialized functionality";
        return std::make_unique<Item>(id, provider, subtype);
    }

    // Check for specialized types using the provider
    if (provider->isContainer(id)) {
        return std::make_unique<ContainerItem>(id, provider, subtype);
    } else if (provider->isTeleport(id)) {
        return std::make_unique<TeleportItem>(id, provider, subtype);
    } else if (provider->isDoor(id)) {
        return std::make_unique<DoorItem>(id, provider, subtype);
    } else if (provider->isDepot(id)) { // Assuming isDepot exists on provider
        return std::make_unique<DepotItem>(id, provider, subtype);
    } else if (provider->isPodium(id)) { // Assuming isPodium exists on provider
        return std::make_unique<PodiumItem>(id, provider, subtype);
    }

    // Default case: create a base Item
    return std::make_unique<Item>(id, provider, subtype);
}

std::unique_ptr<Item> Item::deepCopy() const {
    // Create a new Item using the factory or direct constructor
    // Ensure the subtype and provider are passed correctly.
    auto newItem = std::make_unique<Item>(this->id, this->itemTypeProvider, this->subtype);
    this->copyBaseMembersTo(*newItem);
    return newItem;
}

void Item::copyBaseMembersTo(Item& targetItem) const {
    // targetItem.id = this->id; // Already set by constructor
    // targetItem.subtype = this->subtype; // Already set by constructor
    // targetItem.itemTypeProvider = this.itemTypeProvider; // Already set by constructor
    targetItem.attributes = this->attributes; // Deep copy of QMap
}

bool Item::hasSubtype() const {
    // Definition of "has subtype" can vary.
    // For stackable items, subtype is count. If count > 1, it "has subtype".
    // For splashes/fluids, subtype indicates fluid type.
    // For now, a simple check if it's not a common default (0 or 1 for count).
    // This logic might need refinement based on how g_items determined this.
    if (isStackable()) {
        return subtype > 1;
    }
    // For other types, subtype might be relevant even if 0 (e.g. fluid type 0)
    // For now, let's assume non-stackable items with subtype != 0 have a meaningful subtype.
    // A common default for non-countable items is 0 or 0xFFFF.
    // Let's assume subtype is meaningful if not 0 for non-stackables.
    return subtype != 0;
}

// Attribute Management
void Item::setAttribute(const QString& key, const QVariant& value) {
    attributes.insert(key, value);
}

QVariant Item::getAttribute(const QString& key) const {
    return attributes.value(key); // Returns default-constructed QVariant if key not found
}

bool Item::hasAttribute(const QString& key) const {
    return attributes.contains(key);
}

void Item::clearAttribute(const QString& key) {
    attributes.remove(key);
}

// Convenience Attribute Accessors
void Item::setUniqueID(uint16_t uid) {
    setAttribute("uid", uid);
}
uint16_t Item::getUniqueID() const {
    return getAttribute("uid").toUInt(); // QVariant handles conversion
}
void Item::setActionID(uint16_t aid) {
    setAttribute("aid", aid);
}
uint16_t Item::getActionID() const {
    return getAttribute("aid").toUInt();
}
void Item::setText(const QString& text) {
    setAttribute("text", text);
}
QString Item::getText() const {
    return getAttribute("text").toString();
}

// Item Properties (delegated to itemTypeProvider)
// Macro to safely call provider methods
#define PROVIDER_CALL(method, default_val, ...) (itemTypeProvider ? itemTypeProvider->method(__VA_ARGS__) : default_val)

QString Item::getName() const {
    return PROVIDER_CALL(getName, "Unknown Item", id);
}
QString Item::getDescription() const {
    return PROVIDER_CALL(getDescription, "", id);
}
double Item::getWeight() const {
    return PROVIDER_CALL(getWeight, 0.0, id, subtype);
}
bool Item::isBlocking() const {
    return PROVIDER_CALL(isBlocking, true, id); // Default to blocking if no provider
}
bool Item::isProjectileBlocking() const {
    return PROVIDER_CALL(isProjectileBlocking, true, id);
}
bool Item::isPathBlocking() const {
    return PROVIDER_CALL(isPathBlocking, true, id);
}
bool Item::isWalkable() const {
    return PROVIDER_CALL(isWalkable, false, id); // Default to not walkable
}
bool Item::isStackable() const {
    return PROVIDER_CALL(isStackable, false, id);
}
bool Item::isGround() const {
    return PROVIDER_CALL(isGround, false, id);
}
bool Item::isAlwaysOnTop() const {
    return PROVIDER_CALL(isAlwaysOnTop, false, id);
}
bool Item::isReadable() const {
    return PROVIDER_CALL(isReadable, false, id);
}
bool Item::isWriteable() const {
    return PROVIDER_CALL(isWriteable, false, id);
}
bool Item::isFluidContainer() const {
    return PROVIDER_CALL(isFluidContainer, false, id);
}
bool Item::isSplash() const {
    return PROVIDER_CALL(isSplash, false, id);
}
bool Item::isMoveable() const {
    return PROVIDER_CALL(isMoveable, true, id); // Most items are moveable by default
}
bool Item::hasHeight() const {
    return PROVIDER_CALL(hasHeight, false, id);
}
bool Item::isContainer() const {
    return PROVIDER_CALL(isContainer, false, id);
}
bool Item::isTeleport() const {
    return PROVIDER_CALL(isTeleport, false, id);
}
bool Item::isDoor() const {
    return PROVIDER_CALL(isDoor, false, id);
}
bool Item::isPodium() const {
    return PROVIDER_CALL(isPodium, false, id);
}
bool Item::isDepot() const {
    return PROVIDER_CALL(isDepot, false, id);
}

// Lighting support
bool Item::hasLight() const {
    return PROVIDER_CALL(hasLight, false, id);
}

uint8_t Item::getLightIntensity() const {
    return PROVIDER_CALL(getLightIntensity, 0, id);
}

uint8_t Item::getLightColor() const {
    return PROVIDER_CALL(getLightColor, 0, id);
}

/**
 * @brief Estimates the memory usage of this Item object.
 *
 * This method provides a basic estimation. A more accurate version
 * would iterate through the `attributes` QMap and sum the memory
 * used by keys and QVariant values.
 *
 * @return size_t Estimated memory usage in bytes.
 */
size_t Item::estimateMemoryUsage() const {
    size_t memory = sizeof(Item);
    // Add cost of attributes. This is a rough estimate.
    // A more accurate calculation would iterate attributes:
    // for (auto it = attributes.begin(); it != attributes.end(); ++it) {
    //     memory += it.key().capacity() + sizeof(QVariant); // QVariant size is tricky
    // }
    memory += attributes.size() * (sizeof(QString) + sizeof(QVariant) + 30); // Rough estimate per attribute
    return memory + 50; // Placeholder for basic item + general overhead
}

// OTBM Attribute Handling
bool Item::deserializeOtbmAttribute(uint8_t /*attributeId*/, RME::core::io::BinaryNode* /*node*/, RME::core::assets::AssetManager* /*assetManager*/) {
    return false; // Base implementation: unhandled attribute
}

void Item::serializeOtbmAttributes(RME::core::io::NodeFileWriteHandle& /*writer*/, RME::core::assets::AssetManager* /*assetManager*/) const {
    // Base implementation: no specific attributes to write
}

} // namespace RME
