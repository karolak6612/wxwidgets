#include <QtTest/QtTest>
#include "core/map/Map.h"
#include "core/spawns/SpawnData.h"
#include "core/world/TownData.h"
#include "core/houses/HouseData.h"
#include "core/Position.h"

class TestMapQueries : public QObject {
    Q_OBJECT
private:
    RME::Map* mapInstance = nullptr;

private slots:
    void init() {
        mapInstance = new RME::Map(100, 100, 8, nullptr); // AssetManager is null
    }
    void cleanup() {
        delete mapInstance;
        mapInstance = nullptr;
    }

    void testGetSpawnOverlapCount() {
        // Z-level 7 for all tests here
        RME::Position pos1(10, 10, 7); // Test point

        // No spawns
        QCOMPARE(mapInstance->getSpawnOverlapCount(pos1), 0);

        // Spawn 1: covers pos1
        RME::SpawnData spawn1(RME::Position(10, 10, 7), 5, 60, {"Dragon"}); // radius 5
        mapInstance->addSpawn(RME::SpawnData(spawn1));
        QCOMPARE(mapInstance->getSpawnOverlapCount(pos1), 1);
        QCOMPARE(mapInstance->getSpawnOverlapCount(RME::Position(15, 10, 7)), 1); // Edge of spawn1
        QCOMPARE(mapInstance->getSpawnOverlapCount(RME::Position(16, 10, 7)), 0); // Outside spawn1

        // Spawn 2: also covers pos1 (overlap)
        RME::SpawnData spawn2(RME::Position(12, 12, 7), 3, 60, {"Demon"}); // radius 3, center near pos1
        mapInstance->addSpawn(RME::SpawnData(spawn2));
        QCOMPARE(mapInstance->getSpawnOverlapCount(pos1), 2); // pos1 is within 3 units of (12,12)

        // Spawn 3: different Z-level, does not cover pos1
        RME::SpawnData spawn3(RME::Position(10, 10, 6), 5, 60, {"Cyclops"});
        mapInstance->addSpawn(RME::SpawnData(spawn3));
        QCOMPARE(mapInstance->getSpawnOverlapCount(pos1), 2); // Still 2, as spawn3 is on different Z

        // Spawn 4: same Z, but does not cover pos1
        RME::SpawnData spawn4(RME::Position(30, 30, 7), 2, 60, {"Orc"});
        mapInstance->addSpawn(RME::SpawnData(spawn4));
        QCOMPARE(mapInstance->getSpawnOverlapCount(pos1), 2);
        QCOMPARE(mapInstance->getSpawnOverlapCount(RME::Position(30,30,7)), 1); // Covered by spawn4 only
    }

    void testGetTownByTempleLocation() {
        RME::Position templePos1(50, 50, 7);
        RME::Position templePos2(60, 60, 7);
        RME::Position nonTemplePos(70, 70, 7);

        RME::TownData town1(1, "TownA", templePos1);
        mapInstance->addTown(RME::TownData(town1));

        RME::TownData town2(2, "TownB", templePos2);
        mapInstance->addTown(RME::TownData(town2));

        // Test finding existing towns
        const RME::TownData* foundTown1 = mapInstance->getTownByTempleLocation(templePos1);
        QVERIFY(foundTown1 != nullptr);
        QCOMPARE(foundTown1->getId(), town1.getId());

        RME::TownData* foundTown2NonConst = mapInstance->getTownByTempleLocation(templePos2);
        QVERIFY(foundTown2NonConst != nullptr);
        QCOMPARE(foundTown2NonConst->getId(), town2.getId());
        foundTown2NonConst->setName("TownB_Renamed"); // Test mutability
        QCOMPARE(mapInstance->getTown(2)->getName(), QString("TownB_Renamed"));


        // Test not finding a town
        QVERIFY(mapInstance->getTownByTempleLocation(nonTemplePos) == nullptr);

        // Test const version
        const RME::Map* constMap = mapInstance;
        const RME::TownData* foundTown1Const = constMap->getTownByTempleLocation(templePos1);
        QVERIFY(foundTown1Const != nullptr);
        QCOMPARE(foundTown1Const->getId(), town1.getId());
    }

    void testGetHousesWithExitAt() {
        RME::Position exitPos1(20, 20, 7);
        RME::Position exitPos2(20, 21, 7);
        RME::Position nonExitPos(25, 25, 7);

        RME::HouseData house1(101, "House1");
        house1.addExit(exitPos1);
        mapInstance->addHouse(RME::HouseData(house1));

        RME::HouseData house2(102, "House2");
        house2.addExit(exitPos2);
        mapInstance->addHouse(RME::HouseData(house2));

        RME::HouseData house3(103, "House3");
        house3.addExit(exitPos1); // house3 also exits at exitPos1
        mapInstance->addHouse(RME::HouseData(house3));


        // Test finding houses for exitPos1
        QList<const RME::HouseData*> housesAtExit1Const = static_cast<const RME::Map*>(mapInstance)->getHousesWithExitAt(exitPos1);
        QCOMPARE(housesAtExit1Const.size(), 2);
        bool foundH1=false, foundH3=false;
        for(const RME::HouseData* h : housesAtExit1Const){
            if(h->getId() == 101) foundH1=true;
            if(h->getId() == 103) foundH3=true;
        }
        QVERIFY(foundH1);
        QVERIFY(foundH3);

        // Test non-const version
        QList<RME::HouseData*> housesAtExit1 = mapInstance->getHousesWithExitAt(exitPos1);
        QCOMPARE(housesAtExit1.size(), 2);
        // (Can optionally modify a house here to test mutability if needed)

        // Test finding house for exitPos2
        QList<const RME::HouseData*> housesAtExit2 = static_cast<const RME::Map*>(mapInstance)->getHousesWithExitAt(exitPos2);
        QCOMPARE(housesAtExit2.size(), 1);
        QVERIFY(housesAtExit2.first()->getId() == 102);

        // Test not finding houses
        QVERIFY(mapInstance->getHousesWithExitAt(nonExitPos).isEmpty());
    }
};
// QTEST_APPLESS_MAIN(TestMapQueries)
// #include "TestMapQueries.moc"
