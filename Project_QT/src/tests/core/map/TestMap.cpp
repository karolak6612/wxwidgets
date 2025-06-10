#include <QtTest/QtTest>
#include "core/map/Map.h" // Class to test
#include "core/assets/AssetManager.h"
#include "core/map/MapElements.h"
#include "core/map_constants.h"
#include "core/Position.h"
#include "core/Tile.h"
#include <memory> // For std::unique_ptr

// Use RME namespace
using namespace RME;

// Minimal AssetManager for tests
class TestMap_MinimalAssetManager : public AssetManager {
public:
    TestMap_MinimalAssetManager() {}
};


class TestMap : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<TestMap_MinimalAssetManager> testAssetManagerOwner; // Owner
    AssetManager* testAssetManager = nullptr; // Raw pointer for use

    const int testMapWidth = 100;
    const int testMapHeight = 70;
    const int testMapFloors = MAP_MAX_FLOORS;

    std::unique_ptr<Map> testMap;

private slots:
    void init();    // Called before each test function
    void cleanup(); // Called after each test function

    // BaseMap functionality (indirectly tested via Map)
    void testMap_TileOperations();

    // Map specific metadata
    void testMap_Metadata_Description();
    void testMap_Metadata_VersionInfo();
    void testMap_Metadata_DataFiles();
    void testMap_ChangeTracking();

    // Map elements management
    void testMap_TownsManagement();
    void testMap_HousesManagement();
    void testMap_WaypointsManagement();

    void testMap_StubMethods();
};

void TestMap::init() {
    testAssetManagerOwner = std::make_unique<TestMap_MinimalAssetManager>();
    testAssetManager = testAssetManagerOwner.get();
    testMap = std::make_unique<Map>(testMapWidth, testMapHeight, testMapFloors, testAssetManager);
}
void TestMap::cleanup() {
    testMap.reset();
    testAssetManagerOwner.reset();
    testAssetManager = nullptr;
}

void TestMap::testMap_TileOperations() {
    QVERIFY(testMap != nullptr);
    Position pos(10, 10, 1);
    bool created = false;

    testMap->setChanged(false); // Start with a known state for change tracking
    Tile* tile1 = testMap->getOrCreateTile(pos, created);
    QVERIFY(tile1 != nullptr);
    QVERIFY(created);
    if(tile1) QCOMPARE(tile1->getPosition(), pos);
    QVERIFY(testMap->hasChanged());

    testMap->setChanged(false);
    Tile* tile1_again = testMap->getOrCreateTile(pos, created);
    QVERIFY(tile1_again == tile1);
    QVERIFY(!created);
    // Retrieving an existing tile via getOrCreateTile should not mark map as changed
    // if the tile already exists and no modification is made to it by this call.
    QVERIFY(!testMap->hasChanged());

    QVERIFY(testMap->removeTile(pos));
    QVERIFY(testMap->hasChanged());
    QVERIFY(testMap->getTile(pos) == nullptr);
}

void TestMap::testMap_Metadata_Description() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);
    // Default description is set by Map constructor
    // QCOMPARE(testMap->getDescription(), QString("New RME Map"));

    testMap->setDescription("My Test Map");
    QCOMPARE(testMap->getDescription(), QString("My Test Map"));
    QVERIFY(testMap->hasChanged());
}

void TestMap::testMap_Metadata_VersionInfo() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);

    MapVersionInfo vi = testMap->getVersionInfo();
    // Default values from Map constructor
    // QCOMPARE(vi.otbmVersion, 4u);
    // QCOMPARE(vi.clientVersionID, 0u);

    vi.otbmVersion = 3;
    vi.clientVersionID = 1098;
    vi.description = "OTBM v3 - Tibia 10.98";
    testMap->setVersionInfo(vi);

    MapVersionInfo updated_vi = testMap->getVersionInfo();
    QCOMPARE(updated_vi.otbmVersion, 3u);
    QCOMPARE(updated_vi.clientVersionID, 1098u);
    QCOMPARE(updated_vi.description, QString("OTBM v3 - Tibia 10.98"));
    QVERIFY(testMap->hasChanged());

    testMap->setChanged(false);
    testMap->setOtbmVersion(2);
    QCOMPARE(testMap->getVersionInfo().otbmVersion, 2u);
    QVERIFY(testMap->hasChanged());
}

void TestMap::testMap_Metadata_DataFiles() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);

    QVERIFY(testMap->getHouseFile().isEmpty());
    testMap->setHouseFile("myhouses.xml");
    QCOMPARE(testMap->getHouseFile(), QString("myhouses.xml"));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    testMap->setSpawnFile("myspawns.xml");
    QCOMPARE(testMap->getSpawnFile(), QString("myspawns.xml"));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    testMap->setWaypointFile("mywaypoints.xml");
    QCOMPARE(testMap->getWaypointFile(), QString("mywaypoints.xml"));
    QVERIFY(testMap->hasChanged());
}

void TestMap::testMap_ChangeTracking() {
    QVERIFY(testMap != nullptr);
    // Map constructor sets default description and version info,
    // which calls setChanged(true) via the setters.
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false); // Reset for this test
    QVERIFY(!testMap->hasChanged());

    testMap->setDescription("test");
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    testMap->addTown(TownData(1, "Test Town", Position(100,100,7)));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    testMap->removeTown(1);
    QVERIFY(testMap->hasChanged());
}


void TestMap::testMap_TownsManagement() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false); // Ignore constructor changes for this test focus
    QVERIFY(testMap->getTowns().isEmpty());

    TownData town1(1, "Townsville", Position(100,100,7));
    testMap->addTown(town1);
    QCOMPARE(testMap->getTowns().size(), 1);
    QVERIFY(testMap->getTown(1) != nullptr);
    if(testMap->getTown(1)) QCOMPARE(testMap->getTown(1)->name, QString("Townsville"));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    TownData town2(2, "Cityburg", Position(200,200,7));
    testMap->addTown(town2);
    QCOMPARE(testMap->getTowns().size(), 2);
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    QVERIFY(testMap->removeTown(1));
    QCOMPARE(testMap->getTowns().size(), 1);
    QVERIFY(testMap->getTown(1) == nullptr);
    QVERIFY(testMap->getTown(2) != nullptr);
    QVERIFY(testMap->hasChanged());

    QVERIFY(!testMap->removeTown(99)); // Non-existent, should not change status
    QVERIFY(testMap->hasChanged()); // Still true from previous successful removal
}

void TestMap::testMap_HousesManagement() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);
    QVERIFY(testMap->getHouses().isEmpty());

    HouseData house1;
    house1.houseId = 101;
    house1.name = "Adventurer's Guild";
    house1.entryPosition = Position(150,150,7);
    testMap->addHouse(house1);
    QCOMPARE(testMap->getHouses().size(), 1);
    QVERIFY(testMap->getHouse(101) != nullptr);
    if(testMap->getHouse(101)) QCOMPARE(testMap->getHouse(101)->name, QString("Adventurer's Guild"));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    house1.name = "Renamed Guild"; // Update by re-adding
    testMap->addHouse(house1);
    QCOMPARE(testMap->getHouses().size(), 1);
    if(testMap->getHouse(101)) QCOMPARE(testMap->getHouse(101)->name, QString("Renamed Guild"));
    QVERIFY(testMap->hasChanged());
}

void TestMap::testMap_WaypointsManagement() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);
    QVERIFY(testMap->getWaypoints().isEmpty());

    WaypointData wp1("Central", Position(128,128,7));
    testMap->addWaypoint(wp1);
    QCOMPARE(testMap->getWaypoints().size(), 1);
    QVERIFY(testMap->getWaypoint("Central") != nullptr);
    if(testMap->getWaypoint("Central")) QCOMPARE(testMap->getWaypoint("Central")->position, Position(128,128,7));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    WaypointData wp1_updated("Central", Position(130,130,7)); // Update by re-adding
    testMap->addWaypoint(wp1_updated);
    QCOMPARE(testMap->getWaypoints().size(), 1);
    if(testMap->getWaypoint("Central")) QCOMPARE(testMap->getWaypoint("Central")->position, Position(130,130,7));
    QVERIFY(testMap->hasChanged());
}

void TestMap::testMap_StubMethods() {
    QVERIFY(testMap != nullptr);
    testMap->setChanged(false);

    QVERIFY(!testMap->convertFormat(3, 1098));
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    QVERIFY(!testMap->exportMinimap("test_minimap.bmp"));
    QVERIFY(!testMap->hasChanged()); // exportMinimap likely shouldn't set changed

    QCOMPARE(testMap->cleanInvalidTiles(), 0);
    QVERIFY(testMap->hasChanged());
    testMap->setChanged(false);

    QCOMPARE(testMap->cleanDuplicateItems(), 0);
    QVERIFY(testMap->hasChanged());
}


QTEST_MAIN(TestMap)
#include "TestMap.moc"
