#include <QtTest/QtTest>
#include "core/map/BaseMap.h"
#include "core/assets/AssetManager.h"
#include "core/map_constants.h"
#include "core/Position.h"
#include "core/Tile.h"
#include <memory> // For std::pow in rootSize check

// Use RME namespace
using namespace RME;

// Minimal AssetManager for BaseMap tests
class TestBaseMap_MinimalAssetManager : public AssetManager {
public:
    TestBaseMap_MinimalAssetManager() {
        // Base AssetManager constructor handles PIMPL initialization.
        // No further setup needed if tests only rely on it as an IItemTypeProvider
        // that returns default ItemData for any ID (which is base AssetManager behavior).
    }
};


class TestBaseMap : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<TestBaseMap_MinimalAssetManager> testAssetManager;

    // Using smaller dimensions for most tests to keep QuadTree depth manageable
    // but large enough to allow some subdivision.
    // If SECTOR_WIDTH_TILES is 32, MAX_DEPTH for 128 is log2(128/32)=log2(4)=2.
    // If MAX_DEPTH for 65536 is 11, root size for 128x128 map is 128.
    const int testMapWidth = 128;
    const int testMapHeight = 128;
    const int testMapFloors = MAP_MAX_FLOORS;

private slots:
    void init();    // Called before each test function
    void cleanup(); // Called after each test function

    void testBaseMap_Creation();
    void testBaseMap_IsPositionValid();
    void testBaseMap_GetOrCreateTile_Simple();
    void testBaseMap_GetOrCreateTile_Boundaries();
    void testBaseMap_GetOrCreateTile_DifferentFloors();
    void testBaseMap_RemoveTile();
    void testBaseMap_TilePositioning();
    void testBaseMap_RootNodeSizingAndCoverage();
};

void TestBaseMap::init() {
    testAssetManager = std::make_unique<TestBaseMap_MinimalAssetManager>();
}

void TestBaseMap::cleanup() {
    testAssetManager.reset();
}

void TestBaseMap::testBaseMap_Creation() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    QCOMPARE(map.getWidth(), testMapWidth);
    QCOMPARE(map.getHeight(), testMapHeight);
    QCOMPARE(map.getNumFloors(), testMapFloors);
    QVERIFY(map.getRootNode() != nullptr);
}

void TestBaseMap::testBaseMap_IsPositionValid() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());

    QVERIFY(map.isPositionValid(Position(0, 0, 0)));
    QVERIFY(map.isPositionValid(Position(testMapWidth - 1, testMapHeight - 1, testMapFloors - 1)));

    QVERIFY(!map.isPositionValid(Position(-1, 0, 0)));
    QVERIFY(!map.isPositionValid(Position(testMapWidth, 0, 0)));
    QVERIFY(!map.isPositionValid(Position(0, -1, 0)));
    QVERIFY(!map.isPositionValid(Position(0, testMapHeight, 0)));
    QVERIFY(!map.isPositionValid(Position(0, 0, -1)));
    QVERIFY(!map.isPositionValid(Position(0, 0, testMapFloors)));
    QVERIFY(!map.isPositionValid(Position(0, 0, MAP_MAX_FLOOR + 1))); // Global max
}

void TestBaseMap::testBaseMap_GetOrCreateTile_Simple() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    Position pos(10, 15, 7);
    bool created = false;

    Tile* tile1 = map.getOrCreateTile(pos, created);
    QVERIFY(tile1 != nullptr);
    QVERIFY(created);
    if(tile1) QCOMPARE(tile1->getPosition(), pos);

    Tile* tile1_again = map.getOrCreateTile(pos, created);
    QVERIFY(tile1_again == tile1);
    QVERIFY(!created);

    const BaseMap& constMap = map;
    const Tile* constTile = constMap.getTile(pos);
    QVERIFY(constTile == tile1);
}

void TestBaseMap::testBaseMap_GetOrCreateTile_Boundaries() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    bool created = false;

    Position topLeft(0, 0, 0);
    Tile* tileTL = map.getOrCreateTile(topLeft, created);
    QVERIFY(tileTL != nullptr && created);
    if(tileTL) QCOMPARE(tileTL->getPosition(), topLeft);

    Position bottomRight(testMapWidth - 1, testMapHeight - 1, testMapFloors - 1);
    Tile* tileBR = map.getOrCreateTile(bottomRight, created);
    QVERIFY(tileBR != nullptr && created);
    if(tileBR) QCOMPARE(tileBR->getPosition(), bottomRight);

    Position invalidX(testMapWidth, 0, 0);
    QVERIFY(map.getOrCreateTile(invalidX, created) == nullptr);

    Position invalidZ(0, 0, testMapFloors);
    QVERIFY(map.getOrCreateTile(invalidZ, created) == nullptr);
}

void TestBaseMap::testBaseMap_GetOrCreateTile_DifferentFloors() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    Position pos_f0(50, 50, 0);
    Position pos_f7(50, 50, 7);
    Position pos_f15(50, 50, testMapFloors - 1); // Max valid floor for this map instance
    bool created = false;

    Tile* tile_f0 = map.getOrCreateTile(pos_f0, created);
    QVERIFY(tile_f0 != nullptr && created);
    if(tile_f0) QCOMPARE(tile_f0->getPosition(), pos_f0);

    Tile* tile_f7 = map.getOrCreateTile(pos_f7, created);
    QVERIFY(tile_f7 != nullptr && created);
    if(tile_f7) QCOMPARE(tile_f7->getPosition(), pos_f7);
    QVERIFY(tile_f0 != tile_f7);

    Tile* tile_f15 = map.getOrCreateTile(pos_f15, created);
    QVERIFY(tile_f15 != nullptr && created);
    if(tile_f15) QCOMPARE(tile_f15->getPosition(), pos_f15);
}

void TestBaseMap::testBaseMap_RemoveTile() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    Position pos(20, 25, 3);
    bool created = false;

    map.getOrCreateTile(pos, created);
    QVERIFY(map.getTile(pos) != nullptr);

    QVERIFY(map.removeTile(pos));
    QVERIFY(map.getTile(pos) == nullptr);
    QVERIFY(!map.removeTile(pos));

    QVERIFY(!map.removeTile(Position(testMapWidth + 10, 25, 3)));
}

void TestBaseMap::testBaseMap_TilePositioning() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    bool created = false;

    // Sample fewer points to reduce test time but still cover different areas
    for (int z = 0; z < testMapFloors; z += std::max(1, testMapFloors / 3)) {
        for (int y = 0; y < testMapHeight; y += std::max(1, testMapHeight / 4)) {
            for (int x = 0; x < testMapWidth; x += std::max(1, testMapWidth / 4)) {
                Position currentPos(x,y,z);
                if (!map.isPositionValid(currentPos)) continue;

                Tile* tile = map.getOrCreateTile(currentPos, created);
                QVERIFY(tile != nullptr);
                QVERIFY(created); // Tile should be newly created each time for unique pos
                if(tile) QCOMPARE(tile->getPosition(), currentPos);

                Tile* retrievedTile = map.getTile(currentPos);
                QVERIFY(retrievedTile == tile);
                if(retrievedTile) QCOMPARE(retrievedTile->getPosition(), currentPos);
            }
        }
    }
}

void TestBaseMap::testBaseMap_RootNodeSizingAndCoverage() {
    BaseMap map(testMapWidth, testMapHeight, testMapFloors, testAssetManager.get());
    QTreeNode* root = map.getRootNode();
    QVERIFY(root != nullptr);
    if(!root) return;

    int expectedSize = BaseMap::calculateRootNodeSize(testMapWidth, testMapHeight);
    QCOMPARE(root->getSize(), expectedSize);
    QCOMPARE(root->getX(), 0);
    QCOMPARE(root->getY(), 0);

    // Check if tiles at map extremities can be placed
    Position corners[] = {
        Position(0, 0, 0),
        Position(testMapWidth - 1, 0, 0),
        Position(0, testMapHeight - 1, 0),
        Position(testMapWidth - 1, testMapHeight - 1, testMapFloors - 1)
    };

    for(const auto& cornerPos : corners) {
        if (map.isPositionValid(cornerPos)) {
            bool created;
            Tile* tile = map.getOrCreateTile(cornerPos, created);
            QVERIFY2(tile != nullptr, QString("Failed to create tile at corner %1,%2,%3").arg(cornerPos.x).arg(cornerPos.y).arg(cornerPos.z).toStdString().c_str());
            if(tile) QCOMPARE(tile->getPosition(), cornerPos);
        }
    }
}

QTEST_MAIN(TestBaseMap)
#include "TestBaseMap.moc"
