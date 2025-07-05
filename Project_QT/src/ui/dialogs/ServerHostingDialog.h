#ifndef RME_SERVER_HOSTING_DIALOG_H
#define RME_SERVER_HOSTING_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QProgressBar>
#include <QListWidget>

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for hosting a live collaboration server
 * 
 * This dialog allows users to configure and start a live collaboration
 * server for real-time map editing with other users.
 */
class ServerHostingDialog : public QDialog {
    Q_OBJECT

public:
    struct ServerSettings {
        QString serverName;
        quint16 port;
        QString password;
        int maxClients;
        bool requirePassword;
        bool allowGuests;
        QString welcomeMessage;
    };

    explicit ServerHostingDialog(QWidget* parent = nullptr);
    ~ServerHostingDialog() override = default;

    ServerSettings getServerSettings() const;
    void setServerSettings(const ServerSettings& settings);

public slots:
    void onStartServer();
    void onStopServer();
    void onServerStarted();
    void onServerStopped();
    void onServerError(const QString& error);
    void onClientConnected(const QString& clientName);
    void onClientDisconnected(const QString& clientName);

signals:
    void startServerRequested(const ServerSettings& settings);
    void stopServerRequested();

private slots:
    void onRequirePasswordToggled(bool enabled);
    void updateUI();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    bool validateSettings();
    
    // UI components
    QVBoxLayout* m_mainLayout;
    
    // Server settings group
    QGroupBox* m_settingsGroup;
    QFormLayout* m_settingsLayout;
    QLineEdit* m_serverNameEdit;
    QSpinBox* m_portSpin;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_requirePasswordCheck;
    QSpinBox* m_maxClientsSpin;
    QCheckBox* m_allowGuestsCheck;
    QTextEdit* m_welcomeMessageEdit;
    
    // Server status group
    QGroupBox* m_statusGroup;
    QVBoxLayout* m_statusLayout;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // Connected clients group
    QGroupBox* m_clientsGroup;
    QVBoxLayout* m_clientsLayout;
    QListWidget* m_clientsList;
    QLabel* m_clientCountLabel;
    
    // Button layout
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_closeButton;
    
    // State
    bool m_serverRunning;
    ServerSettings m_currentSettings;
    
    // Constants
    static constexpr quint16 DEFAULT_PORT = 7171;
    static constexpr int DEFAULT_MAX_CLIENTS = 10;
    static const QString SETTINGS_GROUP;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_SERVER_HOSTING_DIALOG_H