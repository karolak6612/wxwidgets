#ifndef RME_WINDOWMANAGERSERVICE_H
#define RME_WINDOWMANAGERSERVICE_H

#include "IWindowManagerService.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>

class QMainWindow;
class QProgressDialog;
class QMessageBox;
class QStatusBar;

namespace RME {
namespace core {

class EditorController;

namespace ui {
namespace widgets {
    class MapView;
}
}

/**
 * @brief Service for managing window and UI operations
 * 
 * This service centralizes all UI management operations including
 * dialogs, status updates, progress handling, and palette management.
 */
class WindowManagerService : public IWindowManagerService
{
    Q_OBJECT

public:
    explicit WindowManagerService(QMainWindow* mainWindow, QObject* parent = nullptr);
    ~WindowManagerService();

    // IWindowManagerService implementation
    QMainWindow* getMainWindow() const override;
    
    void showErrorDialog(const QString& title, const QString& message) override;
    void showInfoDialog(const QString& title, const QString& message) override;
    void showWarningDialog(const QString& title, const QString& message) override;
    bool showConfirmDialog(const QString& title, const QString& message) override;
    
    void updateStatusText(const QString& text) override;
    void updateWindowTitle(const QString& title) override;
    void updateMenuBar() override;
    
    void showProgressDialog(const QString& title, const QString& message) override;
    void updateProgress(int value, int maximum) override;
    void hideProgressDialog() override;
    
    void refreshPalettes() override;
    void showPalette(const QString& paletteName, bool visible) override;
    
    EditorController* getCurrentEditor() const override;
    ui::widgets::MapView* getCurrentMapView() const override;
    
    void savePerspective(const QString& name) override;
    void loadPerspective(const QString& name) override;
    QByteArray getCurrentPerspective() const override;
    void restorePerspective(const QByteArray& perspective) override;

public slots:
    void onEditorChanged(EditorController* editor);

private:
    void initializeUI();
    void setupStatusBar();
    QMessageBox* createMessageBox(const QString& title, const QString& message, QMessageBox::Icon icon);

private:
    QMainWindow* m_mainWindow;
    QProgressDialog* m_progressDialog;
    QStatusBar* m_statusBar;
    EditorController* m_currentEditor;
    ui::widgets::MapView* m_currentMapView;
    QMap<QString, QByteArray> m_perspectives;
    QString m_currentStatusText;
    QString m_baseWindowTitle;
};

} // namespace core
} // namespace RME

#endif // RME_WINDOWMANAGERSERVICE_H