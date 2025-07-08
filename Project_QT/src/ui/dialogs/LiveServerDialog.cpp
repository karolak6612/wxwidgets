#include "LiveServerDialog.h"
#include "LiveServerControlPanelQt.h"
#include <QCloseEvent>
#include <QMessageBox>

namespace RME {
namespace ui {
namespace dialogs {

LiveServerDialog::LiveServerDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setWindowTitle("Live Server Control Panel");
    setMinimumSize(600, 700);
    resize(800, 900);
    
    setupUI();
    connectSignals();
}

void LiveServerDialog::showAsModal()
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    exec();
}

void LiveServerDialog::showAsNonModal()
{
    setModal(false);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    show();
    raise();
    activateWindow();
}

void LiveServerDialog::closeEvent(QCloseEvent* event)
{
    // Check if server is running and warn user
    if (m_controlPanel && m_controlPanel->isServerRunning()) {
        int result = QMessageBox::question(this, "Server Running",
                                           "The live server is currently running. "
                                           "Closing this dialog will not stop the server.\n\n"
                                           "Do you want to close the dialog anyway?",
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::No);
        
        if (result != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    
    QDialog::closeEvent(event);
}

void LiveServerDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create the control panel
    m_controlPanel = new LiveServerControlPanelQt(this);
    m_mainLayout->addWidget(m_controlPanel, 1); // Give it stretch factor
    
    // Create button box (Close button only)
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_mainLayout->addWidget(m_buttonBox);
}

void LiveServerDialog::connectSignals()
{
    // Connect dialog buttons
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
    
    // Connect to control panel signals
    if (m_controlPanel) {
        connect(m_controlPanel, &LiveServerControlPanelQt::serverStateChanged,
                this, &LiveServerDialog::onServerStateChanged);
    }
}

void LiveServerDialog::onServerStateChanged(bool isRunning)
{
    // Update dialog title to reflect server state
    if (isRunning) {
        setWindowTitle(QString("Live Server Control Panel - Running (Port %1)")
                       .arg(m_controlPanel->getCurrentPort()));
    } else {
        setWindowTitle("Live Server Control Panel - Stopped");
    }
}

} // namespace dialogs
} // namespace ui
} // namespace RME

// #include "LiveServerDialog.moc" // Removed - Q_OBJECT is in header