#ifndef RME_MOCK_ITEM_TYPE_PROVIDER_H
#define RME_MOCK_ITEM_TYPE_PROVIDER_H

#include "core/IItemTypeProvider.h" // Original interface
#include <QMap>
#include <QString>
#include <cstdint>

// Basic struct to hold mock item properties
struct MockItemData {
    QString name = "Mock Item";
    QString description = "A mock item type.";
    uint32_t flags = 0;
    double weight = 1.0;
    bool isBlocking = false;
    bool isProjectileBlocking = false;
    bool isPathBlocking = false;
    bool isWalkable = true;
    bool isStackable = false;
    bool isGround = false;
    bool isAlwaysOnTop = false;
    bool isReadable = false;
    bool isWriteable = false;
    bool isFluidContainer = false;
    bool isSplash = false;
    bool isMoveable = true;
    bool hasHeight = false;
    bool isContainer = false;
    bool isTeleport = false;
    bool isDoor = false;
    bool isPodium = false;
    bool isDepot = false;
};

namespace RME {

class MockItemTypeProvider : public IItemTypeProvider {
public:
    QMap<uint16_t, MockItemData> mockData;

    MockItemTypeProvider() = default;
    ~MockItemTypeProvider() override = default;

    // Helper to easily add or modify mock item data for tests
    void setMockData(uint16_t id, const MockItemData& data) {
        mockData[id] = data;
    }

    QString getName(uint16_t id) const override {
        return mockData.value(id, MockItemData()).name;
    }
    QString getDescription(uint16_t id) const override {
        return mockData.value(id, MockItemData()).description;
    }
    uint32_t getFlags(uint16_t id) const override {
        return mockData.value(id, MockItemData()).flags;
    }
    double getWeight(uint16_t id, uint16_t subtype) const override {
        const auto& data = mockData.value(id, MockItemData());
        return data.isStackable && subtype > 0 ? data.weight * subtype : data.weight;
    }
    bool isBlocking(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isBlocking;
    }
    bool isProjectileBlocking(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isProjectileBlocking;
    }
    bool isPathBlocking(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isPathBlocking;
    }
    bool isWalkable(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isWalkable;
    }
    bool isStackable(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isStackable;
    }
    bool isGround(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isGround;
    }
    bool isAlwaysOnTop(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isAlwaysOnTop;
    }
    bool isReadable(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isReadable;
    }
    bool isWriteable(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isWriteable;
    }
    bool isFluidContainer(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isFluidContainer;
    }
    bool isSplash(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isSplash;
    }
    bool isMoveable(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isMoveable;
    }
    bool hasHeight(uint16_t id) const override {
        return mockData.value(id, MockItemData()).hasHeight;
    }
    bool isContainer(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isContainer;
    }
    bool isTeleport(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isTeleport;
    }
    bool isDoor(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isDoor;
    }
    bool isPodium(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isPodium;
    }
    bool isDepot(uint16_t id) const override {
        return mockData.value(id, MockItemData()).isDepot;
    }
};

} // namespace RME

#endif // RME_MOCK_ITEM_TYPE_PROVIDER_H
