#ifndef RME_MAP_CONSTANTS_H
#define RME_MAP_CONSTANTS_H

namespace RME {

// Map dimensions and limits
// These values are based on typical OpenTibia server configurations.
// The original `MAP_MAX_LAYER` seems to correspond to floors.
// Max X and Y are 256 * 256 = 65536, but practically limited by client/server.
// Using values from typical RME/OT sources.
constexpr int MAP_MAX_WIDTH  = 2048 * 16; // Max map width in tiles (e.g. 2048 screens * 16 tiles/screen width) - adjust if too large
constexpr int MAP_MAX_HEIGHT = 2048 * 16; // Max map height in tiles (e.g. 2048 screens * 16 tiles/screen height) - adjust if too large
constexpr int MAP_MAX_FLOORS = 16;       // Max number of floors (0-15)
constexpr int MAP_MIN_FLOOR  = 0;
constexpr int MAP_MAX_FLOOR  = MAP_MAX_FLOORS - 1; // Usually 15 is the highest accessible floor

// Other constants can be added here as needed.

} // namespace RME

#endif // RME_MAP_CONSTANTS_H
