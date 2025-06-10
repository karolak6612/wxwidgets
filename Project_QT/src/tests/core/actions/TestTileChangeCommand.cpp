#include <QtTest/QtTest>
#include <QUndoStack> // Needed for mergeWith testing

#include "core/actions/TileChangeCommand.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Position.h"
#include "core/IItemTypeProvider.h" // For instantiating Map/Tile if needed

// A simple mock item type provider for testing purposes (copied from TestUndoManager.cpp)
// In a real scenario, this would be in a shared test utility.
class MockItemTypeProvider : public RME::core::IItemTypeProvider {
public:
    // Implement pure virtual methods with minimal logic
    QString getName(uint16_t id) const override { Q_UNUSED(id); return "Mock Item"; }
    QString getDescription(uint16_t id) const override { Q_UNUSED(id); return "Mock Description"; }
    double getWeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1.0; }
    bool isBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isProjectileBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isPathBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isWalkable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool isStackable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isGround(uint16_t id) const override { return id == 1; } // ID 1 is ground
    bool isAlwaysOnTop(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isReadable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isWriteable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isFluidContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isSplash(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isMoveable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool hasHeight(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isTeleport(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDoor(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isPodium(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDepot(uint16_t id) const override { Q_UNUSED(id); return false; }
    int getSpriteX(uint16_t id, uint16_t subtype, int animationFrame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(animationFrame); return 0; }
    int getSpriteY(uint16_t id, uint16_t subtype, int animationFrame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(animationFrame); return 0; }
    int getSpriteWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteOffsetX(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getSpriteOffsetY(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getAnimationFrames(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1;}
    const RME::core::SpriteSheet* getSpriteSheet(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return nullptr;}
    bool usesAlternativeSpriteSheet(uint16_t id, uint16_t subtype) const override {Q_UNUSED(id); Q_UNUSED(subtype); return false;}
};


class TestTileChangeCommand : public QObject
{
    Q_OBJECT

public:
    TestTileChangeCommand();
    ~TestTileChangeCommand() override;

private slots:
    void init();
    void cleanup();

    void testConstruction();
    void testRedo();
    void testUndo();
    void testRedo_NoOldTile();
    void testUndo_NoOldTile();
    void testRedo_NewTileIsNull(); // Delete existing tile
    void testUndo_NewTileIsNull();  // Undo deletion

    void testGetAffectedPositions();
    void testCostCalculation();
    void testMerging_SamePosition();
    void testMerging_DifferentPosition();
    void testMerging_DifferentCommandType();
    void testId();

private:
    RME::core::Map* m_map;
    MockItemTypeProvider m_mockItemTypeProvider;
    RME::core::Position m_testPos;

    // Helper to create a simple tile with one item for testing
    std::unique_ptr<RME::core::Tile> createSimpleTile(uint16_t itemId) {
        auto tile = std::make_unique<RME::core::Tile>(m_testPos, &m_mockItemTypeProvider);
        if (itemId > 0) { // 0 can mean no item or use specific non-item ID
             tile->addItem(RME::core::Item::create(itemId, &m_mockItemTypeProvider));
        }
        return tile;
    }
};

TestTileChangeCommand::TestTileChangeCommand() : m_map(nullptr), m_testPos(1, 2, 0) {
}

TestTileChangeCommand::~TestTileChangeCommand() {
    // cleanup() handles deletion
}

void TestTileChangeCommand::init() {
    m_map = new RME::core::Map(&m_mockItemTypeProvider);
    m_map->resize(10, 10, 1); // A small map
}

void TestTileChangeCommand::cleanup() {
    delete m_map;
    m_map = nullptr;
}

void TestTileChangeCommand::testConstruction() {
    // Setup: Place an initial tile (ID 10) on the map at m_testPos
    auto initialMapTile = createSimpleTile(10); // Tile with item 10
    m_map->setTile(m_testPos, initialMapTile->deepCopy()); // Put a copy on map

    // New state for the command: Tile with item 20
    auto newCommandTileState = createSimpleTile(20);

    // Create command
    // The TileChangeCommand constructor will read m_testPos from m_map for old state
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, std::move(newCommandTileState), nullptr);

    // Verify old state was captured (Item 10)
    // This requires friend class or accessor for m_oldTileStateData, which is not ideal for unit tests.
    // Alternative: Test via undo behavior. For now, assume constructor works if undo works.
    // For this specific test, we'll focus on what's testable externally if direct access isn't available.
    // If TileChangeCommand stored raw Tile pointers for old/new state (it stores unique_ptr),
    // we could compare them, but it stores copies.

    // For now, we will infer correct construction from undo/redo tests.
    // A more direct test would require exposing m_oldTileStateData for inspection or having specific getters for test.
    QVERIFY(cmd != nullptr); // Basic check
}

void TestTileChangeCommand::testRedo() {
    // Initial map state: Tile with item 10
    m_map->setTile(m_testPos, createSimpleTile(10));

    // Command to change to tile with item 20
    auto newState = createSimpleTile(20);
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, std::move(newState), nullptr);

    cmd->redo();

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr);
    QVERIFY(tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(20));
    QVERIFY(!cmd->text().isEmpty());
}

void TestTileChangeCommand::testUndo() {
    // Initial map state: Tile with item 10
    m_map->setTile(m_testPos, createSimpleTile(10));

    // Command to change to tile with item 20
    auto newState = createSimpleTile(20);
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, std::move(newState), nullptr);

    cmd->redo(); // Apply the change
    cmd->undo(); // Undo the change

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr);
    QVERIFY(tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(10)); // Back to original
}

void TestTileChangeCommand::testRedo_NoOldTile() {
    // Map is initially empty at m_testPos
    QVERIFY(m_map->getTile(m_testPos) == nullptr || m_map->getTile(m_testPos)->getItemCount() == 0);

    // Command to create a new tile with item 30
    auto newState = createSimpleTile(30);
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, std::move(newState), nullptr);

    cmd->redo();

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr);
    QVERIFY(tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(30));
}

void TestTileChangeCommand::testUndo_NoOldTile() {
    // Map is initially empty at m_testPos

    // Command to create a new tile with item 30
    auto newState = createSimpleTile(30);
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, std::move(newState), nullptr);

    cmd->redo(); // Create the tile
    cmd->undo(); // Undo creation

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    // Depending on Map::removeTile behavior, it might remove the tile object or just its items.
    // Assuming removeTile nullifies or empties it.
    QVERIFY(tileOnMap == nullptr || tileOnMap->getItemCount() == 0);
}

void TestTileChangeCommand::testRedo_NewTileIsNull() {
    // Initial map state: Tile with item 40
    m_map->setTile(m_testPos, createSimpleTile(40));

    // Command to delete the tile (new state is nullptr)
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, nullptr, nullptr);

    cmd->redo(); // Delete the tile

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap == nullptr || tileOnMap->getItemCount() == 0);
}

void TestTileChangeCommand::testUndo_NewTileIsNull() {
    // Initial map state: Tile with item 40
    m_map->setTile(m_testPos, createSimpleTile(40));

    // Command to delete the tile
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, nullptr, nullptr);

    cmd->redo(); // Delete the tile
    cmd->undo(); // Undo deletion

    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr);
    QVERIFY(tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(40)); // Back to original
}


void TestTileChangeCommand::testGetAffectedPositions() {
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, createSimpleTile(1), nullptr);
    QList<RME::core::Position> affected = cmd->getAffectedPositions();
    QCOMPARE(affected.size(), 1);
    QCOMPARE(affected.first(), m_testPos);
}

void TestTileChangeCommand::testCostCalculation() {
    // Old state: tile with item 10
    auto oldTile = createSimpleTile(10);
    // New state: tile with item 20
    auto newTile = createSimpleTile(20);

    // To properly test cost, we need to simulate TileChangeCommand's constructor logic
    // by having an actual old tile on the map.
    m_map->setTile(m_testPos, oldTile->deepCopy());

    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, newTile->deepCopy(), nullptr);

    size_t expectedCost = sizeof(RME::core::actions::TileChangeCommand);
    // Accessing m_oldTileStateData and m_newTileStateData directly is not possible.
    // We rely on the command correctly capturing/storing them.
    // The cost will be calculated internally based on these.
    // If Tile::estimateMemoryUsage() returns sizeof(Tile) + items_cost,
    // and Item::estimateMemoryUsage() returns sizeof(Item) + const_overhead.
    // A tile with one item might be sizeof(Tile) + sizeof(Item) + overheads.
    // Let's assume Tile::estimateMemoryUsage for a simple tile is > 50 for this test.

    // Cost of old tile (item 10) + Cost of new tile (item 20)
    // This is an indirect test. The actual values come from Tile::estimateMemoryUsage()
    if(oldTile) expectedCost += oldTile->estimateMemoryUsage();
    if(newTile) expectedCost += newTile->estimateMemoryUsage();


    // We cannot directly verify internal old/new states here without accessors or friend class.
    // Instead, check if cost is reasonable.
    int cost = cmd->cost();
    QVERIFY(cost >= sizeof(RME::core::actions::TileChangeCommand));
    if (oldTile && newTile) { // Both states are non-empty tiles
         QVERIFY(cost > sizeof(RME::core::actions::TileChangeCommand) + 100); // Rough check, assumes tiles add significant cost
    }

    // Test with one state null (e.g. creating a new tile where old was null)
    m_map->removeTile(m_testPos); // Ensure no tile at m_testPos
    auto cmdCreate = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, createSimpleTile(30), nullptr); // old is null, new has item 30
    int costCreate = cmdCreate->cost();
    QVERIFY(costCreate > sizeof(RME::core::actions::TileChangeCommand));
    QVERIFY(costCreate < cost); // Should be less than command with two full tiles

    // Test with new state null (deleting a tile)
    m_map->setTile(m_testPos, createSimpleTile(40)); // old has item 40
    auto cmdDelete = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, nullptr, nullptr); // new is null
    int costDelete = cmdDelete->cost();
    QVERIFY(costDelete > sizeof(RME::core::actions::TileChangeCommand));
    QVERIFY(costDelete < cost);
}

void TestTileChangeCommand::testMerging_SamePosition() {
    // State A: Tile with item 50
    auto tileStateA_map = createSimpleTile(50);
    m_map->setTile(m_testPos, tileStateA_map->deepCopy());

    // Command 1: A -> B (item 50 -> item 60)
    auto tileStateB_cmd1 = createSimpleTile(60);
    // text is "Change tile at (1, 2, 0)"
    auto cmd1 = new RME::core::actions::TileChangeCommand(
        m_map, m_testPos, std::move(tileStateB_cmd1), nullptr);
    // cmd1 now stores: old=A (item 50), new=B (item 60)

    // Before creating cmd2, the map must reflect state B for cmd2 to pick up B as its "old" state.
    // This is tricky because QUndoStack applies redo automatically.
    // For manual merge testing, we simulate this. If cmd1 was pushed, map would be B.
    // Let's assume cmd1 has been "redone" conceptually for cmd2's creation.
    m_map->setTile(m_testPos, createSimpleTile(60)); // Map is now state B

    // Command 2: B -> C (item 60 -> item 70)
    auto tileStateC_cmd2 = createSimpleTile(70);
    auto cmd2 = new RME::core::actions::TileChangeCommand( // raw pointer for mergeWith
        m_map, m_testPos, std::move(tileStateC_cmd2), nullptr);
    cmd2->setText("Second Change"); // Differentiate text
    // cmd2 stores: old=B (item 60), new=C (item 70)

    QVERIFY(cmd1->mergeWith(cmd2));
    // Merged cmd1 should be: old=A (item 50), new=C (item 70)
    // Text should be from cmd2 ("Second Change")

    QCOMPARE(cmd1->text(), QString("Second Change"));

    // Verify internal states by undo/redo the merged command
    // Map currently reflects state B (from setting up for cmd2)
    // If we undo merged cmd1: map should go to A (item 50)
    cmd1->undo();
    RME::core::Tile* tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr && tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(50)); // Should be A

    // If we redo merged cmd1: map should go to C (item 70)
    cmd1->redo();
    tileOnMap = m_map->getTile(m_testPos);
    QVERIFY(tileOnMap != nullptr && tileOnMap->getItems().size() == 1);
    QCOMPARE(tileOnMap->getItems().first()->getID(), static_cast<uint16_t>(70)); // Should be C

    delete cmd1; // Was raw for mergeWith
    delete cmd2;
}

void TestTileChangeCommand::testMerging_DifferentPosition() {
    RME::core::Position pos1(1,1,0);
    RME::core::Position pos2(2,2,0);

    m_map->setTile(pos1, createSimpleTile(10));
    m_map->setTile(pos2, createSimpleTile(20));

    auto cmd1 = new RME::core::actions::TileChangeCommand(
        m_map, pos1, createSimpleTile(15), nullptr);
    auto cmd2 = new RME::core::actions::TileChangeCommand(
        m_map, pos2, createSimpleTile(25), nullptr);

    QVERIFY(!cmd1->mergeWith(cmd2));

    delete cmd1;
    delete cmd2;
}

void TestTileChangeCommand::testMerging_DifferentCommandType() {
    m_map->setTile(m_testPos, createSimpleTile(10));
    auto cmd1 = new RME::core::actions::TileChangeCommand(
        m_map, m_testPos, createSimpleTile(15), nullptr);

    // Dummy QUndoCommand (not AppUndoCommand or TileChangeCommand)
    auto dummyCmd = new QUndoCommand();

    QVERIFY(!cmd1->mergeWith(dummyCmd));

    delete cmd1;
    delete dummyCmd;
}

void TestTileChangeCommand::testId() {
    auto cmd = std::make_unique<RME::core::actions::TileChangeCommand>(
        m_map, m_testPos, nullptr, nullptr);
    QCOMPARE(cmd->id(), RME::core::actions::TileChangeCommand::CommandID);
    QVERIFY(RME::core::actions::TileChangeCommand::CommandID != -1); // Ensure it's not default no-merge ID
}


// Macro to run the tests
// QTEST_MAIN(TestTileChangeCommand) // Will be run by rme_core_actions_tests
// For individual execution:
// int main(int argc, char *argv[]) {
//     TestTileChangeCommand tc;
//     return QTest::qExec(&tc, argc, argv);
// }
#include "TestTileChangeCommand.moc" // Must be last line
