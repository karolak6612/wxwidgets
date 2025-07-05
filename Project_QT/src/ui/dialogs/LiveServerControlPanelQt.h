#ifndef RME_LIVE_SERVER_CONTROL_PANEL_QT_H
#define RME_LIVE_SERVER_CONTROL_PANEL_QT_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QTextEdit>
#include <QStringListModel>
#include <QLabel>
#include <QSplitter>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>

// Forward declarations
namespace RME {
namespace core {
namespace settings { class AppSettings; }
}
}

// Forward declaration for the live server (from NET-02)
class QtLiveServer;

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Control panel for managing a live editing server
 * 
 * Provides UI for configuring, starting, and stopping a live server,
 * monitoring connected clients, viewing server logs and chat messages,
 * and sending chat messages as the host.
 */
class LiveServerControlPanelQt : public QWidget {
    Q_OBJECT

public:
    explicit LiveServerControlPanelQt(QWidget* parent = nullptr);
    ~LiveServerControlPanelQt() override = default;

    // Server state
    bool isServerRunning() const;
    quint16 getCurrentPort() const;
    QString getCurrentPassword() const;

    // Settings management
    void loadSettings();
    void saveSettings();

public slots:
    void onStartServer();
    void onStopServer();
    void onSendChat();
    void onServerStatusChanged(bool isRunning, quint16 actualPort);
    void onLogMessage(const QString& message);
    void onClientConnected(const QString& clientName, quint32 clientId);
    void onClientDisconnected(const QString& clientName, quint32 clientId);
    void onChatMessageReceived(const QString& speaker, const QString& message);

signals:
    void serverStartRequested(quint16 port, const QString& password);
    void serverStopRequested();
    void chatMessageSent(const QString& message);
    void serverStateChanged(bool isRunning);

private:
    void setupUI();
    void connectSignals();
    void updateServerUIState(bool isRunning);
    void appendLogMessage(const QString& message);
    void appendChatMessage(const QString& speaker, const QString& message);
    void addClientToList(const QString& clientName, quint32 clientId);
    void removeClientFromList(const QString& clientName, quint32 clientId);
    void clearClientList();
    
    // Helper methods
    QString formatTimestamp() const;
    QString formatLogMessage(const QString& message) const;
    QString formatChatMessage(const QString& speaker, const QString& message) const;

    // UI components - Server Configuration
    QGroupBox* m_configGroup = nullptr;
    QFormLayout* m_configLayout = nullptr;
    QSpinBox* m_portSpinBox = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLabel* m_serverStatusLabel = nullptr;

    // UI components - Server Controls
    QGroupBox* m_controlsGroup = nullptr;
    QHBoxLayout* m_controlsLayout = nullptr;
    QPushButton* m_startServerButton = nullptr;
    QPushButton* m_stopServerButton = nullptr;

    // UI components - Connected Clients
    QGroupBox* m_clientsGroup = nullptr;
    QVBoxLayout* m_clientsLayout = nullptr;
    QListView* m_clientListView = nullptr;
    QStringListModel* m_clientListModel = nullptr;
    QLabel* m_clientCountLabel = nullptr;

    // UI components - Log & Chat
    QGroupBox* m_logChatGroup = nullptr;
    QVBoxLayout* m_logChatLayout = nullptr;
    QTextEdit* m_logTextEdit = nullptr;
    QHBoxLayout* m_chatInputLayout = nullptr;
    QLineEdit* m_chatInputEdit = nullptr;
    QPushButton* m_sendChatButton = nullptr;

    // Main layout
    QVBoxLayout* m_mainLayout = nullptr;
    QSplitter* m_splitter = nullptr;

    // Data and state
    QtLiveServer* m_liveServer = nullptr;
    RME::core::settings::AppSettings* m_settings = nullptr;
    QStringList m_connectedClients;
    QHash<quint32, QString> m_clientIdToName;
    
    // Server state
    bool m_serverRunning = false;
    quint16 m_currentPort = 0;
    QString m_currentPassword;
    
    // UI state
    bool m_updatingUI = false;
    
    // Constants
    static const quint16 DEFAULT_PORT = 31313;
    static const QString DEFAULT_PASSWORD;
    static const int MAX_LOG_LINES = 1000;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_LIVE_SERVER_CONTROL_PANEL_QT_H