#include <QtTest>
#include "ui/widgets/MapViewWidget.h"
#include "ui/widgets/MapView.h"
#include "editor_logic/EditorController.h"
#include "core/settings/AppSettings.h"
#include "core/brush/BrushStateManager.h"

class TestMapViewWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testCreation();
    void testMapViewAccess();
    void testFloorNavigation();
    void testZoomControl();
    void testPositionControl();
    void cleanupTestCase();

private:
    RME::editor_logic::EditorController* m_mockEditorController;
    RME::core::brush::BrushStateManager* m_mockBrushStateManager;
    RME::core::settings::AppSettings* m_mockAppSettings;
};

void TestMapViewWidget::initTestCase()
{
    // Create mock objects
    m_mockEditorController = nullptr; // Would be a mock in a real test
    m_mockBrushStateManager = nullptr; // Would be a mock in a real test
    m_mockAppSettings = nullptr; // Would be a mock in a real test
}

void TestMapViewWidget::testCreation()
{
    // Test that the widget can be created
    RME::ui::widgets::MapViewWidget widget;
    QVERIFY(widget.getMapView() != nullptr);
}

void TestMapViewWidget::testMapViewAccess()
{
    // Test that the MapView can be accessed
    RME::ui::widgets::MapViewWidget widget;
    QVERIFY(widget.getMapView() != nullptr);
}

void TestMapViewWidget::testFloorNavigation()
{
    // Test floor navigation
    RME::ui::widgets::MapViewWidget widget;
    int initialFloor = widget.getCurrentFloor();
    
    // Set a new floor
    int newFloor = initialFloor + 1;
    widget.setCurrentFloor(newFloor);
    QCOMPARE(widget.getCurrentFloor(), newFloor);
}

void TestMapViewWidget::testZoomControl()
{
    // Test zoom control
    RME::ui::widgets::MapViewWidget widget;
    float initialZoom = widget.getZoomLevel();
    
    // Set a new zoom level
    float newZoom = initialZoom * 2.0f;
    widget.setZoomLevel(newZoom);
    QCOMPARE(widget.getZoomLevel(), newZoom);
}

void TestMapViewWidget::testPositionControl()
{
    // Test position control
    RME::ui::widgets::MapViewWidget widget;
    RME::core::Position initialPos = widget.getCurrentPosition();
    
    // Set a new position
    RME::core::Position newPos(100, 100, 7);
    widget.centerOnPosition(newPos);
    
    // The position might not be exactly the same due to rounding
    RME::core::Position resultPos = widget.getCurrentPosition();
    QCOMPARE(resultPos.z, newPos.z);
    // Allow for small rounding differences in x and y
    QVERIFY(abs(resultPos.x - newPos.x) <= 1);
    QVERIFY(abs(resultPos.y - newPos.y) <= 1);
}

void TestMapViewWidget::cleanupTestCase()
{
    // Clean up mock objects
}

QTEST_MAIN(TestMapViewWidget)
#include "TestMapViewWidget.moc"