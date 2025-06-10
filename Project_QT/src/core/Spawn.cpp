#include "Spawn.h"

namespace RME {

Spawn::Spawn(uint16_t spawnRadius) : radius(spawnRadius) {
    // Initialization for a spawn area
}

std::unique_ptr<Spawn> Spawn::deepCopy() const {
    auto newSpawn = std::make_unique<Spawn>(this->radius);
    newSpawn->creatureTypes = this->creatureTypes; // QList deep copies by default
    // Copy other members if any
    return newSpawn;
}

uint16_t Spawn::getRadius() const {
    return radius;
}

void Spawn::setRadius(uint16_t newRadius) {
    radius = newRadius;
}

void Spawn::addCreatureType(const QString& creatureName) {
    creatureTypes.append({creatureName});
}

QList<SpawnCreatureInfo> Spawn::getCreatureTypes() const {
    return creatureTypes;
}

} // namespace RME
