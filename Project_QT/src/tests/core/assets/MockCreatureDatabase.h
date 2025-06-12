#ifndef MOCK_CREATURE_DATABASE_H
#define MOCK_CREATURE_DATABASE_H

#include "core/assets/CreatureDatabase.h" // Inherit from the real one if desired, or just mimic interface
#include "core/assets/CreatureData.h"
#include <QMap>
#include <QString>

namespace RME {
namespace tests {
// If CreatureDatabase is an interface or has virtual methods we want to override:
// class MockCreatureDatabase : public RME::core::assets::CreatureDatabase {
// Otherwise, create a standalone mock that fulfills the needs of the tests.
// For simplicity, let's create a standalone mock that can be used by MockEditorController.

class MockCreatureDatabase {
public:
    MockCreatureDatabase() {
        // Setup a default invalid creature data
        m_invalidCreatureData.name = "Invalid Creature";
    }
    ~MockCreatureDatabase() = default;

    // Method to add or update mock creature data
    void addOrUpdateCreatureData(const QString& name, const RME::core::assets::CreatureData& data) {
        m_creatures[name] = data;
    }

    // Mimic methods from the real CreatureDatabase that CreatureBrush might use
    const RME::core::assets::CreatureData* getCreatureData(const QString& name) const {
        auto it = m_creatures.constFind(name);
        if (it != m_creatures.constEnd()) {
            return &(*it);
        }
        return &m_invalidCreatureData; // Return a pointer to a default invalid object
    }

    const RME::core::assets::CreatureData& getDefaultCreatureData() const {
        // In a real scenario, this might be a specific "unknown" creature.
        // For tests, it can be the same as invalid or a specific default.
        return m_invalidCreatureData;
    }


    void clear() {
        m_creatures.clear();
    }

private:
    QMap<QString, RME::core::assets::CreatureData> m_creatures;
    RME::core::assets::CreatureData m_invalidCreatureData; // Used for returning a valid pointer to default data
};

} // namespace tests
} // namespace RME

#endif // MOCK_CREATURE_DATABASE_H
