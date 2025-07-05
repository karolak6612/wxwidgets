#ifndef QTLIVEPEER_H
#define QTLIVEPEER_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <cstdint>

// Core RME library includes
#include "core/network/NetworkMessage.h"    // RME Core library
#include "core/network/MapProtocolCodec.h" // RME Core library
#include "core/network/live_packets.h"     // RME Core library (LivePacketType enum)
#include "core/map_constants.h"            // RME Core library (NetworkColor, MapVersion)
#include "core/Position.h"                 // For LiveCursor if it uses Position

// Forward declarations
class QtLiveServer; // From this project
namespace RME {
namespace core {
    class Map;
    namespace actions { class UndoManager; }
    namespace assets { class AssetManager; }
} // namespace core
} // namespace RME

/**
 * @brief Represents a connected client (peer) in the live server environment.
 *
 * Each QtLivePeer instance manages the TCP socket for a single client,
 * handles incoming data parsing, processes client requests, and sends
 * data back to the client. It interacts with the main QtLiveServer
 * to access shared resources like the map, undo manager, and asset manager.
 */
class QtLivePeer : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs a QtLivePeer.
     * @param server Non-owning pointer to the parent QtLiveServer instance.
     * @param socket The QTcpSocket representing the client connection. Ownership is taken if parent is set.
     * @param mapRef Non-owning pointer to the shared Map instance.
     * @param undoManagerRef Non-owning pointer to the shared UndoManager.
     * @param assetManagerRef Non-owning pointer to the shared AssetManager.
     * @param peerId A unique ID assigned to this peer by the server.
     * @param parent Optional QObject parent.
     */
    explicit QtLivePeer(QtLiveServer* server,
                        QTcpSocket* socket,
                        RME::core::Map* mapRef,
                        RME::core::actions::UndoManager* undoManagerRef,
                        RME::core::assets::AssetManager* assetManagerRef,
                        uint32_t peerId,
                        QObject *parent = nullptr);

    /**
     * @brief Destroys the QtLivePeer.
     * Ensures the socket is properly handled (usually closed by QTcpSocket's parentage).
     */
    ~QtLivePeer() override;

    // Prevent copying
    QtLivePeer(const QtLivePeer&) = delete;
    QtLivePeer& operator=(const QtLivePeer&) = delete;

    /**
     * @brief Sends a pre-formatted NetworkMessage to this peer.
     * The message is prefixed with its size before sending.
     * @param msg The NetworkMessage to send.
     */
    void sendPacket(const RME::core::NetworkMessage& msg);

    /**
     * @brief Alias for sendPacket, often used in server logic.
     * @param msg The NetworkMessage to send.
     */
    void sendNetworkMessage(const RME::core::NetworkMessage& msg) { sendPacket(msg); }

    /** @brief Gets the unique ID of this peer. */
    uint32_t getPeerId() const { return m_peerId; }
    /** @brief Gets the name this client identified with (after successful login). */
    QString getClientName() const { return m_clientName; }
    /** @brief Gets the color assigned to this client for map cursors/identification. */
    RME::core::NetworkColor getClientColor() const { return m_clientColor; }
    /** @brief Checks if this peer has successfully authenticated. */
    bool isAuthenticated() const { return m_isAuthenticated; }

signals:
    /**
     * @brief Emitted when this peer's socket disconnects or a critical error occurs.
     * @param peer Pointer to this QtLivePeer instance.
     */
    void peerDisconnected(QtLivePeer* peer);

    /**
     * @brief Emitted when this peer sends a message that needs to be broadcast to all other peers.
     * The QtLiveServer will typically connect to this signal and call its broadcastMessageToAll method,
     * excluding this peer as the sender.
     * @param msg The NetworkMessage to broadcast.
     */
    void broadcastMessageToOthers(const RME::core::NetworkMessage& msg);

private slots:
    /**
     * @brief Slot connected to the QTcpSocket's readyRead() signal.
     * Reads available data from the socket into a buffer and attempts to process it.
     */
    void onReadyRead();

    /**
     * @brief Slot connected to the QTcpSocket's disconnected() signal.
     * Emits the peerDisconnected() signal.
     */
    void onSocketDisconnected();

    /**
     * @brief Slot connected to the QTcpSocket's errorOccurred() signal.
     * Logs the error and emits the peerDisconnected() signal.
     * @param socketError The type of socket error that occurred.
     */
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    /**
     * @brief Processes the data accumulated in m_receiveBuffer.
     * Attempts to parse complete NetworkMessages (length-prefixed) from the buffer.
     */
    void processReceivedData();

    /**
     * @brief Handles packets received during the login/handshake phase.
     * @param msg The received NetworkMessage (type byte already consumed).
     */
    void handleLoginPacket(RME::core::NetworkMessage& msg); // Type already read by processReceivedData

    /**
     * @brief Handles packets received after successful authentication (editor commands).
     * @param msg The received NetworkMessage (type byte already consumed).
     */
    void handleEditorPacket(RME::core::NetworkMessage& msg); // Type already read by processReceivedData

    // --- Login sequence handlers ---
    /** @brief Parses a PACKET_HELLO_FROM_CLIENT message. */
    void parseClientHello(RME::core::NetworkMessage& msg);
    /** @brief Parses a PACKET_READY_CLIENT message. */
    void parseClientReady(RME::core::NetworkMessage& msg);

    // --- Editor packet handlers ---
    /** @brief Parses a PACKET_NODE_REQUEST message. */
    void parseNodeRequest(RME::core::NetworkMessage& msg);
    /** @brief Parses a PACKET_RECEIVE_CHANGES message. */
    void parseChangesReceived(RME::core::NetworkMessage& msg);
    /** @brief Parses a PACKET_CURSOR_UPDATE message. */
    void parseCursorUpdate(RME::core::NetworkMessage& msg);
    /** @brief Parses a PACKET_CHAT_MESSAGE message. */
    void parseChatMessage(RME::core::NetworkMessage& msg);
    /** @brief Parses a PACKET_CLIENT_COLOR_UPDATE message. */
    void parseClientColorUpdate(RME::core::NetworkMessage& msg);
    // Add more handlers as per LivePacketType enum from live_packets.h

    QTcpSocket* m_socket;        ///< The TCP socket for this client connection.
    QtLiveServer* m_server;      ///< Non-owning pointer to the parent QtLiveServer.
    RME::core::Map* m_mapRef;    ///< Non-owning pointer to the shared Map instance.
    RME::core::actions::UndoManager* m_undoManagerRef; ///< Non-owning pointer to the shared UndoManager.
    RME::core::assets::AssetManager* m_assetManagerRef; ///< Non-owning pointer to the shared AssetManager.

    QByteArray m_receiveBuffer;   ///< Buffer for accumulating incoming socket data.
    bool m_isAuthenticated;       ///< True if the client has successfully authenticated.
    uint32_t m_peerId;            ///< Unique ID for this peer.
    QString m_clientName;         ///< Name provided by the client during handshake.
    RME::core::NetworkColor m_clientColor; ///< Color assigned to this client.
    RME::core::MapVersion m_clientMapVersion; ///< Map version information from client hello.

    RME::core::network::MapProtocolCodec m_codec; ///< Codec for serializing/deserializing map data structures.
};

#endif // QTLIVEPEER_H
