#include <QtTest>
#include "ui/widgets/MinimapViewWidget.h"

class TestMinimapViewWidget : public QObject
{
    Q_OBJECT

private slots:
    void testCreation();
    void testResizing();
    void testMouseEvents();
};

void TestMinimapViewWidget::testCreation()
{
    // Create a minimal widget for testing
    RME::ui::widgets::MinimapViewWidget widget(nullptr, nullptr);
    
    // Verify initial state
    QVERIFY(widget.size().width() >= 200);
    QVERIFY(widget.size().height() >= 200);
}

void TestMinimapViewWidget::testResizing()
{
    // Create a minimal widget for testing
    RME::ui::widgets::MinimapViewWidget widget(nullptr, nullptr);
    
    // Test resizing
    widget.resize(300, 300);
    QCOMPARE(widget.size(), QSize(300, 300));
}

void TestMinimapViewWidget::testMouseEvents()
{
    // Create a minimal widget for testing
    RME::ui::widgets::MinimapViewWidget widget(nullptr, nullptr);
    
    // Connect to the navigationRequested signal
    QSignalSpy spy(&widget, SIGNAL(navigationRequested(const RME::core::Position&)));
    
    // Simulate a mouse click
    QTest::mouseClick(&widget, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    
    // Verify that the signal was emitted
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestMinimapViewWidget)
#include "TestMinimapViewWidget.moc"