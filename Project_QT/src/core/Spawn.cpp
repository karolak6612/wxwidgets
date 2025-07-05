#include "Spawn.h"

namespace RME {

Spawn::Spawn(uint16_t spawnRadius, int intervalSeconds)
    : radius(spawnRadius), m_intervalSeconds(intervalSeconds) {
    // Initialization for a spawn area
}

std::unique_ptr<Spawn> Spawn::deepCopy() const {
    auto newSpawn = std::make_unique<Spawn>(this->radius, this->m_intervalSeconds);
    newSpawn->creatureTypes = this->creatureTypes; // QList deep copies by default
    return newSpawn;
}

uint16_t Spawn::getRadius() const {
    return radius;
}

void Spawn::setRadius(uint16_t newRadius) {
    radius = newRadius;
}

int Spawn::getIntervalSeconds() const {
    return m_intervalSeconds;
}

void Spawn::setIntervalSeconds(int seconds) {
    m_intervalSeconds = seconds;
}

void Spawn::addCreatureType(const QString& creatureName) {
    creatureTypes.append({creatureName});
}

QList<SpawnCreatureInfo> Spawn::getCreatureTypes() const {
    return creatureTypes;
}

} // namespace RME
