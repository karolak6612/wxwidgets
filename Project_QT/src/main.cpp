#include <QApplication>
#include <QWidget>
#include <QLabel> // For a simple label widget
#include <QVBoxLayout> // For layout
#include "core/utils/ResourcePathManager.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    QCoreApplication::setOrganizationName("RME");
    QCoreApplication::setApplicationName("Remere's Map Editor");
    
    // Initialize resource path manager
    RME::core::utils::ResourcePathManager::instance().initialize(QCoreApplication::applicationDirPath());
    
    // Create the main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
