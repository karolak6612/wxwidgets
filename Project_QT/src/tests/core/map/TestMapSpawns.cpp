#include <QtTest/QtTest>
#include "core/map/Map.h" // Adjust path
#include "core/spawns/SpawnData.h" // Adjust path
#include "core/Position.h"
// #include "core/assets/AssetManager.h" // For RME::core::assets::AssetManager if not passing nullptr

// If a full AssetManager is not needed and Map constructor can take nullptr:
namespace RME { namespace core { namespace assets { class AssetManager; } } }

class TestMapSpawns : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init(); // Per-test setup
    void cleanup(); // Per-test teardown

    void addAndGetSpawns();
    void removeSpawns();
    void getSpawnsConstness();

private:
    RME::Map* mapInstance = nullptr;
    // RME::core::assets::AssetManager* mockAssetManager = nullptr; // Example if needed
};

void TestMapSpawns::initTestCase() {
    // qInfo("TestMapSpawns: Starting test cases...");
    // mockAssetManager = new RME::core::assets::AssetManager(...); // If needed
}

void TestMapSpawns::cleanupTestCase() {
    // qInfo("TestMapSpawns: Finished test cases.");
    // delete mockAssetManager;
    // mockAssetManager = nullptr;
}

void TestMapSpawns::init() {
    // Pass nullptr for AssetManager if the tested spawn logic doesn't rely on it.
    // Otherwise, a mock or minimal instance of RME::core::assets::AssetManager is required.
    mapInstance = new RME::Map(100, 100, 1, nullptr /*mockAssetManager*/);
}

void TestMapSpawns::cleanup() {
    delete mapInstance;
    mapInstance = nullptr;
}

void TestMapSpawns::addAndGetSpawns() {
    QVERIFY(mapInstance->getSpawns().isEmpty());

    RME::SpawnData spawn1_orig(RME::Position(10,10,7), 1, 30, {"Wolf"});
    mapInstance->addSpawn(RME::SpawnData(spawn1_orig)); // Add a copy, then move the temporary

    QCOMPARE(mapInstance->getSpawns().size(), 1);
    QCOMPARE(mapInstance->getSpawns().first(), spawn1_orig); // Compare with original

    RME::SpawnData spawn2_orig(RME::Position(20,20,7), 2, 60, {"Bear", "Wolf"});
    mapInstance->addSpawn(RME::SpawnData(spawn2_orig)); // Add a copy, then move the temporary

    QCOMPARE(mapInstance->getSpawns().size(), 2);
    QVERIFY(mapInstance->getSpawns().contains(spawn1_orig));
    QVERIFY(mapInstance->getSpawns().contains(spawn2_orig));
}

void TestMapSpawns::removeSpawns() {
    RME::SpawnData spawn1(RME::Position(10,10,7), 1, 30, {"Wolf"});
    RME::SpawnData spawn2(RME::Position(20,20,7), 2, 60, {"Bear"});
    RME::SpawnData spawn3(RME::Position(30,30,7), 3, 90, {"Dragon"});

    mapInstance->addSpawn(RME::SpawnData(spawn1)); // Add copies
    mapInstance->addSpawn(RME::SpawnData(spawn2));
    mapInstance->addSpawn(RME::SpawnData(spawn3));
    QCOMPARE(mapInstance->getSpawns().size(), 3);

    bool removed = mapInstance->removeSpawn(spawn2); // Remove by value
    QVERIFY(removed);
    QCOMPARE(mapInstance->getSpawns().size(), 2);
    QVERIFY(mapInstance->getSpawns().contains(spawn1));
    QVERIFY(!mapInstance->getSpawns().contains(spawn2));
    QVERIFY(mapInstance->getSpawns().contains(spawn3));

    removed = mapInstance->removeSpawn(RME::SpawnData(RME::Position(5,5,5),0,0,{})); // Try removing non-existent
    QVERIFY(!removed);
    QCOMPARE(mapInstance->getSpawns().size(), 2);

    removed = mapInstance->removeSpawn(spawn1);
    QVERIFY(removed);
    QCOMPARE(mapInstance->getSpawns().size(), 1);
    QVERIFY(mapInstance->getSpawns().contains(spawn3));

    removed = mapInstance->removeSpawn(spawn3);
    QVERIFY(removed);
    QVERIFY(mapInstance->getSpawns().isEmpty());
}

void TestMapSpawns::getSpawnsConstness() {
    RME::SpawnData spawn1_orig(RME::Position(10,10,7), 1, 30, {"ConstTest"});
    mapInstance->addSpawn(RME::SpawnData(spawn1_orig));

    const RME::Map* constMap = mapInstance;
    const QList<RME::SpawnData>& constSpawns = constMap->getSpawns();
    QCOMPARE(constSpawns.size(), 1);
    QCOMPARE(constSpawns.first(), spawn1_orig);

    // Example of checking if modification is disallowed (would be a compile-time check ideally)
    // For runtime, you could try QVERIFY_THROWS or check if a non-const method is callable.
    // However, QList::first() returns a const reference if QList is const, so direct modification
    // like constSpawns.first().setRadius(100); would fail to compile.
}

// QTEST_APPLESS_MAIN(TestMapSpawns)
// #include "TestMapSpawns.moc" // Include at the end if not using QTEST_APPLESS_MAIN for a common runner
