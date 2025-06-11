#ifndef RME_TOWNDATA_H
#define RME_TOWNDATA_H

#include "core/Position.h" // For RME::Position
#include <QString>
#include <cstdint> // For uint32_t

namespace RME {

class TownData {
public:
    TownData(uint32_t id_ = 0, const QString& name_ = QString(), const Position& templePos_ = Position());

    // Accessors
    uint32_t getId() const { return m_id; }
    const QString& getName() const { return m_name; }
    const Position& getTemplePosition() const { return m_templePosition; }

    // Mutators
    void setName(const QString& name) { m_name = name; }
    void setTemplePosition(const Position& templePos) { m_templePosition = templePos; }
    // ID is set at construction and generally not changed, as it's a map key.

    // Comparison operators
    bool operator==(const TownData& other) const;
    bool operator!=(const TownData& other) const;

private:
    uint32_t m_id;
    QString m_name;
    Position m_templePosition;
};

} // namespace RME

#endif // RME_TOWNDATA_H
