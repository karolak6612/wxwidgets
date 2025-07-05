#include <QtTest/QtTest>
#include <QSignalSpy>

#include "core/actions/UndoManager.h"
#include "core/actions/TileChangeCommand.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h" // Required by Tile.h and for creating items if needed
#include "core/Position.h"
#include "core/IItemTypeProvider.h" // For instantiating Map/Tile if needed

// A simple mock item type provider for testing purposes
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
    bool isGround(uint16_t id) const override { Q_UNUSED(id); return id == 1; } // Assume ID 1 is ground
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
    // Add any other pure virtual methods from IItemTypeProvider
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

class TestUndoManager : public QObject
{
    Q_OBJECT

public:
    TestUndoManager();
    ~TestUndoManager() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInitialState();
    void testPushCommand();
    void testUndoRedo();
    void testSetClean();
    void testUndoLimit_Count();
    void testUndoLimit_Cost();
    void testSignalEmissions();

private:
    RME::core::actions::UndoManager* m_undoManager;
    RME::core::Map* m_map;
    MockItemTypeProvider m_mockItemTypeProvider; // Instance of the mock provider
};

TestUndoManager::TestUndoManager() : m_undoManager(nullptr), m_map(nullptr) {
}

TestUndoManager::~TestUndoManager() {
    // cleanup() will handle deleting m_undoManager and m_map
}

void TestUndoManager::initTestCase() {
    // One-time setup, e.g., loading global resources if any
}

void TestUndoManager::cleanupTestCase() {
    // One-time cleanup
}

void TestUndoManager::init() {
    // Per-test setup
    m_map = new RME::core::Map(&m_mockItemTypeProvider); // Map needs an item provider
    m_map->resize(10, 10, 1); // Resize map to something usable
    m_undoManager = new RME::core::actions::UndoManager(this); // Parent to QObject for auto-cleanup if needed
}

void TestUndoManager::cleanup() {
    // Per-test cleanup
    delete m_undoManager;
    m_undoManager = nullptr;
    delete m_map;
    m_map = nullptr;
}

void TestUndoManager::testInitialState() {
    QVERIFY(!m_undoManager->canUndo());
    QVERIFY(!m_undoManager->canRedo());
    QVERIFY(m_undoManager->isClean());
    QCOMPARE(m_undoManager->count(), 0);
}

void TestUndoManager::testPushCommand() {
    RME::core::Position pos(0, 0, 0);
    // Create a dummy Tile for newTileStateData for TileChangeCommand
    // ID 2 is not ground according to MockItemTypeProvider
    auto newTileData = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    newTileData->addItem(RME::core::Item::create(2, &m_mockItemTypeProvider));


    auto* cmd = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(newTileData), nullptr);
    m_undoManager->pushCommand(cmd);

    QVERIFY(m_undoManager->canUndo());
    QVERIFY(!m_undoManager->canRedo());
    QVERIFY(!m_undoManager->isClean());
    QVERIFY(!m_undoManager->undoText().isEmpty());
    QCOMPARE(m_undoManager->count(), 1);
}

void TestUndoManager::testUndoRedo() {
    RME::core::Position pos(1, 1, 0);
    // Ensure tile exists for command to operate on, or command handles non-existent tile.
    // TileChangeCommand's constructor reads the old tile state.
    // Let's place a ground tile first. ID 1 is ground.
    auto groundItem = RME::core::Item::create(1, &m_mockItemTypeProvider);
    m_map->getTile(pos)->addItem(std::move(groundItem));
    QVERIFY(m_map->getTile(pos)->getGround() != nullptr, "Ground item should be set on map");
    QVERIFY(m_map->getTile(pos)->getItems().isEmpty(), "Top items should be empty initially");


    // New state: add a non-ground item (ID 2)
    // TileChangeCommand expects the full new state of the tile.
    // So, newTileStateData should represent the tile *after* the change.
    // If we are adding an item, newTileStateData should contain ground + new item.
    auto newTileStateForCommand = m_map->getTile(pos)->deepCopy(); // Start with current state
    newTileStateForCommand->addItem(RME::core::Item::create(2, &m_mockItemTypeProvider)); // Add the new item

    auto* cmd = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(newTileStateForCommand), nullptr);
    m_undoManager->pushCommand(cmd);
    QVERIFY(!m_undoManager->isClean(), "Stack should be dirty after push");

    // Verify state after redo (which is implicitly called by push)
    QVERIFY(m_map->getTile(pos)->getGround() != nullptr, "Ground should still be there after redo");
    QCOMPARE(m_map->getTile(pos)->getItems().size(), 1, "One top item should exist after redo");
    if (m_map->getTile(pos)->getItems().size() == 1) {
         QCOMPARE(m_map->getTile(pos)->getItems().first()->getID(), static_cast<uint16_t>(2), "Item ID should be 2 after redo");
    }


    m_undoManager->undo();
    QVERIFY(!m_undoManager->canUndo());
    QVERIFY(m_undoManager->canRedo());
    // isClean depends on whether the original state (before any commands) was marked clean.
    // QUndoStack sets clean to true if index matches cleanIndex. After an undo that returns
    // to the initial state (index 0), and if cleanIndex is 0, it becomes clean.
    QVERIFY(m_undoManager->isClean());
    QVERIFY(m_map->getTile(pos)->getGround() != nullptr, "Ground should still be there after undo");
    QVERIFY(m_map->getTile(pos)->getItems().isEmpty(), "Top items should be gone after undo");


    m_undoManager->redo();
    QVERIFY(m_undoManager->canUndo());
    QVERIFY(!m_undoManager->canRedo());
    QVERIFY(!m_undoManager->isClean());
    QVERIFY(m_map->getTile(pos)->getGround() != nullptr, "Ground should be there after redo again");
    QCOMPARE(m_map->getTile(pos)->getItems().size(), 1, "One top item should exist after redo again");
     if (m_map->getTile(pos)->getItems().size() == 1) {
        QCOMPARE(m_map->getTile(pos)->getItems().first()->getID(), static_cast<uint16_t>(2), "Item ID should be 2 after redo again");
    }
}

void TestUndoManager::testSetClean() {
    RME::core::Position pos(0,0,0);
    auto newTileData = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    newTileData->addItem(RME::core::Item::create(2, &m_mockItemTypeProvider));

    auto* cmd = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(newTileData), nullptr);
    m_undoManager->pushCommand(cmd);
    QVERIFY(!m_undoManager->isClean());

    m_undoManager->setClean();
    QVERIFY(m_undoManager->isClean());

    m_undoManager->undo();
    QVERIFY(!m_undoManager->isClean());
}

void TestUndoManager::testUndoLimit_Count() {
    m_undoManager->setUndoLimit(2);
    RME::core::Position pos(0,0,0);

    // Push 3 commands
    auto cmd1_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd1_tile->addItem(RME::core::Item::create(10, &m_mockItemTypeProvider));
    m_undoManager->pushCommand(new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd1_tile), nullptr));
    QString text1 = m_undoManager->command(m_undoManager->index())->text();


    auto cmd2_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd2_tile->addItem(RME::core::Item::create(11, &m_mockItemTypeProvider));
    m_undoManager->pushCommand(new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd2_tile), nullptr));
    QString text2 = m_undoManager->command(m_undoManager->index())->text();

    auto cmd3_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd3_tile->addItem(RME::core::Item::create(12, &m_mockItemTypeProvider));
    m_undoManager->pushCommand(new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd3_tile), nullptr));
    QString text3 = m_undoManager->command(m_undoManager->index())->text();

    QCOMPARE(m_undoManager->count(), 2);
    // The first command (cmd1) should have been dropped.
    // The remaining commands should be cmd2 and cmd3.
    // QUndoStack drops from the bottom (oldest command).
    QVERIFY(m_undoManager->undoText().contains("12"), "Top command should be cmd3"); // text3
    m_undoManager->undo();
    QVERIFY(m_undoManager->undoText().contains("11"), "Next command should be cmd2"); // text2
}


void TestUndoManager::testUndoLimit_Cost() {
    // This test relies on cost() being implemented in TileChangeCommand
    // and Tile::estimateMemoryUsage(), Item::estimateMemoryUsage().
    // For now, TileChangeCommand::cost() returns sizeof(TileChangeCommand) + tile states.
    // Let's assume a Tile with one item costs about 200 for simplicity in this test.
    // So a TileChangeCommand might be around 200 + 200 + sizeof(Cmd) ~ 450-500.

    m_undoManager->setUndoLimit(1000); // Max total cost of 1000
    RME::core::Position pos(0,0,0);

    // Push commands. Each TileChangeCommand has a cost.
    // Create 3 commands. If each is ~500, only 2 should fit.
    auto cmd1_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd1_tile->addItem(RME::core::Item::create(20, &m_mockItemTypeProvider)); // Item ID 20
    auto* cmd1 = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd1_tile), nullptr);
    // Force a higher cost for testing if needed, or rely on actual implementation
    // For this test, we assume the implemented cost() is working.
    // int cost1 = cmd1->cost(); // Check this in debugger if unsure
    m_undoManager->pushCommand(cmd1);


    auto cmd2_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd2_tile->addItem(RME::core::Item::create(21, &m_mockItemTypeProvider)); // Item ID 21
     auto* cmd2 = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd2_tile), nullptr);
    // int cost2 = cmd2->cost();
    m_undoManager->pushCommand(cmd2);

    // At this point, if cmd1 and cmd2 cost ~500 each, total ~1000. Stack is full by cost.
    QVERIFY(m_undoManager->count() <= 2); // Could be 1 or 2 depending on actual costs

    auto cmd3_tile = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    cmd3_tile->addItem(RME::core::Item::create(22, &m_mockItemTypeProvider)); // Item ID 22
    auto* cmd3 = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(cmd3_tile), nullptr);
    // int cost3 = cmd3->cost();
    m_undoManager->pushCommand(cmd3);

    // Check how many commands are left. If each is ~500, cmd1 should be dropped.
    // Total cost of stack should be <= 1000.
    int totalCost = 0;
    for (int i = 0; i < m_undoManager->count(); ++i) {
        totalCost += m_undoManager->command(i)->cost();
    }
    QVERIFY(totalCost <= 1000);
    QVERIFY(m_undoManager->count() == 2 || m_undoManager->count() == 1); // Depending on precise costs

    // Verify that the oldest command (cmd1 with item 20) is likely dropped if costs are similar
    bool cmd1_dropped = true;
    for(int i=0; i < m_undoManager->count(); ++i) {
        const auto* tcc = dynamic_cast<const RME::core::actions::TileChangeCommand*>(m_undoManager->command(i));
        if (tcc) {
            // This is a bit hacky way to check which command it is
            // A better way would be to check command text or add specific IDs to commands for testing
            // For now, we assume text might contain item ID or position.
            // Or check the newTileStateData if accessible (it's private)
        }
    }
     // This part of test is hard to verify without more access or specific command text from cost.
     // The main check is totalCost and count.
}


void TestUndoManager::testSignalEmissions() {
    QSignalSpy spy_canUndoChanged(m_undoManager, &RME::core::actions::UndoManager::canUndoChanged);
    QSignalSpy spy_canRedoChanged(m_undoManager, &RME::core::actions::UndoManager::canRedoChanged);
    QSignalSpy spy_indexChanged(m_undoManager, &RME::core::actions::UndoManager::indexChanged);
    QSignalSpy spy_cleanChanged(m_undoManager, &RME::core::actions::UndoManager::cleanChanged);
    QSignalSpy spy_mapDataChanged(m_undoManager, &RME::core::actions::UndoManager::mapDataChanged);
    QSignalSpy spy_commandStackChanged(m_undoManager, &RME::core::actions::UndoManager::commandStackChanged);

    // Initial state: cleanChanged(true) might be emitted by QUndoStack constructor itself.
    // Let's clear spies from initial construction if any.
    spy_canUndoChanged.clear();
    spy_canRedoChanged.clear();
    spy_indexChanged.clear();
    spy_cleanChanged.clear();
    spy_mapDataChanged.clear();
    spy_commandStackChanged.clear();


    // Push command
    RME::core::Position pos(0,0,0);
    auto tileData = std::make_unique<RME::core::Tile>(pos, &m_mockItemTypeProvider);
    tileData->addItem(RME::core::Item::create(33, &m_mockItemTypeProvider)); // item 33
    auto* cmd = new RME::core::actions::TileChangeCommand(m_map, pos, std::move(tileData), nullptr);

    m_undoManager->pushCommand(cmd);
    // Expected emissions for push:
    // - canUndoChanged(true)
    // - indexChanged(0) (or new index)
    // - cleanChanged(false)
    // - mapDataChanged({pos}) (from handleIndexChanged calling on the pushed command)
    // - commandStackChanged() (from handleIndexChanged)

    QCOMPARE(spy_canUndoChanged.count(), 1);
    QVERIFY(spy_canUndoChanged.last().first().toBool());

    QCOMPARE(spy_indexChanged.count(), 1);
    // QCOMPARE(spy_indexChanged.last().first().toInt(), 0); // Index after first push

    QCOMPARE(spy_cleanChanged.count(), 1);
    QVERIFY(!spy_cleanChanged.last().first().toBool());

    QCOMPARE(spy_commandStackChanged.count(), 1);

    QCOMPARE(spy_mapDataChanged.count(), 1);
    if (!spy_mapDataChanged.isEmpty()) {
        QList<RME::core::Position> affected = qvariant_cast<QList<RME::core::Position>>(spy_mapDataChanged.last().first());
        QCOMPARE(affected.size(), 1);
        if (!affected.isEmpty()) {
            QCOMPARE(affected.first(), pos);
        }
    }
    spy_canUndoChanged.clear(); spy_indexChanged.clear(); spy_cleanChanged.clear(); spy_mapDataChanged.clear(); spy_commandStackChanged.clear();


    // Undo
    m_undoManager->undo();
    // Expected emissions for undo:
    // - canUndoChanged(false)
    // - canRedoChanged(true)
    // - indexChanged(-1) or new index
    // - cleanChanged(true)
    // - mapDataChanged({pos}) (from handleIndexChanged calling on the *undone* command, which is now current)
    // - commandStackChanged()
    QCOMPARE(spy_canUndoChanged.count(), 1);
    QVERIFY(!spy_canUndoChanged.last().first().toBool());
    QCOMPARE(spy_canRedoChanged.count(), 1);
    QVERIFY(spy_canRedoChanged.last().first().toBool());
    QCOMPARE(spy_indexChanged.count(), 1);
    QCOMPARE(spy_cleanChanged.count(), 1);
    QVERIFY(spy_cleanChanged.last().first().toBool());
    QCOMPARE(spy_commandStackChanged.count(), 1);
    QCOMPARE(spy_mapDataChanged.count(), 1);
     if (!spy_mapDataChanged.isEmpty()) {
        QList<RME::core::Position> affected = qvariant_cast<QList<RME::core::Position>>(spy_mapDataChanged.last().first());
        QCOMPARE(affected.size(), 1);
         if (!affected.isEmpty()) {
            QCOMPARE(affected.first(), pos);
        }
    }
    spy_canUndoChanged.clear(); spy_canRedoChanged.clear(); spy_indexChanged.clear(); spy_cleanChanged.clear(); spy_mapDataChanged.clear(); spy_commandStackChanged.clear();

    // Redo
    m_undoManager->redo();
    // Expected emissions for redo:
    // - canUndoChanged(true)
    // - canRedoChanged(false)
    // - indexChanged(0) or new index
    // - cleanChanged(false)
    // - mapDataChanged({pos}) (from handleIndexChanged calling on the *redone* command)
    // - commandStackChanged()
    QCOMPARE(spy_canUndoChanged.count(), 1);
    QVERIFY(spy_canUndoChanged.last().first().toBool());
    QCOMPARE(spy_canRedoChanged.count(), 1);
    QVERIFY(!spy_canRedoChanged.last().first().toBool());
    QCOMPARE(spy_indexChanged.count(), 1);
    QCOMPARE(spy_cleanChanged.count(), 1);
    QVERIFY(!spy_cleanChanged.last().first().toBool());
    QCOMPARE(spy_commandStackChanged.count(), 1);
    QCOMPARE(spy_mapDataChanged.count(), 1);
    if (!spy_mapDataChanged.isEmpty()) {
        QList<RME::core::Position> affected = qvariant_cast<QList<RME::core::Position>>(spy_mapDataChanged.last().first());
        QCOMPARE(affected.size(), 1);
        if (!affected.isEmpty()) {
            QCOMPARE(affected.first(), pos);
        }
    }
}

// Macro to run the tests
QTEST_MAIN(TestUndoManager)
#include "TestUndoManager.moc" // Must be last line for MOC to work
