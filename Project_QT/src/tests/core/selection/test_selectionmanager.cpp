#include <QtTest>
#include <QUndoStack>
#include "Project_QT/src/core/selection/SelectionManager.h"
#include "Project_QT/src/core/selection/SelectionCommand.h"
#include "Project_QT/src/tests/core/mocks/MockMapElements.h"

// Test class
class TestSelectionManager : public QObject {
    Q_OBJECT

public:
    TestSelectionManager();
    ~TestSelectionManager();

private slots:
    void init();
    void cleanup();

    void testInitialState();
    void testAddRemoveTileSelection();
    void testAddItemSelection();
    void testClearSelection();
    void testUndoRedoSelection();
    // Add more tests for creatures, spawns, toggle, multiple selections etc.

private:
    RME::Map* m_mockMap;
    QUndoStack* m_undoStack;
    RME::SelectionManager* m_selectionManager;

    MockTile* m_tile1;
    MockTile* m_tile2;
    MockItem* m_item1_t1;
    MockItem* m_item2_t1;
    MockItem* m_item1_t2; // For clear test
};

TestSelectionManager::TestSelectionManager() :
    m_mockMap(nullptr), m_undoStack(nullptr), m_selectionManager(nullptr),
    m_tile1(nullptr), m_tile2(nullptr), m_item1_t1(nullptr), m_item2_t1(nullptr), m_item1_t2(nullptr)
{}

TestSelectionManager::~TestSelectionManager() {
    cleanup(); // Ensure cleanup if object is destroyed prematurely
}

void TestSelectionManager::init() {
    m_mockMap = new MockMap();
    m_undoStack = new QUndoStack(this);
    m_selectionManager = new RME::SelectionManager(m_mockMap, m_undoStack, this);

    m_tile1 = static_cast<MockTile*>(m_mockMap->getOrCreateTile(RME::Position(10, 10, 7)));
    m_tile2 = static_cast<MockTile*>(m_mockMap->getOrCreateTile(RME::Position(11, 10, 7)));

    m_item1_t1 = new MockItem(101);
    m_item2_t1 = new MockItem(102);
    m_tile1->addItem(m_item1_t1, false);
    m_tile1->addItem(m_item2_t1, false);

    m_item1_t2 = new MockItem(201);
    m_tile2->addItem(m_item1_t2, false);
}

void TestSelectionManager::cleanup() {
    // QUndoStack is a child of TestSelectionManager, will be deleted with `this` by Qt if not explicitly.
    // Or, delete m_undoStack first if it wasn't parented.
    // SelectionManager is also parented.
    // MockMap is not parented, delete it.
    delete m_mockMap;
    m_mockMap = nullptr;

    // Nullify pointers to objects managed by MockMap/MockTile destructors
    m_tile1 = nullptr;
    m_tile2 = nullptr;
    m_item1_t1 = nullptr;
    m_item2_t1 = nullptr;
    m_item1_t2 = nullptr;

    // If undoStack and selectionManager were not parented to this QObject, they'd need:
    // delete m_selectionManager; m_selectionManager = nullptr;
    // delete m_undoStack; m_undoStack = nullptr;
    // But since they are QObject children of 'this', they are auto-deleted.
    // However, it's safer to clear the stack to avoid issues if tests run sequentially in some runners.
    if (m_undoStack) {
        m_undoStack->clear(); // Clear commands
    }
}

void TestSelectionManager::testInitialState() {
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
    QVERIFY(!m_selectionManager->isSelected(m_tile1));
    QVERIFY(!m_selectionManager->isSelected(m_tile1, m_item1_t1));
}

void TestSelectionManager::testAddRemoveTileSelection() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addTile(m_tile1);
    m_selectionManager->finishSelectionChange("Select Tile 1");

    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_selectionManager->isSelected(m_tile1));
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);
    QVERIFY(m_selectionManager->getSelectedTiles().contains(m_tile1));

    m_selectionManager->startSelectionChange();
    m_selectionManager->removeTile(m_tile1);
    m_selectionManager->finishSelectionChange("Deselect Tile 1");

    QVERIFY(!m_tile1->isSelected());
    QVERIFY(!m_selectionManager->isSelected(m_tile1));
    // After removing the only selected tile, m_selectedTiles in SelectionManager should be empty.
    // This depends on SelectionCommand::applyChanges correctly updating m_selectedTiles via tile->hasSelectedElements().
    QVERIFY(!m_tile1->hasSelectedElements()); // Verify mock state
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
}

void TestSelectionManager::testAddItemSelection() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addItem(m_tile1, m_item1_t1);
    m_selectionManager->finishSelectionChange("Select Item 1 on Tile 1");

    QVERIFY(m_item1_t1->isSelected());
    QVERIFY(m_selectionManager->isSelected(m_tile1, m_item1_t1));
    QVERIFY(m_tile1->hasSelectedElements());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);
    QVERIFY(m_selectionManager->getSelectedTiles().contains(m_tile1));

    m_selectionManager->startSelectionChange();
    m_selectionManager->removeItem(m_tile1, m_item1_t1);
    m_selectionManager->finishSelectionChange("Deselect Item 1 on Tile 1");

    QVERIFY(!m_item1_t1->isSelected());
    QVERIFY(!m_selectionManager->isSelected(m_tile1, m_item1_t1));
    QVERIFY(!m_tile1->hasSelectedElements());
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
}

void TestSelectionManager::testClearSelection() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addTile(m_tile1); // tile1 selected
    m_selectionManager->addItem(m_tile2, m_item1_t2); // item on tile2 selected
    m_selectionManager->finishSelectionChange("Setup for Clear");

    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_item1_t2->isSelected());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 2); // tile1 and tile2 are involved

    m_selectionManager->startSelectionChange();
    m_selectionManager->clear(); // This records changes to deselect everything
    m_selectionManager->finishSelectionChange("Clear Selection");

    QVERIFY(!m_tile1->isSelected());
    QVERIFY(!m_selectionManager->isSelected(m_tile1));
    QVERIFY(!m_item1_t2->isSelected());
    QVERIFY(!m_selectionManager->isSelected(m_tile2, m_item1_t2));
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
}


void TestSelectionManager::testUndoRedoSelection() {
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());

    m_selectionManager->startSelectionChange();
    m_selectionManager->addTile(m_tile1);
    m_selectionManager->finishSelectionChange("Select Tile 1");
    QVERIFY(m_tile1->isSelected());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);

    m_selectionManager->startSelectionChange();
    m_selectionManager->addItem(m_tile1, m_item1_t1);
    m_selectionManager->finishSelectionChange("Select Item 1 on Tile 1");
    QVERIFY(m_item1_t1->isSelected());
    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_tile1->hasSelectedElements());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);

    m_undoStack->undo(); // Undo item selection
    QVERIFY(!m_item1_t1->isSelected());
    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_tile1->hasSelectedElements()); // Tile itself is selected
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);
    QVERIFY(m_selectionManager->getSelectedTiles().contains(m_tile1));

    m_undoStack->undo(); // Undo tile selection
    QVERIFY(!m_tile1->isSelected());
    QVERIFY(!m_tile1->hasSelectedElements());
    QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
    QVERIFY(!m_item1_t1->isSelected());

    m_undoStack->redo(); // Redo tile selection
    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_tile1->hasSelectedElements());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);
    QVERIFY(m_selectionManager->getSelectedTiles().contains(m_tile1));


    m_undoStack->redo(); // Redo item selection
    QVERIFY(m_item1_t1->isSelected());
    QVERIFY(m_tile1->isSelected());
    QVERIFY(m_tile1->hasSelectedElements());
    QCOMPARE(m_selectionManager->getSelectedTiles().count(), 1);
}

QTEST_APPLESS_MAIN(TestSelectionManager)
#include "test_selectionmanager.moc"
