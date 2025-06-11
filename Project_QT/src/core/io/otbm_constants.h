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

// OTBM General Node Types (V1/V2 distinction might be by attributes or specific node type values)
// These are conceptual types; the actual byte value might differ or be combined.
// The OTBM loader usually reads a ULEB128 or similar for node type, which can be > 255.
// For simplicity, if using single byte types as in current NodeFileReadHandle:
constexpr uint8_t OTBM_NODE_ROOT            = 0x00; // Often implied, or a specific type like 0x01 for root_v1
constexpr uint8_t OTBM_NODE_MAP_DATA        = 0x01; // Or 0x02 for MAP_DATA_V2 etc.
constexpr uint8_t OTBM_NODE_TILE_AREA       = 0x02; // OTBM_TILE_AREA_V2
constexpr uint8_t OTBM_NODE_TILE            = 0x03; // OTBM_TILE_V2
constexpr uint8_t OTBM_NODE_ITEM            = 0x04; // OTBM_ITEM_V2
constexpr uint8_t OTBM_NODE_HOUSETILE       = 0x05; // OTBM_HOUSETILE_V2 (if distinct from TILE)
constexpr uint8_t OTBM_NODE_WAYPOINTS       = 0x06; // List of waypoints
constexpr uint8_t OTBM_NODE_WAYPOINT        = 0x07;
constexpr uint8_t OTBM_NODE_TOWNS           = 0x08; // List of towns
constexpr uint8_t OTBM_NODE_TOWN            = 0x09;
constexpr uint8_t OTBM_NODE_CREATURE        = 0x0A; // In a tile
constexpr uint8_t OTBM_NODE_SPAWN           = 0x0B; // In a tile area, or map-level for older spawns

// OTBM Node Flags (can be combined)
constexpr uint8_t OTBM_FLAG_NONE            = 0x00;
constexpr uint8_t OTBM_FLAG_COMPRESSION     = 0x08; // Indicates properties are ZLIB compressed
// Other flags might exist for extended features, etc.


// OTBM Map Attributes (for OTBM_NODE_MAP_DATA)
constexpr uint8_t OTBM_ATTR_DESCRIPTION         = 0x01; // String
constexpr uint8_t OTBM_ATTR_EXT_HOUSE_FILE    = 0x02; // String
constexpr uint8_t OTBM_ATTR_EXT_SPAWN_FILE    = 0x03; // String
constexpr uint8_t OTBM_ATTR_MAP_WIDTH           = 0x04; // U16 (Often implicit or header, but can be attribute)
constexpr uint8_t OTBM_ATTR_MAP_HEIGHT          = 0x05; // U16 (Often implicit or header, but can be attribute)
// OTBM V2+ adds these to MAP_DATA node:
constexpr uint8_t OTBM_ATTR_VERSION_MAJOR       = 0x06; // U32
constexpr uint8_t OTBM_ATTR_VERSION_MINOR       = 0x07; // U32
constexpr uint8_t OTBM_ATTR_VERSION_BUILD       = 0x08; // U32


// OTBM Tile Attributes (for OTBM_NODE_TILE, OTBM_NODE_HOUSETILE)
constexpr uint8_t OTBM_ATTR_TILE_FLAGS          = 0x10; // U32, TileState flags (protection zone, no pvp, etc.)
constexpr uint8_t OTBM_ATTR_HOUSETILE_HOUSEID   = 0x11; // U32, House ID
// OTBM_ATTR_ITEM (0x02 or similar) was listed before, but items are child nodes, not attributes of a tile.

// OTBM Item Attributes (for OTBM_NODE_ITEM)
constexpr uint8_t OTBM_ATTR_COUNT               = 0x20; // U8 (for stackables) or U16 (charges, fluid type etc.)
constexpr uint8_t OTBM_ATTR_ACTION_ID           = 0x21; // U16
constexpr uint8_t OTBM_ATTR_UNIQUE_ID           = 0x22; // U16
constexpr uint8_t OTBM_ATTR_TEXT                = 0x23; // String
constexpr uint8_t OTBM_ATTR_WRITTENBY           = 0x24; // String
constexpr uint8_t OTBM_ATTR_WRITTENDATE         = 0x25; // U32 (timestamp)
constexpr uint8_t OTBM_ATTR_DEPOT_ID            = 0x26; // U16 (Depot chest ID)
constexpr uint8_t OTBM_ATTR_CHARGES             = OTBM_ATTR_COUNT; // Often same ID, context implies meaning

// HACK attributes for NodeData simulation (remove when NodeFileWriteHandle supports raw node data)
constexpr uint8_t OTBM_ATTR_AREA_BASE_X_HACK    = 0xF0; // Used in save path for now
constexpr uint8_t OTBM_ATTR_AREA_BASE_Y_HACK    = 0xF1; // Used in save path for now
constexpr uint8_t OTBM_ATTR_AREA_BASE_Z_HACK    = 0xF2; // Used in save path for now
constexpr uint8_t OTBM_ATTR_TILE_REL_X_HACK     = 0xF3; // Used in save path for now
constexpr uint8_t OTBM_ATTR_TILE_REL_Y_HACK     = 0xF4; // Used in save path for now
constexpr uint8_t OTBM_ATTR_ITEM_ID_HACK        = 0xF5; // Used in save path for now

// OTBM Waypoint Attributes (for OTBM_NODE_WAYPOINT)
constexpr uint8_t OTBM_ATTR_WAYPOINT_NAME           = 0x30; // String (Waypoint name itself is often node data or a primary fixed attribute)
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_X     = 0x31; // U16 (Position X)
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_Y     = 0x32; // U16 (Position Y)
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_Z     = 0x33; // U8  (Position Z)
constexpr uint8_t OTBM_ATTR_WAYPOINT_CONNECTION_TO  = 0x34; // String (Name of connected waypoint, can appear multiple times)


// OTBM Tile Flags (bitfield for OTBM_ATTR_TILE_FLAGS)
constexpr uint32_t OTBM_TILEFLAG_PROTECTIONZONE  = 0x0001; // TILESTATE_PROTECTIONZONE
constexpr uint32_t OTBM_TILEFLAG_NOPVPZONE       = 0x0002; // TILESTATE_NOPVPZONE (deprecated by some)
constexpr uint32_t OTBM_TILEFLAG_NOLOGOUT        = 0x0004; // TILESTATE_NOLOGOUT
constexpr uint32_t OTBM_TILEFLAG_PVPZONE         = 0x0008; // TILESTATE_PVPZONE
// Add more flags as needed based on OTBM version (REFRESH, etc.)


// RME specific error codes for OTBM I/O (used in m_error)
constexpr int RME_OTBM_IO_NO_ERROR = 0;
constexpr int RME_OTBM_IO_ERROR_UNEXPECTED_EOF = 1;
constexpr int RME_OTBM_IO_ERROR_SYNTAX = 2; // Bad node sequence, etc.
constexpr int RME_OTBM_IO_ERROR_FILE_OPEN = 3;
constexpr int RME_OTBM_IO_ERROR_FILE_OPEN_WRITE = 4;
constexpr int RME_OTBM_IO_ERROR_FILE_NOT_OPEN = 5;
constexpr int RME_OTBM_IO_ERROR_WRITE_FAILED = 6;
constexpr int RME_OTBM_IO_ERROR_READ_FAILED = 7;
constexpr int RME_OTBM_IO_ERROR_DECOMPRESSION = 8;
constexpr int RME_OTBM_IO_ERROR_INVALID_NODE_PROP_SIZE = 9;


} // namespace io
} // namespace core
} // namespace RME

#endif // RME_OTBM_CONSTANTS_H
