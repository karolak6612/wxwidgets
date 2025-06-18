#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QTabWidget>
#include <QUndoStack>

#include "ui/EditorInstanceWidget.h"
#include "ui/MainWindow.h"
#include "core/map/Map.h"
#include "core/Position.h"
#include "tests/core/MockItemTypeProvider.h"

class TestUIEditorWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testEditorInstanceWidgetCreation();
    void testEditorInstanceWidgetFileHandling();
    void testEditorInstanceWidgetModification();
    void testMainWindowTabIntegration();
    void testTabManagement();
    void testWindowTitleUpdates();

private:
    QWidget* m_testWidget = nullptr;
    MockItemTypeProvider* m_mockProvider = nullptr;
    RME::core::Map* m_testMap = nullptr;
};

void TestUIEditorWindow::initTestCase()
{
    m_testWidget = new QWidget();
    m_mockProvider = new MockItemTypeProvider();
    m_testMap = new RME::core::Map(m_mockProvider);
}

void TestUIEditorWindow::cleanupTestCase()
{
    delete m_testWidget;
    delete m_testMap;
    delete m_mockProvider;
}

void TestUIEditorWindow::testEditorInstanceWidgetCreation()
{
    RME::ui::EditorInstanceWidget* editorInstance = 
        new RME::ui::EditorInstanceWidget(m_testMap, "test.otbm", m_testWidget);
    
    QVERIFY(editorInstance != nullptr);
    QVERIFY(editorInstance->parent() == m_testWidget);
    QVERIFY(editorInstance->getMap() == m_testMap);
    QVERIFY(editorInstance->getMapView() != nullptr);
    QVERIFY(editorInstance->getEditorController() != nullptr);
    QVERIFY(editorInstance->getUndoStack() != nullptr);
    
    delete editorInstance;
}

void TestUIEditorWindow::testEditorInstanceWidgetFileHandling()
{
    RME::ui::EditorInstanceWidget* editorInstance = 
        new RME::ui::EditorInstanceWidget(m_testMap, "test.otbm", m_testWidget);
    
    // Test file path handling
    QCOMPARE(editorInstance->getFilePath(), QString("test.otbm"));
    QVERIFY(!editorInstance->isUntitled());
    
    // Test untitled map
    RME::ui::EditorInstanceWidget* untitledInstance = 
        new RME::ui::EditorInstanceWidget(m_testMap, "", m_testWidget);
    
    QVERIFY(untitledInstance->isUntitled());
    QVERIFY(untitledInstance->getDisplayName().contains("Untitled"));
    
    // Test file path change
    editorInstance->setFilePath("newfile.otbm");
    QCOMPARE(editorInstance->getFilePath(), QString("newfile.otbm"));
    
    delete editorInstance;
    delete untitledInstance;
}

void TestUIEditorWindow::testEditorInstanceWidgetModification()
{
    RME::ui::EditorInstanceWidget* editorInstance = 
        new RME::ui::EditorInstanceWidget(m_testMap, "test.otbm", m_testWidget);
    
    // Test initial state
    QVERIFY(!editorInstance->isModified());
    
    // Test modification signal
    QSignalSpy modificationSpy(editorInstance, &RME::ui::EditorInstanceWidget::modificationChanged);
    QSignalSpy displayNameSpy(editorInstance, &RME::ui::EditorInstanceWidget::displayNameChanged);
    
    // Simulate modification
    editorInstance->onMapModified();
    
    QVERIFY(editorInstance->isModified());
    QCOMPARE(modificationSpy.count(), 1);
    QCOMPARE(displayNameSpy.count(), 1);
    
    // Check that display name includes asterisk
    QString displayName = editorInstance->getDisplayName();
    QVERIFY(displayName.endsWith("*"));
    
    delete editorInstance;
}

void TestUIEditorWindow::testMainWindowTabIntegration()
{
    RME::ui::MainWindow* mainWindow = new RME::ui::MainWindow();
    
    // Find the tab widget
    QTabWidget* tabWidget = mainWindow->findChild<QTabWidget*>();
    QVERIFY(tabWidget != nullptr);
    
    // Test initial state
    QCOMPARE(tabWidget->count(), 0);
    QVERIFY(tabWidget->isTabsClosable());
    QVERIFY(tabWidget->isMovable());
    
    delete mainWindow;
}

void TestUIEditorWindow::testTabManagement()
{
    RME::ui::MainWindow* mainWindow = new RME::ui::MainWindow();
    QTabWidget* tabWidget = mainWindow->findChild<QTabWidget*>();
    
    QVERIFY(tabWidget != nullptr);
    
    // Test that we can access the tab management methods
    // Note: These are private methods, so we test indirectly through public interface
    
    // Test window title updates
    QString initialTitle = mainWindow->windowTitle();
    QVERIFY(initialTitle.contains("Remere's Map Editor"));
    
    delete mainWindow;
}

void TestUIEditorWindow::testWindowTitleUpdates()
{
    RME::ui::EditorInstanceWidget* editorInstance = 
        new RME::ui::EditorInstanceWidget(m_testMap, "test.otbm", m_testWidget);
    
    // Test display name changes
    QSignalSpy displayNameSpy(editorInstance, &RME::ui::EditorInstanceWidget::displayNameChanged);
    
    // Change file path
    editorInstance->setFilePath("newfile.otbm");
    QCOMPARE(displayNameSpy.count(), 1);
    
    // Modify the map
    editorInstance->onMapModified();
    QCOMPARE(displayNameSpy.count(), 2);
    
    // Check display name format
    QString displayName = editorInstance->getDisplayName();
    QVERIFY(displayName.contains("newfile.otbm"));
    QVERIFY(displayName.endsWith("*"));
    
    delete editorInstance;
}

QTEST_MAIN(TestUIEditorWindow)
#include "TestUIEditorWindow.moc"