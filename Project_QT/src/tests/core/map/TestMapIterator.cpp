#include <QtTest/QtTest>
#include "core/map/BaseMap.h"
#include "core/map/MapIterator.h"
#include "core/assets/AssetManager.h"
#include "core/Position.h"
#include "core/Tile.h"
#include "core/map_constants.h"

#include <set>
#include <memory> // For std::unique_ptr

using namespace RME;

class TestMapIterator_MinimalAssetManager : public AssetManager {
public:
    TestMapIterator_MinimalAssetManager() {}
};


class TestMapIterator : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<TestMapIterator_MinimalAssetManager> testAssetManager;

    const int singleSectorMapDim = SECTOR_WIDTH_TILES;
    const int multiSectorMapDim = SECTOR_WIDTH_TILES * 2;
    const int testMapMaxFloors = 3; // Max Z index will be 2 (0, 1, 2)

private slots:
    void init();    // Called before each test function
    void cleanup(); // Called after each test function

    void testEmptyMapIteration();
    void testSingleTileIteration();
    void testMultipleTilesSameFloorSectorIteration();
    void testMultipleTilesDifferentFloorsIteration();
    void testSparseTilesAcrossSectorsAndFloorsIteration();
    void testIteratorEquality();
    void testIteratorDereference();
    void testPostIncrement();

    // Helper to populate a map and return expected positions
    std::set<Position> populateMapAndGetExpected(BaseMap& map, const QList<Position>& positionsToCreate);
};

void TestMapIterator::init() {
    testAssetManager = std::make_unique<TestMapIterator_MinimalAssetManager>();
}
void TestMapIterator::cleanup() {
    testAssetManager.reset();
}

std::set<Position> TestMapIterator::populateMapAndGetExpected(BaseMap& map, const QList<Position>& positionsToCreate) {
    std::set<Position> expectedPositions;
    bool created;
    for (const Position& pos : positionsToCreate) {
        if (map.isPositionValid(pos)) {
            map.getOrCreateTile(pos, created);
            if (created) {
                expectedPositions.insert(pos);
            }
        }
    }
    return expectedPositions;
}

void TestMapIterator::testEmptyMapIteration() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    MapIterator it = map.begin();
    MapIterator endIt = map.end();
    QVERIFY(it == endIt);

    int count = 0;
    for (Tile* tile : map) {
        Q_UNUSED(tile);
        count++;
    }
    QCOMPARE(count, 0);
}

void TestMapIterator::testSingleTileIteration() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    Position pos(5, 5, 0);
    populateMapAndGetExpected(map, {pos});

    int count = 0;
    bool found = false;
    for (Tile* tile : map) {
        QVERIFY(tile != nullptr);
        QCOMPARE(tile->getPosition(), pos);
        found = true;
        count++;
    }
    QCOMPARE(count, 1);
    QVERIFY(found);
}

void TestMapIterator::testMultipleTilesSameFloorSectorIteration() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    QList<Position> positions = {
        Position(0, 0, 0), Position(SECTOR_WIDTH_TILES - 1, SECTOR_HEIGHT_TILES - 1, 0), Position(10, 10, 0)
    };
    std::set<Position> expected = populateMapAndGetExpected(map, positions);

    std::set<Position> found;
    for (Tile* tile : map) {
        QVERIFY(tile != nullptr);
        found.insert(tile->getPosition());
    }
    QCOMPARE(found.size(), static_cast<int>(expected.size()));
    QCOMPARE(found, expected);
}

void TestMapIterator::testMultipleTilesDifferentFloorsIteration() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    QList<Position> positions = {
        Position(5, 5, 0), Position(5, 5, 1), Position(5, 5, testMapMaxFloors - 1)
    };
    std::set<Position> expected = populateMapAndGetExpected(map, positions);

    std::set<Position> found;
    for (Tile* tile : map) {
        QVERIFY(tile != nullptr);
        found.insert(tile->getPosition());
    }
    QCOMPARE(found.size(), static_cast<int>(expected.size()));
    QCOMPARE(found, expected);
}

void TestMapIterator::testSparseTilesAcrossSectorsAndFloorsIteration() {
    // This test requires a map large enough for multiple sectors.
    // QTreeNode::MAX_DEPTH for multiSectorMapDim (64) and SECTOR_WIDTH (32) is log2(64/32) = log2(2) = 1.
    // So, root (depth 0) subdivides into 4 children (depth 1), which are leaves.
    if (QTreeNode::MAX_DEPTH < 1 && multiSectorMapDim > singleSectorMapDim) {
        qWarning("Skipping testSparseTilesAcrossSectorsAndFloorsIteration as QTreeNode::MAX_DEPTH is too low for multi-sector tests with current constants.");
        return;
    }

    BaseMap map(multiSectorMapDim, multiSectorMapDim, testMapMaxFloors, testAssetManager.get());
    QList<Position> positions = {
        Position(10, 10, 0),                                           // Sector (0,0) on floor 0
        Position(SECTOR_WIDTH_TILES + 5, 5, 0),                        // Sector (1,0) on floor 0
        Position(5, SECTOR_HEIGHT_TILES + 5, 1),                       // Sector (0,1) on floor 1
        Position(multiSectorMapDim - 1, multiSectorMapDim - 1, testMapMaxFloors - 1) // Far corner
    };
    std::set<Position> expected = populateMapAndGetExpected(map, positions);

    std::set<Position> found;
    for (Tile* tile : map) {
        QVERIFY(tile != nullptr);
        found.insert(tile->getPosition());
    }
    QCOMPARE(found.size(), static_cast<int>(expected.size()));
    QCOMPARE(found, expected);
}

void TestMapIterator::testIteratorEquality() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    MapIterator begin1 = map.begin();
    MapIterator end1 = map.end();
    QVERIFY(begin1 == end1); // Empty map

    Position pos1(1,1,0);
    populateMapAndGetExpected(map, {pos1});

    MapIterator begin2 = map.begin();
    MapIterator end2 = map.end(); // New end iterator for the non-empty map context
    QVERIFY(begin2 != end2);
    QVERIFY(begin2 == map.begin());

    MapIterator it = map.begin();
    QVERIFY(it == begin2);
    ++it;
    // After iterating over the single tile, 'it' should become the end iterator.
    // Its currentTile will be nullptr, and its stack empty (or rootNode_ptr_ set to null by iterator logic for exhaustion).
    QVERIFY(it == end2);
    QVERIFY(it != begin2);

    MapIterator defaultEnd;
    QVERIFY(defaultEnd == end1);
    QVERIFY(defaultEnd == end2);
}

void TestMapIterator::testIteratorDereference() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    Position pos(3, 3, 0);
    populateMapAndGetExpected(map, {pos});

    MapIterator it = map.begin();
    QVERIFY(it != map.end());

    Tile* tilePtr = *it;
    QVERIFY(tilePtr != nullptr);
    if(tilePtr) QCOMPARE(tilePtr->getPosition(), pos);

    if(it.operator->() != nullptr) { // Check before dereferencing
        QCOMPARE(it->getPosition(), pos);
    } else {
        QFAIL("Iterator operator-> returned nullptr unexpectedly.");
    }
}

void TestMapIterator::testPostIncrement() {
    BaseMap map(singleSectorMapDim, singleSectorMapDim, testMapMaxFloors, testAssetManager.get());
    Position pos1(1,1,0);
    Position pos2(2,2,0);
    populateMapAndGetExpected(map, {pos1, pos2});

    MapIterator it = map.begin();
    QVERIFY(it != map.end());

    // Test post-increment
    MapIterator prevIt = it++;
    QVERIFY(prevIt != map.end());
    QVERIFY(it != map.end());
    QVERIFY(prevIt != it);
    if(prevIt.operator->() && it.operator->()) { // Ensure they are not end iterators
      QVERIFY(prevIt->getPosition() != it->getPosition()); // They should point to different tiles now
    }

    ++prevIt; // prevIt should now be equal to current 'it'
    QVERIFY(prevIt == it);
}


QTEST_MAIN(TestMapIterator)
#include "TestMapIterator.moc"
