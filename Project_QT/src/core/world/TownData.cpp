#include "core/world/TownData.h"

namespace RME {
namespace core {
namespace world {

TownData::TownData() :
    id(0),
    name(""),
    templePosition()
{
}

TownData::TownData(uint32_t id_, const QString& name_, const Position& templePos_)
    : id(id_), name(name_), templePosition(templePos_) {
}

bool TownData::operator==(const TownData& other) const {
    return id == other.id &&
           name == other.name &&
           templePosition == other.templePosition;
}

bool TownData::operator!=(const TownData& other) const {
    return !(*this == other);
}

} // namespace world
} // namespace core
} // namespace RME
