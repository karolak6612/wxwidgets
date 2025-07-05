#include <QApplication>
#include <QWidget>
#include <QLabel> // For a simple label widget
#include <QVBoxLayout> // For layout

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create a simple main window (QWidget)
    QWidget mainWindow;
    mainWindow.setWindowTitle("RME-Qt6 Placeholder");
    mainWindow.setMinimumSize(300, 200);

    // Add a label to the window
    QLabel *label = new QLabel("Hello, RME-Qt6 World!", &mainWindow);
    label->setAlignment(Qt::AlignCenter);

    // Set up a layout
    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);
    layout->addWidget(label);
    mainWindow.setLayout(layout);

    // Show the window
    mainWindow.show();

    return app.exec();
}
