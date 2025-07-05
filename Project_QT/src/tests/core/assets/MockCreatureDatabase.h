#ifndef MOCK_CREATURE_DATABASE_H
#define MOCK_CREATURE_DATABASE_H

#include "core/assets/CreatureDatabase.h" // Base class
#include "core/assets/CreatureData.h"
#include <QMap>
#include <QString>

namespace RME {
namespace tests {

class MockCreatureDatabase : public RME::core::assets::CreatureDatabase {
public:
    MockCreatureDatabase() {
        // Call base constructor if needed.
        // RME::core::assets::CreatureDatabase::CreatureDatabase();

        // Setup a default invalid creature data (if base doesn't provide one suitable for mocking)
        // The base class already has an 'invalidCreatureData' member, which should be sufficient.
        // m_invalidCreatureDataForMock.name = "Invalid Mock Creature";
    }
    ~MockCreatureDatabase() override = default;

    // --- Methods for test setup ---
    void addOrUpdateCreatureData(const QString& name, const RME::core::assets::CreatureData& data) {
        // The base class uses a PIMPL (d-pointer) for m_creatures.
        // We can't directly access m_creatures.
        // So, this mock will have its own storage and override the getters.
        m_mockCreatures[name] = data;
    }

    void clear() {
        m_mockCreatures.clear();
    }

    // --- Override virtual methods from CreatureDatabase ---
    // bool loadFromXML(const QString& filePath) override {
    //     Q_UNUSED(filePath);
    //     return true; // Mock: do nothing or log
    // }
    // bool importFromOtServerXml(const QString& filePath) override {
    //     Q_UNUSED(filePath);
    //     return true; // Mock: do nothing or log
    // }

    const RME::core::assets::CreatureData* getCreatureData(const QString& name) const override {
        auto it = m_mockCreatures.constFind(name);
        if (it != m_mockCreatures.constEnd()) {
            return &(*it);
        }
        // Fallback to base class's default/invalid creature data if desired, or return nullptr.
        // The base class returns a reference to its 'invalidCreatureData' if not found.
        // To behave similarly:
        // return &RME::core::assets::CreatureDatabase::getDefaultCreatureData();
        // However, the base's invalidCreatureData is not accessible directly if it's in the PIMPL.
        // For a mock, returning nullptr if not found in m_mockCreatures is often clearer for tests.
        // Or, if the base's getDefaultCreatureData() is suitable:
        if (m_mockCreatures.empty() && name != "Invalid Mock Creature") { // Avoid infinite recursion if base calls this
             //return RME::core::assets::CreatureDatabase::getCreatureData(name); // Call base if we want its default
        }
        static RME::core::assets::CreatureData localInvalid;
        localInvalid.name = "MockNotFound";
        return &localInvalid; // Return our own local static invalid if not found in mock map.
    }

    // const RME::core::assets::CreatureData& getDefaultCreatureData() const override {
    //     // This method in base might not be virtual or might have specific internal logic.
    //     // If we need to override its behavior for tests:
    //     // static RME::core::assets::CreatureData localDefault;
    //     // localDefault.name = "Mock Default Creature";
    //     // return localDefault;
    //     // For now, assume base implementation is fine or not overridden.
    //     return RME::core::assets::CreatureDatabase::getDefaultCreatureData();
    // }

    int getCreatureCount() const override {
        return m_mockCreatures.size();
    }

    QMap<QString, RME::core::assets::CreatureData> getAllCreatures() const override {
        // The base class returns a copy.
        return m_mockCreatures;
    }

private:
    QMap<QString, RME::core::assets::CreatureData> m_mockCreatures;
    // RME::core::assets::CreatureData m_invalidCreatureDataForMock;
};

} // namespace tests
} // namespace RME

#endif // MOCK_CREATURE_DATABASE_H
