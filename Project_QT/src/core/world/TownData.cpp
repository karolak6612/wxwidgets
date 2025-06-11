#include "core/world/TownData.h"

namespace RME {

TownData::TownData(uint32_t id_, const QString& name_, const Position& templePos_)
    : m_id(id_), m_name(name_), m_templePosition(templePos_) {
}

bool TownData::operator==(const TownData& other) const {
    return m_id == other.m_id &&
           m_name == other.m_name &&
           m_templePosition == other.m_templePosition;
}

bool TownData::operator!=(const TownData& other) const {
    return !(*this == other);
}

} // namespace RME
