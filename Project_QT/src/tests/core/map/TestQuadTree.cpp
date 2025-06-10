#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <memory> // For std::pow in rootSize calculation for tests

#include "core/map/Floor.h"
#include "core/map/QTreeNode.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ClientProfile.h"
#include "core/map_constants.h"

using namespace RME;

// Minimal AssetManager for testing Tile creation
class MinimalAssetManager : public AssetManager {
public:
    MinimalAssetManager() {
        // If AssetManager's constructor or other parts require a basic setup,
        // it could be done here. For now, assuming default construction is enough
        // for it to function as an IItemTypeProvider that returns defaults.
    }
};


class TestQuadTree : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<MinimalAssetManager> testAssetManager;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init(); // Per-test setup
    void cleanup(); // Per-test cleanup

    // Floor Tests
    void testFloor_Creation();
    void testFloor_GetSetRemoveTile();
    void testFloor_IsEmpty();

    // QTreeNode Tests
    void testQTreeNode_CreationAndBounds();
    void testQTreeNode_Subdivision();
    void testQTreeNode_GetSetTile_SimpleLeaf();
    void testQTreeNode_GetSetTile_RequiresSubdivision();
    void testQTreeNode_RemoveTile_And_CleanTree();
    void testQTreeNode_IsEmptyLogic(); // More focused isEmpty test
    void testQTreeNode_MAX_DEPTH_Calculation_Sanity(); // Renamed for clarity
};

void TestQuadTree::initTestCase() {
    // One-time setup for the entire test class
    qInfo() << "QTreeNode MAX_DEPTH static const is:" << QTreeNode::MAX_DEPTH;
    qInfo() << "Map Max Width:" << MAP_MAX_WIDTH << ", Sector Width:" << SECTOR_WIDTH_TILES;
    int expected_max_depth = 0;
    if (MAP_MAX_WIDTH > 0 && SECTOR_WIDTH_TILES > 0 && MAP_MAX_WIDTH >= SECTOR_WIDTH_TILES) {
        int tempSize = SECTOR_WIDTH_TILES;
        while(tempSize < MAP_MAX_WIDTH) { // Assuming square power-of-2 map for this simple check
            tempSize *= 2;
            expected_max_depth++;
             if (expected_max_depth > 20) break;
        }
    }
    qInfo() << "Expected MAX_DEPTH based on MAP_MAX_WIDTH would be around:" << expected_max_depth;
}

void TestQuadTree::cleanupTestCase() { }

void TestQuadTree::init() {
    // Create a new AssetManager for each test to ensure isolation
    testAssetManager = std::make_unique<MinimalAssetManager>();
}
void TestQuadTree::cleanup() {
    testAssetManager.reset();
}


// --- Floor Tests ---
void TestQuadTree::testFloor_Creation() {
    Floor floor(7, testAssetManager.get());
    QCOMPARE(floor.getZLevel(), 7);
    QVERIFY(floor.isEmpty());
}

void TestQuadTree::testFloor_GetSetRemoveTile() {
    Floor floor(7, testAssetManager.get());
    bool created = false;
    Position tileGlobalPos(10, 10, 7);

    Tile* tile1 = floor.getOrCreateTile(5, 5, created, tileGlobalPos);
    QVERIFY(tile1 != nullptr);
    QVERIFY(created);
    if(tile1) { // Check position only if tile exists
      QCOMPARE(tile1->getPosition(), tileGlobalPos);
    }
    QVERIFY(!floor.isEmpty());

    Position tileGlobalPos2(10, 10, 7); // Same pos, different instance for call
    Tile* tile1_again = floor.getOrCreateTile(5, 5, created, tileGlobalPos2);
    QVERIFY(tile1_again == tile1);
    QVERIFY(!created);

    QVERIFY(floor.getTile(5, 5) == tile1);
    QVERIFY(floor.getTile(0, 0) == nullptr);

    QVERIFY(floor.removeTile(5, 5));
    QVERIFY(floor.getTile(5, 5) == nullptr);
    QVERIFY(!floor.removeTile(5, 5));
    QVERIFY(floor.isEmpty());

    QVERIFY(floor.getTile(SECTOR_WIDTH_TILES, SECTOR_HEIGHT_TILES -1) == nullptr);
    QVERIFY(floor.getTile(SECTOR_WIDTH_TILES -1 , SECTOR_HEIGHT_TILES) == nullptr);

    // Test Z-coordinate correction in Floor::getOrCreateTile
    Position wrongZPos(10,11,8);
    Tile* tileWrongZ = floor.getOrCreateTile(5,6, created, wrongZPos);
    QVERIFY(tileWrongZ != nullptr);
    QVERIFY(created);
    if(tileWrongZ) {
        QCOMPARE(tileWrongZ->getPosition(), Position(10,11,7)); // Should use floor's Z
    }
}

void TestQuadTree::testFloor_IsEmpty() {
    Floor floor(7, testAssetManager.get());
    QVERIFY(floor.isEmpty());
    bool created;
    floor.getOrCreateTile(0, 0, created, Position(0,0,7));
    QVERIFY(!floor.isEmpty());
    floor.removeTile(0,0);
    QVERIFY(floor.isEmpty());
}

// --- QTreeNode Tests ---
// Helper to calculate expected root size for tests
int calculateTestRootSize() {
    int maxMapDim = std::max(MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
    if (maxMapDim <= 0) maxMapDim = SECTOR_WIDTH_TILES;
     int calculatedDepth = 0;
    int tempSize = SECTOR_WIDTH_TILES;
    while(tempSize < maxMapDim) {
        tempSize *= 2;
        calculatedDepth++;
        if (calculatedDepth > 20) break;
    }
    // This calculatedDepth should match QTreeNode::MAX_DEPTH
    // So, root size is SECTOR_WIDTH_TILES * 2^MAX_DEPTH
    return SECTOR_WIDTH_TILES * static_cast<int>(std::pow(2.0, static_cast<double>(QTreeNode::MAX_DEPTH)));
}


void TestQuadTree::testQTreeNode_CreationAndBounds() {
    int rootSize = calculateTestRootSize();
    QTreeNode root(0, 0, rootSize, 0, testAssetManager.get());
    QVERIFY(root.isLeaf());
    QVERIFY(root.isEmpty());
    QCOMPARE(root.getX(), 0);
    QCOMPARE(root.getY(), 0);
    QCOMPARE(root.getSize(), rootSize);
    QCOMPARE(root.getDepth(), 0);
}

void TestQuadTree::testQTreeNode_Subdivision() {
    if (QTreeNode::MAX_DEPTH < 1) { // Need at least depth 1 to test subdivision from depth 0 to 1
        qWarning("Skipping testQTreeNode_Subdivision as MAX_DEPTH < 1 for current map/sector constants.");
        return;
    }
    int rootSize = SECTOR_WIDTH_TILES * static_cast<int>(std::pow(2.0, QTreeNode::MAX_DEPTH));
    // Create a node at depth MAX_DEPTH - 1, so it can subdivide into MAX_DEPTH leaves
    int nodeSize = SECTOR_WIDTH_TILES * 2; // Size for a node at MAX_DEPTH - 1
    if (rootSize < nodeSize && QTreeNode::MAX_DEPTH > 0) { // If map is just one sector, rootSize is SECTOR_WIDTH
         nodeSize = rootSize; // Max depth node is root itself
    } else if (QTreeNode::MAX_DEPTH == 0) { // Map is one sector or smaller
         nodeSize = SECTOR_WIDTH_TILES; // Cannot subdivide
    }


    QTreeNode node(0, 0, nodeSize, QTreeNode::MAX_DEPTH > 0 ? QTreeNode::MAX_DEPTH - 1 : 0, testAssetManager.get());
    QVERIFY(node.isLeaf());

    if (QTreeNode::MAX_DEPTH == 0) { // Cannot subdivide if already at max depth (which is 0)
        bool created_ignored;
        node.getOrCreateTile(Position(0,0,0), created_ignored); // Should not subdivide
        QVERIFY(node.isLeaf());
        return;
    }

    bool created;
    node.getOrCreateTile(Position(1, 1, 7), created);

    QVERIFY(!node.isLeaf());
    for(int i=0; i<4; ++i) QVERIFY(node.children[i] != nullptr);

    int childSize = nodeSize / 2;
    QCOMPARE(node.children[0]->getSize(), childSize);
    QCOMPARE(node.children[0]->getX(), 0);
    QCOMPARE(node.children[0]->getY(), 0);
    QCOMPARE(node.children[0]->getDepth(), node.getDepth() + 1);

    QCOMPARE(node.children[1]->getX(), childSize);
    QCOMPARE(node.children[1]->getY(), 0);
}

void TestQuadTree::testQTreeNode_GetSetTile_SimpleLeaf() {
    int leafNodeSize = SECTOR_WIDTH_TILES;
    QTreeNode leafNode(0, 0, leafNodeSize, QTreeNode::MAX_DEPTH, testAssetManager.get());
    QVERIFY(leafNode.isLeaf());

    Position pos1(5, 5, 7);
    bool created = false;
    Tile* tile1 = leafNode.getOrCreateTile(pos1, created);
    QVERIFY(tile1 != nullptr);
    QVERIFY(created);
    QCOMPARE(tile1->getPosition(), pos1);
    QVERIFY(leafNode.z_level_floors.contains(7));
    QVERIFY(leafNode.z_level_floors.value(7)->getTile(5,5) == tile1);

    Tile* tile1_again = leafNode.getTile(pos1);
    QVERIFY(tile1_again == tile1);
}

void TestQuadTree::testQTreeNode_GetSetTile_RequiresSubdivision() {
    if (QTreeNode::MAX_DEPTH == 0) {
        qWarning("Skipping testQTreeNode_GetSetTile_RequiresSubdivision as MAX_DEPTH is 0.");
        return;
    }
    int nodeSize = SECTOR_WIDTH_TILES * 2;
    QTreeNode node(0, 0, nodeSize, QTreeNode::MAX_DEPTH - 1 , testAssetManager.get());
    QVERIFY(node.isLeaf());

    Position posInNW(SECTOR_WIDTH_TILES/2, SECTOR_WIDTH_TILES/2, 7);
    bool created = false;
    Tile* tileNW = node.getOrCreateTile(posInNW, created);
    QVERIFY(tileNW != nullptr);
    QVERIFY(created);
    QVERIFY(!node.isLeaf());
    QVERIFY(node.children[0] != nullptr && !node.children[0]->isEmpty());
    QCOMPARE(tileNW->getPosition(), posInNW);

    QTreeNode* childNW = node.children[0].get(); // NW child
    QVERIFY(childNW->isLeaf() && childNW->getDepth() == QTreeNode::MAX_DEPTH);
    QVERIFY(childNW->z_level_floors.contains(7));
    int localX = posInNW.x - childNW->getX();
    int localY = posInNW.y - childNW->getY();
    QCOMPARE(childNW->z_level_floors.value(7)->getTile(localX, localY), tileNW);
}

void TestQuadTree::testQTreeNode_RemoveTile_And_CleanTree() {
     if (QTreeNode::MAX_DEPTH == 0) {
        qWarning("Skipping testQTreeNode_RemoveTile_And_CleanTree as MAX_DEPTH is 0.");
        return;
    }
    int nodeSize = SECTOR_WIDTH_TILES * 2;
    QTreeNode node(0, 0, nodeSize, QTreeNode::MAX_DEPTH - 1, testAssetManager.get());

    Position pos(5, 5, 7);
    bool created;
    node.getOrCreateTile(pos, created);
    QVERIFY(!node.isLeaf());
    int quadrant = node.getQuadrant(pos.x, pos.y);
    QVERIFY(node.children[quadrant] && !node.children[quadrant]->isEmpty());

    QVERIFY(node.removeTile(pos));
    QVERIFY(node.getTile(pos) == nullptr);
    QVERIFY(node.isLeaf());
    QVERIFY(node.isEmpty());
}

void TestQuadTree::testQTreeNode_IsEmptyLogic() {
    int rootSize = calculateTestRootSize();
    QTreeNode root(0,0, rootSize, 0, testAssetManager.get());
    QVERIFY(root.isEmpty()); // Initially empty

    bool created;
    root.getOrCreateTile(Position(1,1,1), created);
    QVERIFY(!root.isEmpty());

    root.removeTile(Position(1,1,1));
    QVERIFY(root.isEmpty());
}

void TestQuadTree::testQTreeNode_MAX_DEPTH_Calculation_Sanity() {
    int expected_max_depth = 0;
    int max_map_dim = std::max(MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
    if (max_map_dim > 0 && SECTOR_WIDTH_TILES > 0 && max_map_dim >= SECTOR_WIDTH_TILES) {
        int tempSize = SECTOR_WIDTH_TILES;
        while(tempSize < max_map_dim) {
            tempSize *= 2;
            expected_max_depth++;
            if (expected_max_depth > 20) {expected_max_depth = 10; break;} // Safety break
        }
    }
    QCOMPARE(QTreeNode::MAX_DEPTH, expected_max_depth);
}


QTEST_MAIN(TestQuadTree)
#include "TestQuadTree.moc"
