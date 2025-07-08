#include "WelcomeDialog.h"
#include "PreferencesDialog.h"
#include "core/settings/AppSettings.h"

#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>

namespace RME {
namespace ui {
namespace dialogs {

WelcomeDialog::WelcomeDialog(RME::core::settings::AppSettings& settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setWindowTitle(tr("Welcome to Remere's Map Editor"));
    setModal(true);
    setMinimumSize(800, 600);
    
    setupUI();
    loadSettings();
    loadRecentFiles();
    loadWhatsNew();
}

void WelcomeDialog::setupUI() {
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create main splitter
    m_splitter = new QSplitter(Qt::Horizontal);
    m_mainLayout->addWidget(m_splitter);
    
    // Create panels
    createLeftPanel();
    createCenterPanel();
    createRightPanel();
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0); // Left panel - fixed width
    m_splitter->setStretchFactor(1, 1); // Center panel - expandable
    m_splitter->setStretchFactor(2, 0); // Right panel - fixed width
    
    // Create bottom panel
    createBottomPanel();
    
    // Connect signals
    connect(m_newMapButton, &QPushButton::clicked, this, &WelcomeDialog::onNewMapClicked);
    connect(m_openMapButton, &QPushButton::clicked, this, &WelcomeDialog::onOpenMapClicked);
    connect(m_preferencesButton, &QPushButton::clicked, this, &WelcomeDialog::onPreferencesClicked);
    connect(m_recentFilesList, &QListWidget::itemDoubleClicked, this, &WelcomeDialog::onRecentFileDoubleClicked);
    connect(m_showOnStartupCheck, &QCheckBox::toggled, this, &WelcomeDialog::onShowOnStartupToggled);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

void WelcomeDialog::createLeftPanel() {
    m_leftPanel = new QWidget();
    m_leftPanel->setMinimumWidth(200);
    m_leftPanel->setMaximumWidth(250);
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setSpacing(10);
    
    // Logo
    m_logoLabel = new QLabel();
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setPixmap(createLogo());
    m_leftLayout->addWidget(m_logoLabel);
    
    // Title
    m_titleLabel = new QLabel(tr("Remere's Map Editor"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_leftLayout->addWidget(m_titleLabel);
    
    // Version
    QString version = QApplication::applicationVersion();
    if (version.isEmpty()) {
        version = "1.0.0";
    }
    m_versionLabel = new QLabel(tr("Version %1").arg(version));
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_leftLayout->addWidget(m_versionLabel);
    
    m_leftLayout->addSpacing(20);
    
    // Action buttons
    m_newMapButton = new QPushButton(tr("New Map"));
    m_newMapButton->setMinimumHeight(BUTTON_HEIGHT);
    m_newMapButton->setIcon(QIcon(":/icons/new.png")); // TODO: Add actual icon
    m_leftLayout->addWidget(m_newMapButton);
    
    m_openMapButton = new QPushButton(tr("Open Map"));
    m_openMapButton->setMinimumHeight(BUTTON_HEIGHT);
    m_openMapButton->setIcon(QIcon(":/icons/open.png")); // TODO: Add actual icon
    m_leftLayout->addWidget(m_openMapButton);
    
    m_preferencesButton = new QPushButton(tr("Preferences"));
    m_preferencesButton->setMinimumHeight(BUTTON_HEIGHT);
    m_preferencesButton->setIcon(QIcon(":/icons/preferences.png")); // TODO: Add actual icon
    m_leftLayout->addWidget(m_preferencesButton);
    
    m_leftLayout->addStretch();
    
    m_splitter->addWidget(m_leftPanel);
}

void WelcomeDialog::createCenterPanel() {
    m_centerPanel = new QWidget();
    m_centerLayout = new QVBoxLayout(m_centerPanel);
    
    // What's New label
    m_whatsNewLabel = new QLabel(tr("What's New"));
    QFont labelFont = m_whatsNewLabel->font();
    labelFont.setPointSize(labelFont.pointSize() + 1);
    labelFont.setBold(true);
    m_whatsNewLabel->setFont(labelFont);
    m_centerLayout->addWidget(m_whatsNewLabel);
    
    // What's New content
    m_whatsNewText = new QTextBrowser();
    m_whatsNewText->setOpenExternalLinks(true);
    m_centerLayout->addWidget(m_whatsNewText);
    
    m_splitter->addWidget(m_centerPanel);
}

void WelcomeDialog::createRightPanel() {
    m_rightPanel = new QWidget();
    m_rightPanel->setMinimumWidth(200);
    m_rightPanel->setMaximumWidth(250);
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    
    // Recent Files label
    m_recentFilesLabel = new QLabel(tr("Recent Files"));
    QFont labelFont = m_recentFilesLabel->font();
    labelFont.setPointSize(labelFont.pointSize() + 1);
    labelFont.setBold(true);
    m_recentFilesLabel->setFont(labelFont);
    m_rightLayout->addWidget(m_recentFilesLabel);
    
    // Recent Files list
    m_recentFilesList = new QListWidget();
    m_recentFilesList->setAlternatingRowColors(true);
    m_rightLayout->addWidget(m_recentFilesList);
    
    m_splitter->addWidget(m_rightPanel);
}

void WelcomeDialog::createBottomPanel() {
    // Create a container widget for the bottom panel
    QWidget* bottomContainer = new QWidget();
    QVBoxLayout* containerLayout = new QVBoxLayout(bottomContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // Add the splitter first
    containerLayout->addWidget(m_splitter);
    
    // Create bottom panel
    m_bottomPanel = new QWidget();
    m_bottomLayout = new QHBoxLayout(m_bottomPanel);
    
    // Show on startup checkbox
    m_showOnStartupCheck = new QCheckBox(tr("Show this dialog on startup"));
    m_bottomLayout->addWidget(m_showOnStartupCheck);
    
    m_bottomLayout->addStretch();
    
    // Close button
    m_closeButton = new QPushButton(tr("Close"));
    m_closeButton->setDefault(true);
    m_bottomLayout->addWidget(m_closeButton);
    
    containerLayout->addWidget(m_bottomPanel);
    
    // Replace the main layout content
    delete m_mainLayout;
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->addWidget(bottomContainer);
}

void WelcomeDialog::loadSettings() {
    // Load show on startup setting
    bool showOnStartup = m_settings.getBool("general/showWelcome", true);
    m_showOnStartupCheck->setChecked(showOnStartup);
}

void WelcomeDialog::loadRecentFiles() {
    m_recentFilesList->clear();
    
    // Get recent files from settings
    QStringList recentFiles = m_settings.getStringList("recentFiles/fileList", QStringList());
    
    int count = 0;
    for (const QString& filePath : recentFiles) {
        if (count >= MAX_RECENT_FILES) {
            break;
        }
        
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(fileInfo.baseName());
            item->setToolTip(filePath);
            item->setData(Qt::UserRole, filePath);
            m_recentFilesList->addItem(item);
            count++;
        }
    }
    
    if (m_recentFilesList->count() == 0) {
        QListWidgetItem* item = new QListWidgetItem(tr("No recent files"));
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        m_recentFilesList->addItem(item);
    }
}

void WelcomeDialog::loadWhatsNew() {
    m_whatsNewText->setHtml(getWhatsNewContent());
}

QString WelcomeDialog::getWhatsNewContent() const {
    // Try to load from resource file first
    QFile whatsNewFile(":/whatsnew.txt");
    if (whatsNewFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&whatsNewFile);
        return stream.readAll();
    }
    
    // Fallback to embedded content
    return tr(
        "<h3>Welcome to Remere's Map Editor Qt6 Edition!</h3>"
        
        "<h4>New in this version:</h4>"
        "<ul>"
        "<li><b>Modern Qt6 Interface</b> - Complete rewrite using Qt6 framework</li>"
        "<li><b>Improved Performance</b> - Faster rendering and map operations</li>"
        "<li><b>Enhanced UI</b> - Modern styling with Qlementine theme</li>"
        "<li><b>Better Cross-Platform Support</b> - Native look and feel on all platforms</li>"
        "<li><b>Live Collaboration</b> - Real-time collaborative editing support</li>"
        "<li><b>Advanced Brush System</b> - More powerful and flexible brush tools</li>"
        "<li><b>Improved File Handling</b> - Better OTBM support and error handling</li>"
        "</ul>"
        
        "<h4>Key Features:</h4>"
        "<ul>"
        "<li>Full OTBM map editing support</li>"
        "<li>Advanced brush system for terrain, objects, and creatures</li>"
        "<li>House and spawn management</li>"
        "<li>Waypoint system for navigation</li>"
        "<li>Undo/redo system for all operations</li>"
        "<li>Comprehensive preferences and customization</li>"
        "<li>Live server collaboration</li>"
        "</ul>"
        
        "<h4>Getting Started:</h4>"
        "<p>Click <b>New Map</b> to create a new map, or <b>Open Map</b> to load an existing OTBM file. "
        "Use the <b>Preferences</b> to configure the editor to your liking.</p>"
        
        "<p>For more information and tutorials, visit the project documentation.</p>"
    );
}

QPixmap WelcomeDialog::createLogo() const {
    // Create a simple placeholder logo
    QPixmap pixmap(LOGO_SIZE, LOGO_SIZE);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw a simple map-like icon
    painter.setBrush(QBrush(QColor(100, 150, 200)));
    painter.setPen(QPen(QColor(50, 100, 150), 2));
    painter.drawRoundedRect(8, 8, LOGO_SIZE - 16, LOGO_SIZE - 16, 8, 8);
    
    // Draw grid lines
    painter.setPen(QPen(QColor(255, 255, 255, 128), 1));
    for (int i = 16; i < LOGO_SIZE - 8; i += 8) {
        painter.drawLine(i, 12, i, LOGO_SIZE - 12);
        painter.drawLine(12, i, LOGO_SIZE - 12, i);
    }
    
    // Draw "RME" text
    painter.setPen(QPen(Qt::white));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "RME");
    
    return pixmap;
}

void WelcomeDialog::onNewMapClicked() {
    emit createNewMapRequested();
    accept();
}

void WelcomeDialog::onOpenMapClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Map"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTBM Files (*.otbm);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        emit openSpecificMapRequested(fileName);
        accept();
    }
}

void WelcomeDialog::onPreferencesClicked() {
    PreferencesDialog dialog(m_settings, this);
    dialog.exec();
}

void WelcomeDialog::onRecentFileDoubleClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            emit openSpecificMapRequested(filePath);
            accept();
        } else {
            // File no longer exists, remove from recent files
            // TODO: Implement recent files cleanup
        }
    }
}

void WelcomeDialog::onShowOnStartupToggled(bool checked) {
    m_settings.setBool("general/showWelcome", checked);
    m_settings.save();
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "WelcomeDialog.moc" // Removed - Q_OBJECT is in header