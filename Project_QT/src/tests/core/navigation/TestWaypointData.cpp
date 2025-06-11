#include <QtTest/QtTest>
#include "core/navigation/WaypointData.h"
#include "core/Position.h"
#include <QString>
#include <QSet>

class TestWaypointData : public QObject
{
    Q_OBJECT

public:
    TestWaypointData();
    ~TestWaypointData() override;

private slots:
    void testDefaultConstructor();
    void testParameterizedConstructor();
    void testCopyConstructor();
    void testMoveConstructor();
    void testCopyAssignment();
    void testMoveAssignment();
    void testConnectionManagement();
    void testPropertyGetters();
    void testComparisonOperators();

};

TestWaypointData::TestWaypointData() {}
TestWaypointData::~TestWaypointData() {}

void TestWaypointData::testDefaultConstructor() {
    RME::core::navigation::WaypointData wp;
    QVERIFY(wp.name.isEmpty());
    QCOMPARE(wp.position, RME::core::Position(0,0,0)); // Assuming default Position is 0,0,0
    QVERIFY(wp.connectedWaypointNames.isEmpty());
    QVERIFY(wp.getConnections().isEmpty());
}

void TestWaypointData::testParameterizedConstructor() {
    RME::core::Position pos(10, 20, 7);
    RME::core::navigation::WaypointData wp("TestWP", pos);
    QCOMPARE(wp.name, QString("TestWP"));
    QCOMPARE(wp.position, pos);
    QVERIFY(wp.connectedWaypointNames.isEmpty());
}

void TestWaypointData::testCopyConstructor() {
    RME::core::Position pos(10, 20, 7);
    RME::core::navigation::WaypointData original("OriginalWP", pos);
    original.addConnection("OtherWP1");

    RME::core::navigation::WaypointData copy(original);
    QCOMPARE(copy.name, original.name);
    QCOMPARE(copy.position, original.position);
    QCOMPARE(copy.connectedWaypointNames, original.connectedWaypointNames);
    QVERIFY(copy == original);
}

void TestWaypointData::testMoveConstructor() {
    RME::core::Position pos(10, 20, 7);
    RME::core::navigation::WaypointData original("OriginalWP", pos);
    original.addConnection("OtherWP1");

    // Save state for comparison
    QString origName = original.name;
    RME::core::Position origPos = original.position;
    QSet<QString> origConnections = original.connectedWaypointNames;

    RME::core::navigation::WaypointData moved(std::move(original));
    QCOMPARE(moved.name, origName);
    QCOMPARE(moved.position, origPos);
    QCOMPARE(moved.connectedWaypointNames, origConnections);

    // Original should be in a valid but unspecified state (likely empty for strings/sets)
    QVERIFY(original.name.isEmpty() || original.name != origName); // Name might be empty orimplementation-defined
    QVERIFY(original.connectedWaypointNames.isEmpty());
}

void TestWaypointData::testCopyAssignment() {
    RME::core::Position pos1(10, 20, 7);
    RME::core::navigation::WaypointData original("OriginalWP", pos1);
    original.addConnection("OtherWP1");

    RME::core::Position pos2(30, 40, 5);
    RME::core::navigation::WaypointData copyTarget("TargetWP", pos2);
    copyTarget.addConnection("TargetConnection");

    copyTarget = original; // Copy assignment

    QCOMPARE(copyTarget.name, original.name);
    QCOMPARE(copyTarget.position, original.position);
    QCOMPARE(copyTarget.connectedWaypointNames, original.connectedWaypointNames);
    QVERIFY(copyTarget == original);
}

void TestWaypointData::testMoveAssignment() {
    RME::core::Position pos1(10, 20, 7);
    RME::core::navigation::WaypointData original("OriginalWP", pos1);
    original.addConnection("OtherWP1");

    // Save state for comparison
    QString origName = original.name;
    RME::core::Position origPos = original.position;
    QSet<QString> origConnections = original.connectedWaypointNames;

    RME::core::Position pos2(30, 40, 5);
    RME::core::navigation::WaypointData moveTarget("TargetWP", pos2);
    moveTarget.addConnection("TargetConnection");

    moveTarget = std::move(original); // Move assignment

    QCOMPARE(moveTarget.name, origName);
    QCOMPARE(moveTarget.position, origPos);
    QCOMPARE(moveTarget.connectedWaypointNames, origConnections);

    QVERIFY(original.name.isEmpty() || original.name != origName);
    QVERIFY(original.connectedWaypointNames.isEmpty());
}

void TestWaypointData::testConnectionManagement() {
    RME::core::navigation::WaypointData wp("WP_A", {1,1,1});
    QVERIFY(wp.getConnections().isEmpty());

    wp.addConnection("WP_B");
    QVERIFY(wp.isConnectedTo("WP_B"));
    QCOMPARE(wp.getConnections().size(), 1);
    QVERIFY(wp.getConnections().contains("WP_B"));

    wp.addConnection("WP_C");
    QVERIFY(wp.isConnectedTo("WP_B"));
    QVERIFY(wp.isConnectedTo("WP_C"));
    QCOMPARE(wp.getConnections().size(), 2);

    // Add duplicate - QSet should handle this, size remains 2
    wp.addConnection("WP_B");
    QCOMPARE(wp.getConnections().size(), 2);

    // Add self - should be prevented
    wp.addConnection("WP_A");
    QVERIFY(!wp.isConnectedTo("WP_A"));
    QCOMPARE(wp.getConnections().size(), 2);

    // Add empty string - should be prevented
    wp.addConnection("");
    QVERIFY(!wp.isConnectedTo(""));
    QCOMPARE(wp.getConnections().size(), 2);

    wp.removeConnection("WP_B");
    QVERIFY(!wp.isConnectedTo("WP_B"));
    QVERIFY(wp.isConnectedTo("WP_C"));
    QCOMPARE(wp.getConnections().size(), 1);

    // Remove non-existent
    wp.removeConnection("WP_DNE");
    QCOMPARE(wp.getConnections().size(), 1);

    wp.removeConnection("WP_C");
    QVERIFY(wp.getConnections().isEmpty());
}

void TestWaypointData::testPropertyGetters() {
    RME::core::Position pos(100, 200, 7);
    RME::core::navigation::WaypointData wp("MyWaypoint", pos);
    wp.addConnection("LinkedWP");

    QCOMPARE(wp.name, QString("MyWaypoint"));
    QCOMPARE(wp.position, pos);

    const QSet<QString>& connections = wp.getConnections();
    QCOMPARE(connections.size(), 1);
    QVERIFY(connections.contains("LinkedWP"));
}

void TestWaypointData::testComparisonOperators() {
    RME::core::Position p1(1,2,3), p2(4,5,6);
    RME::core::navigation::WaypointData wp1("WP1", p1);
    wp1.addConnection("WP_Link1");

    RME::core::navigation::WaypointData wp2("WP1", p1); // Same as wp1 before its connection
    wp2.addConnection("WP_Link1"); // Now fully same as wp1

    RME::core::navigation::WaypointData wp3("WP_DifferentName", p1);
    wp3.addConnection("WP_Link1");

    RME::core::navigation::WaypointData wp4("WP1", p2);
    wp4.addConnection("WP_Link1");

    RME::core::navigation::WaypointData wp5("WP1", p1);
    wp5.addConnection("WP_DifferentLink");

    RME::core::navigation::WaypointData wp6("WP1", p1); // wp1 without connections

    QVERIFY(wp1 == wp2);
    QVERIFY(!(wp1 != wp2));

    QVERIFY(wp1 != wp3); // Different name
    QVERIFY(wp1 != wp4); // Different position
    QVERIFY(wp1 != wp5); // Different connections
    QVERIFY(wp1 != wp6); // wp1 has connection, wp6 does not
}


// QTEST_MAIN(TestWaypointData) // Will be run by a main test runner
#include "TestWaypointData.moc" // Must be last line for MOC to work
