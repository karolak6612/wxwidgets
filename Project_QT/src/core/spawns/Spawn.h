#ifndef RME_CORE_SPAWNS_SPAWN_H
#define RME_CORE_SPAWNS_SPAWN_H

#include "core/Position.h"
#include <QString>
#include <QStringList>

namespace RME {
namespace core {
namespace spawns {

/**
 * @brief Unified Spawn class that replaces the broken dual-class architecture
 * 
 * This class combines all spawn data and behavior in one place, eliminating
 * the maintenance nightmare of the old Spawn wrapper + SpawnData system.
 * 
 * Key improvements:
 * - Single source of truth for spawn data
 * - No namespace confusion
 * - No synchronization issues
 * - Cleaner API with one way to do things
 * - Better performance (no unnecessary copying)
 */
class Spawn {
public:
    // Construction
    Spawn() = default;
    explicit Spawn(const Position& center, int radius = 1, int intervalSeconds = 600);
    
    // Copy and move constructors
    Spawn(const Spawn& other) = default;
    Spawn(Spawn&& other) noexcept = default;
    
    // Assignment operators
    Spawn& operator=(const Spawn& other) = default;
    Spawn& operator=(Spawn&& other) noexcept = default;
    
    // Destructor
    ~Spawn() = default;
    
    // Core Properties
    const Position& getCenter() const { return m_center; }
    void setCenter(const Position& center);
    
    int getRadius() const { return m_radius; }
    void setRadius(int radius);
    
    int getIntervalSeconds() const { return m_intervalSeconds; }
    void setIntervalSeconds(int seconds);
    
    const QStringList& getCreatureTypes() const { return m_creatureTypes; }
    void setCreatureTypes(const QStringList& types);
    void addCreatureType(const QString& type);
    bool removeCreatureType(const QString& type);
    
    // State Management
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }
    void select() { m_selected = true; }
    void deselect() { m_selected = false; }
    
    bool isAutoCreated() const { return m_autoCreated; }
    void setAutoCreated(bool autoCreated) { m_autoCreated = autoCreated; }
    
    // Legacy compatibility methods (for smooth migration)
    void setSpawnTime(int time) { setIntervalSeconds(time); }
    int getSpawnTime() const { return getIntervalSeconds(); }
    void setSize(int size) { setRadius(size); }
    int getSize() const { return getRadius(); }
    
    // Single creature type methods (for compatibility)
    void setCreatureType(const QString& type);
    QString getCreatureType() const;
    
    // Utility Methods
    bool containsPosition(const Position& pos) const;
    QString getDescription() const;
    Spawn deepCopy() const;
    
    // Operators
    bool operator==(const Spawn& other) const;
    bool operator!=(const Spawn& other) const;

private:
    Position m_center;
    int m_radius = 1;
    int m_intervalSeconds = 600;
    QStringList m_creatureTypes;
    bool m_selected = false;
    bool m_autoCreated = false;
};

} // namespace spawns
} // namespace core
} // namespace RME

#endif // RME_CORE_SPAWNS_SPAWN_H