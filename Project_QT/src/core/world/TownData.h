#ifndef RME_CORE_WORLD_TOWNDATA_H
#define RME_CORE_WORLD_TOWNDATA_H

#include <QString>
#include "core/Position.h" // Path confirmed from previous ls output

namespace RME {
namespace core {
namespace world {

class TownData {
public:
    uint32_t id = 0;
    QString name;
    Position templePosition;

    TownData() = default; // Explicit default constructor
    TownData(uint32_t id_, const QString& name_, const Position& templePos_)
        : id(id_), name(name_), templePosition(templePos_) {}

    // Basic equality operator
    bool operator==(const TownData& other) const {
        return id == other.id &&
               name == other.name &&
               templePosition == other.templePosition;
    }

    bool operator!=(const TownData& other) const {
        return !(*this == other);
    }
};

} // namespace world
} // namespace core
} // namespace RME

#endif // RME_CORE_WORLD_TOWNDATA_H
