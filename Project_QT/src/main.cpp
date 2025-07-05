#include <QApplication>
#include <QWidget>
#include <QLabel> // For a simple label widget
#include <QVBoxLayout> // For layout
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// Qlementine includes (FINAL-07)
#include <oclero/qlementine.hpp>

#include "core/utils/ResourcePathManager.h"
#include "ui/MainWindow.h"
#include "ui/dialogs/WelcomeDialog.h"
#include "core/settings/AppSettings.h"

// Function to initialize Qlementine theme (FINAL-07)
void initializeQlementineTheme(QApplication& app, RME::core::settings::AppSettings& settings) {
    // Get theme preference from settings (default to light theme)
    QString themeName = settings.getString("ui/theme", "light");
    
    // Load theme from resources
    QString themeResourcePath = QString(":/themes/%1.json").arg(themeName);
    QFile themeFile(themeResourcePath);
    
    if (!themeFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open theme file:" << themeResourcePath;
        qWarning() << "Falling back to default light theme";
        themeResourcePath = ":/themes/light.json";
        themeFile.setFileName(themeResourcePath);
        if (!themeFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open fallback theme file, using default Qlementine theme";
            // Apply default Qlementine theme without custom JSON
            oclero::qlementine::installTheme(&app);
            return;
        }
    }
    
    // Parse theme JSON
    QByteArray themeData = themeFile.readAll();
    QJsonParseError parseError;
    QJsonDocument themeDoc = QJsonDocument::fromJson(themeData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse theme JSON:" << parseError.errorString();
        qWarning() << "Using default Qlementine theme";
        oclero::qlementine::installTheme(&app);
        return;
    }
    
    // Apply Qlementine theme with custom JSON
    try {
        oclero::qlementine::installTheme(&app, themeDoc.object());
        qInfo() << "Successfully applied Qlementine theme:" << themeName;
    } catch (const std::exception& e) {
        qWarning() << "Failed to apply Qlementine theme:" << e.what();
        qWarning() << "Using default Qlementine theme";
        oclero::qlementine::installTheme(&app);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    QCoreApplication::setOrganizationName("RME");
    QCoreApplication::setApplicationName("Remere's Map Editor");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // Initialize resource path manager
    RME::core::utils::ResourcePathManager::instance().initialize(QCoreApplication::applicationDirPath());
    
    // Initialize settings early for theme loading
    RME::core::settings::AppSettings settings;
    
    // Apply Qlementine theme (FINAL-07)
    initializeQlementineTheme(app, settings);
    
    // Create the main window
    MainWindow mainWindow;
    
    // Check if we should show the welcome dialog
    bool showWelcome = settings.getBool("general/showWelcome", true);
    bool hasCommandLineFile = argc > 1; // Simple check for command line file
    
    if (showWelcome && !hasCommandLineFile) {
        RME::ui::dialogs::WelcomeDialog welcomeDialog(settings);
        
        // Connect welcome dialog signals to main window
        QObject::connect(&welcomeDialog, &RME::ui::dialogs::WelcomeDialog::createNewMapRequested,
                        &mainWindow, [&mainWindow]() {
                            // Trigger new map creation
                            mainWindow.show();
                            // TODO: Call mainWindow.onNewMap() when it's public or add a public method
                        });
        
        QObject::connect(&welcomeDialog, &RME::ui::dialogs::WelcomeDialog::openSpecificMapRequested,
                        &mainWindow, [&mainWindow](const QString& filePath) {
                            // Trigger map opening
                            mainWindow.show();
                            // TODO: Call mainWindow.openMap(filePath) when available
                        });
        
        if (welcomeDialog.exec() == QDialog::Accepted) {
            // User performed an action, main window should already be shown
        } else {
            // User closed dialog, show main window anyway
            mainWindow.show();
        }
    } else {
        // Show main window directly
        mainWindow.show();
    }

    return app.exec();
}
