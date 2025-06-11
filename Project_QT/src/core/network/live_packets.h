//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_LIVE_PACKETS_H
#define RME_LIVE_PACKETS_H

#include <cstdint> // For uint8_t
#include "core/map_constants.h" // For NetworkColor, MapVersion (needs RME::core::MapVersion)
#include "core/Position.h"      // For RME::core::Position
#include <QString>
#include <QList>
#include <QByteArray>           // For TileChangeData raw tile blob

namespace RME {
namespace core {
// Forward declare MapVersion if its full definition isn't needed here directly
// Or ensure map_constants.h provides it.
// Assuming MapVersion is defined in map_constants.h like:
// namespace RME { namespace core { struct MapVersion { ... }; }}
// For NetworkColor, it's an enum class, so it should be fine.
} // namespace core
} // namespace RME


namespace RME {
namespace core {
namespace network {

/**
 * @brief Defines the types of packets used in the live collaboration protocol.
 */
enum class LivePacketType : uint8_t {
    // Client to Server
    PACKET_HELLO_FROM_CLIENT = 0x10, ///< Client initiates connection with version, name, password.
    PACKET_READY_CLIENT = 0x11,      ///< Client signals readiness after receiving server hello.
    PACKET_NODE_REQUEST = 0x20,      ///< Client requests map data for a specific node/area.
    PACKET_RECEIVE_CHANGES = 0x21,   ///< Client sends a list of map changes (actions).
    // PACKET_ADD_HOUSE = 0x23,      // Potentially part of a generic change system
    // PACKET_EDIT_HOUSE = 0x24,
    // PACKET_REMOVE_HOUSE = 0x25,
    PACKET_CHAT_MESSAGE_FROM_CLIENT = 0x30, ///< Client sends a chat message.
    PACKET_CURSOR_UPDATE_FROM_CLIENT = 0x31,///< Client sends its cursor position and state.
    PACKET_CLIENT_COLOR_UPDATE = 0x32,///< Client requests a color change or informs of its color.

    // Server to Client
    PACKET_HELLO_FROM_SERVER = 0x80, ///< Server responds to client hello with server info & map details.
    PACKET_KICK_FROM_SERVER = 0x81,  ///< Server kicks client (e.g. wrong password, version mismatch).
    PACKET_YOUR_ID_COLOR = 0x82,     ///< Server assigns peer ID and color to client.
    // PACKET_CHANGE_CLIENT_VERSION = 0x83, // Server requests client to change version (rare)
    PACKET_CHAT_MESSAGE_TO_CLIENTS = 0x84, ///< Server broadcasts a chat message from a peer.
    PACKET_CLIENT_COLOR_TO_CLIENTS = 0x85, ///< Server broadcasts a peer's color change.
    PACKET_PEER_JOINED = 0x86,       ///< Server informs clients a new peer has joined.
    PACKET_PEER_LEFT = 0x87,         ///< Server informs clients a peer has left.
    PACKET_PEER_LIST = 0x88,         ///< Server sends list of currently connected peers.

    PACKET_NODE_DATA = 0x90,         ///< Server sends requested map node data.
    PACKET_MAP_CHANGES_TO_CLIENTS = 0x91, ///< Server broadcasts map changes made by a peer.
    PACKET_CURSOR_UPDATE_TO_CLIENTS = 0x92, ///< Server broadcasts a peer's cursor update.
    // PACKET_START_OPERATION = 0x92, // These might be too specific if changes are generic
    // PACKET_UPDATE_OPERATION = 0x93,
    PACKET_FULL_MAP_DATA = 0x95,     ///< Server sends the entire map (e.g., on initial join for small maps).
    PACKET_UNDO_STACK_RESET = 0x96   ///< Server informs client that its local undo stack is now invalid.
};


// --- Data Structures for Packet Payloads ---

/** @brief Data sent by client in PACKET_HELLO_FROM_CLIENT. */
struct ClientHelloClientData {
    RME::core::MapVersion clientMapVersion; ///< Client's understanding of the map version.
    QString clientName;                     ///< Desired name/alias of the client.
    QString passwordAttempt;                ///< Password attempt from the client.
};

/** @brief Data sent by server in PACKET_HELLO_FROM_SERVER. */
struct ServerHelloServerData {
    QString serverName;     ///< Name of the live server.
    QString mapName;        ///< Name of the map currently hosted.
    quint16 mapWidth;       ///< Width of the map.
    quint16 mapHeight;      ///< Height of the map.
    quint8 mapFloors;       ///< Number of floors in the map.
    // Consider adding RME::core::MapVersion serverMapVersion;
    // Consider adding server capabilities flags.
};

/** @brief Data sent by server in PACKET_YOUR_ID_COLOR. */
struct YourIdColorData {
    quint32 peerId;         ///< Unique ID assigned to this client by the server.
    NetworkColor color;     ///< Color assigned to this client for map cursors, etc.
};

/** @brief Information about a single peer, used in peer lists. */
struct PeerInfoData {
    quint32 peerId;                 ///< Unique ID of the peer.
    QString name;                   ///< Name of the peer.
    NetworkColor color;             ///< Color of the peer.
    RME::core::Position lastCursorPos; ///< Last known cursor position of the peer (example).
    // Add other relevant state if needed for initial peer list.
};

/** @brief Data sent by server in PACKET_PEER_LIST or PACKET_PEER_JOINED (for single peer). */
struct PeerListServerData {
    QList<PeerInfoData> peers; ///< List of peers. For PACKET_PEER_JOINED, this list contains one entry.
};

/** @brief Data sent by client in PACKET_NODE_REQUEST. */
struct MapNodeRequestClientData {
    RME::core::Position position; ///< Position of the QTreeNode/sector/tile requested.
                                  ///< Interpretation (tile vs sector) depends on server logic.
};
// MapNodeDataServerData (for PACKET_NODE_DATA) is not a struct;
// response uses MapProtocolCodec::serializeMapSector or serializeTileData directly into message body.


/** @brief Represents a single tile change. */
struct TileChange {
    RME::core::Position position;   ///< Position of the tile that changed.
    QByteArray newTileDataOtbm;   ///< Serialized TileData (using MapProtocolCodec::serializeTileData).
                                  ///< If empty, implies tile was cleared/deleted.
                                  ///< For future: could be a more complex diff structure.
};

/** @brief Data sent by client in PACKET_RECEIVE_CHANGES. */
struct MapChangesClientData {
    QList<TileChange> changes;      ///< List of tile changes made by the client.
    // quint32 clientActionId;      // Optional: client's local ID for this action group.
};

/** @brief Data sent by server in PACKET_MAP_CHANGES_TO_CLIENTS. */
struct MapChangesServerData {
    quint32 originatorPeerId;       ///< Peer ID of who initiated this change (0 for server itself).
    QList<TileChange> changes;      ///< List of tile changes to be applied.
    // quint32 serverActionId;      // Optional: server's ID for this action group (for undo linking).
};

/** @brief Data sent by client in PACKET_CHAT_MESSAGE_FROM_CLIENT. */
struct ChatMessageClientData {
    QString message; ///< The chat message text.
    // quint8 channelId; // Optional: if chat channels are supported
};

/** @brief Data sent by server in PACKET_CHAT_MESSAGE_TO_CLIENTS. */
struct ChatMessageServerData {
    quint32 speakerPeerId;  ///< Peer ID of the speaker (0 for server messages).
    QString speakerName;    ///< Name of the speaker.
    QString message;        ///< The chat message text.
    NetworkColor color;     ///< Color associated with the speaker.
    // quint8 channelId;    // Optional
};

// ClientColorUpdateClientData: Payload is just NetworkColor (U8), read directly.
// ClientColorUpdateServerData: Payload is PeerID (U32) + NetworkColor (U8).

/** @brief Data sent by server in PACKET_KICK_FROM_SERVER. */
struct KickClientData {
    QString reason; ///< Reason for being kicked.
};


} // namespace network
    PACKET_ADD_HOUSE = 0x23,
    PACKET_EDIT_HOUSE = 0x24,
    PACKET_REMOVE_HOUSE = 0x25,

    PACKET_CLIENT_TALK = 0x30,
    PACKET_CLIENT_UPDATE_CURSOR = 0x31,
    PACKET_CLIENT_COLOR_UPDATE = 0x32,

    PACKET_HELLO_FROM_SERVER = 0x80,
    PACKET_KICK = 0x81,
    PACKET_ACCEPTED_CLIENT = 0x82,
    PACKET_CHANGE_CLIENT_VERSION = 0x83,
    PACKET_SERVER_TALK = 0x84,
    PACKET_COLOR_UPDATE = 0x85,

    PACKET_NODE = 0x90,
    PACKET_CURSOR_UPDATE = 0x91,
    PACKET_START_OPERATION = 0x92,
    PACKET_UPDATE_OPERATION = 0x93,
    PACKET_CHAT_MESSAGE = 0x94,
    // Consider adding an UNKNOWN or ERROR type if useful
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_LIVE_PACKETS_H
