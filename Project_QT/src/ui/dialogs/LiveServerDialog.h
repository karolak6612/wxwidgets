#ifndef RME_LIVE_SERVER_DIALOG_H
#define RME_LIVE_SERVER_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

// Forward declarations
namespace RME {
namespace ui {
namespace dialogs {
    class LiveServerControlPanelQt;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog wrapper for the Live Server Control Panel
 * 
 * Provides a modal or non-modal dialog interface for the LiveServerControlPanelQt.
 * This can be used when the control panel needs to be shown as a standalone dialog
 * rather than embedded in a dock widget.
 */
class LiveServerDialog : public QDialog {
    Q_OBJECT

public:
    explicit LiveServerDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~LiveServerDialog() override = default;

    // Access to the control panel
    LiveServerControlPanelQt* getControlPanel() const { return m_controlPanel; }

    // Dialog management
    void showAsModal();
    void showAsNonModal();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onServerStateChanged(bool isRunning);

private:
    void setupUI();
    void connectSignals();

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    LiveServerControlPanelQt* m_controlPanel = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_LIVE_SERVER_DIALOG_H