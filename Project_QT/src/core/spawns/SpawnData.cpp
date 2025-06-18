#include "core/spawns/SpawnData.h" // Adjust path if core/ is not directly in include path for spawns/ subdir

namespace RME {
namespace core {
namespace spawns {

SpawnData::SpawnData() :
    m_radius(0),
    m_intervalSeconds(60),
    m_isAutoCreated(false),
    m_selected(false) {
    // m_center will be default constructed by Position's default constructor
}

SpawnData::SpawnData(const RME::Position& center, int radius, int intervalSeconds, const QStringList& creatureTypes) :
    m_center(center),
    m_radius(radius),
    m_intervalSeconds(intervalSeconds),
    m_creatureTypes(creatureTypes),
    m_isAutoCreated(false),
    m_selected(false) {
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

// Deep copy method (from original wxWidgets)
SpawnData SpawnData::deepCopy() const {
    SpawnData copy = *this; // Copy constructor handles all members
    return copy;
}

// Utility methods
bool SpawnData::containsPosition(const RME::Position& pos) const {
    if (!m_center.isValid() || m_radius <= 0) {
        return false;
    }
    
    // Check if position is within spawn radius (using Manhattan distance for simplicity)
    int dx = abs(pos.x() - m_center.x());
    int dy = abs(pos.y() - m_center.y());
    int dz = abs(pos.z() - m_center.z());
    
    // Position must be on same floor and within radius
    return (dz == 0) && (dx <= m_radius) && (dy <= m_radius);
}

QString SpawnData::getDescription() const {
    QString desc = QString("Spawn at %1 (Radius: %2, Interval: %3s)")
                       .arg(m_center.toString())
                       .arg(m_radius)
                       .arg(m_intervalSeconds);
    
    if (!m_creatureTypes.isEmpty()) {
        desc += QString(" - Creatures: %1").arg(m_creatureTypes.join(", "));
    }
    
    return desc;
}

} // namespace spawns
} // namespace core
} // namespace RME
