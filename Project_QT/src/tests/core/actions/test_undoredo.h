#ifndef TEST_UNDOREDO_H
#define TEST_UNDOREDO_H

#include <QtTest/QtTest>
#include <QObject>

// Forward declarations
class QUndoStack;
class Map;
class Tile;
class Position;
class ChangeTileCommand;

class TestUndoRedo : public QObject
{
    Q_OBJECT

public:
    TestUndoRedo();
    ~TestUndoRedo() override;

private slots:
    void initTestCase();    // Called before the first test function
    void cleanupTestCase(); // Called after the last test function
    void init();            // Called before each test function
    void cleanup();         // Called after each test function

    // Test methods
    void testQUndoStack_BasicPushUndoRedo();
    void testQUndoStack_UndoLimit();
    void testChangeTileCommand_UndoRedoLogic();
    void testChangeTileCommand_MemoryManagement(); // Verify unique_ptrs release data
    void testChangeTileCommand_Merging();
    void testChangeTileCommand_GetChangedPositions();
    void testBatchCommand_UndoRedoLogic();
    // void testBatchCommand_Merging(); // If BatchCommand supports merging

private:
    QUndoStack* m_undo_stack;
    Map* m_mock_map; // Using a real Map object for simplicity, assuming it's lightweight enough
                     // Or a dedicated mock map class could be used.
};

#endif // TEST_UNDOREDO_H
