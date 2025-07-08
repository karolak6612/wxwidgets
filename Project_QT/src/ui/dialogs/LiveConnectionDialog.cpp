#include "LiveConnectionDialog.h"
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QHostAddress>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace RME {
namespace ui {
namespace dialogs {

const QString LiveConnectionDialog::SETTINGS_GROUP = "LiveConnection";

LiveConnectionDialog::LiveConnectionDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_isConnecting(false)
{
    setWindowTitle(tr("Connect to Live Server"));
    setModal(true);
    setMinimumSize(400, 500);
    
    setupUI();
    setupConnections();
    loadSettings();
    updateUI();
}

LiveConnectionDialog::ConnectionSettings LiveConnectionDialog::getConnectionSettings() const
{
    ConnectionSettings settings;
    settings.hostname = m_hostnameEdit->text().trimmed();
    settings.port = static_cast<quint16>(m_portSpinBox->value());
    settings.username = m_usernameEdit->text().trimmed();
    settings.password = m_passwordEdit->text();
    settings.rememberSettings = m_rememberSettingsCheck->isChecked();
    return settings;
}

void LiveConnectionDialog::setConnectionSettings(const ConnectionSettings& settings)
{
    m_hostnameEdit->setText(settings.hostname);
    m_portSpinBox->setValue(settings.port);
    m_usernameEdit->setText(settings.username);
    m_passwordEdit->setText(settings.password);
    m_rememberSettingsCheck->setChecked(settings.rememberSettings);
    m_currentSettings = settings;
}

void LiveConnectionDialog::setConnecting(bool connecting)
{
    m_isConnecting = connecting;
    updateUI();
    
    if (connecting) {
        m_progressBar->setRange(0, 0); // Indeterminate progress
        setConnectionProgress(tr("Connecting..."));
    } else {
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        m_statusLabel->clear();
    }
}

void LiveConnectionDialog::setConnectionProgress(const QString& status)
{
    m_statusLabel->setText(status);
}

void LiveConnectionDialog::setConnectionError(const QString& error)
{
    m_statusLabel->setText(tr("Error: %1").arg(error));
    m_statusLabel->setStyleSheet("QLabel { color: red; }");
    setConnecting(false);
}

void LiveConnectionDialog::onConnectClicked()
{
    if (!validateInput()) {
        return;
    }
    
    m_currentSettings = getConnectionSettings();
    
    if (m_currentSettings.rememberSettings) {
        saveSettings();
    }
    
    setConnecting(true);
    emit connectRequested(m_currentSettings);
}

void LiveConnectionDialog::onCancelClicked()
{
    if (m_isConnecting) {
        emit cancelRequested();
        setConnecting(false);
    } else {
        reject();
    }
}

void LiveConnectionDialog::onConnectionStateChanged()
{
    // This can be connected to client state changes
    updateUI();
}

void LiveConnectionDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Server settings group
    m_serverGroup = new QGroupBox(tr("Server Settings"));
    m_serverLayout = new QFormLayout(m_serverGroup);
    
    m_hostnameEdit = new QLineEdit();
    m_hostnameEdit->setPlaceholderText(tr("localhost"));
    m_serverLayout->addRow(tr("Hostname:"), m_hostnameEdit);
    
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(DEFAULT_PORT);
    m_serverLayout->addRow(tr("Port:"), m_portSpinBox);
    
    m_mainLayout->addWidget(m_serverGroup);
    
    // User settings group
    m_userGroup = new QGroupBox(tr("User Settings"));
    m_userLayout = new QFormLayout(m_userGroup);
    
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText(tr("Enter your username"));
    m_userLayout->addRow(tr("Username:"), m_usernameEdit);
    
    QHBoxLayout* passwordLayout = new QHBoxLayout();
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Optional server password"));
    m_showPasswordCheck = new QCheckBox(tr("Show"));
    passwordLayout->addWidget(m_passwordEdit);
    passwordLayout->addWidget(m_showPasswordCheck);
    m_userLayout->addRow(tr("Password:"), passwordLayout);
    
    m_rememberSettingsCheck = new QCheckBox(tr("Remember these settings"));
    m_userLayout->addRow(m_rememberSettingsCheck);
    
    m_mainLayout->addWidget(m_userGroup);
    
    // Recent connections group
    m_recentGroup = new QGroupBox(tr("Recent Connections"));
    m_recentLayout = new QVBoxLayout(m_recentGroup);
    
    m_recentCombo = new QComboBox();
    m_recentCombo->setEditable(false);
    m_recentLayout->addWidget(m_recentCombo);
    
    QHBoxLayout* recentButtonLayout = new QHBoxLayout();
    m_loadRecentButton = new QPushButton(tr("Load Selected"));
    m_deleteRecentButton = new QPushButton(tr("Delete Selected"));
    recentButtonLayout->addWidget(m_loadRecentButton);
    recentButtonLayout->addWidget(m_deleteRecentButton);
    recentButtonLayout->addStretch();
    m_recentLayout->addLayout(recentButtonLayout);
    
    m_mainLayout->addWidget(m_recentGroup);
    
    // Connection status group
    m_statusGroup = new QGroupBox(tr("Connection Status"));
    m_statusLayout = new QVBoxLayout(m_statusGroup);
    
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    m_statusLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_statusLayout->addWidget(m_progressBar);
    
    m_mainLayout->addWidget(m_statusGroup);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_connectButton = new QPushButton(tr("Connect"));
    m_connectButton->setDefault(true);
    m_buttonLayout->addWidget(m_connectButton);
    
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Set initial focus
    m_hostnameEdit->setFocus();
}

void LiveConnectionDialog::setupConnections()
{
    connect(m_connectButton, &QPushButton::clicked, this, &LiveConnectionDialog::onConnectClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LiveConnectionDialog::onCancelClicked);
    
    connect(m_hostnameEdit, &QLineEdit::textChanged, this, &LiveConnectionDialog::onInputChanged);
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LiveConnectionDialog::onInputChanged);
    connect(m_portSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &LiveConnectionDialog::onInputChanged);
    
    connect(m_showPasswordCheck, &QCheckBox::toggled, this, &LiveConnectionDialog::onShowPasswordToggled);
    
    connect(m_loadRecentButton, &QPushButton::clicked, [this]() {
        int index = m_recentCombo->currentIndex();
        if (index >= 0) {
            QVariant data = m_recentCombo->itemData(index);
            if (data.isValid()) {
                // TODO: Load recent connection settings
            }
        }
    });
    
    connect(m_deleteRecentButton, &QPushButton::clicked, [this]() {
        int index = m_recentCombo->currentIndex();
        if (index >= 0) {
            m_recentCombo->removeItem(index);
            // TODO: Remove from settings
        }
    });
    
    // Enter key handling
    connect(m_hostnameEdit, &QLineEdit::returnPressed, this, &LiveConnectionDialog::onConnectClicked);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LiveConnectionDialog::onConnectClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LiveConnectionDialog::onConnectClicked);
}

void LiveConnectionDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    // Load last used settings
    QString hostname = settings.value("hostname", "localhost").toString();
    quint16 port = settings.value("port", DEFAULT_PORT).toUInt();
    QString username = settings.value("username").toString();
    bool rememberSettings = settings.value("rememberSettings", false).toBool();
    
    m_hostnameEdit->setText(hostname);
    m_portSpinBox->setValue(port);
    m_usernameEdit->setText(username);
    m_rememberSettingsCheck->setChecked(rememberSettings);
    
    // Load recent connections
    int recentCount = settings.beginReadArray("recentConnections");
    for (int i = 0; i < recentCount; ++i) {
        settings.setArrayIndex(i);
        QString displayName = QString("%1@%2:%3")
            .arg(settings.value("username").toString())
            .arg(settings.value("hostname").toString())
            .arg(settings.value("port").toUInt());
        
        ConnectionSettings recentSettings;
        recentSettings.hostname = settings.value("hostname").toString();
        recentSettings.port = settings.value("port").toUInt();
        recentSettings.username = settings.value("username").toString();
        recentSettings.rememberSettings = true;
        
        m_recentCombo->addItem(displayName, QVariant::fromValue(recentSettings));
    }
    settings.endArray();
    
    settings.endGroup();
}

void LiveConnectionDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup(SETTINGS_GROUP);
    
    // Save current settings
    settings.setValue("hostname", m_hostnameEdit->text());
    settings.setValue("port", m_portSpinBox->value());
    settings.setValue("username", m_usernameEdit->text());
    settings.setValue("rememberSettings", m_rememberSettingsCheck->isChecked());
    
    // TODO: Save to recent connections list
    
    settings.endGroup();
}

bool LiveConnectionDialog::validateInput()
{
    QString hostname = m_hostnameEdit->text().trimmed();
    QString username = m_usernameEdit->text().trimmed();
    
    if (hostname.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Please enter a hostname or IP address."));
        m_hostnameEdit->setFocus();
        return false;
    }
    
    if (username.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Please enter a username."));
        m_usernameEdit->setFocus();
        return false;
    }
    
    // Validate hostname/IP
    QHostAddress address;
    if (!address.setAddress(hostname)) {
        // Not an IP address, check if it's a valid hostname
        QRegularExpression hostnameRegex("^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?$");
        if (!hostnameRegex.match(hostname).hasMatch()) {
            QMessageBox::warning(this, tr("Invalid Input"), 
                               tr("Please enter a valid hostname or IP address."));
            m_hostnameEdit->setFocus();
            return false;
        }
    }
    
    // Validate username (basic check)
    if (username.length() > 32) {
        QMessageBox::warning(this, tr("Invalid Input"), 
                           tr("Username must be 32 characters or less."));
        m_usernameEdit->setFocus();
        return false;
    }
    
    return true;
}

void LiveConnectionDialog::updateUI()
{
    bool canConnect = !m_isConnecting && 
                     !m_hostnameEdit->text().trimmed().isEmpty() &&
                     !m_usernameEdit->text().trimmed().isEmpty();
    
    m_connectButton->setEnabled(canConnect);
    m_connectButton->setText(m_isConnecting ? tr("Connecting...") : tr("Connect"));
    
    m_cancelButton->setText(m_isConnecting ? tr("Cancel") : tr("Close"));
    
    // Disable input fields while connecting
    m_serverGroup->setEnabled(!m_isConnecting);
    m_userGroup->setEnabled(!m_isConnecting);
    m_recentGroup->setEnabled(!m_isConnecting);
    
    m_progressBar->setVisible(m_isConnecting);
    
    if (!m_isConnecting) {
        m_statusLabel->setStyleSheet(""); // Reset style
    }
}

void LiveConnectionDialog::onInputChanged()
{
    updateUI();
}

void LiveConnectionDialog::onShowPasswordToggled(bool show)
{
    m_passwordEdit->setEchoMode(show ? QLineEdit::Normal : QLineEdit::Password);
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "LiveConnectionDialog.moc" // Removed - Q_OBJECT is in header