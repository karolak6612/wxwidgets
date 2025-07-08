#ifndef RME_ABOUT_DIALOG_H
#define RME_ABOUT_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTabWidget>
#include <QScrollArea>
#include <QApplication>

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog displaying application information, credits, and license
 * 
 * This dialog shows the application name, version, Qt version, credits,
 * license information, and optionally includes easter egg games.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override = default;

private slots:
    void onLicenseButtonClicked();
    void onCreditsButtonClicked();

private:
    void setupUI();
    void createMainTab();
    void createCreditsTab();
    void createLicenseTab();
    void createSystemInfoTab();
    
    QString getApplicationInfo() const;
    QString getSystemInfo() const;
    QString getCreditsText() const;
    QString getLicenseText() const;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Main tab
    QWidget* m_mainTab;
    QLabel* m_logoLabel;
    QLabel* m_titleLabel;
    QLabel* m_versionLabel;
    QLabel* m_qtVersionLabel;
    QLabel* m_descriptionLabel;
    
    // Credits tab
    QWidget* m_creditsTab;
    QTextBrowser* m_creditsText;
    
    // License tab
    QWidget* m_licenseTab;
    QTextBrowser* m_licenseText;
    
    // System info tab
    QWidget* m_systemInfoTab;
    QTextBrowser* m_systemInfoText;
    
    // Button layout
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_ABOUT_DIALOG_H