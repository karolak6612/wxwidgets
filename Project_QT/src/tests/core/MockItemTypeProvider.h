#ifndef MOCK_ITEMTYPEPROVIDER_H
#define MOCK_ITEMTYPEPROVIDER_H

#include "core/assets/IItemTypeProvider.h"
#include "core/assets/ItemData.h" // For RME::core::assets::ItemData definition
#include <QMap>
#include <QString>
#include <cstdint> // For uint16_t

// Forward declare AssetManager if it's part of IItemTypeProvider's interface
// namespace RME { namespace core { namespace assets { class AssetManager; }}}


namespace RME {
namespace tests {

// This struct is used internally by MockItemTypeProvider to store mock data.
// It should mirror relevant fields from RME::core::assets::ItemData for testing.
struct MockItemData {
    QString name = "Unknown Mock Item";
    uint16_t id = 0; // Typically server ID
    bool isGround = false;
    bool isBorder = false;
    // Add other flags/attributes as needed for different brush tests
    uint16_t clientID = 0; // Often same as ID if not specified
    QString materialId;    // NEW FIELD

    // Default constructor
    MockItemData() = default;

    // Constructor for easier initialization in tests
    MockItemData(QString n, uint16_t an_id, bool ground, bool border = false, QString matId = QString(), uint16_t cId = 0)
        : name(n), id(an_id), isGround(ground), isBorder(border), materialId(matId), clientID(cId == 0 ? an_id : cId) {}
};


class MockItemTypeProvider : public RME::core::assets::IItemTypeProvider {
public:
    MockItemTypeProvider() = default;

    void setMockData(uint16_t itemId, const MockItemData& data) {
        m_mockData[itemId] = data;
        // Invalidate cache for this item if it's updated
        if (m_convertedDataCache.contains(itemId)) {
            m_convertedDataCache.remove(itemId);
        }
    }

    // --- Implementation of IItemTypeProvider ---
    const RME::core::assets::ItemData* getItemData(uint16_t server_id) const override {
        auto it = m_mockData.constFind(server_id);
        if (it != m_mockData.constEnd()) {
            // Need to convert MockItemData to RME::core::assets::ItemData
            // This is problematic if we only store MockItemData.
            // For tests to work, this should return a real ItemData struct.
            // Let's store RME::core::assets::ItemData directly, populated from MockItemData.

            // If m_convertedDataCache is not populated for server_id, create it.
            // This is a bit inefficient for a const method, ideally populate on setMockData.
            // For simplicity in mock:
            if (m_convertedDataCache.constFind(server_id) == m_convertedDataCache.constEnd()) {
                RME::core::assets::ItemData realData;
                realData.serverID = it.value().id;
                realData.clientID = it.value().clientID != 0 ? it.value().clientID : it.value().id;
                realData.name = it.value().name;
                realData.isGround = it.value().isGround;
                realData.isBorder = it.value().isBorder;
                realData.materialId = it.value().materialId; // Copy the new field
                // Copy other relevant fields from MockItemData to ItemData as needed by tests
                m_convertedDataCache.insert(server_id, realData);
            }
            return &m_convertedDataCache[server_id];
        }
        return nullptr; // Not found
    }

    // getItemType is an old name, getItemData is current.
    // const RME::core::assets::ItemType* getItemType(uint16_t server_id) const override {
    //     return getItemData(server_id); // Assuming ItemType is typedef or same as ItemData
    // }


    // getAssetManager() might not be relevant for a pure ItemTypeProvider mock,
    // unless ItemData itself needs it for some deeper lookups (unlikely for static data).
    // If IItemTypeProvider has this method, it needs to be implemented.
    // RME::core::assets::AssetManager* getAssetManager() const override {
    //     return nullptr; // Or a mock asset manager if needed by ItemData methods
    // }

    void clear() {
        m_mockData.clear();
        m_convertedDataCache.clear();
    }

private:
    QMap<uint16_t, MockItemData> m_mockData; // Internal storage using simple struct
    // Cache to hold converted RME::core::assets::ItemData objects
    mutable QMap<uint16_t, RME::core::assets::ItemData> m_convertedDataCache;
};

} // namespace tests
} // namespace RME

#endif // MOCK_ITEMTYPEPROVIDER_H
