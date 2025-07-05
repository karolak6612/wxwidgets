#include "Spawn.h"
#include <QDebug>
#include <QtMath>

namespace RME {
namespace core {
namespace spawns {

Spawn::Spawn(const Position& center, int radius, int intervalSeconds)
    : m_center(center)
    , m_radius(qMax(1, radius))
    , m_intervalSeconds(qMax(1, intervalSeconds))
    , m_selected(false)
    , m_autoCreated(false)
{
}

void Spawn::setCenter(const Position& center)
{
    m_center = center;
}

void Spawn::setRadius(int radius)
{
    m_radius = qMax(1, radius); // Minimum 1 tile
}

void Spawn::setIntervalSeconds(int seconds)
{
    m_intervalSeconds = qMax(1, seconds); // Minimum 1 second
}

void Spawn::setCreatureTypes(const QStringList& types)
{
    m_creatureTypes = types;
}

void Spawn::addCreatureType(const QString& type)
{
    if (!type.isEmpty() && !m_creatureTypes.contains(type)) {
        m_creatureTypes.append(type);
    }
}

bool Spawn::removeCreatureType(const QString& type)
{
    return m_creatureTypes.removeOne(type);
}

void Spawn::setCreatureType(const QString& type)
{
    m_creatureTypes.clear();
    if (!type.isEmpty()) {
        m_creatureTypes.append(type);
    }
}

QString Spawn::getCreatureType() const
{
    return m_creatureTypes.isEmpty() ? QString() : m_creatureTypes.first();
}

bool Spawn::containsPosition(const Position& pos) const
{
    if (!m_center.isValid() || m_radius <= 0) {
        return false;
    }
    
    // Check if position is within spawn radius
    // Position must be on same floor and within radius
    if (pos.z() != m_center.z()) {
        return false;
    }
    
    // Use circular distance check
    int dx = pos.x() - m_center.x();
    int dy = pos.y() - m_center.y();
    int distanceSquared = dx * dx + dy * dy;
    int radiusSquared = m_radius * m_radius;
    
    return distanceSquared <= radiusSquared;
}

QString Spawn::getDescription() const
{
    QString desc = QString("Spawn at %1 (Radius: %2, Interval: %3s)")
                       .arg(m_center.toString())
                       .arg(m_radius)
                       .arg(m_intervalSeconds);
    
    if (!m_creatureTypes.isEmpty()) {
        desc += QString(" - Creatures: %1").arg(m_creatureTypes.join(", "));
    }
    
    if (m_selected) {
        desc += " [SELECTED]";
    }
    
    if (m_autoCreated) {
        desc += " [AUTO]";
    }
    
    return desc;
}

Spawn Spawn::deepCopy() const
{
    return *this; // Copy constructor handles everything
}

bool Spawn::operator==(const Spawn& other) const
{
    return m_center == other.m_center &&
           m_radius == other.m_radius &&
           m_intervalSeconds == other.m_intervalSeconds &&
           m_creatureTypes == other.m_creatureTypes &&
           m_autoCreated == other.m_autoCreated;
    // Note: m_selected is intentionally excluded from equality comparison
    // as selection state is UI-specific and shouldn't affect data equality
}

bool Spawn::operator!=(const Spawn& other) const
{
    return !(*this == other);
}

} // namespace spawns
} // namespace core
} // namespace RME