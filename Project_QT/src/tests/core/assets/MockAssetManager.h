#ifndef MOCK_ASSET_MANAGER_H
#define MOCK_ASSET_MANAGER_H

#include "core/assets/AssetManager.h"
#include "tests/core/assets/MockMaterialManager.h" // Now inherits core::assets::MaterialManager
#include "tests/core/assets/MockCreatureDatabase.h" // Now inherits core::assets::CreatureDatabase
#include "tests/core/MockItemTypeProvider.h" // For item access

// Forward declare ItemDatabase if AssetManager provides it and we are not fully mocking it here.
// namespace RME { namespace core { namespace assets { class ItemDatabase; }}} // Already in AssetManager.h

namespace RME {
namespace tests {

class MockAssetManager : public RME::core::assets::AssetManager {
public:
    // Constructor now takes the correctly typed mocks (or base pointers if preferred for flexibility)
    MockAssetManager(RME::tests::MockItemTypeProvider* itemProvider, // Still using MockItemTypeProvider conceptually for item data
                     RME::tests::MockCreatureDatabase* creatureDb,
                     RME::tests::MockMaterialManager* materialMgr)
        : RME::core::assets::AssetManager("", nullptr), // Call base constructor appropriately
          m_mockItemProvider(itemProvider),
          m_mockCreatureDb(creatureDb),
          m_mockMaterialMgr(materialMgr)
           {}

    // Override methods from AssetManager that are needed
    RME::core::assets::ItemDatabase* getItemDatabase() override {
        // MockItemTypeProvider is not an ItemDatabase.
        // If GroundBrush tests require item lookups via ItemDatabase,
        // this would need a proper MockItemDatabase that inherits core::assets::ItemDatabase.
        // For now, returning nullptr as ItemDatabase is not directly used by GroundBrush's core logic,
        // (it uses MaterialData for its items).
        return nullptr;
    }
    const RME::core::assets::ItemDatabase* getItemDatabase() const override { return nullptr; }

    RME::core::assets::CreatureDatabase* getCreatureDatabase() override {
        return m_mockCreatureDb; // Now type-compatible
    }
    const RME::core::assets::CreatureDatabase* getCreatureDatabase() const override {
        return m_mockCreatureDb; // Now type-compatible
    }

    RME::core::assets::MaterialManager* getMaterialManager() override {
        return m_mockMaterialMgr; // Now type-compatible
    }
    const RME::core::assets::MaterialManager* getMaterialManager() const override {
        return m_mockMaterialMgr; // Now type-compatible
    }

    // Provide direct access to specific mock instances for test setup convenience
    // This is useful if tests need to interact with the mock's specific setup methods.
    RME::tests::MockItemTypeProvider* getMockItemProvider() { return m_mockItemProvider; }
    // getMockCreatureDb() and getMockMaterialManager() are already implicitly available
    // if the above getters return the derived mock types, but explicit ones can be clearer.
    RME::tests::MockCreatureDatabase* getTypedMockCreatureDb() { return m_mockCreatureDb; }
    RME::tests::MockMaterialManager* getTypedMockMaterialManager() { return m_mockMaterialMgr; }


private:
    RME::tests::MockItemTypeProvider* m_mockItemProvider;   // Non-owning
    RME::tests::MockCreatureDatabase* m_mockCreatureDb;     // Non-owning
    RME::tests::MockMaterialManager* m_mockMaterialMgr;     // Non-owning
};

} // namespace tests
} // namespace RME

#endif // MOCK_ASSET_MANAGER_H
