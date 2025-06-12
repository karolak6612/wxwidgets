#include "core/spawns/SpawnData.h" // Adjust path if core/ is not directly in include path for spawns/ subdir

namespace RME {

SpawnData::SpawnData() :
    m_radius(0),
    m_intervalSeconds(60) {
    // m_center will be default constructed by Position's default constructor
}

SpawnData::SpawnData(const RME::Position& center, int radius, int intervalSeconds, const QStringList& creatureTypes) :
    m_center(center),
    m_radius(radius),
    m_intervalSeconds(intervalSeconds),
    m_creatureTypes(creatureTypes) {
}

// Accessors
const RME::Position& SpawnData::getCenter() const {
    return m_center;
}

int SpawnData::getRadius() const {
    return m_radius;
}

int SpawnData::getIntervalSeconds() const {
    return m_intervalSeconds;
}

const QStringList& SpawnData::getCreatureTypes() const {
    return m_creatureTypes;
}

// Mutators
void SpawnData::setCenter(const RME::Position& center) {
    m_center = center;
}

void SpawnData::setRadius(int radius) {
    m_radius = radius;
}

void SpawnData::setIntervalSeconds(int intervalSeconds) {
    m_intervalSeconds = intervalSeconds;
}

void SpawnData::setCreatureTypes(const QStringList& creatureTypes) {
    m_creatureTypes = creatureTypes;
}

// Utility methods
void SpawnData::addCreatureType(const QString& type) {
    if (!m_creatureTypes.contains(type)) {
        m_creatureTypes.append(type);
    }
}

bool SpawnData::removeCreatureType(const QString& type) {
    return m_creatureTypes.removeOne(type);
}

// Equality operator
bool SpawnData::operator==(const SpawnData& other) const {
    return m_center == other.m_center &&
           m_radius == other.m_radius &&
           m_intervalSeconds == other.m_intervalSeconds &&
           m_creatureTypes == other.m_creatureTypes;
}

bool SpawnData::operator!=(const SpawnData& other) const {
    return !(*this == other);
}

} // namespace RME
