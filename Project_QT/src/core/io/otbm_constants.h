#ifndef RME_OTBM_CONSTANTS_H
#define RME_OTBM_CONSTANTS_H

#include <cstdint>

namespace RME {
namespace core {
namespace io {

// OTBM Node markers (from wxwidgets/filehandle.h NodeType)
constexpr uint8_t NODE_START = 0xFE;
constexpr uint8_t NODE_END = 0xFF;
constexpr uint8_t ESCAPE_CHAR = 0xFD;

// OTBM General Node Types (Derived from Qt6 original and wxwidgets/iomap_otbm.h OTBM_NodeTypes_t)
// Existing Qt6 values are kept. wxWidgets specific types are noted.
constexpr uint8_t OTBM_NODE_ROOT            = 0x00; // wx: OTBM_ROOTV1 = 1 (value diff)
constexpr uint8_t OTBM_NODE_MAP_DATA        = 0x01; // wx: OTBM_MAP_DATA = 2 (value diff)
constexpr uint8_t OTBM_NODE_TILE_AREA       = 0x02; // wx: OTBM_TILE_AREA = 4 (value diff)
constexpr uint8_t OTBM_NODE_TILE            = 0x03; // wx: OTBM_TILE = 5 (value diff)
constexpr uint8_t OTBM_NODE_ITEM            = 0x04; // wx: OTBM_ITEM = 6 (value diff)
constexpr uint8_t OTBM_NODE_HOUSETILE       = 0x05; // wx: OTBM_HOUSETILE = 14 (value diff)
constexpr uint8_t OTBM_NODE_WAYPOINTS       = 0x06; // wx: OTBM_WAYPOINTS = 15 (value diff)
constexpr uint8_t OTBM_NODE_WAYPOINT        = 0x07; // wx: OTBM_WAYPOINT = 16 (value diff)
constexpr uint8_t OTBM_NODE_TOWNS           = 0x08; // wx: OTBM_TOWNS = 12 (value diff)
constexpr uint8_t OTBM_NODE_TOWN            = 0x09; // wx: OTBM_TOWN = 13 (value diff)
constexpr uint8_t OTBM_NODE_CREATURE        = 0x0A; // Not explicitly in wx enum, but used contextually
constexpr uint8_t OTBM_NODE_SPAWN           = 0x0B; // wx: OTBM_SPAWN_AREA = 10 (related, wx also has OTBM_SPAWNS = 9, OTBM_MONSTER = 11)

// Comments for wxWidgets node types not directly in Qt6 current list:
// constexpr uint8_t OTBM_NODE_ITEM_DEF        = 0xXX; // wx: OTBM_ITEM_DEF = 3
// constexpr uint8_t OTBM_NODE_TILE_SQUARE     = 0xXX; // wx: OTBM_TILE_SQUARE = 7
// constexpr uint8_t OTBM_NODE_TILE_REF        = 0xXX; // wx: OTBM_TILE_REF = 8


// OTBM Node Flags (can be combined)
constexpr uint8_t OTBM_FLAG_NONE            = 0x00;
// In this Qt6 version, using 0x08. Standard OTBM might use 0x01. This is an internal choice for now.
constexpr uint8_t OTBM_FLAG_COMPRESSION     = 0x08; // Indicates properties are ZLIB compressed


// OTBM Attributes (Merged from Qt6 original and wxwidgets/iomap_otbm.h OTBM_ItemAttribute)
// Prefixed like OTBM_ATTR_GROUP_NAME for clarity where applicable.
// Using Qt6 original values where direct counterparts existed. New values are assigned to avoid clashes.

// Map Data Attributes (for OTBM_NODE_MAP_DATA)
constexpr uint8_t OTBM_ATTR_DESCRIPTION      = 0x01; // String (wx: 1)
constexpr uint8_t OTBM_ATTR_EXT_HOUSE_FILE = 0x02; // String (wx: 13)
constexpr uint8_t OTBM_ATTR_EXT_SPAWN_FILE = 0x03; // String (wx: 11)
// constexpr uint8_t OTBM_ATTR_MAP_WIDTH        = 0x04; // U16 (Qt original, often implicit in OTBM)
// constexpr uint8_t OTBM_ATTR_MAP_HEIGHT       = 0x05; // U16 (Qt original, often implicit in OTBM)
constexpr uint8_t OTBM_ATTR_MAP_VERSION_MAJOR  = 0x06; // U32 (Qt original, for client version linked to map features)
constexpr uint8_t OTBM_ATTR_MAP_VERSION_MINOR  = 0x07; // U32 (Qt original)
constexpr uint8_t OTBM_ATTR_MAP_VERSION_BUILD  = 0x08; // U32 (Qt original)
// wx: OTBM_ATTR_EXT_SPAWN_NPC_FILE (23) - Canary RME, skip for now.

// Tile Attributes (for OTBM_NODE_TILE, OTBM_NODE_HOUSETILE)
constexpr uint8_t OTBM_ATTR_TILE_FLAGS          = 0x10; // U32, TileState flags (wx: 3, value conflict, using Qt one)
constexpr uint8_t OTBM_ATTR_HOUSETILE_HOUSEID   = 0x11; // U32, House ID (wx: OTBM_ATTR_HOUSEDOORID = 14, different concept/value)
                                                      // wx OTBM_ATTR_ITEM (9) was for items on tile, handled as child nodes in Qt.

// Item Attributes (for OTBM_NODE_ITEM)
constexpr uint8_t OTBM_ATTR_COUNT               = 0x20; // U8/U16 (wx: 15 for count, 22 for charges)
constexpr uint8_t OTBM_ATTR_ACTION_ID           = 0x21; // U16 (wx: 4)
constexpr uint8_t OTBM_ATTR_UNIQUE_ID           = 0x22; // U16 (wx: 5)
constexpr uint8_t OTBM_ATTR_TEXT                = 0x23; // String (wx: 6)
constexpr uint8_t OTBM_ATTR_WRITTENBY           = 0x24; // String (wx: 19)
constexpr uint8_t OTBM_ATTR_WRITTENDATE         = 0x25; // U32 (wx: 18)
constexpr uint8_t OTBM_ATTR_DEPOT_ID            = 0x26; // U16 (wx: 10)
constexpr uint8_t OTBM_ATTR_CHARGES             = OTBM_ATTR_COUNT; // Alias for now (wx: 22)

// Additional Item Attributes from wxwidgets, assigned new values to avoid conflict
constexpr uint8_t OTBM_ATTR_ITEM_DESC         = 0x27; // String (wx: OTBM_ATTR_DESC = 7)
constexpr uint8_t OTBM_ATTR_TELE_DEST         = 0x28; // Position struct (wx: 8)
constexpr uint8_t OTBM_ATTR_RUNE_CHARGES      = 0x29; // U8/U16 (wx: 12) (Potentially also map to OTBM_ATTR_COUNT/CHARGES)
constexpr uint8_t OTBM_ATTR_HOUSEDOORID_ITEM  = 0x2A; // U8 (wx: OTBM_ATTR_HOUSEDOORID = 14, for items that are doors)
constexpr uint8_t OTBM_ATTR_DURATION          = 0x2B; // (wx: 16)
constexpr uint8_t OTBM_ATTR_DECAYING_STATE    = 0x2C; // (wx: 17)
constexpr uint8_t OTBM_ATTR_SLEEPERGUID       = 0x2D; // (wx: 20)
constexpr uint8_t OTBM_ATTR_SLEEPSTART        = 0x2E; // (wx: 21)
constexpr uint8_t OTBM_ATTR_PODIUMOUTFIT      = 0x2F; // (wx: 40)
constexpr uint8_t OTBM_ATTR_TIER              = 0x30; // (wx: 41)
// General Item Attributes (for OTBM_NODE_ITEM)
constexpr uint8_t OTBM_ATTR_ATTRIBUTE_MAP     = 0x80; // (wx: 128)


// Waypoint Attributes (for OTBM_NODE_WAYPOINT)
// Values from Qt original, wx had no specific values for these in OTBM_ItemAttribute enum, handled by string matching
constexpr uint8_t OTBM_ATTR_WAYPOINT_NAME           = 0x31; // String
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_X     = 0x32; // U16
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_Y     = 0x33; // U16
constexpr uint8_t OTBM_ATTR_WAYPOINT_POSITION_Z     = 0x34; // U8
constexpr uint8_t OTBM_ATTR_WAYPOINT_CONNECTION_TO  = 0x35; // String

// Misc Attributes from wxwidgets
constexpr uint8_t OTBM_ATTR_EXT_FILE          = 0x0D; // (wx: 2) Generic external file link.


// OTBM Tile Flags (bitfield for OTBM_ATTR_TILE_FLAGS) (from wxwidgets/tile.h TILESTATE_*)
// Using Qt6 original values and adding missing ones.
constexpr uint32_t OTBM_TILEFLAG_PROTECTIONZONE  = 0x0001; // wx: TILESTATE_PROTECTIONZONE = 0x0001
constexpr uint32_t OTBM_TILEFLAG_NOPVPZONE       = 0x0002; // wx: TILESTATE_NOPVP = 0x0004 (value diff)
constexpr uint32_t OTBM_TILEFLAG_NOLOGOUT        = 0x0004; // wx: TILESTATE_NOLOGOUT = 0x0008 (value diff)
constexpr uint32_t OTBM_TILEFLAG_PVPZONE         = 0x0008; // wx: TILESTATE_PVPZONE = 0x0010 (value diff)
constexpr uint32_t OTBM_TILEFLAG_REFRESH         = 0x0010; // wx: TILESTATE_REFRESH = 0x0020 (value diff, new to Qt)
constexpr uint32_t OTBM_TILEFLAG_ZONE_BRUSH      = 0x0020; // wx: TILESTATE_ZONE_BRUSH = 0x0040 (value diff, new to Qt, for zone tool)
// Note: wx TILESTATE_DEPRECATED = 0x0002 is skipped.

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
// Add more specific errors if needed
constexpr int RME_OTBM_IO_ERROR_VERSION_MISMATCH = 10;
constexpr int RME_OTBM_IO_ERROR_DATA_CORRUPTED = 11;


} // namespace io
} // namespace core
} // namespace RME

#endif // RME_OTBM_CONSTANTS_H
