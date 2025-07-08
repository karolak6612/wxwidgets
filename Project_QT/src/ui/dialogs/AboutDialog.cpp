#include "AboutDialog.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QSysInfo>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>

namespace RME {
namespace ui {
namespace dialogs {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
{
    setWindowTitle(tr("About Remere's Map Editor"));
    setModal(true);
    setMinimumSize(500, 400);
    setMaximumSize(800, 600);
    
    setupUI();
}

void AboutDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create tabs
    createMainTab();
    createCreditsTab();
    createLicenseTab();
    createSystemInfoTab();
    
    // Create button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setDefault(true);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
}

void AboutDialog::createMainTab() {
    m_mainTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_mainTab);
    
    // Logo (placeholder for now)
    m_logoLabel = new QLabel();
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setMinimumHeight(64);
    m_logoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    m_logoLabel->setText(tr("RME Logo"));
    layout->addWidget(m_logoLabel);
    
    // Application title
    m_titleLabel = new QLabel(tr("Remere's Map Editor"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 4);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    layout->addWidget(m_titleLabel);
    
    // Version information
    m_versionLabel = new QLabel(getApplicationInfo());
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setWordWrap(true);
    layout->addWidget(m_versionLabel);
    
    // Qt version
    m_qtVersionLabel = new QLabel(tr("Built with Qt %1").arg(QT_VERSION_STR));
    m_qtVersionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_qtVersionLabel);
    
    // Description
    m_descriptionLabel = new QLabel(
        tr("A powerful map editor for OpenTibia servers.\n"
           "Create and edit OTBM maps with advanced tools and features.")
    );
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);
    layout->addWidget(m_descriptionLabel);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_mainTab, tr("About"));
}

void AboutDialog::createCreditsTab() {
    m_creditsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_creditsTab);
    
    m_creditsText = new QTextBrowser();
    m_creditsText->setHtml(getCreditsText());
    layout->addWidget(m_creditsText);
    
    m_tabWidget->addTab(m_creditsTab, tr("Credits"));
}

void AboutDialog::createLicenseTab() {
    m_licenseTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_licenseTab);
    
    m_licenseText = new QTextBrowser();
    m_licenseText->setPlainText(getLicenseText());
    layout->addWidget(m_licenseText);
    
    m_tabWidget->addTab(m_licenseTab, tr("License"));
}

void AboutDialog::createSystemInfoTab() {
    m_systemInfoTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_systemInfoTab);
    
    m_systemInfoText = new QTextBrowser();
    m_systemInfoText->setPlainText(getSystemInfo());
    layout->addWidget(m_systemInfoText);
    
    m_tabWidget->addTab(m_systemInfoTab, tr("System Info"));
}

QString AboutDialog::getApplicationInfo() const {
    QString version = QApplication::applicationVersion();
    if (version.isEmpty()) {
        version = "1.0.0"; // Default version
    }
    
    return tr("Version %1\nQt6 Edition").arg(version);
}

QString AboutDialog::getSystemInfo() const {
    QString info;
    QTextStream stream(&info);
    
    stream << "Application Information:\n";
    stream << "  Name: " << QApplication::applicationName() << "\n";
    stream << "  Version: " << QApplication::applicationVersion() << "\n";
    stream << "  Organization: " << QApplication::organizationName() << "\n";
    stream << "\n";
    
    stream << "Qt Information:\n";
    stream << "  Qt Version: " << QT_VERSION_STR << "\n";
    stream << "  Qt Runtime Version: " << qVersion() << "\n";
    stream << "\n";
    
    stream << "System Information:\n";
    stream << "  OS: " << QSysInfo::prettyProductName() << "\n";
    stream << "  Kernel: " << QSysInfo::kernelType() << " " << QSysInfo::kernelVersion() << "\n";
    stream << "  Architecture: " << QSysInfo::currentCpuArchitecture() << "\n";
    stream << "  Machine: " << QSysInfo::machineHostName() << "\n";
    stream << "\n";
    
    // OpenGL information
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context) {
        stream << "OpenGL Information:\n";
        QOpenGLFunctions* functions = context->functions();
        if (functions) {
            stream << "  Vendor: " << reinterpret_cast<const char*>(functions->glGetString(GL_VENDOR)) << "\n";
            stream << "  Renderer: " << reinterpret_cast<const char*>(functions->glGetString(GL_RENDERER)) << "\n";
            stream << "  Version: " << reinterpret_cast<const char*>(functions->glGetString(GL_VERSION)) << "\n";
        }
    } else {
        stream << "OpenGL Information:\n";
        stream << "  No OpenGL context available\n";
    }
    
    return info;
}

QString AboutDialog::getCreditsText() const {
    return tr(
        "<h3>Remere's Map Editor - Qt6 Edition</h3>"
        "<p>A modern Qt6 port of the popular OpenTibia map editor.</p>"
        
        "<h4>Original RME Development</h4>"
        "<ul>"
        "<li><b>Remere</b> - Original creator and main developer</li>"
        "<li><b>Dalkon</b> - Major contributor</li>"
        "<li><b>Kornholijo</b> - Contributor</li>"
        "<li><b>Comedinha</b> - Contributor</li>"
        "<li><b>And many others</b> - Various contributions</li>"
        "</ul>"
        
        "<h4>Qt6 Port Development</h4>"
        "<ul>"
        "<li><b>Development Team</b> - Qt6 migration and modernization</li>"
        "</ul>"
        
        "<h4>Third-Party Libraries</h4>"
        "<ul>"
        "<li><b>Qt6</b> - Cross-platform application framework</li>"
        "<li><b>Qlementine</b> - Modern Qt styling library</li>"
        "<li><b>pugixml</b> - XML parsing library</li>"
        "<li><b>zlib</b> - Compression library</li>"
        "</ul>"
        
        "<h4>Special Thanks</h4>"
        "<ul>"
        "<li><b>OpenTibia Community</b> - For continued support and feedback</li>"
        "<li><b>Contributors</b> - Everyone who helped with testing and bug reports</li>"
        "</ul>"
    );
}

QString AboutDialog::getLicenseText() const {
    // Try to load license from file first
    QFile licenseFile(":/license.txt");
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&licenseFile);
        return stream.readAll();
    }
    
    // Fallback to embedded license text
    return tr(
        "GNU GENERAL PUBLIC LICENSE\n"
        "Version 3, 29 June 2007\n"
        "\n"
        "Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>\n"
        "Everyone is permitted to copy and distribute verbatim copies\n"
        "of this license document, but changing it is not allowed.\n"
        "\n"
        "Preamble\n"
        "\n"
        "The GNU General Public License is a free, copyleft license for\n"
        "software and other kinds of works.\n"
        "\n"
        "The licenses for most software and other practical works are designed\n"
        "to take away your freedom to share and change the works. By contrast,\n"
        "the GNU General Public License is intended to guarantee your freedom to\n"
        "share and change all versions of a program--to make sure it remains free\n"
        "software for all its users.\n"
        "\n"
        "[Full GPL v3 license text would continue here...]\n"
        "\n"
        "For the complete license text, please visit:\n"
        "https://www.gnu.org/licenses/gpl-3.0.html"
    );
}

void AboutDialog::onLicenseButtonClicked() {
    m_tabWidget->setCurrentWidget(m_licenseTab);
}

void AboutDialog::onCreditsButtonClicked() {
    m_tabWidget->setCurrentWidget(m_creditsTab);
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "AboutDialog.moc" // Removed - Q_OBJECT is in header