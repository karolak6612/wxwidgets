#ifndef QT_LIVE_CLIENT_H
#define QT_LIVE_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <cstdint>

// Core RME library includes
#include "core/network/NetworkMessage.h"
#include "core/network/MapProtocolCodec.h"
#include "core/network/live_packets.h"
#include "core/Position.h"
#include "core/map_constants.h"

// Forward declarations
namespace RME {
namespace core {
    class Map;
    namespace actions { class UndoManager; }
    namespace assets { class AssetManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace network {

/**
 * @brief Represents a live collaboration client that connects to a QtLiveServer
 * 
 * This class manages the TCP connection to a live server, handles the login
 * sequence, processes incoming map changes, and sends local changes to the server.
 */
class QtLiveClient : public QObject {
    Q_OBJECT

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Authenticating,
        Connected,
        Error
    };
    Q_ENUM(ConnectionState)

    explicit QtLiveClient(QObject* parent = nullptr);
    ~QtLiveClient() override;

    // Connection management
    bool connectToServer(const QString& hostname, quint16 port, 
                        const QString& username, const QString& password = QString());
    void disconnectFromServer();
    
    // State queries
    ConnectionState getConnectionState() const { return m_connectionState; }
    bool isConnected() const { return m_connectionState == ConnectionState::Connected; }
    QString getLastError() const { return m_lastError; }
    
    // Client information
    uint32_t getClientId() const { return m_clientId; }
    QString getClientName() const { return m_clientName; }
    RME::core::network::NetworkColor getClientColor() const { return m_clientColor; }
    
    // Map integration
    void setMapContext(RME::core::Map* map, 
                      RME::core::actions::UndoManager* undoManager,
                      RME::core::assets::AssetManager* assetManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);
    
    // Live collaboration features
    void sendMapChanges(const QList<RME::core::network::TileChange>& changes);
    void sendCursorUpdate(const RME::core::Position& position);
    void sendChatMessage(const QString& message);
    void requestMapNode(const RME::core::Position& nodePosition);
    
    // Peer information
    QMap<uint32_t, QString> getConnectedPeers() const { return m_connectedPeers; }
    QMap<uint32_t, RME::core::network::NetworkColor> getPeerColors() const { return m_peerColors; }
    QMap<uint32_t, RME::core::Position> getPeerCursors() const { return m_peerCursors; }
    
    // Cursor tracking
    void enableCursorTracking(bool enabled);
    bool isCursorTrackingEnabled() const { return m_cursorTrackingEnabled; }
    void setCursorUpdateInterval(int intervalMs);
    int getCursorUpdateInterval() const { return m_cursorUpdateIntervalMs; }

public slots:
    void onMapChanged(const QList<RME::core::network::TileChange>& changes);
    void onCursorMoved(const RME::core::Position& position);

signals:
    void connectionStateChanged(ConnectionState state);
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    
    // Live collaboration events
    void mapChangesReceived(const QList<RME::core::network::TileChange>& changes);
    void peerCursorUpdated(uint32_t peerId, const RME::core::Position& position, 
                          const RME::core::network::NetworkColor& color);
    void chatMessageReceived(uint32_t peerId, const QString& senderName, const QString& message);
    void peerJoined(uint32_t peerId, const QString& peerName, const RME::core::network::NetworkColor& color);
    void peerLeft(uint32_t peerId, const QString& peerName);
    void serverKicked(const QString& reason);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onConnectionTimeout();
    void onCursorUpdateTimer();

private:
    // Connection management
    void setConnectionState(ConnectionState state);
    void setError(const QString& error);
    
    // Message processing
    void processReceivedData();
    void handleLoginPacket(RME::core::network::NetworkMessage& msg);
    void handleServerPacket(RME::core::network::NetworkMessage& msg);
    
    // Login sequence handlers
    void parseServerHello(RME::core::network::NetworkMessage& msg);
    void parseYourIdColor(RME::core::network::NetworkMessage& msg);
    void parsePeerList(RME::core::network::NetworkMessage& msg);
    void parseKickMessage(RME::core::network::NetworkMessage& msg);
    
    // Server packet handlers
    void parseMapChanges(RME::core::network::NetworkMessage& msg);
    void parseCursorUpdate(RME::core::network::NetworkMessage& msg);
    void parseChatMessage(RME::core::network::NetworkMessage& msg);
    void parsePeerJoined(RME::core::network::NetworkMessage& msg);
    void parsePeerLeft(RME::core::network::NetworkMessage& msg);
    
    // Message sending helpers
    void sendClientHello();
    void sendClientReady();
    bool sendPacket(const RME::core::network::NetworkMessage& msg);
    
    // Network components
    QTcpSocket* m_socket;
    QTimer* m_connectionTimer;
    QByteArray m_receiveBuffer;
    
    // Connection state
    ConnectionState m_connectionState;
    QString m_lastError;
    QString m_serverHostname;
    quint16 m_serverPort;
    QString m_username;
    QString m_password;
    
    // Client information
    uint32_t m_clientId;
    QString m_clientName;
    RME::core::network::NetworkColor m_clientColor;
    RME::core::MapVersion m_mapVersion;
    
    // Map context
    RME::core::Map* m_mapRef;
    RME::core::actions::UndoManager* m_undoManagerRef;
    RME::core::assets::AssetManager* m_assetManagerRef;
    RME::core::editor::EditorControllerInterface* m_editorController;
    
    // Peer tracking
    QMap<uint32_t, QString> m_connectedPeers;
    QMap<uint32_t, RME::core::network::NetworkColor> m_peerColors;
    QMap<uint32_t, RME::core::Position> m_peerCursors;
    
    // Protocol codec
    RME::core::network::MapProtocolCodec m_codec;
    
    // Cursor tracking
    bool m_cursorTrackingEnabled;
    int m_cursorUpdateIntervalMs;
    QTimer* m_cursorUpdateTimer;
    RME::core::Position m_lastCursorPosition;
    
    // Constants
    static constexpr int CONNECTION_TIMEOUT_MS = 10000; // 10 seconds
};

} // namespace network
} // namespace RME

#endif // QT_LIVE_CLIENT_H