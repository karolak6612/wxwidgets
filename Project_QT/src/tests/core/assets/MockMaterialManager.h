#ifndef MOCK_MATERIAL_MANAGER_H
#define MOCK_MATERIAL_MANAGER_H

#include "core/assets/MaterialManager.h" // Base class
#include "core/assets/MaterialData.h"
#include <QMap>
#include <QString>

namespace RME {
namespace tests {

class MockMaterialManager : public RME::core::assets::MaterialManager {
public:
    MockMaterialManager() {
        // Call base constructor if necessary, or ensure base state is valid.
        // RME::core::assets::MaterialManager::MaterialManager();
    }
    ~MockMaterialManager() override = default;

    // --- Methods for test setup ---
    void addMaterial(const RME::core::assets::MaterialData& material) {
        m_materials[material.id] = material;
        if (material.isGround()) {
            const auto* specifics = std::get_if<RME::core::assets::MaterialGroundSpecifics>(&material.specificData);
            if (specifics) {
                for (const auto& itemEntry : specifics->items) {
                    m_itemToMaterialMap[itemEntry.itemId] = material.id;
                }
            }
        }
    }

    void clear() {
        m_materials.clear();
        m_itemToMaterialMap.clear();
        // m_lastError from base class could be cleared too if used.
    }

    // --- Override virtual methods from MaterialManager ---
    // bool loadMaterialsFromDirectory(const QString& baseDir, const QString& mainXmlFile,
    //                                 RME::core::assets::AssetManager& assetManager) override {
    //     // Mock implementation: typically do nothing or log, return true.
    //     // For tests, materials are usually added directly via addMaterial().
    //     Q_UNUSED(baseDir);
    //     Q_UNUSED(mainXmlFile);
    //     Q_UNUSED(assetManager);
    //     return true;
    // }

    const RME::core::assets::MaterialData* getMaterial(const QString& id) const override {
        auto it = m_materials.constFind(id);
        if (it != m_materials.constEnd()) {
            return &(*it);
        }
        return nullptr;
    }

    const QMap<QString, RME::core::assets::MaterialData>& getAllMaterials() const override {
        return m_materials;
    }

    // getLastError() is not virtual in base, so can't override.
    // We can hide it if needed: QString getLastError() const { return "MockError"; }

    // Method needed by getMaterialFromTile helper (not part of base MaterialManager interface)
    // This should ideally be part of MaterialManager interface if it's a common need,
    // or GroundBrush's helper finds another way (e.g. iterates all materials).
    // For now, keeping it specific to the mock for testing convenience.
    const RME::core::assets::MaterialData* getMaterialForItem(uint16_t itemId) const {
        auto it = m_itemToMaterialMap.constFind(itemId);
        if (it != m_itemToMaterialMap.constEnd()) {
            return getMaterial(it.value());
        }
        return nullptr;
    }


private:
    QMap<QString, RME::core::assets::MaterialData> m_materials;
    QMap<uint16_t, QString> m_itemToMaterialMap;
};

} // namespace tests
} // namespace RME

#endif // MOCK_MATERIAL_MANAGER_H
