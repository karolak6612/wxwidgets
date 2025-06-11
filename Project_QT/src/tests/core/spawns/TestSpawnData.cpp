#include <QtTest/QtTest>
#include "core/spawns/SpawnData.h" // Adjust path as per include rules for tests
#include "core/Position.h"    // For Position

class TestSpawnData : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void defaultConstructor();
    void parameterizedConstructor();
    void accessorsAndMutators();
    void creatureTypeManagement();
    void equalityOperators();

private:
    // Helper if needed, or direct use in test cases
};

void TestSpawnData::initTestCase() {
    // qInfo("TestSpawnData: Starting test cases...");
}

void TestSpawnData::cleanupTestCase() {
    // qInfo("TestSpawnData: Finished test cases.");
}

void TestSpawnData::defaultConstructor() {
    RME::SpawnData spawn;
    QCOMPARE(spawn.getCenter(), RME::Position(0, 0, 0)); // Assuming default Position is 0,0,0
    QCOMPARE(spawn.getRadius(), 0);
    QCOMPARE(spawn.getIntervalSeconds(), 60); // Default from YAML
    QVERIFY(spawn.getCreatureTypes().isEmpty());
}

void TestSpawnData::parameterizedConstructor() {
    RME::Position centerPos(10, 20, 7);
    QStringList creatures = {"Dragon", "Demon"};
    RME::SpawnData spawn(centerPos, 5, 120, creatures);

    QCOMPARE(spawn.getCenter(), centerPos);
    QCOMPARE(spawn.getRadius(), 5);
    QCOMPARE(spawn.getIntervalSeconds(), 120);
    QCOMPARE(spawn.getCreatureTypes(), creatures);
}

void TestSpawnData::accessorsAndMutators() {
    RME::SpawnData spawn;
    RME::Position newCenter(100, 100, 7);
    QStringList newCreatures = {"Cyclops", "Hero"};

    spawn.setCenter(newCenter);
    QCOMPARE(spawn.getCenter(), newCenter);

    spawn.setRadius(10);
    QCOMPARE(spawn.getRadius(), 10);

    spawn.setIntervalSeconds(30);
    QCOMPARE(spawn.getIntervalSeconds(), 30);

    spawn.setCreatureTypes(newCreatures);
    QCOMPARE(spawn.getCreatureTypes(), newCreatures);
}

void TestSpawnData::creatureTypeManagement() {
    RME::SpawnData spawn;
    QVERIFY(spawn.getCreatureTypes().isEmpty());

    spawn.addCreatureType("Rat");
    QCOMPARE(spawn.getCreatureTypes().size(), 1);
    QVERIFY(spawn.getCreatureTypes().contains("Rat"));

    spawn.addCreatureType("Bat");
    QCOMPARE(spawn.getCreatureTypes().size(), 2);
    QVERIFY(spawn.getCreatureTypes().contains("Bat"));

    // Adding duplicate should not change anything
    spawn.addCreatureType("Rat");
    QCOMPARE(spawn.getCreatureTypes().size(), 2);

    bool removed = spawn.removeCreatureType("Rat");
    QVERIFY(removed);
    QCOMPARE(spawn.getCreatureTypes().size(), 1);
    QVERIFY(!spawn.getCreatureTypes().contains("Rat"));
    QVERIFY(spawn.getCreatureTypes().contains("Bat"));

    removed = spawn.removeCreatureType("NonExistent");
    QVERIFY(!removed);
    QCOMPARE(spawn.getCreatureTypes().size(), 1);

    removed = spawn.removeCreatureType("Bat");
    QVERIFY(removed);
    QVERIFY(spawn.getCreatureTypes().isEmpty());
}

void TestSpawnData::equalityOperators() {
    RME::Position pos1(1,2,3);
    RME::Position pos2(4,5,6);
    QStringList creatures1 = {"A", "B"};
    QStringList creatures2 = {"C", "D"};

    RME::SpawnData s1(pos1, 1, 10, creatures1);
    RME::SpawnData s2(pos1, 1, 10, creatures1); // Same as s1
    RME::SpawnData s3(pos2, 1, 10, creatures1); // Different position
    RME::SpawnData s4(pos1, 2, 10, creatures1); // Different radius
    RME::SpawnData s5(pos1, 1, 20, creatures1); // Different interval
    RME::SpawnData s6(pos1, 1, 10, creatures2); // Different creatures

    QVERIFY(s1 == s2);
    QVERIFY(s1 != s3);
    QVERIFY(s1 != s4);
    QVERIFY(s1 != s5);
    QVERIFY(s1 != s6);
    QVERIFY(!(s1 == s3));
}

// QTEST_APPLESS_MAIN(TestSpawnData)
// #include "TestSpawnData.moc" // Include at the end if not using QTEST_APPLESS_MAIN for a common runner
