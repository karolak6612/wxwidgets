#include "qtliveserver.h"
#include "qtlivepeer.h" // Will be created in a subsequent task
#include "core/io/OtbmMapIO.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>

/**
 * @brief Constructs the QtLiveServer.
 * Initializes the map instance with the asset manager and the undo manager with the map instance.
 * Sets a default server name.
 * @param parent Optional QObject parent.
 */
QtLiveServer::QtLiveServer(QObject *parent) :
    QObject(parent),
    m_mapInstance(&(m_assetManager.getItemDatabase())), // Map needs IItemTypeProvider, get it from AssetManager's ItemDatabase
    m_undoManager(&m_mapInstance)
{
    m_serverName = "RME Live Server";
    m_nextPeerId = 1;
    // AssetManager and AppSettings are default constructed.
    // Map and UndoManager are constructed with their dependencies.
}

/**
 * @brief Destroys the QtLiveServer.
 * Stops the TCP server if it's running and cleans up all peer connections.
 */
QtLiveServer::~QtLiveServer() {
    if (m_tcpServer) {
        m_tcpServer->close();
        // m_tcpServer is a child QObject, will be deleted with this, or delete it explicitly.
        // delete m_tcpServer; // Or rely on QObject parent ownership
    }
    // Peers are QObjects and should have this as parent, or be deleted via deleteLater
    qDeleteAll(m_peers); // This is safe if peers are heap-allocated and not parented for auto-deletion elsewhere
    m_peers.clear();
}

/**
 * @brief Starts the live server.
 * Loads assets, loads the specified map, and begins listening for incoming connections.
 * @param port Port to listen on.
 * @param mapFilePath Path to the .otbm map file.
 * @param serverPassword Optional password for clients.
 * @param dataPackPath Path to game data assets.
 * @param serverClientVersion Client version string for assets.
 * @return True if server started successfully, false otherwise.
 */
bool QtLiveServer::startServer(quint16 port,
                               const QString& mapFilePath,
                               const QString& serverPassword,
                               const QString& dataPackPath,
                               const QString& serverClientVersion) {
    m_serverPassword = serverPassword;
    qInfo() << "Attempting to start RME Live Server on port" << port << "for map" << mapFilePath;
    qInfo() << "Data pack path:" << dataPackPath << "Client version for assets:" << serverClientVersion;

    // 1. Load Assets
    if (!m_assetManager.loadAllAssets(dataPackPath, serverClientVersion)) {
        qCritical() << "QtLiveServer: Failed to load server assets from" << dataPackPath
                    << "for client version" << serverClientVersion
                    << ". Check AssetManager logs for details.";
        return false;
    }
    qInfo() << "QtLiveServer: Assets loaded successfully.";

    // 2. Load Map
    RME::core::io::OtbmMapIO mapIo;
    if (!mapIo.loadMap(mapFilePath, m_mapInstance, m_assetManager, m_appSettings)) {
        qCritical() << "QtLiveServer: Failed to load map file:" << mapFilePath
                    << ". Error:" << mapIo.getLastError();
        return false;
    }
    qInfo() << "QtLiveServer: Map" << mapFilePath << "loaded successfully.";
    qInfo() << "Map description:" << m_mapInstance.getDescription();
    qInfo() << "Map dimensions:" << m_mapInstance.getWidth() << "x" << m_mapInstance.getHeight() << "x" << m_mapInstance.getDepth();


    // 3. Start TCP Server
    if (m_tcpServer) { // Should not happen, but good practice
        m_tcpServer->close();
        delete m_tcpServer;
    }
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &QtLiveServer::onNewConnection);

    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        qCritical() << "QtLiveServer: Server failed to listen on port" << port
                    << "- Error:" << m_tcpServer->errorString();
        delete m_tcpServer;
        m_tcpServer = nullptr;
        return false;
    }

    qInfo() << "QtLiveServer: Server is listening on port" << port;
    return true;
}

/**
 * @brief Slot for handling new incoming TCP connections.
 * Creates a QtLivePeer for each new valid connection.
 */
void QtLiveServer::onNewConnection() {
    if (!m_tcpServer || !m_tcpServer->hasPendingConnections()) {
        return;
    }

    QTcpSocket* clientSocket = m_tcpServer->nextPendingConnection();
    if (clientSocket) {
        qInfo() << "QtLiveServer: New connection from:" << clientSocket->peerAddress().toString()
                << "port" << clientSocket->peerPort();

        // QtLivePeer needs to be defined. For now, this is placeholder logic.
        // Assuming QtLivePeer takes this server as parent for QObject hierarchy.
        QtLivePeer* newPeer = new QtLivePeer(this, clientSocket, &m_mapInstance, &m_undoManager, &m_assetManager, m_nextPeerId++);
        // newPeer->setParent(this); // Done if 'this' is passed as parent to constructor

        connect(newPeer, &QtLivePeer::peerDisconnected, this, &QtLiveServer::onPeerDisconnected);
        connect(newPeer, &QtLivePeer::broadcastMessageToOthers, this, [this, newPeer](const RME::core::NetworkMessage& msg){
            this->broadcastMessageToAll(msg, newPeer);
        });

        m_peers.append(newPeer);
        qInfo() << "QtLiveServer: Peer" << newPeer->getPeerId() << "added. Total peers:" << m_peers.size();
        // TODO: Peer needs to start its handshake/login process here.
        // newPeer->startHandshake(); // Or similar
    }
}

/**
 * @brief Slot for handling peer disconnections.
 * Removes the peer from the managed list and schedules it for deletion.
 * @param peer The peer that disconnected.
 */
void QtLiveServer::onPeerDisconnected(QtLivePeer* peer) {
    if (peer) {
        qInfo() << "QtLiveServer: Peer" << peer->getPeerId() << "disconnected.";
        m_peers.removeAll(peer);
        peer->deleteLater(); // Safe deletion
        qInfo() << "QtLiveServer: Peer" << peer->getPeerId() << "removed. Total peers:" << m_peers.size();
        // TODO: Broadcast peer leave message to other clients if protocol supports it.
        // RME::core::NetworkMessage leaveMsg;
        // leaveMsg.prepare(RME::core::NetworkMessageType::PEER_LEFT); // Example
        // leaveMsg.addU32(peer->getPeerId());
        // broadcastMessageToAll(leaveMsg, nullptr);
    }
}

/**
 * @brief Broadcasts a network message to all connected peers, optionally excluding one.
 * @param msg The message to broadcast.
 * @param excludePeer Peer to exclude from broadcast (typically the originator), or nullptr.
 */
void QtLiveServer::broadcastMessageToAll(const RME::core::NetworkMessage& msg, QtLivePeer* excludePeer) {
    if (msg.isEmpty()) {
        qWarning() << "QtLiveServer: Attempted to broadcast an empty message.";
        return;
    }
    // qInfo() << "QtLiveServer: Broadcasting message of type" << static_cast<int>(msg.getMessageType()) << "to" << (m_peers.size() - (excludePeer ? 1:0)) << "peer(s).";
    for (QtLivePeer* peer : m_peers) {
        if (peer && peer != excludePeer) {
            peer->sendNetworkMessage(msg); // Assuming QtLivePeer has sendNetworkMessage
        }
    }
}
