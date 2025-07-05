#ifndef RME_IWINDOWMANAGERSERVICE_H
#define RME_IWINDOWMANAGERSERVICE_H

#include <QObject>
#include <QString>
#include <QByteArray>

class QMainWindow;
class QProgressDialog;

namespace RME {
namespace core {

class EditorController;

namespace ui {
namespace widgets {
    class MapView;
}
}

/**
 * @brief Interface for window and UI management service
 * 
 * This interface defines the contract for managing UI elements including
 * dialogs, status updates, progress handling, and palette management.
 */
class IWindowManagerService : public QObject {
    Q_OBJECT

public:
    virtual ~IWindowManagerService() = default;

    // Main window access
    virtual QMainWindow* getMainWindow() const = 0;
    
    // Dialog management
    virtual void showErrorDialog(const QString& title, const QString& message) = 0;
    virtual void showInfoDialog(const QString& title, const QString& message) = 0;
    virtual void showWarningDialog(const QString& title, const QString& message) = 0;
    virtual bool showConfirmDialog(const QString& title, const QString& message) = 0;
    
    // Status and title updates
    virtual void updateStatusText(const QString& text) = 0;
    virtual void updateWindowTitle(const QString& title) = 0;
    virtual void updateMenuBar() = 0;
    
    // Progress management
    virtual void showProgressDialog(const QString& title, const QString& message) = 0;
    virtual void updateProgress(int value, int maximum) = 0;
    virtual void hideProgressDialog() = 0;
    
    // Palette management
    virtual void refreshPalettes() = 0;
    virtual void showPalette(const QString& paletteName, bool visible) = 0;
    
    // Editor tabs
    virtual EditorController* getCurrentEditor() const = 0;
    virtual ui::widgets::MapView* getCurrentMapView() const = 0;
    
    // Perspective management
    virtual void savePerspective(const QString& name) = 0;
    virtual void loadPerspective(const QString& name) = 0;
    virtual QByteArray getCurrentPerspective() const = 0;
    virtual void restorePerspective(const QByteArray& perspective) = 0;

signals:
    void currentEditorChanged(EditorController* editor);
    void perspectiveChanged(const QString& name);
    void statusTextChanged(const QString& text);
    void windowTitleChanged(const QString& title);
};

} // namespace core
} // namespace RME

#endif // RME_IWINDOWMANAGERSERVICE_H