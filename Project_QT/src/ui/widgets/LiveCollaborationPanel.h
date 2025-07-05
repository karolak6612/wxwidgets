#ifndef LIVE_COLLABORATION_PANEL_H
#define LIVE_COLLABORATION_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QTimer>
#include <QMap>

#include "core/Position.h"
#include "core/network/live_packets.h"

// Forward declarations
namespace RME {
namespace network {
    class QtLiveClient;
}
namespace core {
    class Map;
    namespace actions { class UndoManager; }
    namespace assets { class AssetManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace ui {
namespace widgets {

/**
 * @brief Panel for live collaboration features
 * 
 * This widget provides the user interface for live collaboration including
 * connection status, peer list, chat, and collaboration controls.
 */
class LiveCollaborationPanel : public QWidget {
    Q_OBJECT

public:
    explicit LiveCollaborationPanel(QWidget* parent = nullptr);
    ~LiveCollaborationPanel() override = default;

    // Integration
    void setLiveClient(RME::network::QtLiveClient* client);
    void setMapContext(RME::core::Map* map, 
                      RME::core::actions::UndoManager* undoManager,
                      RME::core::assets::AssetManager* assetManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);

public slots:
    void onConnectToServer();
    void onDisconnectFromServer();
    void onSendChatMessage();
    void onClearChat();
    
    // Client event handlers
    void onConnectionStateChanged();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(const QString& error);
    void onMapChangesReceived(const QList<RME::core::network::TileChange>& changes);
    void onPeerCursorUpdated(uint32_t peerId, const RME::core::Position& position, 
                           const RME::core::network::NetworkColor& color);
    void onChatMessageReceived(uint32_t peerId, const QString& senderName, const QString& message);
    void onPeerJoined(uint32_t peerId, const QString& peerName, const RME::core::network::NetworkColor& color);
    void onPeerLeft(uint32_t peerId, const QString& peerName);
    void onServerKicked(const QString& reason);

signals:
    void connectionRequested();
    void disconnectionRequested();
    void chatMessageSent(const QString& message);

protected:
    void setupUI();
    void setupConnections();
    void updateConnectionStatus();
    void updatePeerList();
    void addChatMessage(const QString& sender, const QString& message, 
                       const QColor& color = QColor());
    void addSystemMessage(const QString& message);

private slots:
    void onChatInputReturnPressed();
    void onPeerListItemDoubleClicked(QListWidgetItem* item);
    void onUpdateTimer();

private:
    // UI components
    QVBoxLayout* m_mainLayout;
    
    // Connection group
    QGroupBox* m_connectionGroup;
    QVBoxLayout* m_connectionLayout;
    QLabel* m_statusLabel;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    
    // Peers group
    QGroupBox* m_peersGroup;
    QVBoxLayout* m_peersLayout;
    QListWidget* m_peersList;
    QLabel* m_peersCountLabel;
    
    // Chat group
    QGroupBox* m_chatGroup;
    QVBoxLayout* m_chatLayout;
    QTextEdit* m_chatDisplay;
    QHBoxLayout* m_chatInputLayout;
    QLineEdit* m_chatInput;
    QPushButton* m_sendButton;
    QPushButton* m_clearChatButton;
    
    // Splitter for resizable sections
    QSplitter* m_splitter;
    
    // Integration
    RME::network::QtLiveClient* m_liveClient;
    RME::core::Map* m_mapRef;
    RME::core::actions::UndoManager* m_undoManagerRef;
    RME::core::assets::AssetManager* m_assetManagerRef;
    RME::core::editor::EditorControllerInterface* m_editorController;
    
    // State tracking
    QMap<uint32_t, QString> m_peerNames;
    QMap<uint32_t, RME::core::network::NetworkColor> m_peerColors;
    QMap<uint32_t, RME::core::Position> m_peerCursors;
    
    // UI update timer
    QTimer* m_updateTimer;
    
    // Constants
    static constexpr int UPDATE_INTERVAL_MS = 1000; // 1 second
    static constexpr int MAX_CHAT_LINES = 1000;
};

} // namespace widgets
} // namespace ui
} // namespace RME

#endif // LIVE_COLLABORATION_PANEL_H