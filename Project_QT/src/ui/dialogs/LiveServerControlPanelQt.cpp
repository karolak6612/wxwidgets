#include "LiveServerControlPanelQt.h"
#include "core/settings/AppSettings.h"
#include <QDateTime>
#include <QScrollBar>
#include <QApplication>

// Forward declaration - will be properly integrated when NET-02 is available
// For now, we'll create a placeholder interface
class QtLiveServer : public QObject {
    Q_OBJECT
public:
    explicit QtLiveServer(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~QtLiveServer() = default;
    
    // Placeholder methods - will be replaced with actual implementation from NET-02
    virtual bool startListening(quint16 port, const QString& password) { 
        Q_UNUSED(port); Q_UNUSED(password); 
        return false; 
    }
    virtual void closeServer() {}
    virtual quint16 getCurrentPort() const { return 0; }
    virtual bool isRunning() const { return false; }
    virtual void broadcastChatMessageAsHost(const QString& message) { Q_UNUSED(message); }
    
signals:
    void logMessage(const QString& message);
    void clientConnected(const QString& clientName, quint32 clientId);
    void clientDisconnected(const QString& clientName, quint32 clientId);
    void chatMessageReceived(const QString& speaker, const QString& message);
    void serverStatusChanged(bool isRunning, quint16 actualPort);
};

namespace RME {
namespace ui {
namespace dialogs {

const QString LiveServerControlPanelQt::DEFAULT_PASSWORD = "";

LiveServerControlPanelQt::LiveServerControlPanelQt(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
    updateServerUIState(false);
}

bool LiveServerControlPanelQt::isServerRunning() const
{
    return m_serverRunning;
}

quint16 LiveServerControlPanelQt::getCurrentPort() const
{
    return m_currentPort;
}

QString LiveServerControlPanelQt::getCurrentPassword() const
{
    return m_currentPassword;
}

void LiveServerControlPanelQt::loadSettings()
{
    // Use consistent singleton access pattern
    m_settings = &RME::core::settings::AppSettings::getInstance();
    if (m_settings) {
        // Load server settings
        quint16 savedPort = m_settings->value("liveServer/port", DEFAULT_PORT).toUInt();
        QString savedPassword = m_settings->value("liveServer/password", DEFAULT_PASSWORD).toString();
        
        m_portSpinBox->setValue(savedPort);
        m_passwordEdit->setText(savedPassword);
    } else {
        // Fallback defaults
        m_portSpinBox->setValue(DEFAULT_PORT);
        m_passwordEdit->setText(DEFAULT_PASSWORD);
    }
}

void LiveServerControlPanelQt::saveSettings()
{
    if (m_settings) {
        m_settings->setValue("liveServer/port", m_portSpinBox->value());
        m_settings->setValue("liveServer/password", m_passwordEdit->text());
    }
}

void LiveServerControlPanelQt::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create main splitter
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_mainLayout->addWidget(m_splitter);
    
    // Top widget for configuration and controls
    QWidget* topWidget = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
    
    // Server Configuration Group
    m_configGroup = new QGroupBox("Server Configuration", this);
    m_configGroup->setObjectName("configGroup");
    m_configLayout = new QFormLayout(m_configGroup);
    
    // Port configuration
    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setObjectName("portSpinBox");
    m_portSpinBox->setMinimum(1);
    m_portSpinBox->setMaximum(65535);
    m_portSpinBox->setValue(DEFAULT_PORT);
    m_portSpinBox->setToolTip("Port number for the server to listen on");
    m_configLayout->addRow("Port:", m_portSpinBox);
    
    // Password configuration
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setObjectName("passwordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setToolTip("Password required for clients to connect (leave empty for no password)");
    m_configLayout->addRow("Password:", m_passwordEdit);
    
    // Server status
    m_serverStatusLabel = new QLabel("Server Status: Stopped", this);
    m_serverStatusLabel->setObjectName("serverStatusLabel");
    m_serverStatusLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_configLayout->addRow(m_serverStatusLabel);
    
    topLayout->addWidget(m_configGroup);
    
    // Server Controls Group
    m_controlsGroup = new QGroupBox("Server Controls", this);
    m_controlsGroup->setObjectName("controlsGroup");
    m_controlsLayout = new QHBoxLayout(m_controlsGroup);
    
    m_startServerButton = new QPushButton("Start Server", this);
    m_startServerButton->setObjectName("startServerButton");
    m_startServerButton->setToolTip("Start the live collaboration server");
    
    m_stopServerButton = new QPushButton("Stop Server", this);
    m_stopServerButton->setObjectName("stopServerButton");
    m_stopServerButton->setEnabled(false);
    m_stopServerButton->setToolTip("Stop the live collaboration server");
    
    m_controlsLayout->addWidget(m_startServerButton);
    m_controlsLayout->addWidget(m_stopServerButton);
    m_controlsLayout->addStretch();
    
    topLayout->addWidget(m_controlsGroup);
    
    // Connected Clients Group
    m_clientsGroup = new QGroupBox("Connected Clients", this);
    m_clientsGroup->setObjectName("clientsGroup");
    m_clientsLayout = new QVBoxLayout(m_clientsGroup);
    
    m_clientCountLabel = new QLabel("Clients: 0", this);
    m_clientCountLabel->setObjectName("clientCountLabel");
    m_clientsLayout->addWidget(m_clientCountLabel);
    
    m_clientListView = new QListView(this);
    m_clientListView->setObjectName("clientListView");
    m_clientListModel = new QStringListModel(this);
    m_clientListView->setModel(m_clientListModel);
    m_clientListView->setToolTip("List of currently connected clients");
    m_clientsLayout->addWidget(m_clientListView);
    
    topLayout->addWidget(m_clientsGroup);
    
    m_splitter->addWidget(topWidget);
    
    // Log & Chat Group (bottom part of splitter)
    m_logChatGroup = new QGroupBox("Server Log & Chat", this);
    m_logChatGroup->setObjectName("logChatGroup");
    m_logChatLayout = new QVBoxLayout(m_logChatGroup);
    
    // Log display
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setObjectName("logTextEdit");
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    m_logTextEdit->setToolTip("Server log messages and chat history");
    m_logChatLayout->addWidget(m_logTextEdit, 1); // Give it stretch factor
    
    // Chat input
    m_chatInputLayout = new QHBoxLayout();
    m_chatInputEdit = new QLineEdit(this);
    m_chatInputEdit->setObjectName("chatInputEdit");
    m_chatInputEdit->setPlaceholderText("Type a message to send to all clients...");
    m_chatInputEdit->setEnabled(false);
    m_chatInputEdit->setToolTip("Type a chat message to send to all connected clients");
    
    m_sendChatButton = new QPushButton("Send", this);
    m_sendChatButton->setObjectName("sendChatButton");
    m_sendChatButton->setEnabled(false);
    m_sendChatButton->setToolTip("Send the chat message to all clients");
    
    m_chatInputLayout->addWidget(m_chatInputEdit, 1);
    m_chatInputLayout->addWidget(m_sendChatButton);
    m_logChatLayout->addLayout(m_chatInputLayout);
    
    m_splitter->addWidget(m_logChatGroup);
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0); // Top widget doesn't stretch
    m_splitter->setStretchFactor(1, 1); // Log/chat area stretches
    
    // Initial log message
    appendLogMessage("Live Server Control Panel initialized.");
}

void LiveServerControlPanelQt::connectSignals()
{
    connect(m_startServerButton, &QPushButton::clicked, this, &LiveServerControlPanelQt::onStartServer);
    connect(m_stopServerButton, &QPushButton::clicked, this, &LiveServerControlPanelQt::onStopServer);
    connect(m_sendChatButton, &QPushButton::clicked, this, &LiveServerControlPanelQt::onSendChat);
    connect(m_chatInputEdit, &QLineEdit::returnPressed, this, &LiveServerControlPanelQt::onSendChat);
    
    // TODO: Connect to actual QtLiveServer when NET-02 is available
    // For now, we'll create a placeholder server instance
    m_liveServer = new QtLiveServer(this);
    
    // Connect server signals (these will work when NET-02 is implemented)
    connect(m_liveServer, &QtLiveServer::logMessage, this, &LiveServerControlPanelQt::onLogMessage);
    connect(m_liveServer, &QtLiveServer::clientConnected, this, &LiveServerControlPanelQt::onClientConnected);
    connect(m_liveServer, &QtLiveServer::clientDisconnected, this, &LiveServerControlPanelQt::onClientDisconnected);
    connect(m_liveServer, &QtLiveServer::chatMessageReceived, this, &LiveServerControlPanelQt::onChatMessageReceived);
    connect(m_liveServer, &QtLiveServer::serverStatusChanged, this, &LiveServerControlPanelQt::onServerStatusChanged);
}

void LiveServerControlPanelQt::updateServerUIState(bool isRunning)
{
    if (m_updatingUI) return;
    m_updatingUI = true;
    
    m_serverRunning = isRunning;
    
    // Update button states
    m_startServerButton->setEnabled(!isRunning);
    m_stopServerButton->setEnabled(isRunning);
    
    // Update configuration controls
    m_portSpinBox->setEnabled(!isRunning);
    m_passwordEdit->setEnabled(!isRunning);
    
    // Update chat controls
    m_chatInputEdit->setEnabled(isRunning);
    m_sendChatButton->setEnabled(isRunning);
    
    // Update status label
    if (isRunning) {
        m_serverStatusLabel->setText(QString("Server Status: Running on port %1").arg(m_currentPort));
        m_serverStatusLabel->setStyleSheet("QLabel { font-weight: bold; color: green; }");
    } else {
        m_serverStatusLabel->setText("Server Status: Stopped");
        m_serverStatusLabel->setStyleSheet("QLabel { font-weight: bold; color: red; }");
        clearClientList();
    }
    
    emit serverStateChanged(isRunning);
    m_updatingUI = false;
}

void LiveServerControlPanelQt::appendLogMessage(const QString& message)
{
    QString formattedMessage = formatLogMessage(message);
    m_logTextEdit->append(formattedMessage);
    
    // Limit log size
    if (m_logTextEdit->document()->lineCount() > MAX_LOG_LINES) {
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 
                           m_logTextEdit->document()->lineCount() - MAX_LOG_LINES);
        cursor.removeSelectedText();
    }
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LiveServerControlPanelQt::appendChatMessage(const QString& speaker, const QString& message)
{
    QString formattedMessage = formatChatMessage(speaker, message);
    m_logTextEdit->append(formattedMessage);
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LiveServerControlPanelQt::addClientToList(const QString& clientName, quint32 clientId)
{
    if (!m_connectedClients.contains(clientName)) {
        m_connectedClients.append(clientName);
        m_clientIdToName[clientId] = clientName;
        m_clientListModel->setStringList(m_connectedClients);
        
        m_clientCountLabel->setText(QString("Clients: %1").arg(m_connectedClients.size()));
        appendLogMessage(QString("Client connected: %1 (ID: %2)").arg(clientName).arg(clientId));
    }
}

void LiveServerControlPanelQt::removeClientFromList(const QString& clientName, quint32 clientId)
{
    m_connectedClients.removeAll(clientName);
    m_clientIdToName.remove(clientId);
    m_clientListModel->setStringList(m_connectedClients);
    
    m_clientCountLabel->setText(QString("Clients: %1").arg(m_connectedClients.size()));
    appendLogMessage(QString("Client disconnected: %1 (ID: %2)").arg(clientName).arg(clientId));
}

void LiveServerControlPanelQt::clearClientList()
{
    m_connectedClients.clear();
    m_clientIdToName.clear();
    m_clientListModel->setStringList(m_connectedClients);
    m_clientCountLabel->setText("Clients: 0");
}

QString LiveServerControlPanelQt::formatTimestamp() const
{
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

QString LiveServerControlPanelQt::formatLogMessage(const QString& message) const
{
    return QString("[%1] %2").arg(formatTimestamp(), message);
}

QString LiveServerControlPanelQt::formatChatMessage(const QString& speaker, const QString& message) const
{
    return QString("[%1] <%2> %3").arg(formatTimestamp(), speaker, message);
}

void LiveServerControlPanelQt::onStartServer()
{
    quint16 port = static_cast<quint16>(m_portSpinBox->value());
    QString password = m_passwordEdit->text();
    
    appendLogMessage(QString("Starting server on port %1...").arg(port));
    
    // Save settings
    saveSettings();
    
    // Emit signal for external handling (when NET-02 is integrated)
    emit serverStartRequested(port, password);
    
    // For now, simulate server start (will be replaced with actual server logic)
    bool success = m_liveServer->startListening(port, password);
    if (success) {
        m_currentPort = port;
        m_currentPassword = password;
        updateServerUIState(true);
        appendLogMessage(QString("Server started successfully on port %1").arg(port));
    } else {
        appendLogMessage("Failed to start server. Port may be in use.");
        QMessageBox::warning(this, "Server Start Failed", 
                             QString("Failed to start server on port %1. The port may already be in use.")
                             .arg(port));
    }
}

void LiveServerControlPanelQt::onStopServer()
{
    appendLogMessage("Stopping server...");
    
    // Emit signal for external handling
    emit serverStopRequested();
    
    // Stop the server
    if (m_liveServer) {
        m_liveServer->closeServer();
    }
    
    m_currentPort = 0;
    m_currentPassword.clear();
    updateServerUIState(false);
    appendLogMessage("Server stopped.");
}

void LiveServerControlPanelQt::onSendChat()
{
    QString message = m_chatInputEdit->text().trimmed();
    if (message.isEmpty() || !m_serverRunning) {
        return;
    }
    
    // Send via server
    if (m_liveServer) {
        m_liveServer->broadcastChatMessageAsHost(message);
    }
    
    // Display in our own log
    appendChatMessage("HOST", message);
    
    // Clear input
    m_chatInputEdit->clear();
    
    // Emit signal for external handling
    emit chatMessageSent(message);
}

void LiveServerControlPanelQt::onServerStatusChanged(bool isRunning, quint16 actualPort)
{
    m_currentPort = actualPort;
    updateServerUIState(isRunning);
}

void LiveServerControlPanelQt::onLogMessage(const QString& message)
{
    appendLogMessage(message);
}

void LiveServerControlPanelQt::onClientConnected(const QString& clientName, quint32 clientId)
{
    addClientToList(clientName, clientId);
}

void LiveServerControlPanelQt::onClientDisconnected(const QString& clientName, quint32 clientId)
{
    removeClientFromList(clientName, clientId);
}

void LiveServerControlPanelQt::onChatMessageReceived(const QString& speaker, const QString& message)
{
    appendChatMessage(speaker, message);
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// Include the moc file for the placeholder QtLiveServer
#include "LiveServerControlPanelQt.moc"