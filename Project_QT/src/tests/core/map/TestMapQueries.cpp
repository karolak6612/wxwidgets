#include <QtTest/QtTest>
#include "core/map/Map.h"
#include "core/spawns/SpawnData.h"
#include "core/world/TownData.h"
#include "core/houses/HouseData.h"
#include "core/Position.h"
#include "core/Item.h" // Added for item creation
#include "tests/core/MockItemTypeProvider.h" // Added for item properties
#include "core/assets/AssetManager.h" // Added for map creation
#include "core/assets/ItemDatabase.h" // Added for AssetManager
#include "core/assets/CreatureDatabase.h" // Added for AssetManager
#include "core/assets/SpriteManager.h" // Added for AssetManager
#include "core/assets/ClientVersionManager.h" // Added for AssetManager
#include "core/assets/MaterialManager.h" // Added for AssetManager
#include "core/Tile.h" // Added for tile->update

class TestMapQueries : public QObject {
    Q_OBJECT
private:
    RME::Map* mapInstance = nullptr;
    RME::MockItemTypeProvider* m_mockItemProvider = nullptr;
    RME::core::assets::AssetManager* m_assetManager = nullptr;
    // Minimal AssetManager dependencies (could be further mocked if needed)
    RME::core::assets::ClientVersionManager* m_clientVersionManager = nullptr;
    RME::core::assets::ItemDatabase* m_itemDatabase = nullptr;
    RME::core::assets::CreatureDatabase* m_creatureDatabase = nullptr;
    RME::core::sprites::SpriteManager* m_spriteManager = nullptr;
    RME::core::assets::MaterialManager* m_materialManager = nullptr;


private slots:
    void init() {
        m_mockItemProvider = new RME::MockItemTypeProvider();
        m_clientVersionManager = new RME::core::assets::ClientVersionManager();
        m_itemDatabase = new RME::core::assets::ItemDatabase(*m_clientVersionManager);
        // Pass item provider to item database if it takes it, or ensure AssetManager uses our mock provider
        // For now, assume AssetManager can be configured or will use a provider that can be influenced.
        // The most direct way is if AssetManager can take an IItemTypeProvider.
        // The current AssetManager constructor takes ItemDatabase etc.
        // For simplicity, we'll ensure Tiles get created with our m_mockItemProvider.
        // This is tricky as AssetManager owns ItemDatabase which is the IItemTypeProvider.
        // A simple solution for testing: make AssetManager take the mock provider.
        // However, sticking to existing AssetManager constructor for now.
        // The key is that Tiles created get *an* IItemTypeProvider.
        // We will pass m_mockItemProvider to Item::create directly in tests.
        // Map needs a valid AssetManager for Tile creation via getOrCreateTile.

        m_creatureDatabase = new RME::core::assets::CreatureDatabase();
        m_spriteManager = new RME::core::sprites::SpriteManager(*m_clientVersionManager);
        m_materialManager = new RME::core::assets::MaterialManager(*m_clientVersionManager);

        m_assetManager = new RME::core::assets::AssetManager(
            *m_itemDatabase, *m_creatureDatabase, *m_spriteManager,
            *m_clientVersionManager, *m_materialManager
        );
        // Crucially, if we want Item::create within Tile to use our mock provider,
        // the AssetManager given to Map must somehow provide it.
        // This setup means Tiles created by map->getOrCreateTile will use the real ItemDB.
        // For this test, we'll manually create items using our m_mockItemProvider.

        mapInstance = new RME::Map(100, 100, 8, m_assetManager);
    }
    void cleanup() {
        delete mapInstance; mapInstance = nullptr;
        delete m_assetManager; m_assetManager = nullptr;
        delete m_materialManager; m_materialManager = nullptr;
        delete m_spriteManager; m_spriteManager = nullptr;
        delete m_creatureDatabase; m_creatureDatabase = nullptr;
        delete m_itemDatabase; m_itemDatabase = nullptr;
        delete m_clientVersionManager; m_clientVersionManager = nullptr;
        delete m_mockItemProvider; m_mockItemProvider = nullptr;
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

    void testIsValidHouseExitLocation() {
        RME::core::Position validPos(5,5,7);
        RME::core::Position noTilePos(5,6,7);
        RME::core::Position noGroundPos(5,7,7);
        RME::core::Position houseTilePos(5,8,7);
        RME::core::Position blockingItemPos(5,9,7);

        bool created; // Dummy for getOrCreateTile

        // Setup for validPos
        RME::Tile* tileAtValidPos = mapInstance->getOrCreateTile(validPos, created);
        QVERIFY(tileAtValidPos);
        // Item::create needs an IItemTypeProvider. Tiles get it from AssetManager.
        // If map's AssetManager doesn't use m_mockItemProvider, this item won't have mock properties.
        // For this test, we need items with specific mocked properties (ground, blocking).
        // So, we should use m_mockItemProvider for creating test items.
        MockItemData groundItemData; groundItemData.isGround = true;
        m_mockItemProvider->setMockData(1, groundItemData);
        tileAtValidPos->addItem(RME::Item::create(1, m_mockItemProvider));
        tileAtValidPos->update(); // Ensure tile state (like isBlocking) is up-to-date

        // Setup for noGroundPos
        /*RME::Tile* tileAtNoGroundPos =*/ mapInstance->getOrCreateTile(noGroundPos, created);
        // No ground on tileAtNoGroundPos, it's created empty by default.

        // Setup for houseTilePos
        RME::Tile* tileAtHouseTilePos = mapInstance->getOrCreateTile(houseTilePos, created);
        QVERIFY(tileAtHouseTilePos);
        tileAtHouseTilePos->addItem(RME::Item::create(1, m_mockItemProvider)); // Ground
        tileAtHouseTilePos->setHouseId(10);
        tileAtHouseTilePos->update();

        // Setup for blockingItemPos
        RME::Tile* tileAtBlockingItemPos = mapInstance->getOrCreateTile(blockingItemPos, created);
        QVERIFY(tileAtBlockingItemPos);
        tileAtBlockingItemPos->addItem(RME::Item::create(1, m_mockItemProvider)); // Ground
        MockItemData blockingItemData; blockingItemData.isBlocking = true;
        m_mockItemProvider->setMockData(99, blockingItemData);
        tileAtBlockingItemPos->addItem(RME::Item::create(99, m_mockItemProvider));
        tileAtBlockingItemPos->update();

        QVERIFY(mapInstance->isValidHouseExitLocation(validPos));
        QVERIFY(!mapInstance->isValidHouseExitLocation(RME::core::Position(99,99,7))); // Invalid map pos

        // For noTilePos: getOrCreateTile would create it if it doesn't exist.
        // isValidHouseExitLocation calls map->getTile(). If a tile was never created at noTilePos, getTile returns nullptr.
        QVERIFY(!mapInstance->isValidHouseExitLocation(noTilePos));

        QVERIFY(!mapInstance->isValidHouseExitLocation(noGroundPos));
        QVERIFY(!mapInstance->isValidHouseExitLocation(houseTilePos));
        QVERIFY(!mapInstance->isValidHouseExitLocation(blockingItemPos));
    }
};
// QTEST_APPLESS_MAIN(TestMapQueries)
// #include "TestMapQueries.moc"
