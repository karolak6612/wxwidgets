#include "test_undoredo.h"
#include "map/map.h"        // From CORE-03
#include "Tile.h"       // From CORE-01
#include "Item.h"       // From CORE-01
#include "Position.h"   // From CORE-01
#include "actions/appundocommand.h" // Base class
#include "actions/changetilecommand.h"
#include "actions/batchcommand.h"
#include <QUndoStack>
#include <memory> // For std::make_unique

// Helper function to create a simple tile for testing
std::unique_ptr<Tile> createTestTile(uint16_t tile_id_for_item = 0) {
    // Assuming Tile constructor Tile(Position p) is not the primary one.
    // Assuming Tile has a default constructor or one that doesn't need position,
    // as position is context from the map.
    // Or, if Tile needs a position for its internal state even when not on map:
    // Tile(const Position& p);
    // For now, assume a simple Tile that can be created and items added.
    auto tile = std::make_unique<Tile>(); // Use appropriate Tile constructor
    if (tile_id_for_item > 0) {
        // Assuming Item has a constructor Item(uint16_t id) and Tile::addItem(std::unique_ptr<Item>)
        // tile->addItem(std::make_unique<Item>(tile_id_for_item));
    }
    return tile;
}


TestUndoRedo::TestUndoRedo() : m_undo_stack(nullptr), m_mock_map(nullptr) {
}

TestUndoRedo::~TestUndoRedo() {
    // cleanupTestCase should handle this, but good practice for direct deletion if not using QtTest macros fully
    delete m_undo_stack;
    delete m_mock_map;
}

void TestUndoRedo::initTestCase() {
    // Initialize static settings for ChangeTileCommand merging for predictable tests
    ChangeTileCommand::setGroupActions(true); // Enable merging for tests
    ChangeTileCommand::setStackingDelay(500); // 0.5 second delay for tests
}

void TestUndoRedo::cleanupTestCase() {
    // Clean up any global resources if necessary
}

void TestUndoRedo::init() {
    // Called before each test function
    m_mock_map = new Map(); // Assuming Map default constructor is sufficient
    m_undo_stack = new QUndoStack();
}

void TestUndoRedo::cleanup() {
    // Called after each test function
    delete m_undo_stack;
    m_undo_stack = nullptr;
    delete m_mock_map;
    m_mock_map = nullptr;
}

void TestUndoRedo::testQUndoStack_BasicPushUndoRedo() {
    QVERIFY(m_undo_stack->count() == 0);
    QVERIFY(!m_undo_stack->canUndo());
    QVERIFY(!m_undo_stack->canRedo());

    Position pos(10, 20, 7);
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos, createTestTile(1)));

    QVERIFY(m_undo_stack->count() == 1);
    QVERIFY(m_undo_stack->canUndo());
    QVERIFY(!m_undo_stack->canRedo());
    QVERIFY(m_undo_stack->command(0) != nullptr);

    // Check map state after redo (initial push)
    const Tile* tile_after_redo = m_mock_map->getTile(pos);
    QVERIFY(tile_after_redo != nullptr);
    // Add more specific checks if createTestTile(1) results in identifiable features

    m_undo_stack->undo();
    QVERIFY(m_undo_stack->count() == 1); // Command is still there, index changes
    QVERIFY(m_undo_stack->index() == 0);
    QVERIFY(!m_undo_stack->canUndo()); // At the beginning
    QVERIFY(m_undo_stack->canRedo());

    // Check map state after undo (should be original, likely null)
    const Tile* tile_after_undo = m_mock_map->getTile(pos);
    QVERIFY(tile_after_undo == nullptr); // Assuming map was initially empty at pos

    m_undo_stack->redo();
    QVERIFY(m_undo_stack->index() == 1);
    QVERIFY(m_undo_stack->canUndo());
    QVERIFY(!m_undo_stack->canRedo());
    const Tile* tile_after_second_redo = m_mock_map->getTile(pos);
    QVERIFY(tile_after_second_redo != nullptr);
}

void TestUndoRedo::testQUndoStack_UndoLimit() {
    m_undo_stack->setUndoLimit(2);
    Position pos1(1,1,7), pos2(2,2,7), pos3(3,3,7);

    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos1, createTestTile(10)));
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos2, createTestTile(11)));
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos3, createTestTile(12))); // This should push out the first cmd

    QCOMPARE(m_undo_stack->count(), 2); // Only last two commands should be present
    QVERIFY(m_undo_stack->command(0) != nullptr);
    QVERIFY(m_undo_stack->command(1) != nullptr);
    // Check if the correct commands are present by examining their text or properties if possible
    // For example, by checking the position they affect if commands store it accessibly for tests.
    // ChangeTileCommand* cmd0 = dynamic_cast<ChangeTileCommand*>(m_undo_stack->command(0));
    // ChangeTileCommand* cmd1 = dynamic_cast<ChangeTileCommand*>(m_undo_stack->command(1));
    // QVERIFY(cmd0->text().contains("2, 2, 7")); // Assuming text reflects position
    // QVERIFY(cmd1->text().contains("3, 3, 7"));
}

void TestUndoRedo::testChangeTileCommand_UndoRedoLogic() {
    Position pos(5, 5, 7);

    // Initial state: no tile
    QVERIFY(m_mock_map->getTile(pos) == nullptr);

    // Command 1: Place tile A
    auto tileA_data = createTestTile(100); // Tile with item 100
    // Need to get a raw pointer for the command but keep ownership with unique_ptr for a moment
    // Or clone it. ChangeTileCommand takes ownership.
    ChangeTileCommand* cmd1 = new ChangeTileCommand(m_mock_map, pos, std::make_unique<Tile>(*tileA_data));
    m_undo_stack->push(cmd1);

    const Tile* map_tile1 = m_mock_map->getTile(pos);
    QVERIFY(map_tile1 != nullptr);
    // Add specific check for item 100 if Tile::getItems() or similar exists
    // QVERIFY(map_tile1->getItems().first()->getID() == 100);

    // Command 2: Replace with tile B (or clear it)
    auto tileB_data = createTestTile(200); // Tile with item 200
    ChangeTileCommand* cmd2 = new ChangeTileCommand(m_mock_map, pos, std::make_unique<Tile>(*tileB_data));
    m_undo_stack->push(cmd2);

    const Tile* map_tile2 = m_mock_map->getTile(pos);
    QVERIFY(map_tile2 != nullptr);
    // Check for item 200

    // Undo command 2 (restore tile A)
    m_undo_stack->undo();
    const Tile* map_tile_after_undo2 = m_mock_map->getTile(pos);
    QVERIFY(map_tile_after_undo2 != nullptr);
    // Check for item 100

    // Undo command 1 (restore empty)
    m_undo_stack->undo();
    QVERIFY(m_mock_map->getTile(pos) == nullptr);

    // Redo command 1 (place tile A)
    m_undo_stack->redo();
    const Tile* map_tile_after_redo1 = m_mock_map->getTile(pos);
    QVERIFY(map_tile_after_redo1 != nullptr);
    // Check for item 100

    // Redo command 2 (place tile B)
    m_undo_stack->redo();
    const Tile* map_tile_after_redo2 = m_mock_map->getTile(pos);
    QVERIFY(map_tile_after_redo2 != nullptr);
    // Check for item 200
}

void TestUndoRedo::testChangeTileCommand_MemoryManagement() {
    // This is hard to test directly without tool like Valgrind or specific memory tracking.
    // We rely on QUndoStack deleting commands it owns, and unique_ptr in commands deleting tile data.
    // We can test that after commands are pushed and stack cleared, map interactions don't crash.
    Position pos(1,1,1);
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos, createTestTile(1)));
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos, createTestTile(2)));

    m_undo_stack->setUndoLimit(0); // Clear stack
    m_undo_stack->setUndoLimit(10); // Reset limit

    QVERIFY(m_undo_stack->count() == 0);
    // If memory was leaked by commands/tiles, map operations might be unstable, but this isn't a direct test.
    // Accessing map at 'pos' should be fine.
    m_mock_map->setTile(pos, createTestTile(3));
    QVERIFY(m_mock_map->getTile(pos) != nullptr);
    QPASS("Memory management test relies on correct unique_ptr/QUndoStack behavior, not directly verified beyond absence of crashes.");
}

void TestUndoRedo::testChangeTileCommand_Merging() {
    ChangeTileCommand::setGroupActions(true);
    ChangeTileCommand::setStackingDelay(100); // Short delay for testing

    Position pos(7,7,7);

    // Command 1
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos, createTestTile(301)));
    QCOMPARE(m_undo_stack->count(), 1);
    const Tile* tile1 = m_mock_map->getTile(pos);
    QVERIFY(tile1); // Check for item 301 if possible

    // Command 2 (should merge with Command 1)
    // QTest::qWait(50); // Wait less than stacking_delay
    // QUndoStack processes mergeWith synchronously when push is called. Timestamp matters.
    // To ensure timestamp difference, create commands with slight delay or manipulate timestamp if AppUndoCommand allows.
    // For this test, let's assume direct subsequent pushes are close enough in time if not for QTest::qWait.
    // The timestamp is set on command creation.

    // Create second command. Its timestamp will be slightly after the first.
    ChangeTileCommand* cmd2 = new ChangeTileCommand(m_mock_map, pos, createTestTile(302));
    // Manually adjust timestamp for testing if needed, or ensure test execution is fast.
    // Forcing cmd2's timestamp to be very close to cmd1's for reliable merge:
    // cmd2->m_creation_timestamp = m_undo_stack->command(0)->creationTimestamp() + 1; // (If m_creation_timestamp were public/friend)
    // This is tricky. Let's rely on fast execution or assume ChangeTileCommand was updated to allow timestamp injection for tests.
    // The current AppUndoCommand sets timestamp in constructor.

    m_undo_stack->push(cmd2); // cmd2 should merge into the command from step 1.

    QCOMPARE(m_undo_stack->count(), 1); // Still 1 command due to merge
    const Tile* tile_after_merge = m_mock_map->getTile(pos);
    QVERIFY(tile_after_merge); // Check for item 302 (state from cmd2)
    // QVERIFY(m_undo_stack->command(0)->text().contains("merged")); // Check if text updated

    // Command 3 (different position, should not merge)
    Position pos2(8,8,8);
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos2, createTestTile(303)));
    QCOMPARE(m_undo_stack->count(), 2);

    // Command 4 (same position as first, but after long delay - simulated by changing delay)
    ChangeTileCommand::setStackingDelay(1); // Very short delay
    // QTest::qWait(10); // Wait longer than new stacking_delay
    // Create command 4, its timestamp will be naturally later.
    m_undo_stack->push(new ChangeTileCommand(m_mock_map, pos, createTestTile(304)));
    ChangeTileCommand::setStackingDelay(100); // Reset delay
    QCOMPARE(m_undo_stack->count(), 3); // Should not merge with the first command on 'pos'

    // Undo the last command (non-merged one)
    m_undo_stack->undo(); // Undoes cmd for 304 -> map at pos has 302
    const Tile* tile_after_undo_304 = m_mock_map->getTile(pos);
    QVERIFY(tile_after_undo_304); // Check for 302

    m_undo_stack->undo(); // Undoes cmd for 303 (pos2)
    m_undo_stack->undo(); // Undoes merged command (pos, originally 301, then 302) -> map at pos is empty
    QVERIFY(m_mock_map->getTile(pos) == nullptr);
}

void TestUndoRedo::testChangeTileCommand_GetChangedPositions() {
    Position pos(1,2,3);
    auto cmd = new ChangeTileCommand(m_mock_map, pos, createTestTile(1));
    m_undo_stack->push(cmd);

    QList<Position> changed_positions = cmd->getChangedPositions();
    QCOMPARE(changed_positions.size(), 1);
    QVERIFY(changed_positions.contains(pos));
}

void TestUndoRedo::testBatchCommand_UndoRedoLogic() {
    Position posA(10,10,7);
    Position posB(11,11,7);

    QList<QUndoCommand*> commands_for_batch;
    commands_for_batch.append(new ChangeTileCommand(m_mock_map, posA, createTestTile(401)));
    commands_for_batch.append(new ChangeTileCommand(m_mock_map, posB, createTestTile(402)));

    BatchCommand* batch_cmd = new BatchCommand(m_mock_map, commands_for_batch, "Test Batch");
    m_undo_stack->push(batch_cmd);

    // Check map state after redo (initial push)
    QVERIFY(m_mock_map->getTile(posA) != nullptr); // Check for 401
    QVERIFY(m_mock_map->getTile(posB) != nullptr); // Check for 402

    m_undo_stack->undo();
    QVERIFY(m_mock_map->getTile(posA) == nullptr); // Assuming map was empty
    QVERIFY(m_mock_map->getTile(posB) == nullptr); // Assuming map was empty

    m_undo_stack->redo();
    QVERIFY(m_mock_map->getTile(posA) != nullptr); // Check for 401
    QVERIFY(m_mock_map->getTile(posB) != nullptr); // Check for 402
}

// To run the tests, ensure your main.cpp or test runner includes:
// TestUndoRedo testUndoRedo;
// QTest::qExec(&testUndoRedo);
