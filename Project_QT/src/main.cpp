#include <QApplication>
#include <QWidget>
#include <QLabel> // For a simple label widget
#include <QVBoxLayout> // For layout
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QPalette>

// Qlementine removed - using default Qt styling

#include "core/utils/ResourcePathManager.h"
#include "ui/MainWindow.h"
#include "ui/dialogs/WelcomeDialog.h"
#include "core/settings/AppSettings.h"

// Function to initialize Qt theme (simplified - no Qlementine)
void initializeQtTheme(QApplication& app, RME::core::settings::AppSettings& settings) {
    // Get theme preference from settings (default to system theme)
    QString themeName = settings.getString("ui/theme", "system");
    
    if (themeName == "dark") {
        // Apply dark palette
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        app.setPalette(darkPalette);
        qInfo() << "Applied dark theme";
    } else {
        // Use system default theme
        qInfo() << "Using system default theme";
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
    
    // Apply Qt theme (simplified - no Qlementine)
    initializeQtTheme(app, settings);
    
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
