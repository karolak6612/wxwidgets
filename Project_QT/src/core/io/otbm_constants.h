#ifndef RME_OTBM_CONSTANTS_H
#define RME_OTBM_CONSTANTS_H

#include <cstdint>

namespace RME {
namespace core {
namespace io {

// OTBM Node markers
constexpr uint8_t NODE_START = 0xFE;
constexpr uint8_t NODE_END = 0xFF;
constexpr uint8_t ESCAPE_CHAR = 0xFD;

// OTBM node types (attributes for parent node)
constexpr uint8_t OTBM_TILE = 1;
constexpr uint8_t OTBM_HOUSETILE = 2;
constexpr uint8_t OTBM_ITEM = 3;

// OTBM Attributes for Tiles
constexpr uint8_t OTBM_ATTR_TILE_FLAGS = 0x01;
constexpr uint8_t OTBM_ATTR_ITEM = 0x02;
// TILESTATE_ZONE_BRUSH is a flag, not an attribute ID here.
// It would be part of the data for OTBM_ATTR_TILE_FLAGS.

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_OTBM_CONSTANTS_H
