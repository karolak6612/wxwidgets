#ifndef LIVE_CONNECTION_DIALOG_H
#define LIVE_CONNECTION_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for connecting to a live collaboration server
 * 
 * This dialog allows users to enter server connection details and
 * initiate a connection to a live collaboration server.
 */
class LiveConnectionDialog : public QDialog {
    Q_OBJECT

public:
    struct ConnectionSettings {
        QString hostname;
        quint16 port;
        QString username;
        QString password;
        bool rememberSettings;
    };

    explicit LiveConnectionDialog(QWidget* parent = nullptr);
    ~LiveConnectionDialog() override = default;

    // Connection settings
    ConnectionSettings getConnectionSettings() const;
    void setConnectionSettings(const ConnectionSettings& settings);
    
    // Connection state
    void setConnecting(bool connecting);
    void setConnectionProgress(const QString& status);
    void setConnectionError(const QString& error);

public slots:
    void onConnectClicked();
    void onCancelClicked();
    void onConnectionStateChanged();

signals:
    void connectRequested(const ConnectionSettings& settings);
    void cancelRequested();

protected:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    bool validateInput();
    void updateUI();

private slots:
    void onInputChanged();
    void onShowPasswordToggled(bool show);

private:
    // UI components
    QVBoxLayout* m_mainLayout;
    
    // Server settings group
    QGroupBox* m_serverGroup;
    QFormLayout* m_serverLayout;
    QLineEdit* m_hostnameEdit;
    QSpinBox* m_portSpinBox;
    
    // User settings group
    QGroupBox* m_userGroup;
    QFormLayout* m_userLayout;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_showPasswordCheck;
    QCheckBox* m_rememberSettingsCheck;
    
    // Recent connections
    QGroupBox* m_recentGroup;
    QVBoxLayout* m_recentLayout;
    QComboBox* m_recentCombo;
    QPushButton* m_loadRecentButton;
    QPushButton* m_deleteRecentButton;
    
    // Connection status
    QGroupBox* m_statusGroup;
    QVBoxLayout* m_statusLayout;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // Buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_connectButton;
    QPushButton* m_cancelButton;
    
    // State
    bool m_isConnecting;
    ConnectionSettings m_currentSettings;
    
    // Constants
    static constexpr quint16 DEFAULT_PORT = 7171;
    static const QString SETTINGS_GROUP;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // LIVE_CONNECTION_DIALOG_H