#include <QtTest/QtTest>
#include <QUndoStack>

#include "core/actions/ChangeSetCommand.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Position.h"
#include "core/IItemTypeProvider.h"

// Copied MockItemTypeProvider (ideally from a shared test utility)
class MockItemTypeProvider : public RME::core::IItemTypeProvider {
public:
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

class TestChangeSetCommand : public QObject
{
    Q_OBJECT

public:
    TestChangeSetCommand();
    ~TestChangeSetCommand() override;

private slots:
    void init();
    void cleanup();

    void testConstruction();
    void testRedo_MultipleChanges();
    void testUndo_MultipleChanges();
    void testRedo_WithTileRemoval();
    void testUndo_WithTileRemoval();
    void testRedo_AddingToEmpty();
    void testUndo_AddingToEmpty();
    void testGetAffectedPositions();
    void testCostCalculation();
    void testMerging_IsDisabled();

private:
    RME::core::Map* m_map;
    MockItemTypeProvider m_mockItemTypeProvider;

    // Helper to create a simple tile with one item
    std::unique_ptr<RME::core::Tile> createSimpleTile(const RME::core::Position& pos, uint16_t itemId) {
        auto tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
        if (itemId > 0) { // Assuming 0 means no specific item, or use a dedicated "empty" ID
            tile->addItem(RME::core::Item::create(itemId, &m_mockItemTypeProvider));
        }
        return tile;
    }

    // Helper to get item ID from a tile's first item, or 0 if no items
    uint16_t getTileItemID(const RME::core::Tile* tile) {
        if (tile && !tile->getItems().isEmpty()) {
            return tile->getItems().first()->getID();
        }
        if (tile && tile->getGround()) { // Check ground if no top items
            return tile->getGround()->getID();
        }
        return 0; // 0 if no items or tile is null
    }
};

TestChangeSetCommand::TestChangeSetCommand() : m_map(nullptr) {
}

TestChangeSetCommand::~TestChangeSetCommand() {
    // cleanup() handles deletion
}

void TestChangeSetCommand::init() {
    m_map = new RME::core::Map(&m_mockItemTypeProvider);
    m_map->resize(10, 10, 1);
}

void TestChangeSetCommand::cleanup() {
    delete m_map;
    m_map = nullptr;
}

void TestChangeSetCommand::testConstruction() {
    RME::core::Position pos1(1,1,0);
    RME::core::Position pos2(1,2,0);

    // Initial map state
    m_map->setTile(pos1, createSimpleTile(pos1, 10)); // Tile at pos1 has item 10
    // pos2 is initially empty

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    // Change 1: Modify tile at pos1 to item 11
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 11)));
    // Change 2: Add new tile at pos2 with item 12
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 12)));

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "TestConstruct", nullptr);

    // To verify internal m_changes, we'd need access.
    // We'll infer from undo/redo and getAffectedPositions.
    // For now, check basic properties.
    QCOMPARE(cmd->text(), QString("TestConstruct"));
    QList<RME::core::Position> affected = cmd->getAffectedPositions();
    QCOMPARE(affected.size(), 2);
    QVERIFY(affected.contains(pos1));
    QVERIFY(affected.contains(pos2));

    // A more thorough test would involve creating the command, then undoing it,
    // and checking if the map reverted to the state *before* the command was constructed.
    // This means m_oldTileStateData was captured correctly.
    cmd->redo(); // Apply changes
    cmd->undo(); // Revert to original state

    QCOMPARE(getTileItemID(m_map->getTile(pos1)), static_cast<uint16_t>(10));
    QVERIFY(m_map->getTile(pos2) == nullptr || m_map->getTile(pos2)->getItemCount() == 0);
}

void TestChangeSetCommand::testRedo_MultipleChanges() {
    RME::core::Position pos1(2,1,0);
    RME::core::Position pos2(2,2,0);
    RME::core::Position pos3(2,3,0); // Initially empty

    // Initial map state
    m_map->setTile(pos1, createSimpleTile(pos1, 20)); // Item 20
    m_map->setTile(pos2, createSimpleTile(pos2, 21)); // Item 21

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 101))); // Change item 20 -> 101
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 102))); // Change item 21 -> 102
    initialChanges.append(qMakePair(pos3, createSimpleTile(pos3, 103))); // Add item 103 to empty

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "RedoMulti", nullptr);

    cmd->redo();

    QCOMPARE(getTileItemID(m_map->getTile(pos1)), static_cast<uint16_t>(101));
    QCOMPARE(getTileItemID(m_map->getTile(pos2)), static_cast<uint16_t>(102));
    QCOMPARE(getTileItemID(m_map->getTile(pos3)), static_cast<uint16_t>(103));
    QCOMPARE(cmd->text(), QString("RedoMulti"));
}

void TestChangeSetCommand::testUndo_MultipleChanges() {
    RME::core::Position pos1(3,1,0);
    RME::core::Position pos2(3,2,0);
    RME::core::Position pos3(3,3,0); // Initially empty

    // Initial map state
    m_map->setTile(pos1, createSimpleTile(pos1, 30)); // Original item 30
    m_map->setTile(pos2, createSimpleTile(pos2, 31)); // Original item 31

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 201))); // Change 30 -> 201
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 202))); // Change 31 -> 202
    initialChanges.append(qMakePair(pos3, createSimpleTile(pos3, 203))); // Add 203 to empty

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "UndoMulti", nullptr);

    cmd->redo(); // Apply changes
    cmd->undo(); // Revert

    QCOMPARE(getTileItemID(m_map->getTile(pos1)), static_cast<uint16_t>(30)); // Back to 30
    QCOMPARE(getTileItemID(m_map->getTile(pos2)), static_cast<uint16_t>(31)); // Back to 31
    QVERIFY(m_map->getTile(pos3) == nullptr || m_map->getTile(pos3)->getItemCount() == 0); // Back to empty
}

void TestChangeSetCommand::testRedo_WithTileRemoval() {
    RME::core::Position pos1(4,1,0); // Will be removed
    RME::core::Position pos2(4,2,0); // Will be changed

    m_map->setTile(pos1, createSimpleTile(pos1, 40)); // Item 40
    m_map->setTile(pos2, createSimpleTile(pos2, 41)); // Item 41

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, nullptr)); // Remove tile at pos1
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 141))); // Change item 41 -> 141

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "RedoRemove", nullptr);
    cmd->redo();

    QVERIFY(m_map->getTile(pos1) == nullptr || m_map->getTile(pos1)->getItemCount() == 0);
    QCOMPARE(getTileItemID(m_map->getTile(pos2)), static_cast<uint16_t>(141));
}

void TestChangeSetCommand::testUndo_WithTileRemoval() {
    RME::core::Position pos1(5,1,0); // Originally has item 50, will be removed, then restored
    RME::core::Position pos2(5,2,0); // Originally has item 51, will be changed to 151, then restored

    m_map->setTile(pos1, createSimpleTile(pos1, 50));
    m_map->setTile(pos2, createSimpleTile(pos2, 51));

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, nullptr)); // Remove tile at pos1
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 151))); // Change item 51 -> 151

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "UndoRemove", nullptr);
    cmd->redo();
    cmd->undo();

    QCOMPARE(getTileItemID(m_map->getTile(pos1)), static_cast<uint16_t>(50)); // Restored
    QCOMPARE(getTileItemID(m_map->getTile(pos2)), static_cast<uint16_t>(51)); // Restored
}

void TestChangeSetCommand::testRedo_AddingToEmpty() {
    RME::core::Position pos1(6,1,0); // Starts empty, add item 60
    QVERIFY(m_map->getTile(pos1) == nullptr || m_map->getTile(pos1)->getItemCount() == 0);

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 60)));

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "RedoAddEmpty", nullptr);
    cmd->redo();
    QCOMPARE(getTileItemID(m_map->getTile(pos1)), static_cast<uint16_t>(60));
}

void TestChangeSetCommand::testUndo_AddingToEmpty() {
    RME::core::Position pos1(7,1,0); // Starts empty
    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 70)));

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "UndoAddEmpty", nullptr);
    cmd->redo();
    cmd->undo();
    QVERIFY(m_map->getTile(pos1) == nullptr || m_map->getTile(pos1)->getItemCount() == 0); // Back to empty
}

void TestChangeSetCommand::testGetAffectedPositions() {
    RME::core::Position pos1(1,1,0);
    RME::core::Position pos2(1,2,0);
    RME::core::Position pos3(1,3,0);

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 1)));
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 2)));
    initialChanges.append(qMakePair(pos3, nullptr)); // Remove

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "GetAffected", nullptr);

    QList<RME::core::Position> affected = cmd->getAffectedPositions();
    QCOMPARE(affected.size(), 3);
    QVERIFY(affected.contains(pos1));
    QVERIFY(affected.contains(pos2));
    QVERIFY(affected.contains(pos3));
}

void TestChangeSetCommand::testCostCalculation() {
    RME::core::Position pos1(8,1,0);
    RME::core::Position pos2(8,2,0);

    // Initial map state for calculating old states
    m_map->setTile(pos1, createSimpleTile(pos1, 80)); // Item 80
    // pos2 is empty

    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> initialChanges;
    // Change 1: pos1 (item 80) -> item 81
    initialChanges.append(qMakePair(pos1, createSimpleTile(pos1, 81)));
    // Change 2: pos2 (empty) -> item 82
    initialChanges.append(qMakePair(pos2, createSimpleTile(pos2, 82)));

    auto cmd = std::make_unique<RME::core::actions::ChangeSetCommand>(
        m_map, initialChanges, "CostTest", nullptr);

    int cost = cmd->cost();
    size_t expectedMinCost = sizeof(RME::core::actions::ChangeSetCommand);
    // Cost of 2x TileChange structs in QList + 3 non-null Tiles (1 old, 2 new)
    // TileChange struct contains 2 unique_ptrs and a Position.
    expectedMinCost += 2 * sizeof(RME::core::actions::ChangeSetCommand::TileChange);
    // Add cost of 3 tiles (old for pos1, new for pos1, new for pos2)
    // Assuming a simple tile costs at least 50 (from Tile::estimateMemoryUsage rough estimate)
    expectedMinCost += 3 * 50;


    QVERIFY(cost >= expectedMinCost);
    QVERIFY(cost > 1); // Must be at least 1
}

void TestChangeSetCommand::testMerging_IsDisabled() {
    RME::core::Position pos1(9,1,0);
    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> changes1;
    changes1.append(qMakePair(pos1, createSimpleTile(pos1, 90)));
    auto cmd1 = new RME::core::actions::ChangeSetCommand(m_map, changes1, "Cmd1", nullptr);

    RME::core::Position pos2(9,2,0); // Different pos for simplicity, though content doesn't matter for disabled merge
    QList<QPair<RME::core::Position, std::unique_ptr<RME::core::Tile>>> changes2;
    changes2.append(qMakePair(pos2, createSimpleTile(pos2, 91)));
    auto cmd2 = new RME::core::actions::ChangeSetCommand(m_map, changes2, "Cmd2", nullptr);

    QVERIFY(!cmd1->mergeWith(cmd2));
    QCOMPARE(cmd1->id(), -1); // Default non-merging ID

    delete cmd1;
    delete cmd2;
}

// QTEST_MAIN(TestChangeSetCommand) // For individual execution
#include "TestChangeSetCommand.moc" // Must be last line
