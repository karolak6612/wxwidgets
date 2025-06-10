#ifndef RME_MAP_PROTOCOL_CODEC_H
#define RME_MAP_PROTOCOL_CODEC_H

#include "core/network/NetworkMessage.h" // For NetworkMessage class
#include "core/Position.h"             // For RME::core::Position
#include <QString>                     // For QString in LiveCursor
#include <memory>                      // For std::unique_ptr

// Forward declarations for mapcore types
namespace RME {
namespace core {
class Tile;
class Item;
class IItemTypeProvider;
class Map; // For context in deserialization, e.g. creating new tiles if needed by node
namespace map { // Assuming QTreeNode and MapVersionInfo might be in map submodule
    class QTreeNode;
    struct MapVersionInfo; // Assuming this is the correct struct name and location
} // namespace map
} // namespace core
} // namespace RME


namespace RME {
namespace core {
namespace network {

// UI-agnostic color structure
struct NetworkColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255; // Default to fully opaque
};

// Live cursor information
struct LiveCursor {
    RME::core::Position position;
    NetworkColor color;
    QString userName; // User associated with the cursor
};

class MapProtocolCodec {
public:
    MapProtocolCodec() = delete; // Static class

    // Tile Serialization / Deserialization
    // Serializes a complete tile (position, items, flags, etc.)
    static bool serializeTileData(const RME::core::Tile* tile, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version);

    // Deserializes content into an existing tile. Returns true on success.
    // The tile is assumed to be at the correct position already.
    static bool deserializeTileContent(RME::core::Tile* tile, NetworkMessage& msg,
                                       const RME::core::map::MapVersionInfo& version,
                                       RME::core::IItemTypeProvider* itemProvider,
                                       RME::core::Map& map); // Map context might be needed for item creation

    // Item Serialization / Deserialization (helper for tile content, or if items sent standalone)
    // Returns nullptr on failure.
    static std::unique_ptr<RME::core::Item> deserializeItem(NetworkMessage& msg,
                                                            const RME::core::map::MapVersionInfo& version,
                                                            RME::core::IItemTypeProvider* itemProvider);
    static bool serializeItem(const RME::core::Item* item, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version);


    // QTreeNode (Map Sector/QuadTree Node) Serialization / Deserialization
    // Serializes a QTreeNode and its descendant tiles/nodes.
    static bool serializeMapSector(const RME::core::map::QTreeNode* node, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version);

    // Deserializes data into an existing QTreeNode structure.
    // Map object is needed to get/create tiles.
    static bool deserializeMapSector(RME::core::map::QTreeNode* node, NetworkMessage& msg,
                                     RME::core::Map& map,
                                     const RME::core::map::MapVersionInfo& version,
                                     RME::core::IItemTypeProvider* itemProvider);

    // Live Cursor Serialization / Deserialization
    static bool serializeCursor(const LiveCursor& cursor, NetworkMessage& msg);
    static bool deserializeCursor(NetworkMessage& msg, LiveCursor& outCursor); // Returns true on success

    // TODO: Add methods for Floor serialization/deserialization if QTreeNodes don't directly contain all tile data.
    // Based on CORE-03, QTreeNode contains Floors, which contain Tiles.
    // So serializeMapSector should handle this hierarchy.

    // TODO: Add methods for other map elements if needed by live protocol,
    // e.g., house data, spawn data, if they are sent granularly.
    // For now, focusing on core map content (tiles, items, nodes) and cursors.
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_MAP_PROTOCOL_CODEC_H
