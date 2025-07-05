#ifndef QTLIVESERVER_H
#define QTLIVESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>
#include <QString>
#include <cstdint>

// Core RME library includes
#include "core/map/Map.h"
#include "core/assets/AssetManager.h"
#include "core/actions/UndoManager.h" // For QUndoStack wrapper
#include "core/network/NetworkMessage.h" // For broadcast method signature
#include "core/settings/AppSettings.h" // For map loading

// Forward declaration
class QtLivePeer;
namespace RME { namespace core { namespace io { class OtbmMapIO; }}} // For map loading

/**
 * @brief Manages the TCP server for live collaboration, client connections,
 * and the authoritative map instance.
 */
class QtLiveServer : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs the QtLiveServer.
     * @param parent Optional QObject parent.
     */
    explicit QtLiveServer(QObject *parent = nullptr);

    /**
     * @brief Destroys the QtLiveServer, cleaning up server and peers.
     */
    ~QtLiveServer() override;

    // Prevent copying
    QtLiveServer(const QtLiveServer&) = delete;
    QtLiveServer& operator=(const QtLiveServer&) = delete;

    /**
     * @brief Starts the live server.
     * This involves loading assets, loading the specified map, and beginning
     * to listen for incoming TCP connections on the given port.
     * @param port Port for the server to listen on.
     * @param mapFilePath Path to the .otbm map file to load as the authoritative version.
     * @param serverPassword Optional password for clients to connect. If empty, no password is required.
     * @param dataPackPath Path to the game data assets (e.g., containing Tibia.dat/spr, items.otb).
     * @param serverClientVersion Client version string (e.g., "10.98") the server will use for its assets.
     * @return True if the server started successfully and is listening, false otherwise.
     */
    bool startServer(quint16 port,
                     const QString& mapFilePath,
                     const QString& serverPassword,
                     const QString& dataPackPath,
                     const QString& serverClientVersion);

    /**
     * @brief Broadcasts a network message to all currently connected peers.
     * An optional peer can be excluded from this broadcast (typically the sender of a message).
     * @param msg The NetworkMessage to broadcast.
     * @param excludePeer Peer to exclude from the broadcast, or nullptr to send to all.
     */
    void broadcastMessageToAll(const RME::core::NetworkMessage& msg, QtLivePeer* excludePeer = nullptr);

    /** @brief Gets a pointer to the authoritative Map instance. */
    RME::core::Map* getMap() { return &m_mapInstance; }
    /** @brief Gets a pointer to the UndoManager for map actions. */
    RME::core::actions::UndoManager* getUndoManager() { return &m_undoManager; }
    /** @brief Gets a pointer to the AssetManager for game assets. */
    RME::core::assets::AssetManager* getAssetManager() { return &m_assetManager; }
    /** @brief Gets the server connection password. */
    QString getPassword() const { return m_serverPassword; }
    /** @brief Gets the name of the server. */
    QString getServerName() const { return m_serverName; }

private slots:
    /**
     * @brief Slot called when a new client attempts to connect to the TCP server.
     * Creates a new QtLivePeer to handle the connection.
     */
    void onNewConnection();

    /**
     * @brief Slot called when a QtLivePeer signals that it has disconnected.
     * Removes the peer from the list of active peers and schedules it for deletion.
     * @param peer Pointer to the QtLivePeer that disconnected.
     */
    void onPeerDisconnected(QtLivePeer* peer);

private:
    QTcpServer* m_tcpServer = nullptr;      ///< The underlying Qt TCP server.
    QList<QtLivePeer*> m_peers;             ///< List of currently connected client peers.

    RME::core::assets::AssetManager m_assetManager; ///< Owns and manages all game assets (items, creatures, sprites, materials).
    RME::core::settings::AppSettings m_appSettings; ///< Owns application/server settings, used by map I/O.
    RME::core::Map m_mapInstance;                   ///< The authoritative instance of the map being edited. Initialized with AssetManager.
    RME::core::actions::UndoManager m_undoManager;  ///< Manages the undo/redo stack for map operations. Initialized with Map instance.

    QString m_serverPassword;       ///< Password required for clients to connect (if any).
    QString m_serverName;           ///< Name of this live server.
    uint32_t m_nextPeerId = 1;      ///< Counter for assigning unique IDs to peers.
};

#endif // QTLIVESERVER_H
