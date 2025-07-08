#include "LiveCollaborationPanel.h"
#include "ui/dialogs/LiveConnectionDialog.h"
#include "network/QtLiveClient.h"
#include "core/Map.h"
#include "core/actions/UndoManager.h"
#include "core/assets/AssetManager.h"
#include "core/editor/EditorControllerInterface.h"

#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QListWidgetItem>
#include <QApplication>

namespace RME {
namespace ui {
namespace widgets {

LiveCollaborationPanel::LiveCollaborationPanel(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_liveClient(nullptr)
    , m_mapRef(nullptr)
    , m_undoManagerRef(nullptr)
    , m_assetManagerRef(nullptr)
    , m_editorController(nullptr)
    , m_updateTimer(new QTimer(this))
{
    setupUI();
    setupConnections();
    updateConnectionStatus();
    
    // Setup update timer
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(m_updateTimer, &QTimer::timeout, this, &LiveCollaborationPanel::onUpdateTimer);
    m_updateTimer->start();
}

void LiveCollaborationPanel::setLiveClient(RME::network::QtLiveClient* client)
{
    if (m_liveClient) {
        // Disconnect old client
        disconnect(m_liveClient, nullptr, this, nullptr);
    }
    
    m_liveClient = client;
    
    if (m_liveClient) {
        // Connect to client signals
        connect(m_liveClient, &RME::network::QtLiveClient::connectionStateChanged,
                this, &LiveCollaborationPanel::onConnectionStateChanged);
        connect(m_liveClient, &RME::network::QtLiveClient::connected,
                this, &LiveCollaborationPanel::onConnected);
        connect(m_liveClient, &RME::network::QtLiveClient::disconnected,
                this, &LiveCollaborationPanel::onDisconnected);
        connect(m_liveClient, &RME::network::QtLiveClient::errorOccurred,
                this, &LiveCollaborationPanel::onErrorOccurred);
        connect(m_liveClient, &RME::network::QtLiveClient::mapChangesReceived,
                this, &LiveCollaborationPanel::onMapChangesReceived);
        connect(m_liveClient, &RME::network::QtLiveClient::peerCursorUpdated,
                this, &LiveCollaborationPanel::onPeerCursorUpdated);
        connect(m_liveClient, &RME::network::QtLiveClient::chatMessageReceived,
                this, &LiveCollaborationPanel::onChatMessageReceived);
        connect(m_liveClient, &RME::network::QtLiveClient::peerJoined,
                this, &LiveCollaborationPanel::onPeerJoined);
        connect(m_liveClient, &RME::network::QtLiveClient::peerLeft,
                this, &LiveCollaborationPanel::onPeerLeft);
        connect(m_liveClient, &RME::network::QtLiveClient::serverKicked,
                this, &LiveCollaborationPanel::onServerKicked);
    }
    
    updateConnectionStatus();
}

void LiveCollaborationPanel::setMapContext(RME::core::Map* map, 
                                          RME::core::actions::UndoManager* undoManager,
                                          RME::core::assets::AssetManager* assetManager)
{
    m_mapRef = map;
    m_undoManagerRef = undoManager;
    m_assetManagerRef = assetManager;
    
    if (m_liveClient) {
        m_liveClient->setMapContext(map, undoManager, assetManager);
    }
}

void LiveCollaborationPanel::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
    
    if (m_liveClient) {
        m_liveClient->setEditorController(controller);
    }
}

void LiveCollaborationPanel::onConnectToServer()
{
    if (!m_liveClient) {
        QMessageBox::warning(this, tr("Error"), tr("Live client not initialized"));
        return;
    }
    
    if (m_liveClient->isConnected()) {
        QMessageBox::information(this, tr("Already Connected"), 
                                tr("Already connected to a live server"));
        return;
    }
    
    // Show connection dialog
    dialogs::LiveConnectionDialog dialog(this);
    
    connect(&dialog, &dialogs::LiveConnectionDialog::connectRequested,
            [this](const dialogs::LiveConnectionDialog::ConnectionSettings& settings) {
                if (m_liveClient) {
                    m_liveClient->connectToServer(settings.hostname, settings.port, 
                                                 settings.username, settings.password);
                }
            });
    
    connect(&dialog, &dialogs::LiveConnectionDialog::cancelRequested,
            [this]() {
                if (m_liveClient) {
                    m_liveClient->disconnectFromServer();
                }
            });
    
    // Connect client state changes to dialog
    if (m_liveClient) {
        connect(m_liveClient, &RME::network::QtLiveClient::connectionStateChanged,
                &dialog, [&dialog, this]() {
                    auto state = m_liveClient->getConnectionState();
                    switch (state) {
                        case RME::network::QtLiveClient::ConnectionState::Connecting:
                            dialog.setConnectionProgress(tr("Connecting to server..."));
                            break;
                        case RME::network::QtLiveClient::ConnectionState::Authenticating:
                            dialog.setConnectionProgress(tr("Authenticating..."));
                            break;
                        case RME::network::QtLiveClient::ConnectionState::Connected:
                            dialog.setConnectionProgress(tr("Connected successfully!"));
                            QTimer::singleShot(1000, &dialog, &QDialog::accept);
                            break;
                        case RME::network::QtLiveClient::ConnectionState::Error:
                            dialog.setConnectionError(m_liveClient->getLastError());
                            break;
                        default:
                            break;
                    }
                });
    }
    
    dialog.exec();
}

void LiveCollaborationPanel::onDisconnectFromServer()
{
    if (m_liveClient && m_liveClient->isConnected()) {
        m_liveClient->disconnectFromServer();
        addSystemMessage(tr("Disconnected from server"));
    }
}

void LiveCollaborationPanel::onSendChatMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty() || !m_liveClient || !m_liveClient->isConnected()) {
        return;
    }
    
    m_liveClient->sendChatMessage(message);
    m_chatInput->clear();
    
    // Add to local chat display
    addChatMessage(tr("You"), message, QColor(0, 120, 0)); // Green for own messages
}

void LiveCollaborationPanel::onClearChat()
{
    m_chatDisplay->clear();
    addSystemMessage(tr("Chat cleared"));
}

void LiveCollaborationPanel::onConnectionStateChanged()
{
    updateConnectionStatus();
}

void LiveCollaborationPanel::onConnected()
{
    addSystemMessage(tr("Connected to live server"));
    updateConnectionStatus();
    updatePeerList();
}

void LiveCollaborationPanel::onDisconnected()
{
    addSystemMessage(tr("Disconnected from live server"));
    updateConnectionStatus();
    
    // Clear peer information
    m_peerNames.clear();
    m_peerColors.clear();
    m_peerCursors.clear();
    updatePeerList();
}

void LiveCollaborationPanel::onErrorOccurred(const QString& error)
{
    addSystemMessage(tr("Error: %1").arg(error));
    QMessageBox::warning(this, tr("Connection Error"), error);
}

void LiveCollaborationPanel::onMapChangesReceived(const QList<RME::core::network::TileChange>& changes)
{
    addSystemMessage(tr("Received %1 map changes from server").arg(changes.size()));
    
    // TODO: Apply changes to map through editor controller
    if (m_editorController) {
        // m_editorController->applyRemoteChanges(changes);
    }
}

void LiveCollaborationPanel::onPeerCursorUpdated(uint32_t peerId, const RME::core::Position& position, 
                                                const RME::core::network::NetworkColor& color)
{
    m_peerCursors[peerId] = position;
    m_peerColors[peerId] = color;
    
    // TODO: Update cursor display in map view
}

void LiveCollaborationPanel::onChatMessageReceived(uint32_t peerId, const QString& senderName, const QString& message)
{
    QColor senderColor = QColor(100, 100, 200); // Default blue
    
    auto colorIt = m_peerColors.find(peerId);
    if (colorIt != m_peerColors.end()) {
        const auto& networkColor = colorIt.value();
        senderColor = QColor(networkColor.r, networkColor.g, networkColor.b, networkColor.a);
    }
    
    addChatMessage(senderName, message, senderColor);
}

void LiveCollaborationPanel::onPeerJoined(uint32_t peerId, const QString& peerName, const RME::core::network::NetworkColor& color)
{
    m_peerNames[peerId] = peerName;
    m_peerColors[peerId] = color;
    
    addSystemMessage(tr("%1 joined the session").arg(peerName));
    updatePeerList();
}

void LiveCollaborationPanel::onPeerLeft(uint32_t peerId, const QString& peerName)
{
    m_peerNames.remove(peerId);
    m_peerColors.remove(peerId);
    m_peerCursors.remove(peerId);
    
    addSystemMessage(tr("%1 left the session").arg(peerName));
    updatePeerList();
}

void LiveCollaborationPanel::onServerKicked(const QString& reason)
{
    addSystemMessage(tr("Kicked from server: %1").arg(reason));
    QMessageBox::warning(this, tr("Kicked from Server"), 
                        tr("You have been kicked from the server:\n%1").arg(reason));
}

void LiveCollaborationPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_mainLayout->addWidget(m_splitter);
    
    // Connection group
    m_connectionGroup = new QGroupBox(tr("Connection"));
    m_connectionLayout = new QVBoxLayout(m_connectionGroup);
    
    m_statusLabel = new QLabel(tr("Not connected"));
    m_statusLabel->setWordWrap(true);
    m_connectionLayout->addWidget(m_statusLabel);
    
    QHBoxLayout* connectionButtonLayout = new QHBoxLayout();
    m_connectButton = new QPushButton(tr("Connect"));
    m_disconnectButton = new QPushButton(tr("Disconnect"));
    connectionButtonLayout->addWidget(m_connectButton);
    connectionButtonLayout->addWidget(m_disconnectButton);
    connectionButtonLayout->addStretch();
    m_connectionLayout->addLayout(connectionButtonLayout);
    
    m_splitter->addWidget(m_connectionGroup);
    
    // Peers group
    m_peersGroup = new QGroupBox(tr("Connected Peers"));
    m_peersLayout = new QVBoxLayout(m_peersGroup);
    
    m_peersCountLabel = new QLabel(tr("0 peers connected"));
    m_peersLayout->addWidget(m_peersCountLabel);
    
    m_peersList = new QListWidget();
    m_peersList->setMaximumHeight(150);
    m_peersLayout->addWidget(m_peersList);
    
    m_splitter->addWidget(m_peersGroup);
    
    // Chat group
    m_chatGroup = new QGroupBox(tr("Chat"));
    m_chatLayout = new QVBoxLayout(m_chatGroup);
    
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setMaximumBlockCount(MAX_CHAT_LINES);
    m_chatLayout->addWidget(m_chatDisplay);
    
    m_chatInputLayout = new QHBoxLayout();
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText(tr("Type a message..."));
    m_sendButton = new QPushButton(tr("Send"));
    m_clearChatButton = new QPushButton(tr("Clear"));
    
    m_chatInputLayout->addWidget(m_chatInput);
    m_chatInputLayout->addWidget(m_sendButton);
    m_chatInputLayout->addWidget(m_clearChatButton);
    m_chatLayout->addLayout(m_chatInputLayout);
    
    m_splitter->addWidget(m_chatGroup);
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0); // Connection - fixed
    m_splitter->setStretchFactor(1, 0); // Peers - fixed
    m_splitter->setStretchFactor(2, 1); // Chat - expandable
}

void LiveCollaborationPanel::setupConnections()
{
    connect(m_connectButton, &QPushButton::clicked, this, &LiveCollaborationPanel::onConnectToServer);
    connect(m_disconnectButton, &QPushButton::clicked, this, &LiveCollaborationPanel::onDisconnectFromServer);
    connect(m_sendButton, &QPushButton::clicked, this, &LiveCollaborationPanel::onSendChatMessage);
    connect(m_clearChatButton, &QPushButton::clicked, this, &LiveCollaborationPanel::onClearChat);
    
    connect(m_chatInput, &QLineEdit::returnPressed, this, &LiveCollaborationPanel::onChatInputReturnPressed);
    connect(m_peersList, &QListWidget::itemDoubleClicked, this, &LiveCollaborationPanel::onPeerListItemDoubleClicked);
}

void LiveCollaborationPanel::updateConnectionStatus()
{
    bool connected = m_liveClient && m_liveClient->isConnected();
    
    m_connectButton->setEnabled(!connected);
    m_disconnectButton->setEnabled(connected);
    m_chatInput->setEnabled(connected);
    m_sendButton->setEnabled(connected);
    
    if (!m_liveClient) {
        m_statusLabel->setText(tr("Live client not initialized"));
        m_statusLabel->setStyleSheet("QLabel { color: red; }");
    } else {
        auto state = m_liveClient->getConnectionState();
        switch (state) {
            case RME::network::QtLiveClient::ConnectionState::Disconnected:
                m_statusLabel->setText(tr("Not connected"));
                m_statusLabel->setStyleSheet("QLabel { color: gray; }");
                break;
            case RME::network::QtLiveClient::ConnectionState::Connecting:
                m_statusLabel->setText(tr("Connecting..."));
                m_statusLabel->setStyleSheet("QLabel { color: orange; }");
                break;
            case RME::network::QtLiveClient::ConnectionState::Authenticating:
                m_statusLabel->setText(tr("Authenticating..."));
                m_statusLabel->setStyleSheet("QLabel { color: orange; }");
                break;
            case RME::network::QtLiveClient::ConnectionState::Connected:
                m_statusLabel->setText(tr("Connected as %1 (ID: %2)")
                                     .arg(m_liveClient->getClientName())
                                     .arg(m_liveClient->getClientId()));
                m_statusLabel->setStyleSheet("QLabel { color: green; }");
                break;
            case RME::network::QtLiveClient::ConnectionState::Error:
                m_statusLabel->setText(tr("Error: %1").arg(m_liveClient->getLastError()));
                m_statusLabel->setStyleSheet("QLabel { color: red; }");
                break;
        }
    }
}

void LiveCollaborationPanel::updatePeerList()
{
    m_peersList->clear();
    
    int peerCount = m_peerNames.size();
    m_peersCountLabel->setText(tr("%1 peer(s) connected").arg(peerCount));
    
    for (auto it = m_peerNames.constBegin(); it != m_peerNames.constEnd(); ++it) {
        uint32_t peerId = it.key();
        QString peerName = it.value();
        
        QListWidgetItem* item = new QListWidgetItem(peerName);
        item->setData(Qt::UserRole, peerId);
        
        // Set color indicator
        auto colorIt = m_peerColors.find(peerId);
        if (colorIt != m_peerColors.end()) {
            const auto& networkColor = colorIt.value();
            QColor color(networkColor.r, networkColor.g, networkColor.b, networkColor.a);
            item->setBackground(QBrush(color.lighter(180)));
        }
        
        m_peersList->addItem(item);
    }
}

void LiveCollaborationPanel::addChatMessage(const QString& sender, const QString& message, const QColor& color)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] <span style='color: %2; font-weight: bold;'>%3:</span> %4")
                              .arg(timestamp)
                              .arg(color.name())
                              .arg(sender.toHtmlEscaped())
                              .arg(message.toHtmlEscaped());
    
    m_chatDisplay->append(formattedMessage);
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LiveCollaborationPanel::addSystemMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] <span style='color: gray; font-style: italic;'>* %2</span>")
                              .arg(timestamp)
                              .arg(message.toHtmlEscaped());
    
    m_chatDisplay->append(formattedMessage);
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LiveCollaborationPanel::onChatInputReturnPressed()
{
    onSendChatMessage();
}

void LiveCollaborationPanel::onPeerListItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    uint32_t peerId = item->data(Qt::UserRole).toUInt();
    auto cursorIt = m_peerCursors.find(peerId);
    
    if (cursorIt != m_peerCursors.end()) {
        // TODO: Focus map view on peer's cursor position
        const RME::core::Position& position = cursorIt.value();
        addSystemMessage(tr("Peer %1 is at position (%2, %3, %4)")
                        .arg(item->text())
                        .arg(position.x)
                        .arg(position.y)
                        .arg(position.z));
    }
}

void LiveCollaborationPanel::onUpdateTimer()
{
    // Periodic updates if needed
    if (m_liveClient && m_liveClient->isConnected()) {
        // Update peer list in case of changes
        auto currentPeers = m_liveClient->getConnectedPeers();
        if (currentPeers != m_peerNames) {
            m_peerNames = currentPeers;
            m_peerColors = m_liveClient->getPeerColors();
            updatePeerList();
        }
    }
}

} // namespace widgets
} // namespace ui
} // namespace RME

// #include "LiveCollaborationPanel.moc" // Removed - Q_OBJECT is in header