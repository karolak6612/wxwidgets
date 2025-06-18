#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>

#include "ui/dialogs/LiveServerControlPanelQt.h"
#include "ui/dialogs/LiveServerDialog.h"

class TestUI09Components : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testLiveServerControlPanelCreation();
    void testLiveServerDialogCreation();
    void testLiveServerControlPanelUI();
    void testLiveServerDialogUI();
    void testServerStateManagement();
    void testClientListManagement();
    void testChatFunctionality();
    void testSettingsPersistence();

private:
    QWidget* m_testWidget = nullptr;
};

void TestUI09Components::initTestCase()
{
    m_testWidget = new QWidget();
}

void TestUI09Components::cleanupTestCase()
{
    delete m_testWidget;
}

void TestUI09Components::testLiveServerControlPanelCreation()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    QVERIFY(panel != nullptr);
    QVERIFY(panel->parent() == m_testWidget);
    QVERIFY(!panel->isServerRunning());
    QCOMPARE(panel->getCurrentPort(), quint16(0));
    
    delete panel;
}

void TestUI09Components::testLiveServerDialogCreation()
{
    RME::ui::dialogs::LiveServerDialog* dialog = 
        new RME::ui::dialogs::LiveServerDialog(m_testWidget);
    
    QVERIFY(dialog != nullptr);
    QVERIFY(dialog->parent() == m_testWidget);
    QCOMPARE(dialog->windowTitle(), QString("Live Server Control Panel"));
    QVERIFY(dialog->getControlPanel() != nullptr);
    
    delete dialog;
}

void TestUI09Components::testLiveServerControlPanelUI()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    // Find UI components by object name
    QGroupBox* configGroup = panel->findChild<QGroupBox*>("configGroup");
    QSpinBox* portSpinBox = panel->findChild<QSpinBox*>("portSpinBox");
    QLineEdit* passwordEdit = panel->findChild<QLineEdit*>("passwordEdit");
    QPushButton* startButton = panel->findChild<QPushButton*>("startServerButton");
    QPushButton* stopButton = panel->findChild<QPushButton*>("stopServerButton");
    QListView* clientList = panel->findChild<QListView*>("clientListView");
    QTextEdit* logEdit = panel->findChild<QTextEdit*>("logTextEdit");
    QLineEdit* chatEdit = panel->findChild<QLineEdit*>("chatInputEdit");
    QPushButton* sendButton = panel->findChild<QPushButton*>("sendChatButton");
    
    QVERIFY(configGroup != nullptr);
    QVERIFY(portSpinBox != nullptr);
    QVERIFY(passwordEdit != nullptr);
    QVERIFY(startButton != nullptr);
    QVERIFY(stopButton != nullptr);
    QVERIFY(clientList != nullptr);
    QVERIFY(logEdit != nullptr);
    QVERIFY(chatEdit != nullptr);
    QVERIFY(sendButton != nullptr);
    
    // Test initial states
    QVERIFY(startButton->isEnabled());
    QVERIFY(!stopButton->isEnabled());
    QVERIFY(!chatEdit->isEnabled());
    QVERIFY(!sendButton->isEnabled());
    QVERIFY(logEdit->isReadOnly());
    QCOMPARE(passwordEdit->echoMode(), QLineEdit::Password);
    
    // Test port range
    QVERIFY(portSpinBox->minimum() >= 1);
    QVERIFY(portSpinBox->maximum() <= 65535);
    
    delete panel;
}

void TestUI09Components::testLiveServerDialogUI()
{
    RME::ui::dialogs::LiveServerDialog* dialog = 
        new RME::ui::dialogs::LiveServerDialog(m_testWidget);
    
    // Verify the control panel is embedded
    auto* controlPanel = dialog->getControlPanel();
    QVERIFY(controlPanel != nullptr);
    QVERIFY(controlPanel->parent() == dialog);
    
    // Test minimum size
    QVERIFY(dialog->minimumSize().width() >= 600);
    QVERIFY(dialog->minimumSize().height() >= 700);
    
    delete dialog;
}

void TestUI09Components::testServerStateManagement()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    // Test initial state
    QVERIFY(!panel->isServerRunning());
    
    // Test signal emission
    QSignalSpy stateSpy(panel, &RME::ui::dialogs::LiveServerControlPanelQt::serverStateChanged);
    QSignalSpy startSpy(panel, &RME::ui::dialogs::LiveServerControlPanelQt::serverStartRequested);
    QSignalSpy stopSpy(panel, &RME::ui::dialogs::LiveServerControlPanelQt::serverStopRequested);
    
    // Find buttons
    QPushButton* startButton = panel->findChild<QPushButton*>("startServerButton");
    QPushButton* stopButton = panel->findChild<QPushButton*>("stopServerButton");
    
    QVERIFY(startButton != nullptr);
    QVERIFY(stopButton != nullptr);
    
    // Test start button click
    startButton->click();
    QCOMPARE(startSpy.count(), 1);
    
    delete panel;
}

void TestUI09Components::testClientListManagement()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    // Test adding clients
    panel->onClientConnected("TestClient1", 1);
    panel->onClientConnected("TestClient2", 2);
    
    // Find client list components
    QListView* clientList = panel->findChild<QListView*>("clientListView");
    QLabel* clientCount = panel->findChild<QLabel*>("clientCountLabel");
    
    QVERIFY(clientList != nullptr);
    QVERIFY(clientCount != nullptr);
    
    // Verify client count is updated
    QVERIFY(clientCount->text().contains("2"));
    
    // Test removing clients
    panel->onClientDisconnected("TestClient1", 1);
    QVERIFY(clientCount->text().contains("1"));
    
    delete panel;
}

void TestUI09Components::testChatFunctionality()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    // Test chat message reception
    panel->onChatMessageReceived("TestUser", "Hello World!");
    
    QTextEdit* logEdit = panel->findChild<QTextEdit*>("logTextEdit");
    QVERIFY(logEdit != nullptr);
    
    QString logContent = logEdit->toPlainText();
    QVERIFY(logContent.contains("TestUser"));
    QVERIFY(logContent.contains("Hello World!"));
    
    delete panel;
}

void TestUI09Components::testSettingsPersistence()
{
    RME::ui::dialogs::LiveServerControlPanelQt* panel = 
        new RME::ui::dialogs::LiveServerControlPanelQt(m_testWidget);
    
    // Find port and password controls
    QSpinBox* portSpinBox = panel->findChild<QSpinBox*>("portSpinBox");
    QLineEdit* passwordEdit = panel->findChild<QLineEdit*>("passwordEdit");
    
    QVERIFY(portSpinBox != nullptr);
    QVERIFY(passwordEdit != nullptr);
    
    // Test setting values
    portSpinBox->setValue(12345);
    passwordEdit->setText("testpass");
    
    // Test save settings (should not crash)
    panel->saveSettings();
    
    delete panel;
}

QTEST_MAIN(TestUI09Components)
#include "TestUI09Components.moc"