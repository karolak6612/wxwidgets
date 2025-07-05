#include "QtLiveClient.h"
#include "core/Map.h"
#include "core/actions/UndoManager.h"
#include "core/assets/AssetManager.h"
#include "core/editor/EditorControllerInterface.h"

#include <QHostAddress>
#include <QDataStream>
#include <QDebug>

namespace RME {
namespace network {

QtLiveClient::QtLiveClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_connectionTimer(new QTimer(this))
    , m_connectionState(ConnectionState::Disconnected)
    , m_serverPort(0)
    , m_clientId(0)
    , m_mapRef(nullptr)
    , m_undoManagerRef(nullptr)
    , m_assetManagerRef(nullptr)
    , m_editorController(nullptr)
    , m_cursorTrackingEnabled(true)
    , m_cursorUpdateIntervalMs(100) // 100ms default
    , m_cursorUpdateTimer(new QTimer(this))
    , m_lastCursorPosition(-1, -1, -1)
{
    // Setup socket connections
    connect(m_socket, &QTcpSocket::connected, this, &QtLiveClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &QtLiveClient::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &QtLiveClient::onSocketReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &QtLiveClient::onSocketError);
    
    // Setup connection timer
    m_connectionTimer->setSingleShot(true);
    m_connectionTimer->setInterval(CONNECTION_TIMEOUT_MS);
    connect(m_connectionTimer, &QTimer::timeout, this, &QtLiveClient::onConnectionTimeout);
    
    // Setup cursor update timer
    m_cursorUpdateTimer->setSingleShot(false);
    m_cursorUpdateTimer->setInterval(m_cursorUpdateIntervalMs);
    connect(m_cursorUpdateTimer, &QTimer::timeout, this, &QtLiveClient::onCursorUpdateTimer);
    
    // Initialize default color
    m_clientColor = {255, 255, 255, 255}; // White default
}

QtLiveClient::~QtLiveClient()
{
    disconnectFromServer();
}

bool QtLiveClient::connectToServer(const QString& hostname, quint16 port, 
                                  const QString& username, const QString& password)
{
    if (m_connectionState != ConnectionState::Disconnected) {
        setError("Already connected or connecting");
        return false;
    }
    
    if (hostname.isEmpty() || port == 0 || username.isEmpty()) {
        setError("Invalid connection parameters");
        return false;
    }
    
    m_serverHostname = hostname;
    m_serverPort = port;
    m_username = username;
    m_password = password;
    m_clientName = username;
    
    qInfo() << "QtLiveClient: Connecting to" << hostname << ":" << port << "as" << username;
    
    setConnectionState(ConnectionState::Connecting);
    m_connectionTimer->start();
    m_socket->connectToHost(hostname, port);
    
    return true;
}

void QtLiveClient::disconnectFromServer()
{
    if (m_connectionState == ConnectionState::Disconnected) {
        return;
    }
    
    qInfo() << "QtLiveClient: Disconnecting from server";
    
    m_connectionTimer->stop();
    
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000); // Wait up to 3 seconds
        }
    }
    
    setConnectionState(ConnectionState::Disconnected);
}

void QtLiveClient::setMapContext(RME::core::Map* map, 
                                RME::core::actions::UndoManager* undoManager,
                                RME::core::assets::AssetManager* assetManager)
{
    m_mapRef = map;
    m_undoManagerRef = undoManager;
    m_assetManagerRef = assetManager;
    
    if (map) {
        // Get map version information from the map
        m_mapVersion.format = RME::MapVersionFormat::OTBM;
        m_mapVersion.major = 2; // OTBM major version
        m_mapVersion.minor = 0; // OTBM minor version
        m_mapVersion.build = 0; // Build number
        m_mapVersion.otbmVersion = 2; // OTBM format version
        
        // In a full implementation, these would come from the map:
        // m_mapVersion = map->getVersionInfo();
    }
}

void QtLiveClient::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
}

void QtLiveClient::sendMapChanges(const QList<RME::core::network::TileChange>& changes)
{
    if (!isConnected() || changes.isEmpty()) {
        return;
    }
    
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_RECEIVE_CHANGES));
    
    RME::core::network::MapChangesClientData data;
    data.changes = changes;
    
    if (m_codec.serializeData(data, msg, m_mapVersion)) {
        sendPacket(msg);
    } else {
        qWarning() << "QtLiveClient: Failed to serialize map changes";
    }
}

void QtLiveClient::sendCursorUpdate(const RME::core::Position& position)
{
    if (!isConnected()) {
        return;
    }
    
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_CURSOR_UPDATE));
    msg.addPosition(position);
    
    sendPacket(msg);
}

void QtLiveClient::sendChatMessage(const QString& message)
{
    if (!isConnected() || message.isEmpty()) {
        return;
    }
    
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_CHAT_MESSAGE));
    
    RME::core::network::ChatMessageClientData data;
    data.message = message;
    
    if (m_codec.serializeData(data, msg)) {
        sendPacket(msg);
    } else {
        qWarning() << "QtLiveClient: Failed to serialize chat message";
    }
}

void QtLiveClient::requestMapNode(const RME::core::Position& nodePosition)
{
    if (!isConnected()) {
        return;
    }
    
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_NODE_REQUEST));
    
    RME::core::network::MapNodeRequestClientData data;
    data.nodePosition = nodePosition;
    
    if (m_codec.serializeData(data, msg)) {
        sendPacket(msg);
    } else {
        qWarning() << "QtLiveClient: Failed to serialize node request";
    }
}

void QtLiveClient::onMapChanged(const QList<RME::core::network::TileChange>& changes)
{
    sendMapChanges(changes);
}

void QtLiveClient::onCursorMoved(const RME::core::Position& position)
{
    sendCursorUpdate(position);
}

void QtLiveClient::onSocketConnected()
{
    qInfo() << "QtLiveClient: Socket connected to server";
    m_connectionTimer->stop();
    
    setConnectionState(ConnectionState::Authenticating);
    sendClientHello();
}

void QtLiveClient::onSocketDisconnected()
{
    qInfo() << "QtLiveClient: Socket disconnected from server";
    m_connectionTimer->stop();
    
    // Clear peer information
    m_connectedPeers.clear();
    m_peerColors.clear();
    m_peerCursors.clear();
    
    setConnectionState(ConnectionState::Disconnected);
    emit disconnected();
}

void QtLiveClient::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket->errorString();
    qWarning() << "QtLiveClient: Socket error:" << error << "-" << errorString;
    
    m_connectionTimer->stop();
    setError(QString("Connection error: %1").arg(errorString));
    setConnectionState(ConnectionState::Error);
}

void QtLiveClient::onSocketReadyRead()
{
    QByteArray newData = m_socket->readAll();
    m_receiveBuffer.append(newData);
    processReceivedData();
}

void QtLiveClient::onConnectionTimeout()
{
    qWarning() << "QtLiveClient: Connection timeout";
    setError("Connection timeout");
    disconnectFromServer();
}

void QtLiveClient::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged(state);
        
        if (state == ConnectionState::Connected) {
            emit connected();
        }
    }
}

void QtLiveClient::setError(const QString& error)
{
    m_lastError = error;
    qWarning() << "QtLiveClient error:" << error;
    emit errorOccurred(error);
}

void QtLiveClient::processReceivedData()
{
    while (m_receiveBuffer.size() >= 4) { // Minimum size for length header
        // Read message length
        QDataStream stream(m_receiveBuffer);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        uint32_t messageLength;
        stream >> messageLength;
        
        if (messageLength > 65536) { // Sanity check
            setError("Invalid message length received");
            disconnectFromServer();
            return;
        }
        
        if (m_receiveBuffer.size() < static_cast<int>(4 + messageLength)) {
            // Not enough data yet
            break;
        }
        
        // Extract the complete message
        QByteArray messageData = m_receiveBuffer.mid(4, messageLength);
        m_receiveBuffer.remove(0, 4 + messageLength);
        
        // Process the message
        RME::core::network::NetworkMessage msg;
        msg.addBytes(reinterpret_cast<const uint8_t*>(messageData.constData()), messageData.size());
        msg.resetRead();
        
        uint8_t packetType;
        if (!msg.getU8(packetType)) {
            qWarning() << "QtLiveClient: Failed to read packet type";
            continue;
        }
        
        if (m_connectionState == ConnectionState::Authenticating) {
            handleLoginPacket(msg);
        } else if (m_connectionState == ConnectionState::Connected) {
            handleServerPacket(msg);
        }
    }
}

void QtLiveClient::handleLoginPacket(RME::core::network::NetworkMessage& msg)
{
    uint8_t packetType;
    msg.setReadPosition(0);
    if (!msg.getU8(packetType)) {
        return;
    }
    
    auto type = static_cast<RME::core::network::LivePacketType>(packetType);
    
    switch (type) {
        case RME::core::network::LivePacketType::PACKET_HELLO_FROM_SERVER:
            parseServerHello(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_YOUR_ID_COLOR:
            parseYourIdColor(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_PEER_LIST:
            parsePeerList(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_KICK:
            parseKickMessage(msg);
            break;
            
        default:
            qWarning() << "QtLiveClient: Unexpected packet type during login:" << packetType;
            break;
    }
}

void QtLiveClient::handleServerPacket(RME::core::network::NetworkMessage& msg)
{
    uint8_t packetType;
    msg.setReadPosition(0);
    if (!msg.getU8(packetType)) {
        return;
    }
    
    auto type = static_cast<RME::core::network::LivePacketType>(packetType);
    
    switch (type) {
        case RME::core::network::LivePacketType::PACKET_SEND_CHANGES:
            parseMapChanges(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_CURSOR_UPDATE:
            parseCursorUpdate(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_CHAT_MESSAGE:
            parseChatMessage(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_PEER_JOINED:
            parsePeerJoined(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_PEER_LEFT:
            parsePeerLeft(msg);
            break;
            
        case RME::core::network::LivePacketType::PACKET_KICK:
            parseKickMessage(msg);
            break;
            
        default:
            qWarning() << "QtLiveClient: Unknown packet type:" << packetType;
            break;
    }
}

void QtLiveClient::parseServerHello(RME::core::network::NetworkMessage& msg)
{
    RME::core::network::ServerHelloServerData data;
    if (m_codec.deserializeData(msg, data)) {
        qInfo() << "QtLiveClient: Received server hello from" << data.serverName;
        qInfo() << "Server version:" << data.serverVersion << "Map:" << data.mapName;
        
        // Send client ready response
        sendClientReady();
    } else {
        setError("Failed to parse server hello");
        disconnectFromServer();
    }
}

void QtLiveClient::parseYourIdColor(RME::core::network::NetworkMessage& msg)
{
    RME::core::network::YourIdColorData data;
    if (m_codec.deserializeData(msg, data)) {
        m_clientId = data.yourPeerId;
        m_clientColor = data.yourColor;
        
        qInfo() << "QtLiveClient: Assigned ID" << m_clientId << "and color" 
                << m_clientColor.r << m_clientColor.g << m_clientColor.b;
    } else {
        qWarning() << "QtLiveClient: Failed to parse your ID/color";
    }
}

void QtLiveClient::parsePeerList(RME::core::network::NetworkMessage& msg)
{
    RME::core::network::PeerListServerData data;
    if (m_codec.deserializeData(msg, data)) {
        m_connectedPeers.clear();
        m_peerColors.clear();
        
        for (const auto& peer : data.peers) {
            m_connectedPeers[peer.peerId] = peer.peerName;
            m_peerColors[peer.peerId] = peer.peerColor;
        }
        
        qInfo() << "QtLiveClient: Received peer list with" << data.peers.size() << "peers";
        
        // Authentication complete
        setConnectionState(ConnectionState::Connected);
    } else {
        qWarning() << "QtLiveClient: Failed to parse peer list";
    }
}

void QtLiveClient::parseKickMessage(RME::core::network::NetworkMessage& msg)
{
    RME::core::network::KickClientData data;
    if (m_codec.deserializeData(msg, data)) {
        qWarning() << "QtLiveClient: Kicked from server:" << data.reason;
        emit serverKicked(data.reason);
        disconnectFromServer();
    } else {
        qWarning() << "QtLiveClient: Failed to parse kick message";
    }
}

void QtLiveClient::parseMapChanges(RME::core::network::NetworkMessage& msg)
{
    if (!m_mapRef || !m_assetManagerRef) {
        qWarning() << "QtLiveClient: No map context for processing changes";
        return;
    }
    
    RME::core::network::MapChangesServerData data;
    if (m_codec.deserializeData(msg, data, m_mapVersion, 
                               &m_assetManagerRef->getItemDatabase(), *m_mapRef)) {
        qInfo() << "QtLiveClient: Received" << data.changes.size() << "map changes";
        emit mapChangesReceived(data.changes);
    } else {
        qWarning() << "QtLiveClient: Failed to parse map changes";
    }
}

void QtLiveClient::parseCursorUpdate(RME::core::network::NetworkMessage& msg)
{
    uint32_t peerId;
    RME::core::Position position;
    
    if (msg.getU32(peerId) && msg.getPosition(position)) {
        m_peerCursors[peerId] = position;
        
        auto colorIt = m_peerColors.find(peerId);
        RME::core::network::NetworkColor color = (colorIt != m_peerColors.end()) 
            ? colorIt.value() : RME::core::network::NetworkColor{255, 255, 255, 255};
        
        emit peerCursorUpdated(peerId, position, color);
    } else {
        qWarning() << "QtLiveClient: Failed to parse cursor update";
    }
}

void QtLiveClient::parseChatMessage(RME::core::network::NetworkMessage& msg)
{
    RME::core::network::ChatMessageServerData data;
    if (m_codec.deserializeData(msg, data)) {
        emit chatMessageReceived(data.speakerPeerId, data.speakerName, data.message);
    } else {
        qWarning() << "QtLiveClient: Failed to parse chat message";
    }
}

void QtLiveClient::parsePeerJoined(RME::core::network::NetworkMessage& msg)
{
    uint32_t peerId;
    QString peerName;
    RME::core::network::NetworkColor peerColor;
    
    if (msg.getU32(peerId) && msg.getString(peerName) && 
        msg.getU8(peerColor.r) && msg.getU8(peerColor.g) && msg.getU8(peerColor.b)) {
        
        // Add peer to our tracking
        m_connectedPeers[peerId] = peerName;
        m_peerColors[peerId] = peerColor;
        
        qInfo() << "QtLiveClient: Peer joined -" << peerName << "(" << peerId << ")";
        emit peerJoined(peerId, peerName, peerColor);
    } else {
        qWarning() << "QtLiveClient: Failed to parse peer joined message";
    }
}

void QtLiveClient::parsePeerLeft(RME::core::network::NetworkMessage& msg)
{
    uint32_t peerId;
    
    if (msg.getU32(peerId)) {
        QString peerName = m_connectedPeers.value(peerId, "Unknown");
        
        // Remove peer from our tracking
        m_connectedPeers.remove(peerId);
        m_peerColors.remove(peerId);
        m_peerCursors.remove(peerId);
        
        qInfo() << "QtLiveClient: Peer left -" << peerName << "(" << peerId << ")";
        emit peerLeft(peerId, peerName);
    } else {
        qWarning() << "QtLiveClient: Failed to parse peer left message";
    }
}

void QtLiveClient::sendClientHello()
{
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_HELLO_FROM_CLIENT));
    
    RME::core::network::ClientHelloClientData data;
    data.clientName = m_clientName;
    data.clientPassword = m_password;
    data.clientSwVersion = "RME-Qt6-1.0.0"; // Version from project
    data.clientMapVersion = m_mapVersion;
    
    if (m_codec.serializeData(data, msg)) {
        sendPacket(msg);
    } else {
        setError("Failed to send client hello");
        disconnectFromServer();
    }
}

void QtLiveClient::sendClientReady()
{
    RME::core::network::NetworkMessage msg;
    msg.addU8(static_cast<uint8_t>(RME::core::network::LivePacketType::PACKET_READY_CLIENT));
    
    sendPacket(msg);
}

bool QtLiveClient::sendPacket(const RME::core::network::NetworkMessage& msg)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }
    
    // Prepare message with length header
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    uint32_t messageLength = static_cast<uint32_t>(msg.getSize());
    stream << messageLength;
    
    packet.append(reinterpret_cast<const char*>(msg.getData()), msg.getSize());
    
    qint64 bytesWritten = m_socket->write(packet);
    if (bytesWritten != packet.size()) {
        qWarning() << "QtLiveClient: Failed to send complete packet";
        return false;
    }
    
    return true;
}

void QtLiveClient::onCursorUpdateTimer()
{
    // This timer fires periodically to send cursor updates
    // The actual cursor position is set via onCursorMoved slot
    // We only send if the position has changed and we're connected
    if (isConnected() && m_cursorTrackingEnabled) {
        // The cursor update is sent immediately in onCursorMoved
        // This timer could be used for periodic heartbeat or cleanup
    }
}

// Cursor tracking implementation
void QtLiveClient::enableCursorTracking(bool enabled)
{
    m_cursorTrackingEnabled = enabled;
    
    if (enabled && isConnected()) {
        m_cursorUpdateTimer->start();
    } else {
        m_cursorUpdateTimer->stop();
    }
    
    qInfo() << "QtLiveClient: Cursor tracking" << (enabled ? "enabled" : "disabled");
}

void QtLiveClient::setCursorUpdateInterval(int intervalMs)
{
    m_cursorUpdateIntervalMs = intervalMs;
    m_cursorUpdateTimer->setInterval(intervalMs);
    
    qInfo() << "QtLiveClient: Cursor update interval set to" << intervalMs << "ms";
}

void QtLiveClient::onCursorMoved(const RME::core::Position& position)
{
    if (!isConnected() || !m_cursorTrackingEnabled) {
        return;
    }
    
    // Only send update if position actually changed
    if (position == m_lastCursorPosition) {
        return;
    }
    
    m_lastCursorPosition = position;
    sendCursorUpdate(position);
}

void QtLiveClient::onMapChanged(const QList<RME::core::network::TileChange>& changes)
{
    if (!isConnected()) {
        return;
    }
    
    sendMapChanges(changes);
}

} // namespace network
} // namespace RME