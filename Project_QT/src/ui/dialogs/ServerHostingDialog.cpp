#include "ServerHostingDialog.h"
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QHostAddress>
#include <QNetworkInterface>

namespace RME {
namespace ui {
namespace dialogs {

const QString ServerHostingDialog::SETTINGS_GROUP = "ServerHosting";

ServerHostingDialog::ServerHostingDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_serverRunning(false)
{
    setWindowTitle(tr("Host Live Collaboration Server"));
    setModal(true);
    setMinimumSize(500, 600);
    
    setupUI();
    loadSettings();
    updateUI();
}

void ServerHostingDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Server settings group
    m_settingsGroup = new QGroupBox(tr("Server Settings"));
    m_settingsLayout = new QFormLayout(m_settingsGroup);
    
    m_serverNameEdit = new QLineEdit();
    m_serverNameEdit->setPlaceholderText(tr("My RME Server"));
    m_settingsLayout->addRow(tr("Server name:"), m_serverNameEdit);
    
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1024, 65535);
    m_portSpin->setValue(DEFAULT_PORT);
    m_settingsLayout->addRow(tr("Port:"), m_portSpin);
    
    m_requirePasswordCheck = new QCheckBox(tr("Require password"));
    m_settingsLayout->addRow(m_requirePasswordCheck);
    
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Server password"));
    m_settingsLayout->addRow(tr("Password:"), m_passwordEdit);
    
    m_maxClientsSpin = new QSpinBox();
    m_maxClientsSpin->setRange(1, 50);
    m_maxClientsSpin->setValue(DEFAULT_MAX_CLIENTS);
    m_settingsLayout->addRow(tr("Max clients:"), m_maxClientsSpin);
    
    m_allowGuestsCheck = new QCheckBox(tr("Allow guest connections"));
    m_allowGuestsCheck->setChecked(true);
    m_settingsLayout->addRow(m_allowGuestsCheck);
    
    m_welcomeMessageEdit = new QTextEdit();
    m_welcomeMessageEdit->setMaximumHeight(80);
    m_welcomeMessageEdit->setPlaceholderText(tr("Welcome to my server! Please be respectful."));
    m_settingsLayout->addRow(tr("Welcome message:"), m_welcomeMessageEdit);
    
    m_mainLayout->addWidget(m_settingsGroup);
    
    // Server status group
    m_statusGroup = new QGroupBox(tr("Server Status"));
    m_statusLayout = new QVBoxLayout(m_statusGroup);
    
    m_statusLabel = new QLabel(tr("Server stopped"));
    m_statusLabel->setWordWrap(true);
    m_statusLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_statusLayout->addWidget(m_progressBar);
    
    m_mainLayout->addWidget(m_statusGroup);
    
    // Connected clients group
    m_clientsGroup = new QGroupBox(tr("Connected Clients"));
    m_clientsLayout = new QVBoxLayout(m_clientsGroup);
    
    m_clientCountLabel = new QLabel(tr("0 clients connected"));
    m_clientsLayout->addWidget(m_clientCountLabel);
    
    m_clientsList = new QListWidget();
    m_clientsList->setMaximumHeight(150);
    m_clientsLayout->addWidget(m_clientsList);
    
    m_mainLayout->addWidget(m_clientsGroup);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton(tr("Start Server"));
    m_startButton->setDefault(true);
    m_buttonLayout->addWidget(m_startButton);
    
    m_stopButton = new QPushButton(tr("Stop Server"));
    m_stopButton->setEnabled(false);
    m_buttonLayout->addWidget(m_stopButton);
    
    m_buttonLayout->addStretch();
    
    m_closeButton = new QPushButton(tr("Close"));
    m_buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_startButton, &QPushButton::clicked, this, &ServerHostingDialog::onStartServer);
    connect(m_stopButton, &QPushButton::clicked, this, &ServerHostingDialog::onStopServer);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_requirePasswordCheck, &QCheckBox::toggled, this, &ServerHostingDialog::onRequirePasswordToggled);
    
    // Update initial state
    onRequirePasswordToggled(m_requirePasswordCheck->isChecked());
}

void ServerHostingDialog::loadSettings() {
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    m_serverNameEdit->setText(settings.value("serverName", "My RME Server").toString());
    m_portSpin->setValue(settings.value("port", DEFAULT_PORT).toInt());
    m_requirePasswordCheck->setChecked(settings.value("requirePassword", false).toBool());
    m_passwordEdit->setText(settings.value("password", "").toString());
    m_maxClientsSpin->setValue(settings.value("maxClients", DEFAULT_MAX_CLIENTS).toInt());
    m_allowGuestsCheck->setChecked(settings.value("allowGuests", true).toBool());
    m_welcomeMessageEdit->setPlainText(settings.value("welcomeMessage", 
        "Welcome to my server! Please be respectful.").toString());
    
    settings.endGroup();
}

void ServerHostingDialog::saveSettings() {
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    settings.setValue("serverName", m_serverNameEdit->text());
    settings.setValue("port", m_portSpin->value());
    settings.setValue("requirePassword", m_requirePasswordCheck->isChecked());
    settings.setValue("password", m_passwordEdit->text());
    settings.setValue("maxClients", m_maxClientsSpin->value());
    settings.setValue("allowGuests", m_allowGuestsCheck->isChecked());
    settings.setValue("welcomeMessage", m_welcomeMessageEdit->toPlainText());
    
    settings.endGroup();
}

bool ServerHostingDialog::validateSettings() {
    if (m_serverNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Settings"), 
                           tr("Please enter a server name."));
        m_serverNameEdit->setFocus();
        return false;
    }
    
    if (m_requirePasswordCheck->isChecked() && m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Settings"), 
                           tr("Please enter a password or disable password requirement."));
        m_passwordEdit->setFocus();
        return false;
    }
    
    return true;
}

ServerHostingDialog::ServerSettings ServerHostingDialog::getServerSettings() const {
    ServerSettings settings;
    settings.serverName = m_serverNameEdit->text().trimmed();
    settings.port = static_cast<quint16>(m_portSpin->value());
    settings.password = m_passwordEdit->text();
    settings.maxClients = m_maxClientsSpin->value();
    settings.requirePassword = m_requirePasswordCheck->isChecked();
    settings.allowGuests = m_allowGuestsCheck->isChecked();
    settings.welcomeMessage = m_welcomeMessageEdit->toPlainText();
    return settings;
}

void ServerHostingDialog::setServerSettings(const ServerSettings& settings) {
    m_serverNameEdit->setText(settings.serverName);
    m_portSpin->setValue(settings.port);
    m_passwordEdit->setText(settings.password);
    m_maxClientsSpin->setValue(settings.maxClients);
    m_requirePasswordCheck->setChecked(settings.requirePassword);
    m_allowGuestsCheck->setChecked(settings.allowGuests);
    m_welcomeMessageEdit->setPlainText(settings.welcomeMessage);
    m_currentSettings = settings;
}

void ServerHostingDialog::onStartServer() {
    if (!validateSettings()) {
        return;
    }
    
    m_currentSettings = getServerSettings();
    saveSettings();
    
    m_serverRunning = true;
    m_progressBar->setRange(0, 0); // Indeterminate progress
    m_progressBar->setVisible(true);
    m_statusLabel->setText(tr("Starting server..."));
    
    updateUI();
    
    // Emit signal to start server
    emit startServerRequested(m_currentSettings);
    
    // TODO: This would be connected to actual server implementation
    // For now, simulate server start
    QTimer::singleShot(2000, this, &ServerHostingDialog::onServerStarted);
}

void ServerHostingDialog::onStopServer() {
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(true);
    m_statusLabel->setText(tr("Stopping server..."));
    
    updateUI();
    
    // Emit signal to stop server
    emit stopServerRequested();
    
    // TODO: This would be connected to actual server implementation
    // For now, simulate server stop
    QTimer::singleShot(1000, this, &ServerHostingDialog::onServerStopped);
}

void ServerHostingDialog::onServerStarted() {
    m_serverRunning = true;
    m_progressBar->setVisible(false);
    
    // Get local IP addresses for display
    QStringList localIPs;
    for (const QNetworkInterface& interface : QNetworkInterface::allInterfaces()) {
        if (interface.flags() & QNetworkInterface::IsUp && 
            interface.flags() & QNetworkInterface::IsRunning &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {
            for (const QNetworkAddressEntry& entry : interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    localIPs << entry.ip().toString();
                }
            }
        }
    }
    
    QString statusText = tr("Server running on port %1").arg(m_currentSettings.port);
    if (!localIPs.isEmpty()) {
        statusText += tr("\nLocal IP addresses: %1").arg(localIPs.join(", "));
    }
    statusText += tr("\nClients can connect using these addresses.");
    
    m_statusLabel->setText(statusText);
    updateUI();
}

void ServerHostingDialog::onServerStopped() {
    m_serverRunning = false;
    m_progressBar->setVisible(false);
    m_statusLabel->setText(tr("Server stopped"));
    
    // Clear client list
    m_clientsList->clear();
    m_clientCountLabel->setText(tr("0 clients connected"));
    
    updateUI();
}

void ServerHostingDialog::onServerError(const QString& error) {
    m_serverRunning = false;
    m_progressBar->setVisible(false);
    m_statusLabel->setText(tr("Server error: %1").arg(error));
    m_statusLabel->setStyleSheet("QLabel { color: red; }");
    
    updateUI();
    
    QMessageBox::warning(this, tr("Server Error"), 
                        tr("Failed to start server:\n%1").arg(error));
}

void ServerHostingDialog::onClientConnected(const QString& clientName) {
    QListWidgetItem* item = new QListWidgetItem(clientName);
    item->setIcon(QIcon(":/icons/user.png")); // TODO: Add actual icon
    m_clientsList->addItem(item);
    
    int clientCount = m_clientsList->count();
    m_clientCountLabel->setText(tr("%1 client(s) connected").arg(clientCount));
}

void ServerHostingDialog::onClientDisconnected(const QString& clientName) {
    for (int i = 0; i < m_clientsList->count(); ++i) {
        QListWidgetItem* item = m_clientsList->item(i);
        if (item && item->text() == clientName) {
            delete m_clientsList->takeItem(i);
            break;
        }
    }
    
    int clientCount = m_clientsList->count();
    m_clientCountLabel->setText(tr("%1 client(s) connected").arg(clientCount));
}

void ServerHostingDialog::onRequirePasswordToggled(bool enabled) {
    m_passwordEdit->setEnabled(enabled);
    if (!enabled) {
        m_passwordEdit->clear();
    }
}

void ServerHostingDialog::updateUI() {
    bool canStart = !m_serverRunning;
    bool canStop = m_serverRunning;
    
    m_startButton->setEnabled(canStart);
    m_stopButton->setEnabled(canStop);
    
    // Disable settings while server is running
    m_settingsGroup->setEnabled(canStart);
    
    if (!m_serverRunning) {
        m_statusLabel->setStyleSheet(""); // Reset style
    }
}

} // namespace dialogs
} // namespace ui
} // namespace RME