#include <QtTest/QtTest>
#include "core/map/Map.h"
#include "core/world/TownData.h"
#include "core/Position.h"
// AssetManager might not be needed if Map constructor accepts nullptr
// #include "core/assets/AssetManager.h"

class TestMapTowns : public QObject {
    Q_OBJECT
private:
    RME::Map* mapInstance = nullptr;

private slots:
    void init() {
        // Assuming Map constructor can take nullptr for AssetManager
        // if town logic doesn't directly depend on it.
        mapInstance = new RME::Map(100, 100, 1, nullptr);
    }
    void cleanup() {
        delete mapInstance;
        mapInstance = nullptr;
    }

    void addAndGetTown() {
        QVERIFY(mapInstance->getTownsById().isEmpty());

        RME::TownData town1(1, "TownOne", RME::Position(10,10,7));
        QVERIFY(mapInstance->addTown(RME::TownData(town1))); // Add a copy

        QCOMPARE(mapInstance->getTownsById().size(), 1);
        const RME::TownData* retrievedTown1 = mapInstance->getTown(1);
        QVERIFY(retrievedTown1 != nullptr);
        QCOMPARE(*retrievedTown1, town1);

        // Try adding duplicate ID
        RME::TownData town1Dup(1, "TownOneDup", RME::Position(10,10,7));
        QVERIFY(!mapInstance->addTown(RME::TownData(town1Dup)));
        QCOMPARE(mapInstance->getTownsById().size(), 1); // Size should not change

        // Try adding town with ID 0
        RME::TownData townZeroId(0, "TownZero", RME::Position(1,1,1));
        QVERIFY(!mapInstance->addTown(RME::TownData(townZeroId)));
        QCOMPARE(mapInstance->getTownsById().size(), 1);


        RME::TownData town2(2, "TownTwo", RME::Position(20,20,7));
        QVERIFY(mapInstance->addTown(RME::TownData(town2)));
        QCOMPARE(mapInstance->getTownsById().size(), 2);

        const RME::TownData* retrievedTown2 = mapInstance->getTown(2);
        QVERIFY(retrievedTown2 != nullptr);
        QCOMPARE(*retrievedTown2, town2);

        // Check const getter
        const RME::Map* constMap = mapInstance;
        const QMap<uint32_t, RME::TownData>& townsMap = constMap->getTownsById();
        QCOMPARE(townsMap.size(), 2);
        QVERIFY(townsMap.contains(1));
        QVERIFY(townsMap.contains(2));
    }

    void removeTown() {
        RME::TownData town1(1, "T1", RME::Position(1,1,7));
        RME::TownData town2(2, "T2", RME::Position(2,2,7));
        mapInstance->addTown(RME::TownData(town1));
        mapInstance->addTown(RME::TownData(town2));
        QCOMPARE(mapInstance->getTownsById().size(), 2);

        QVERIFY(mapInstance->removeTown(1));
        QCOMPARE(mapInstance->getTownsById().size(), 1);
        QVERIFY(mapInstance->getTown(1) == nullptr);
        QVERIFY(mapInstance->getTown(2) != nullptr);

        QVERIFY(!mapInstance->removeTown(1)); // Already removed
        QCOMPARE(mapInstance->getTownsById().size(), 1);

        QVERIFY(mapInstance->removeTown(2));
        QVERIFY(mapInstance->getTownsById().isEmpty());
    }

    void getUnusedTownId() {
        QCOMPARE(mapInstance->getUnusedTownId(), static_cast<uint32_t>(1));
        mapInstance->addTown(RME::TownData(1, "T1", {}));
        QCOMPARE(mapInstance->getUnusedTownId(), static_cast<uint32_t>(2));
        mapInstance->addTown(RME::TownData(3, "T3", {})); // Add ID 3, next should be 2 or 4
        // Max ID is 3. Unused should be 3+1 = 4, unless 2 is free. 2 is free.
        QCOMPARE(mapInstance->getUnusedTownId(), static_cast<uint32_t>(2));
        mapInstance->addTown(RME::TownData(2, "T2", {}));
        // Max ID is 3. All IDs 1,2,3 are used. Next should be 4.
        QCOMPARE(mapInstance->getUnusedTownId(), static_cast<uint32_t>(4));

        mapInstance->removeTown(3); // Max is now 2 (or 1 if keys are iterated to find max)
                                    // Current m_maxTownId logic means it becomes 2.
        QCOMPARE(mapInstance->getUnusedTownId(), static_cast<uint32_t>(3));
    }
};
// QTEST_APPLESS_MAIN(TestMapTowns)
// #include "TestMapTowns.moc"
