#ifndef RME_MAP_PROTOCOL_CODEC_H
#define RME_MAP_PROTOCOL_CODEC_H

#include "core/network/NetworkMessage.h"
#include "core/Position.h"
#include "core/network/live_packets.h" // Added for packet data structs
#include <QString>
#include <memory>

// Forward declarations for mapcore types
// Note: MapVersionInfo might need to be a full include if used by value in method signatures.
// For now, assuming RME::core::map::MapVersionInfo is sufficient as forward if only by const&/ptr.
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

    // --- Payload struct (de)serialization methods ---

    // Client -> Server
    static bool serializeData(const ClientHelloClientData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, ClientHelloClientData& outData);

    static bool serializeData(const MapNodeRequestClientData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, MapNodeRequestClientData& outData);

    // MapChangesClientData needs MapVersionInfo for context during item/tile serialization if version-dependent.
    // It also needs itemProvider and mapContext for deserializing tile data string into actual tiles/items.
    static bool serializeData(const MapChangesClientData& data, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version);
    static bool deserializeData(NetworkMessage& msg, MapChangesClientData& outData, const RME::core::map::MapVersionInfo& version, RME::core::IItemTypeProvider* itemProvider, RME::core::Map& mapContext);

    static bool serializeData(const ChatMessageClientData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, ChatMessageClientData& outData);

    // Server -> Client
    static bool serializeData(const ServerHelloServerData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, ServerHelloServerData& outData);

    static bool serializeData(const YourIdColorData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, YourIdColorData& outData);

    static bool serializeData(const PeerListServerData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, PeerListServerData& outData);

    // MapChangesServerData also needs version context and potentially itemProvider/mapContext for similar reasons.
    static bool serializeData(const MapChangesServerData& data, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version);
    static bool deserializeData(NetworkMessage& msg, MapChangesServerData& outData, const RME::core::map::MapVersionInfo& version, RME::core::IItemTypeProvider* itemProvider, RME::core::Map& mapContext);

    static bool serializeData(const ChatMessageServerData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, ChatMessageServerData& outData);

    static bool serializeData(const KickClientData& data, NetworkMessage& msg);
    static bool deserializeData(NetworkMessage& msg, KickClientData& outData);

    // For ClientColorUpdate (client->server: U8 color, server->client: U32 peerId, U8 color)
    // and CursorUpdate, existing serializeCursor/deserializeCursor or direct NetworkMessage add/get
    // methods can be used by QtLivePeer directly if payloads are simple.

    /**
     * @brief Deserializes a full tile from a raw OTBM data blob.
     * This is used by the server to reconstruct a tile state received from a client,
     * or by a client to reconstruct a tile from a map change broadcast.
     * The blob is expected to be a single OTBM_TILE or OTBM_HOUSETILE node structure.
     * @param tileBlob The QByteArray containing the serialized OTBM tile node.
     * @param mapContext Reference to the map, needed for context if items are created
     *                   that might need to interact with map-level systems (though typically
     *                   for pure deserialization, only AssetManager/ItemTypeProvider is essential).
     * @param version The map version information for deserialization context.
     * @param itemProvider Provider for item type information, crucial for creating items.
     * @return A unique_ptr to the deserialized Tile, or nullptr on failure.
     *         The returned Tile will have its content (items, flags) populated but its
     *         position member might be default (0,0,0) as OTBM tile nodes don't store their own position.
     */
    static std::unique_ptr<RME::core::Tile> deserializeTileFromBlob(
        const QByteArray& tileBlob,
        RME::core::Map& mapContext,
        const RME::core::map::MapVersionInfo& version,
        RME::core::IItemTypeProvider* itemProvider);
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_MAP_PROTOCOL_CODEC_H
