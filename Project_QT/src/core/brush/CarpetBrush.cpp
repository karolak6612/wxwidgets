#include "core/brush/CarpetBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/AppSettings.h" // For layering settings
#include "core/assets/AssetManager.h"
// Assuming these exist for controller - commented out as they are not used yet
// #include "core/actions/AddItemCommand.h"
// #include "core/actions/RemoveItemCommand.h"
// #include "core/actions/ReplaceItemCommand.h"

#include <QRandomGenerator>
#include <QDebug>
#include <array> // For std::array
#include <algorithm> // For std::find (needed for QList check)

// Define static members (as before)
uint32_t RME::core::CarpetBrush::s_carpet_types[256];
bool RME::core::CarpetBrush::s_staticDataInitialized = false;

namespace RME {
namespace core {

void CarpetBrush::initializeStaticData() {
    if (s_staticDataInitialized) {
        return;
    }

    // TILE_... constants are from BrushEnums.h (e.g., RME::TILE_NW)
    // Using namespace for brevity within this function
    using namespace RME;
    using BT = RME::BorderType; // Alias for brevity

    // Initialize all to CARPET_CENTER as a base default.
    for (int i = 0; i < 256; ++i) {
        s_carpet_types[i] = static_cast<uint32_t>(BT::CARPET_CENTER);
    }

    // --- Ported data from wxwidgets/brush_tables.cpp CarpetBrush::init() ---
    // This is a direct translation of the assignments.
    // The indices (e.g., 0, TILE_NW, TILE_N | TILE_NW) are the `tiledata` bitmasks.
    // Note: CarpetBrush::carpet_types stores a single BorderType enum per entry, not packed.

    s_carpet_types[0] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_NE] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_W] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_W | TILE_NE] = static_cast<uint32_t>(BT::CARPET_CENTER); // Note: wx original was CARPET_CENTER here
    s_carpet_types[TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // Note: wx original was CARPET_CENTER here
    s_carpet_types[TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER); // Note: wx original was NORTHWEST_CORNER
    s_carpet_types[TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER); // Note: wx original was NORTHWEST_CORNER
    s_carpet_types[TILE_E] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Note: wx original was NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Note: wx original was NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Note: wx original was NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Porting strictly
    s_carpet_types[TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Porting strictly from wx: NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Porting strictly from wx: NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // Porting strictly from wx: NORTHEAST_CORNER
    s_carpet_types[TILE_E | TILE_W] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_SW | TILE_N] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // wx: NORTHEAST_CORNER.
    s_carpet_types[TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // wx: NORTHEAST_CORNER
    s_carpet_types[TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER); // wx: NORTHWEST_CORNER
    s_carpet_types[TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // wx: NORTHEAST_CORNER
    s_carpet_types[TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SW | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SW | TILE_E] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // wx: NORTHEAST_CORNER
    s_carpet_types[TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER); // wx: NORTHEAST_CORNER
    s_carpet_types[TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_NE] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL); // wx: WEST_HORIZONTAL
    s_carpet_types[TILE_S | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER); // wx: NORTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER); // wx: SOUTHEAST_CORNER
    s_carpet_types[TILE_S | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_S | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_SW | TILE_N] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER); // wx: SOUTHWEST_CORNER
    s_carpet_types[TILE_S | TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_DIAGONAL); // wx: NORTHWEST_DIAGONAL
    s_carpet_types[TILE_SE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_N] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER); // wx: SOUTHEAST_CORNER
    s_carpet_types[TILE_SE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_NE] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL); // wx: SOUTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL); // wx: SOUTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER); // wx: SOUTHEAST_CORNER
    s_carpet_types[TILE_SE | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL); // wx: NORTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SE | TILE_SW | TILE_N] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL); // wx: SOUTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SE | TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SE | TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL); // wx: WEST_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHWEST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_CORNER);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL); // wx: NORTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL); // wx: SOUTH_HORIZONTAL
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER); // wx: SOUTHEAST_CORNER
    s_carpet_types[TILE_S | TILE_SE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_S | TILE_SE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SE | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SE | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_NORTHEAST_DIAGONAL); // wx: NORTHEAST_DIAGONAL
    s_carpet_types[TILE_S | TILE_SE | TILE_SW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL); // wx: SOUTH_HORIZONTAL
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL); // wx: EAST_HORIZONTAL
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_WEST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHEAST_CORNER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_EAST_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N] = static_cast<uint32_t>(BT::CARPET_CENTER);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTHWEST_DIAGONAL); // wx: SOUTHWEST_DIAGONAL
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_NW] = static_cast<uint32_t>(BT::WX_SOUTH_HORIZONTAL);
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N] = static_cast<uint32_t>(BT::WX_SOUTHEAST_DIAGONAL); // wx: SOUTHEAST_DIAGONAL
    s_carpet_types[TILE_S | TILE_SE | TILE_SW | TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW] = static_cast<uint32_t>(BT::CARPET_CENTER); // wx: CARPET_CENTER

    qInfo("CarpetBrush::s_carpet_types table has been fully initialized by porting all 256 static assignments from wxwidgets/brush_tables.cpp.");
    s_staticDataInitialized = true;
}

// Constructor, setMaterial, getMaterial, getName, getLookID, canApply (as before)
CarpetBrush::CarpetBrush() : m_materialData(nullptr) {
    initializeStaticData();
}

void CarpetBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isCarpet()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "CarpetBrush::setMaterial: Material is null or not a carpet type.";
    }
}

const RME::core::assets::MaterialData* CarpetBrush::getMaterial() const {
    return m_materialData;
}

QString CarpetBrush::getName() const override {
    if (m_materialData) return m_materialData->id;
    return "Carpet Brush";
}

int CarpetBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) {
        return 0;
    }

    // 1. Prioritize pre-set lookId (assumed to be client ID)
    if (m_materialData->lookId != 0) {
        return m_materialData->lookId;
    }

    // 2. If lookId is not set, check serverLookId
    if (m_materialData->serverLookId != 0) {
        // This serverLookId needs to be converted to a client look ID.
        // This conversion should ideally happen in MaterialManager when materials are loaded.
        // Since we cannot do it here without AssetManager access, log and return 0.
        qWarning() << "CarpetBrush 'getLookID': Material" << m_materialData->id << "has serverLookId"
                   << m_materialData->serverLookId << "but no client lookId. Please ensure MaterialManager converts this.";
        return 0; // Cannot return serverLookId as client lookId
    }

    // 3. If both lookId and serverLookId are zero, try to get a server ID from a default item
    const auto* specifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
    if (specifics) {
        uint16_t fallbackServerItemId = 0;
        QString fallbackType = "";

        // Try "center" item first
        fallbackServerItemId = getRandomItemIdForAlignment("center", specifics);
        if (fallbackServerItemId != 0) {
            fallbackType = "center";
        } else {
            // If no center, try any item from the first part (if any parts/items exist)
            if (!specifics->parts.empty() && !specifics->parts.first().items.empty()) {
                fallbackServerItemId = specifics->parts.first().items.first().itemId;
                if (fallbackServerItemId != 0) {
                    fallbackType = "first available";
                }
            }
        }

        if (fallbackServerItemId != 0) {
            qWarning() << "CarpetBrush 'getLookID': Material" << m_materialData->id
                       << "has no client lookId or serverLookId. Using server ID" << fallbackServerItemId
                       << "from" << fallbackType << "item as a base for what *could* be its look."
                       << "This ID needs conversion to a client look ID by MaterialManager.";
            return 0; // Cannot return serverItemId as client lookId
        }
    }

    qWarning() << "CarpetBrush 'getLookID': Material" << m_materialData->id
               << "has no lookId, serverLookId, or any default items to derive a look from. Returning 0.";
    return 0; // Final fallback
}

bool CarpetBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const override {
    if (!m_materialData) return false;
    const auto* specifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
    if (!specifics || specifics->parts.empty()) return false; // No carpet parts defined
    if (!map || !map->isPositionValid(pos)) return false;
    return true;
}


// --- Main Apply Logic ---
void CarpetBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) override {
    // Enhanced input validation with better error messages
    if (!controller) { 
        qWarning("CarpetBrush::apply: Null controller provided"); 
        return; 
    }
    if (!m_materialData) { 
        qWarning("CarpetBrush::apply: No material set for carpet brush"); 
        return; 
    }

    // Get carpet specifics with better error handling
    const auto* carpetSpecifics = std::get_if<assets::MaterialCarpetSpecifics>(&m_materialData->specificData);
    if (!carpetSpecifics) { 
        qWarning("CarpetBrush::apply: Material '%s' is not a carpet or has invalid specifics", 
                qUtf8Printable(m_materialData->id)); 
        return; 
    }
    
    // EDGE CASE: Check if parts are empty
    if (carpetSpecifics->parts.isEmpty()) {
        qWarning("CarpetBrush::apply: Material '%s' has no carpet parts defined", 
                qUtf8Printable(m_materialData->id));
        return;
    }

    // Get required dependencies
    RME::core::map::Map* map = controller->getMap();
    RME::core::AppSettings* appSettings = controller->getAppSettings(); // Used for layering
    if (!map) { 
        qWarning("CarpetBrush::apply: Null map from controller"); 
        return; 
    }
    if (!appSettings) { 
        qWarning("CarpetBrush::apply: Null appSettings from controller"); 
        return; 
    }

    // Validate position
    if (!map->isPositionValid(pos)) {
        qWarning("CarpetBrush::apply: Invalid position %s", qUtf8Printable(pos.toString()));
        return;
    }

    // Get tile for editing
    Tile* tile = controller->getTileForEditing(pos);
    if (!tile) { 
        qWarning("CarpetBrush::apply: Failed to get tile at %s", qUtf8Printable(pos.toString())); 
        return; 
    }

    // OPTIMIZATION: Create a lookup set of all item IDs in this carpet material for faster checking
    QSet<uint16_t> materialItemIds;
    for (const auto& part : carpetSpecifics->parts) {
        for (const auto& entry : part.items) {
            materialItemIds.insert(entry.itemId);
        }
    }

    // Handle erase mode
    if (settings.isEraseMode) {
        // OPTIMIZATION: Use the lookup set for faster checking
        QList<Item*> itemsOnTileCopy = tile->getAllItems(); // Get a copy to iterate safely
        int removedCount = 0;
        
        for (Item* itemPtr : itemsOnTileCopy) {
            if (itemPtr && materialItemIds.contains(itemPtr->getID())) {
                controller->recordRemoveItem(pos, itemPtr->getID());
                qDebug() << "CarpetBrush: Erased carpet item" << itemPtr->getID() << "at" << pos.toString();
                removedCount++;
            }
        }
        
        if (removedCount == 0) {
            qDebug() << "CarpetBrush: No matching carpet items to erase at" << pos.toString();
        }
        
        // No need to update neighbors when erasing - we're done
        return;
    } 
    
    // Drawing mode
    bool layerCarpets = appSettings->isLayerCarpetsEnabled();

    // Get center item with better error handling
    uint16_t centerItemId = getRandomItemIdForAlignment("center", carpetSpecifics);
    if (centerItemId == 0) {
        // EDGE CASE: Try to find any valid item if center is not available
        for (const auto& part : carpetSpecifics->parts) {
            if (!part.items.isEmpty()) {
                centerItemId = part.items.first().itemId;
                qWarning("CarpetBrush::apply: No 'center' item defined for carpet material %s. Using alternative item %u.", 
                        qUtf8Printable(m_materialData->id), centerItemId);
                break;
            }
        }
        
        if (centerItemId == 0) {
            qWarning("CarpetBrush::apply: No valid items defined for carpet material %s. Cannot draw.", 
                    qUtf8Printable(m_materialData->id));
            return;
        }
    }

    if (layerCarpets) {
        // Simply add the new carpet item on top
        controller->recordAddItem(pos, centerItemId);
        qDebug() << "CarpetBrush: Layering enabled, adding new carpet item" << centerItemId << "at" << pos.toString();
    } else {
        // OPTIMIZATION: Use the lookup set for faster checking of existing items
        QList<uint16_t> idsToRemove;
        const QList<std::unique_ptr<Item>>& itemsOnTile = tile->getItems();
        
        for (const auto& itemPtr : itemsOnTile) {
            if (itemPtr && materialItemIds.contains(itemPtr->getID())) {
                idsToRemove.append(itemPtr->getID());
            }
        }
        
        // Remove existing carpet items of this material
        for (uint16_t id_to_remove : idsToRemove) {
            controller->recordRemoveItem(pos, id_to_remove);
            qDebug() << "CarpetBrush: Removing existing carpet item" << id_to_remove;
        }

        // Add the new center carpet item
        controller->recordAddItem(pos, centerItemId);
        qDebug() << "CarpetBrush: Added new carpet item" << centerItemId << "at" << pos.toString();
    }

    // Update the appearance of this tile and its neighbors
    updateCarpetAppearance(controller, pos, map, m_materialData);
    
    // OPTIMIZATION: Use a static array for neighbor offsets and boundary checking
    static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
        {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
    }};
    
    // Update neighbors
    for (const auto& offset : neighborOffsets) {
        const int nx = pos.x + offset.first;
        const int ny = pos.y + offset.second;
        
        // Skip invalid positions (boundary check optimization)
        if (nx < 0 || ny < 0 || nx >= map->getWidth() || ny >= map->getHeight()) {
            continue;
        }
        
        RMEPosition neighborPos(nx, ny, pos.z);
        updateCarpetAppearance(controller, neighborPos, map, m_materialData);
    }
}

// --- Helper Implementations ---
void CarpetBrush::updateCarpetAppearance(RME::core::editor::EditorControllerInterface* controller,
                                         const RME::core::Position& pos,
                                         const RME::core::map::Map* map,
                                         const RME::core::assets::MaterialData* currentBrushMaterial) {
    // Input validation with better error messages
    if (!controller) {
        qWarning("CarpetBrush::updateCarpetAppearance: Null controller provided");
        return;
    }
    if (!map) {
        qWarning("CarpetBrush::updateCarpetAppearance: Null map provided");
        return;
    }
    if (!currentBrushMaterial) {
        qWarning("CarpetBrush::updateCarpetAppearance: Null material provided");
        return;
    }
    
    // Get carpet specifics with better error handling
    const auto* carpetSpecifics = std::get_if<assets::MaterialCarpetSpecifics>(&currentBrushMaterial->specificData);
    if (!carpetSpecifics) {
        qWarning("CarpetBrush::updateCarpetAppearance: Material '%s' is not a carpet or has invalid specifics",
                qUtf8Printable(currentBrushMaterial->id));
        return;
    }
    
    // Check if parts are empty - edge case handling
    if (carpetSpecifics->parts.isEmpty()) {
        qWarning("CarpetBrush::updateCarpetAppearance: Material '%s' has no carpet parts defined",
                qUtf8Printable(currentBrushMaterial->id));
        return;
    }

    // Get tile with better error handling
    const Tile* tile = map->getTile(pos);
    if (!tile) {
        qWarning("CarpetBrush::updateCarpetAppearance: No tile at position %s",
                qUtf8Printable(pos.toString()));
        return;
    }

    // OPTIMIZATION 1: Create a lookup set of all item IDs in this carpet material for faster checking
    QSet<uint16_t> materialItemIds;
    for (const auto& part : carpetSpecifics->parts) {
        for (const auto& entry : part.items) {
            materialItemIds.insert(entry.itemId);
        }
    }

    // Find the carpet item on this tile that belongs to our material
    Item* targetCarpetItem = nullptr;
    uint16_t oldItemIdOnTile = 0;

    // OPTIMIZATION 2: Use the lookup set for faster checking
    for (const auto& itemPtr : tile->getItems()) {
        if (itemPtr && materialItemIds.contains(itemPtr->getID())) {
            targetCarpetItem = itemPtr.get();
            oldItemIdOnTile = targetCarpetItem->getID();
            break;
        }
    }

    // If no carpet item of our material was found, nothing to update
    if (!targetCarpetItem) {
        return;
    }

    // OPTIMIZATION 3: Combine neighbor checking and tiledata calculation
    uint8_t tiledata = 0;
    static const std::array<std::pair<int, int>, 8> neighborOffsets = {{
        {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
    }};
    
    // Check all 8 neighbors in one pass
    for (int i = 0; i < 8; ++i) {
        const int nx = pos.x + neighborOffsets[i].first;
        const int ny = pos.y + neighborOffsets[i].second;
        const int nz = pos.z;
        
        // Skip invalid positions (boundary check optimization)
        if (nx < 0 || ny < 0 || nx >= map->getWidth() || ny >= map->getHeight()) {
            continue;
        }
        
        RMEPosition neighborPos(nx, ny, nz);
        const Tile* neighborTile = map->getTile(neighborPos);
        if (!neighborTile) continue;
        
        // Check if neighbor has any item from our material
        bool hasMatchingCarpet = false;
        for (const auto& itemPtr : neighborTile->getItems()) {
            // OPTIMIZATION: Use the lookup set instead of nested loops
            if (itemPtr && materialItemIds.contains(itemPtr->getID())) {
                tiledata |= (1 << i);  // Set the bit for this neighbor
                hasMatchingCarpet = true;
                break;  // No need to check other items on this tile
            }
        }
    }

    // Get the appropriate border type and alignment string
    BorderType borderEnum = static_cast<BorderType>(s_carpet_types[tiledata] & 0xFF);
    QString alignStr = borderTypeToAlignmentString(borderEnum);
    
    // Get the item ID for this alignment with better error handling
    uint16_t newItemId = getRandomItemIdForAlignment(alignStr, carpetSpecifics);
    
    // EDGE CASE: If no item found for this alignment, try center as fallback
    if (newItemId == 0) {
        qWarning("CarpetBrush: No item found for alignment '%s', trying 'center' as fallback",
                qUtf8Printable(alignStr));
        newItemId = getRandomItemIdForAlignment("center", carpetSpecifics);
        
        // If still no item, try the first available part
        if (newItemId == 0 && !carpetSpecifics->parts.isEmpty() && !carpetSpecifics->parts.first().items.isEmpty()) {
            newItemId = carpetSpecifics->parts.first().items.first().itemId;
            qWarning("CarpetBrush: No 'center' item found either, using first available item %u as last resort", newItemId);
        }
    }

    // Update the item if needed
    if (newItemId != 0 && oldItemIdOnTile != newItemId) {
        qDebug() << "CarpetBrush: Updating carpet at" << pos.toString() << "from" << oldItemIdOnTile 
                << "to" << newItemId << "(align: " << alignStr << ", tiledata: " << Qt::bin << tiledata << ")";
        controller->recordRemoveItem(pos, oldItemIdOnTile);
        controller->recordAddItem(pos, newItemId);
    } else if (newItemId == 0) {
        // EDGE CASE: If we still couldn't find a valid item, keep the existing one
        qWarning("CarpetBrush: Could not determine any valid carpet item for material %s. Existing item %u not changed.",
                qUtf8Printable(currentBrushMaterial->id), oldItemIdOnTile);
    } else {
        // Item is already correct
        qDebug() << "CarpetBrush: Carpet at" << pos.toString() << "item" << oldItemIdOnTile 
                << "is already correct for align" << alignStr;
    }
}

uint16_t CarpetBrush::getRandomItemIdForAlignment(const QString& alignStr,
                                                const RME::core::assets::MaterialCarpetSpecifics* carpetSpecifics) const {
    // Input validation with better error handling
    if (!carpetSpecifics) {
        qWarning("CarpetBrush::getRandomItemIdForAlignment: Null carpet specifics provided");
        return 0;
    }
    
    if (alignStr.isEmpty()) {
        qWarning("CarpetBrush::getRandomItemIdForAlignment: Empty alignment string provided");
        // Try center as fallback for empty alignment
        return getRandomItemIdForAlignment("center", carpetSpecifics);
    }
    
    // OPTIMIZATION: Use a case-insensitive comparison with early return for common cases
    QString lowerAlignStr = alignStr.toLower();
    
    // First try exact match with the requested alignment
    for (const auto& part : carpetSpecifics->parts) {
        if (part.align.toLower() == lowerAlignStr) {
            // Found matching part, now handle its items
            
            // EDGE CASE: Empty items list
            if (part.items.isEmpty()) {
                qWarning("CarpetBrush::getRandomItemIdForAlignment: Found alignment '%s' but it has no items defined",
                        qUtf8Printable(alignStr));
                return 0;
            }
            
            // Calculate total chance for weighted random selection
            int totalChance = 0;
            for (const auto& entry : part.items) {
                totalChance += entry.chance;
            }
            
            // EDGE CASE: All items have zero chance
            if (totalChance == 0) {
                qWarning("CarpetBrush::getRandomItemIdForAlignment: All items for alignment '%s' have zero chance, using first item",
                        qUtf8Printable(alignStr));
                return part.items.first().itemId;
            }
            
            // Normal case: Select an item based on weighted chance
            int randomValue = QRandomGenerator::global()->bounded(totalChance);
            int currentChanceSum = 0;
            
            for (const auto& entry : part.items) {
                currentChanceSum += entry.chance;
                if (randomValue < currentChanceSum) {
                    return entry.itemId;
                }
            }
            
            // This should never happen if totalChance > 0, but just in case
            qWarning("CarpetBrush::getRandomItemIdForAlignment: Failed to select random item for alignment '%s', using first item",
                    qUtf8Printable(alignStr));
            return part.items.first().itemId;
        }
    }
    
    // EDGE CASE: Alignment not found, try alternative formats
    // Some materials might use different naming conventions (e.g., "nw" vs "cnw" for corners)
    
    // For corner alignments, try with/without the 'c' prefix
    if (lowerAlignStr.startsWith("c") && lowerAlignStr.length() > 1) {
        // Try without the 'c' prefix (e.g., "cnw" -> "nw")
        QString altAlign = lowerAlignStr.mid(1);
        for (const auto& part : carpetSpecifics->parts) {
            if (part.align.toLower() == altAlign && !part.items.isEmpty()) {
                qDebug("CarpetBrush: Alignment '%s' not found, but found alternative '%s'",
                      qUtf8Printable(alignStr), qUtf8Printable(altAlign));
                return getRandomItemIdForAlignment(altAlign, carpetSpecifics);
            }
        }
    } else if (lowerAlignStr.length() == 2) {
        // Try with 'c' prefix (e.g., "nw" -> "cnw")
        QString altAlign = "c" + lowerAlignStr;
        for (const auto& part : carpetSpecifics->parts) {
            if (part.align.toLower() == altAlign && !part.items.isEmpty()) {
                qDebug("CarpetBrush: Alignment '%s' not found, but found alternative '%s'",
                      qUtf8Printable(alignStr), qUtf8Printable(altAlign));
                return getRandomItemIdForAlignment(altAlign, carpetSpecifics);
            }
        }
    }
    
    // If not center already, try center as fallback
    if (lowerAlignStr != "center") {
        qDebug("CarpetBrush: Alignment '%s' not found, trying 'center' as fallback", qUtf8Printable(alignStr));
        return getRandomItemIdForAlignment("center", carpetSpecifics);
    }
    
    // If we get here, we couldn't find any matching alignment including center
    qWarning("CarpetBrush::getRandomItemIdForAlignment: No matching alignment found for '%s' and no center fallback available",
            qUtf8Printable(alignStr));
    
    // Last resort: return the first item from the first part if available
    if (!carpetSpecifics->parts.isEmpty() && !carpetSpecifics->parts.first().items.isEmpty()) {
        uint16_t firstItemId = carpetSpecifics->parts.first().items.first().itemId;
        qWarning("CarpetBrush::getRandomItemIdForAlignment: Using first available item %u as last resort", firstItemId);
        return firstItemId;
    }
    
    return 0;
}

QString CarpetBrush::borderTypeToAlignmentString(RME::BorderType borderType) const {
    // This mapping needs to be precise based on how XML align attributes
    // were converted to BorderType enums in wxwidgets CarpetBrush::load using
    // AutoBorder::edgeNameToID and the "center" special case.
    
    // OPTIMIZATION: Use a static lookup map for faster and more maintainable mapping
    // This avoids the large switch statement and makes the mapping more explicit
    static const QMap<RME::BorderType, QString> borderTypeToAlignMap = {
        // Special cases
        {BorderType::NONE, "center"},
        {BorderType::CARPET_CENTER, "center"},
        
        // Cardinal edges
        {BorderType::WX_NORTH_HORIZONTAL, "n"},
        {BorderType::WX_EAST_HORIZONTAL, "e"},
        {BorderType::WX_SOUTH_HORIZONTAL, "s"},
        {BorderType::WX_WEST_HORIZONTAL, "w"},
        
        // Corners with 'c' prefix as per original RME
        {BorderType::WX_NORTHWEST_CORNER, "cnw"},
        {BorderType::WX_NORTHEAST_CORNER, "cne"},
        {BorderType::WX_SOUTHWEST_CORNER, "csw"},
        {BorderType::WX_SOUTHEAST_CORNER, "cse"},
        
        // Diagonals - map to center by default as they're rarely used in carpets
        {BorderType::WX_NORTHWEST_DIAGONAL, "center"},
        {BorderType::WX_NORTHEAST_DIAGONAL, "center"},
        {BorderType::WX_SOUTHWEST_DIAGONAL, "center"},
        {BorderType::WX_SOUTHEAST_DIAGONAL, "center"}
    };
    
    // Check if we have a direct mapping
    if (borderTypeToAlignMap.contains(borderType)) {
        return borderTypeToAlignMap[borderType];
    }
    
    // EDGE CASE: Handle raw integer values that might not match the enum exactly
    uint8_t rawValue = static_cast<uint8_t>(borderType);
    
    // Check for common values that might be passed as raw integers
    switch (rawValue) {
        case 0: // NONE
            qDebug("CarpetBrush::borderTypeToAlignmentString: Raw BorderType value 0 (NONE) received, mapping to 'center'");
            return "center";
            
        case 1: // NORTH_HORIZONTAL
            return "n";
            
        case 2: // EAST_HORIZONTAL
            return "e";
            
        case 3: // SOUTH_HORIZONTAL
            return "s";
            
        case 4: // WEST_HORIZONTAL
            return "w";
            
        case 5: // NORTHWEST_CORNER
            return "cnw";
            
        case 6: // NORTHEAST_CORNER
            return "cne";
            
        case 7: // SOUTHWEST_CORNER
            return "csw";
            
        case 8: // SOUTHEAST_CORNER
            return "cse";
            
        case 9: // NORTHWEST_DIAGONAL
        case 10: // NORTHEAST_DIAGONAL
        case 11: // SOUTHWEST_DIAGONAL
        case 12: // SOUTHEAST_DIAGONAL
            qWarning("CarpetBrush::borderTypeToAlignmentString: Raw diagonal BorderType value %d received, mapping to 'center'", rawValue);
            return "center";
            
        case 13: // CARPET_CENTER
            return "center";
            
        default:
            // EDGE CASE: Unknown value, log detailed warning and return center as fallback
            qWarning("CarpetBrush::borderTypeToAlignmentString: Unknown BorderType value %d (enum value %d), "
                    "defaulting to 'center'. This may indicate an issue with s_carpet_types table or enum conversion.",
                    rawValue, static_cast<int>(borderType));
            return "center";
    }
}

} // namespace core
} // namespace RME
