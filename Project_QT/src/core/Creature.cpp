#include "Creature.h"

namespace RME {

Creature::Creature(const QString& creatureName) : name(creatureName) {
    // Initialization for a creature
}

std::unique_ptr<Creature> Creature::deepCopy() const {
    auto newCreature = std::make_unique<Creature>(this->name);
    // Copy other members if any are added to the stub
    return newCreature;
}

QString Creature::getName() const {
    return name;
}

void Creature::setName(const QString& newName) {
    name = newName;
}

} // namespace RME
