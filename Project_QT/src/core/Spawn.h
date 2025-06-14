#ifndef RME_SPAWN_H
#define RME_SPAWN_H

#include <QList> // Or std::vector
#include <QString>
#include <cstdint>
#include <memory> // For std::unique_ptr

namespace RME {

// Forward declare Creature if Spawn references it (e.g. list of creature types)
// class Creature;

struct SpawnCreatureInfo {
    QString name; // Creature type name
    // Potentially other info like count, chance, etc.
};

class Spawn {
public:
    Spawn(uint16_t radius = 1, int intervalSeconds = 60);
    virtual ~Spawn() = default;

    // Virtual deep copy
    virtual std::unique_ptr<Spawn> deepCopy() const;

    uint16_t getRadius() const;
    void setRadius(uint16_t newRadius);

    int getIntervalSeconds() const;
    void setIntervalSeconds(int seconds);

    // Basic management of creatures in spawn (example)
    void addCreatureType(const QString& creatureName);
    QList<SpawnCreatureInfo> getCreatureTypes() const;


private:
    uint16_t radius; // Spawn radius
    QList<SpawnCreatureInfo> creatureTypes; // List of creatures that can spawn here
    int m_intervalSeconds;
};

} // namespace RME

#endif // RME_SPAWN_H
