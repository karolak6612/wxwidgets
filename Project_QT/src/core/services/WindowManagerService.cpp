#include "WindowManagerService.h"
#include "editor_logic/EditorController.h"
#include "ui/widgets/MapView.h"
#include <QMainWindow>
#include <QProgressDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QDebug>

namespace RME {
namespace core {

WindowManagerService::WindowManagerService(QMainWindow* mainWindow, QObject* parent)
    : IWindowManagerService(parent)
    , m_mainWindow(mainWindow)
    , m_progressDialog(nullptr)
    , m_statusBar(nullptr)
    , m_currentEditor(nullptr)
    , m_currentMapView(nullptr)
    , m_baseWindowTitle("Remere's Map Editor")
{
    Q_ASSERT(m_mainWindow);
    initializeUI();
}

WindowManagerService::~WindowManagerService()
{
    if (m_progressDialog) {
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
}

void WindowManagerService::initializeUI()
{
    setupStatusBar();
    
    // Set initial window title
    m_mainWindow->setWindowTitle(m_baseWindowTitle);
}

void WindowManagerService::setupStatusBar()
{
    m_statusBar = m_mainWindow->statusBar();
    if (m_statusBar) {
        m_statusBar->showMessage("Ready", 2000);
    }
}

QMainWindow* WindowManagerService::getMainWindow() const
{
    return m_mainWindow;
}

void WindowManagerService::showErrorDialog(const QString& title, const QString& message)
{
    QMessageBox* msgBox = createMessageBox(title, message, QMessageBox::Critical);
    msgBox->exec();
    msgBox->deleteLater();
}

void WindowManagerService::showInfoDialog(const QString& title, const QString& message)
{
    QMessageBox* msgBox = createMessageBox(title, message, QMessageBox::Information);
    msgBox->exec();
    msgBox->deleteLater();
}

void WindowManagerService::showWarningDialog(const QString& title, const QString& message)
{
    QMessageBox* msgBox = createMessageBox(title, message, QMessageBox::Warning);
    msgBox->exec();
    msgBox->deleteLater();
}

bool WindowManagerService::showConfirmDialog(const QString& title, const QString& message)
{
    QMessageBox msgBox(m_mainWindow);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    int result = msgBox.exec();
    return result == QMessageBox::Yes;
}

void WindowManagerService::updateStatusText(const QString& text)
{
    m_currentStatusText = text;
    if (m_statusBar) {
        m_statusBar->showMessage(text);
    }
    emit statusTextChanged(text);
}

void WindowManagerService::updateWindowTitle(const QString& title)
{
    QString fullTitle = title.isEmpty() ? m_baseWindowTitle : QString("%1 - %2").arg(title, m_baseWindowTitle);
    m_mainWindow->setWindowTitle(fullTitle);
    emit windowTitleChanged(fullTitle);
}

void WindowManagerService::updateMenuBar()
{
    // Force menu bar update
    if (m_mainWindow->menuBar()) {
        m_mainWindow->menuBar()->update();
    }
}

void WindowManagerService::showProgressDialog(const QString& title, const QString& message)
{
    if (m_progressDialog) {
        m_progressDialog->deleteLater();
    }
    
    m_progressDialog = new QProgressDialog(message, "Cancel", 0, 100, m_mainWindow);
    m_progressDialog->setWindowTitle(title);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    m_progressDialog->setValue(0);
    m_progressDialog->show();
    
    // Process events to ensure dialog is shown
    QApplication::processEvents();
}

void WindowManagerService::updateProgress(int value, int maximum)
{
    if (m_progressDialog) {
        m_progressDialog->setMaximum(maximum);
        m_progressDialog->setValue(value);
        
        // Process events to keep UI responsive
        QApplication::processEvents();
        
        // Check if user cancelled
        if (m_progressDialog->wasCanceled()) {
            hideProgressDialog();
        }
    }
}

void WindowManagerService::hideProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->hide();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
}

void WindowManagerService::refreshPalettes()
{
    // This would trigger a refresh of all palette panels
    // Implementation depends on how palettes are managed in the main window
    qDebug() << "WindowManagerService: Refreshing palettes";
    
    // TODO: Implement palette refresh logic
    // This might involve emitting a signal that palette panels listen to
}

void WindowManagerService::showPalette(const QString& paletteName, bool visible)
{
    qDebug() << "WindowManagerService: Setting palette" << paletteName << "visibility to" << visible;
    
    // TODO: Implement palette visibility logic
    // This would involve finding the palette by name and setting its visibility
}

EditorController* WindowManagerService::getCurrentEditor() const
{
    return m_currentEditor;
}

ui::widgets::MapView* WindowManagerService::getCurrentMapView() const
{
    return m_currentMapView;
}

void WindowManagerService::savePerspective(const QString& name)
{
    if (m_mainWindow) {
        QByteArray perspective = m_mainWindow->saveState();
        m_perspectives[name] = perspective;
        qDebug() << "WindowManagerService: Saved perspective" << name;
    }
}

void WindowManagerService::loadPerspective(const QString& name)
{
    if (m_perspectives.contains(name) && m_mainWindow) {
        m_mainWindow->restoreState(m_perspectives[name]);
        emit perspectiveChanged(name);
        qDebug() << "WindowManagerService: Loaded perspective" << name;
    } else {
        qWarning() << "WindowManagerService: Perspective" << name << "not found";
    }
}

QByteArray WindowManagerService::getCurrentPerspective() const
{
    return m_mainWindow ? m_mainWindow->saveState() : QByteArray();
}

void WindowManagerService::restorePerspective(const QByteArray& perspective)
{
    if (m_mainWindow && !perspective.isEmpty()) {
        m_mainWindow->restoreState(perspective);
    }
}

void WindowManagerService::onEditorChanged(EditorController* editor)
{
    if (m_currentEditor != editor) {
        m_currentEditor = editor;
        
        // Update current map view based on editor
        if (editor) {
            // Get map view from editor
            m_currentMapView = editor->getMapView();
        } else {
            m_currentMapView = nullptr;
        }
        
        emit currentEditorChanged(editor);
    }
}

QMessageBox* WindowManagerService::createMessageBox(const QString& title, const QString& message, QMessageBox::Icon icon)
{
    QMessageBox* msgBox = new QMessageBox(m_mainWindow);
    msgBox->setWindowTitle(title);
    msgBox->setText(message);
    msgBox->setIcon(icon);
    msgBox->setStandardButtons(QMessageBox::Ok);
    return msgBox;
}

} // namespace core
} // namespace RME