#ifndef RME_SPAWNDATA_H
#define RME_SPAWNDATA_H

#include "core/Position.h" // Assuming Position is in core/ directly
#include <QStringList>
#include <QObject> // For Q_GADGET if we need it later for meta-type system, or signals/slots

// Forward declaration if needed by Position or other headers, though unlikely for SpawnData itself.
namespace RME {
namespace core {
namespace spawns {

class SpawnData {
    // Q_GADGET // Uncomment if properties or invokable methods are needed for QML or meta-system
public:
    SpawnData();
    SpawnData(const RME::Position& center, int radius, int intervalSeconds, const QStringList& creatureTypes);

    // Accessors
    const RME::Position& getCenter() const;
    int getRadius() const;
    int getIntervalSeconds() const;
    const QStringList& getCreatureTypes() const;
    bool isAutoCreated() const { return m_isAutoCreated; } // New getter

    // Mutators
    void setCenter(const RME::Position& center);
    void setRadius(int radius);
    void setIntervalSeconds(int intervalSeconds);
    void setCreatureTypes(const QStringList& creatureTypes);
    void setIsAutoCreated(bool val) { m_isAutoCreated = val; } // New setter

    // Utility methods
    void addCreatureType(const QString& type);
    bool removeCreatureType(const QString& type);

    // Optional: Equality operator for comparisons, e.g., in Map::removeSpawn
    bool operator==(const SpawnData& other) const;
    bool operator!=(const SpawnData& other) const;
    
    // Selection state (from original wxWidgets)
    bool isSelected() const { return m_selected; }
    void select() { m_selected = true; }
    void deselect() { m_selected = false; }
    
    // Deep copy method (from original wxWidgets)
    SpawnData deepCopy() const;
    
    // Utility methods
    bool containsPosition(const RME::Position& pos) const;
    QString getDescription() const;

private:
    RME::Position m_center;
    int m_radius = 0;
    int m_intervalSeconds = 60; // Default as per YAML
    QStringList m_creatureTypes;
    bool m_isAutoCreated = false;
    bool m_selected = false; // Selection state from wxWidgets
};

} // namespace spawns
} // namespace core
} // namespace RME
#endif // RME_SPAWNDATA_H
