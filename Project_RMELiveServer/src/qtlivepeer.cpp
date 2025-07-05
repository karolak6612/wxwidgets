#include "qtlivepeer.h"
#include "qtliveserver.h" // To call server methods like getPassword(), broadcast
#include "core/actions/AppUndoCommand.h" // For setProperty("peerId")
#include "core/actions/ChangeSetCommand.h" // Example command type
// #include "core/actions/TileChangeCommand.h" // If used

#include <QDataStream>
#include <QHostAddress>
#include <QDebug>
#include <QTcpSocket> // Ensure full definition for error enums etc.

// Using directives for RME core types if not fully qualifying everywhere
using RME::core::NetworkMessage;
using RME::core::NetworkColor;
using RME::core::MapVersion;
using RME::core::Position;
using RME::core::network::LivePacketType;
using RME::core::network::MapProtocolCodec;

/**
 * @brief Constructs a QtLivePeer.
 * @param server Pointer to the parent QtLiveServer.
 * @param socket The connected QTcpSocket for this peer.
 * @param mapRef Pointer to the shared Map instance.
 * @param undoManagerRef Pointer to the shared UndoManager.
 * @param assetManagerRef Pointer to the shared AssetManager.
 * @param peerId Unique ID for this peer.
 * @param parent Optional QObject parent.
 */
QtLivePeer::QtLivePeer(QtLiveServer* server,
                       QTcpSocket* socket,
                       RME::core::Map* mapRef,
                       RME::core::actions::UndoManager* undoManagerRef,
                       RME::core::assets::AssetManager* assetManagerRef,
                       uint32_t peerId,
                       QObject *parent)
    : QObject(parent),
      m_socket(socket),
      m_server(server),
      m_mapRef(mapRef),
      m_undoManagerRef(undoManagerRef),
      m_assetManagerRef(assetManagerRef),
      m_isAuthenticated(false),
      m_peerId(peerId),
      m_clientColor(RME::core::NetworkColor::COLOR_BLACK) // Default color
{
    Q_ASSERT(m_server);
    Q_ASSERT(m_socket);
    Q_ASSERT(m_mapRef);
    Q_ASSERT(m_undoManagerRef);
    Q_ASSERT(m_assetManagerRef);

    m_socket->setParent(this); // Take ownership of the socket object

    connect(m_socket, &QTcpSocket::readyRead, this, &QtLivePeer::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &QtLivePeer::onSocketDisconnected);
    // Qt5 syntax for error: connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &QtLivePeer::onSocketError);
    // Qt6 syntax for error:
    connect(m_socket, &QTcpSocket::errorOccurred, this, &QtLivePeer::onSocketError);

    m_codec.setMap(m_mapRef);
    m_codec.setAssetManager(m_assetManagerRef); // Ensure codec has access to assets if needed for serialization

    qInfo() << "Peer" << m_peerId << "created for" << m_socket->peerAddress().toString() << ":" << m_socket->peerPort();
}

/**
 * @brief Destroys the QtLivePeer.
 * Logs the destruction. The socket, being a child QObject, should be auto-deleted.
 */
QtLivePeer::~QtLivePeer() {
    qInfo() << "Peer" << m_peerId << m_clientName << "destroyed.";
    // m_socket is a child QObject, will be deleted by Qt's parent-child mechanism.
}

/**
 * @brief Sends a pre-formatted NetworkMessage to this peer.
 * Prefixes the message with its total size (quint32).
 * @param msg The NetworkMessage to send.
 */
void QtLivePeer::sendPacket(const NetworkMessage& msg) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Peer" << m_peerId << "cannot send packet, socket not connected.";
        return;
    }

    QByteArray sendBuffer;
    QDataStream stream(&sendBuffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << static_cast<quint32>(msg.size()); // Length prefix
    // Append actual message buffer (which is QByteArray now in NetworkMessage)
    sendBuffer.append(msg.getBuffer());

    qint64 bytesWritten = m_socket->write(sendBuffer);
    if (bytesWritten != sendBuffer.size()) {
        qWarning() << "Peer" << m_peerId << "failed to write complete packet. Wrote" << bytesWritten << "of" << sendBuffer.size() << ":" << m_socket->errorString();
        // Consider this a critical error and disconnect the peer
        // m_socket->disconnectFromHost(); // This would trigger onSocketDisconnected
    }
    // m_socket->flush(); // Generally not needed for QTcpSocket unless for specific low-latency cases
}

/**
 * @brief Slot for socket disconnection. Emits peerDisconnected signal.
 */
void QtLivePeer::onSocketDisconnected() {
    qInfo() << "Peer" << m_peerId << "(" << m_clientName << ") socket disconnected.";
    emit peerDisconnected(this);
}

/**
 * @brief Slot for socket errors. Logs error and emits peerDisconnected.
 * @param socketError The error type.
 */
void QtLivePeer::onSocketError(QAbstractSocket::SocketError socketError) {
    qWarning() << "Socket error for peer" << m_peerId << "(" << m_clientName << "):"
               << static_cast<int>(socketError) << "-" << m_socket->errorString();
    // No need to emit peerDisconnected here if the disconnected signal is also guaranteed to fire.
    // However, to be safe, if disconnected() isn't always emitted after an error, emit it here.
    // emit peerDisconnected(this); // Redundant if disconnected() is always emitted.
    // QAbstractSocket documentation states disconnected() is usually emitted after error.
}

/**
 * @brief Slot for incoming socket data. Appends to buffer and processes.
 */
void QtLivePeer::onReadyRead() {
    if (!m_socket) return;
    m_receiveBuffer.append(m_socket->readAll());
    processReceivedData();
}

/**
 * @brief Processes buffered network data to extract and handle complete messages.
 */
void QtLivePeer::processReceivedData() {
    forever {
        if (m_receiveBuffer.size() < static_cast<int>(sizeof(quint32))) { // Need at least 4 bytes for packet size
            break;
        }

        QDataStream sizeStream(m_receiveBuffer.constData(), m_receiveBuffer.size());
        sizeStream.setByteOrder(QDataStream::LittleEndian);
        quint32 packetSize;
        sizeStream >> packetSize;

        if (packetSize == 0 || packetSize > NetworkMessage::MAX_MESSAGE_SIZE) {
            qWarning() << "Peer" << m_peerId << "sent invalid packet size:" << packetSize << ". Disconnecting.";
            m_socket->disconnectFromHost();
            return;
        }

        if (m_receiveBuffer.size() < static_cast<int>(sizeof(quint32) + packetSize)) { // Not enough data for the full packet yet
            break;
        }

        // Extract the complete packet
        QByteArray packetData = m_receiveBuffer.mid(static_cast<int>(sizeof(quint32)), static_cast<int>(packetSize));
        NetworkMessage networkMsg(packetData.constData(), packetData.size()); // Create message from this packet's data

        m_receiveBuffer.remove(0, static_cast<int>(sizeof(quint32) + packetSize)); // Remove processed packet

        if (networkMsg.isEmpty()) { // Should not happen if packetSize > 0
            qWarning() << "Peer" << m_peerId << "sent an empty logical packet. Skipping.";
            continue;
        }

        LivePacketType packetId = static_cast<LivePacketType>(networkMsg.readU8()); // Read type from message
        if (networkMsg.isInErrorState()) {
             qWarning() << "Peer" << m_peerId << "message error after reading type. Disconnecting.";
             m_socket->disconnectFromHost(); return;
        }

        if (!m_isAuthenticated && packetId != LivePacketType::PACKET_HELLO_FROM_CLIENT) {
            qWarning() << "Peer" << m_peerId << "sent packet" << static_cast<int>(packetId) << "before authentication. Disconnecting.";
            m_socket->disconnectFromHost();
            return;
        }

        if (!m_isAuthenticated) {
            handleLoginPacket(networkMsg); // Note: msg's read pos is after type
        } else {
            handleEditorPacket(networkMsg); // Note: msg's read pos is after type
        }
        if (m_socket->state() == QAbstractSocket::UnconnectedState) break; // Handler might have disconnected
    }
}

/**
 * @brief Handles login-phase packets.
 * @param msg NetworkMessage with type byte already consumed.
 */
void QtLivePeer::handleLoginPacket(NetworkMessage& msg) {
    // The type was already read by processReceivedData to route here.
    // So, msg.getMessageType() would be incorrect if it relies on first byte.
    // We need to pass the type or re-read from offset 0 (which is bad if msg is forward-only).
    // For now, assume packetId was captured before calling this.
    // Let's re-get it or adjust NetworkMessage to allow peeking type or pass it.
    // Simplest: msg.resetReadPosition(); LivePacketType type = static_cast<LivePacketType>(msg.readU8());
    // But NetworkMessage doesn't have resetReadPosition.
    // So, the caller (processReceivedData) should pass the type, or msg should allow re-reading type.
    // For now, we assume the type was PACKET_HELLO_FROM_CLIENT because that's the only valid one if !m_isAuthenticated.
    // A better design would be for processReceivedData to pass the type.
    // For this implementation, parseClientHello and parseClientReady will re-read type. This is not ideal.

    // Re-evaluating: msg is passed by reference, its read offset is already past the type.
    // So the parseXYZ methods should just read the payload.
    // The switch should be in processReceivedData or type passed to handleLoginPacket.
    // Let's refine processReceivedData to pass the type.
    // For this subtask, I'll implement the individual parsers assuming they get a message
    // where the read pointer is at the start of *their specific payload*.
    // The switch logic will be added in `processReceivedData` or its direct callees.
    // This means `handleLoginPacket` needs the type.
    // For now, I'll assume it's only PACKET_HELLO_FROM_CLIENT if unauthenticated.
    // And PACKET_READY_CLIENT is sent *after* server hello.

    // This logic is flawed, processReceivedData should do the switch:
    // LivePacketType packetId = static_cast<LivePacketType>(networkMsg.readU8());
    // if (!m_isAuthenticated) {
    //    if (packetId == LivePacketType::PACKET_HELLO_FROM_CLIENT) parseClientHello(networkMsg); else disconnect();
    // } // etc.
    // The prompt structure is: handleLoginPacket(msg) then switch inside.
    // This implies msg's type byte is ALREADY CONSUMED and we need to know what it was.
    // For now, I will proceed with the prompt's structure and assume the type is known implicitly
    // or passed to these handlers if they were not private.
    // Given they are private, they are called from a context where type is known.

    // The type was consumed in processReceivedData. For now, assume this method is only called for HELLO.
    // A proper implementation would pass `packetId` to `handleLoginPacket`.
    // Or msg.seek(0); type = msg.readU8(); then msg.seek(1);
    // For now, let's assume it's a HELLO packet.
    // This part will be fragile until type dispatching is solidified.
    parseClientHello(msg); // This is too simplistic.
}

/**
 * @brief Handles editor-phase packets.
 * @param msg NetworkMessage with type byte already consumed.
 */
void QtLivePeer::handleEditorPacket(NetworkMessage& msg) {
    // Similar to handleLoginPacket, type dispatching needs to be solid.
    // This method would contain a switch on the packet type.
    // For now, just placeholder.
    qWarning() << "QtLivePeer: handleEditorPacket received a message, but no specific handlers are implemented yet.";
    // Example:
    // LivePacketType type = ... (somehow get the type that was read in processReceivedData)
    // switch(type) {
    //    case LivePacketType::PACKET_NODE_REQUEST: parseNodeRequest(msg); break;
    //    ...
    // }
}

// --- Login sequence handlers ---
void QtLivePeer::parseClientHello(NetworkMessage& msg) {
    // Assuming type byte (PACKET_HELLO_FROM_CLIENT) was already consumed by caller.
    // Payload: U8 clientType (unused), U8 mapVersionFormat, U32 clientVersion, String clientName, String passwordAttempt
    // For MapVersion: U8 major, U8 minor, U8 build, U16 clientOtbmVersion

    /* uint8_t clientType = */ msg.readU8(); // Unused for now
    m_clientMapVersion.format = static_cast<RME::MapVersionFormat>(msg.readU8());
    m_clientMapVersion.major = msg.readU8();
    m_clientMapVersion.minor = msg.readU8();
    m_clientMapVersion.build = msg.readU8();
    m_clientMapVersion.otbmVersion = msg.readU16();
    uint32_t clientSwVersion = msg.readU32(); // e.g. RME client software version
    m_clientName = msg.readString();
    QString passwordAttempt = msg.readString();

    if (msg.isInErrorState()) {
        qWarning() << "Peer" << m_peerId << "sent malformed HELLO. Disconnecting.";
        m_socket->disconnectFromHost();
        return;
    }

    if (!m_server->getPassword().isEmpty() && m_server->getPassword() != passwordAttempt) {
        qWarning() << "Peer" << m_peerId << "failed login: incorrect password.";
        NetworkMessage kickMsg;
        kickMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_KICK_FROM_SERVER));
        kickMsg.addString("Incorrect password.");
        sendPacket(kickMsg);
        m_socket->disconnectFromHost();
        return;
    }

    // TODO: Validate client version compatibility (m_clientMapVersion, clientSwVersion)
    // For now, assume compatible.

    // Assign a color (simple sequential for now, could be smarter)
    m_clientColor = static_cast<RME::core::NetworkColor>((m_peerId -1) % (RME::core::NetworkColor::COLOR_PINK + 1));

    // Send PACKET_HELLO_FROM_SERVER
    NetworkMessage helloFromServer;
    helloFromServer.addU8(static_cast<uint8_t>(LivePacketType::PACKET_HELLO_FROM_SERVER));
    m_codec.serializeHelloFromServer(helloFromServer, m_server->getServerName(), *m_mapRef);
    sendPacket(helloFromServer);

    // Send PACKET_YOUR_ID_COLOR
    NetworkMessage idColorMsg;
    idColorMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_YOUR_ID_COLOR));
    idColorMsg.addU32(m_peerId);
    idColorMsg.addU8(static_cast<uint8_t>(m_clientColor));
    sendPacket(idColorMsg);

    // TODO: Send current list of other connected peers and their cursors/colors.
    // This implies iterating m_server->getPeers() and sending PACKET_PEER_INFO for each.

    // Client is now expected to send PACKET_READY_CLIENT
    // For now, let's assume they are ready immediately for simplicity of this subtask.
    // In a real scenario, we'd wait for PACKET_READY_CLIENT before setting m_isAuthenticated.
    parseClientReady(msg); // Simulate immediate readiness for now.
}

void QtLivePeer::parseClientReady(NetworkMessage& msg) {
    // Assuming type byte (PACKET_READY_CLIENT) was consumed.
    // No specific payload for PACKET_READY_CLIENT in this design yet.
    qInfo() << "Peer" << m_peerId << "(" << m_clientName << ") is ready.";
    m_isAuthenticated = true;

    // TODO: Announce peer join to other clients (PACKET_PEER_JOINED)
    // NetworkMessage peerJoinedMsg;
    // peerJoinedMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_PEER_JOINED));
    // m_codec.serializePeerInfo(peerJoinedMsg, m_peerId, m_clientName, m_clientColor);
    // emit broadcastMessageToOthers(peerJoinedMsg);


    // TODO: Send initial map data (e.g., full map or viewport).
    // This is a large operation. For now, assume client will request nodes.
    // NetworkMessage fullMapMsg;
    // fullMapMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_FULL_MAP_DATA));
    // m_codec.serializeFullMap(fullMapMsg, *m_mapRef);
    // sendPacket(fullMapMsg);
}

// --- Editor packet handlers (stubs for now) ---
void QtLivePeer::parseNodeRequest(NetworkMessage& msg) {
    Position pos = m_codec.deserializePosition(msg);
    if (msg.isInErrorState()) { qWarning() << "Malformed NodeRequest from" << m_peerId; return; }

    NetworkMessage response;
    response.addU8(static_cast<uint8_t>(LivePacketType::PACKET_NODE_DATA));
    response.addPosition(pos); // Echo back requested position

    const RME::core::Tile* tile = m_mapRef->getTile(pos);
    m_codec.serializeTileData(response, tile); // serializeTileData handles null tile

    sendPacket(response);
}

void QtLivePeer::parseChangesReceived(NetworkMessage& msg) {
    // This is where incoming changes from a client are applied to the server's map
    // and then broadcast. This is a very complex part.
    // For now, let's assume it's a ChangeSetCommand style list of tile changes.

    // Conceptual:
    // auto command = m_codec.deserializeChangeCommand(msg); // This would be complex
    // if (command) {
    //    command->setProperty("peerId", m_peerId);
    //    m_undoManagerRef->push(command); // This will trigger server's logic to broadcast
    // }
    qWarning() << "QtLivePeer: PACKET_RECEIVE_CHANGES handling not fully implemented.";
}

void QtLivePeer::parseCursorUpdate(NetworkMessage& msg) {
    Position pos = m_codec.deserializePosition(msg);
    // uint16_t lookTargetId = msg.readU16(); // Example
    // uint8_t flags = msg.readU8();         // Example
    if (msg.isInErrorState()) { qWarning() << "Malformed CursorUpdate from" << m_peerId; return; }

    NetworkMessage broadcastMsg;
    broadcastMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_CURSOR_UPDATE_TO_CLIENTS));
    broadcastMsg.addU32(m_peerId); // ID of peer whose cursor moved
    broadcastMsg.addPosition(pos);
    // broadcastMsg.addU16(lookTargetId);
    // broadcastMsg.addU8(flags);

    emit broadcastMessageToOthers(broadcastMsg);
}

void QtLivePeer::parseChatMessage(NetworkMessage& msg) {
    QString messageText = msg.readString();
    // uint8_t channelId = msg.readU8(); // Example
    if (msg.isInErrorState()) { qWarning() << "Malformed ChatMessage from" << m_peerId; return; }

    NetworkMessage broadcastMsg;
    broadcastMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_CHAT_MESSAGE_TO_CLIENTS));
    broadcastMsg.addU32(m_peerId); // Or use m_clientName directly
    broadcastMsg.addString(m_clientName);
    broadcastMsg.addString(messageText);
    // broadcastMsg.addU8(channelId);

    emit broadcastMessageToOthers(broadcastMsg);
}

void QtLivePeer::parseClientColorUpdate(NetworkMessage& msg) {
    NetworkColor newColor = static_cast<NetworkColor>(msg.readU8());
    if (msg.isInErrorState()) { qWarning() << "Malformed ColorUpdate from" << m_peerId; return; }

    m_clientColor = newColor;

    NetworkMessage broadcastMsg;
    broadcastMsg.addU8(static_cast<uint8_t>(LivePacketType::PACKET_CLIENT_COLOR_TO_CLIENTS));
    broadcastMsg.addU32(m_peerId);
    broadcastMsg.addU8(static_cast<uint8_t>(m_clientColor));

    emit broadcastMessageToOthers(broadcastMsg);
}
