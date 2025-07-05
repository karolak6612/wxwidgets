#include <QtTest/QtTest>
#include <QSharedPointer>
#include "editor_logic/commands/RecordSetSpawnCommand.h"
#include "editor_logic/EditorController.h" // For EditorController interface
#include "core/map/Map.h" // For Map interface
#include "core/map/Tile.h"      // For Tile (though not directly used, good for context)
#include "core/Position.h"    // For QPoint alternative if used by Map
// Corrected path for MockEditorController
#include "tests/core/brush/MockEditorController.h" // Adjusted path
#include "tests/mocks/MockMap.h" // Assuming MockMap is still used or a similar mock map is available via MockEditorController

class TestRecordSetSpawnCommand : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testExecute_SetsSpawnAndNotifies();
    void testUndo_RestoresPreviousSpawnAndNotifies();
    void testRedo_SetsSpawnAgainAndNotifies();

private:
    QSharedPointer<MockEditorController> m_mockEditorController;
    QSharedPointer<MockMap> m_mockMap; // This will be the map provided by MockEditorController's getMap()
    QPoint m_initialSpawnPoint;
    QPoint m_newSpawnPoint;
};

void TestRecordSetSpawnCommand::initTestCase() {
    // m_mockMap is created and managed by MockEditorController in this setup
    m_mockEditorController = QSharedPointer<MockEditorController>(new MockEditorController());

    // Retrieve the map from the controller to set initial conditions
    // This assumes MockEditorController initializes or can be made to initialize its own MockMap.
    Map* mapPtr = m_mockEditorController->getMap(); // This must work.
    m_mockMap = QSharedPointer<MockMap>(dynamic_cast<MockMap*>(mapPtr));

    Q_ASSERT(m_mockMap); // Ensure the cast was successful and map is available.

    m_initialSpawnPoint = QPoint(1, 1);
    if (m_mockMap) { // Check if m_mockMap is not null
        m_mockMap->setSpawnPoint(m_initialSpawnPoint);
    } else {
        QFAIL("MockMap could not be obtained from MockEditorController or is null.");
    }


    m_newSpawnPoint = QPoint(5, 5);
}

void TestRecordSetSpawnCommand::cleanupTestCase() {
    m_mockMap.clear();
    m_mockEditorController.clear();
}

void TestRecordSetSpawnCommand::testExecute_SetsSpawnAndNotifies() {
    QVERIFY(m_mockEditorController);
    QVERIFY(m_mockMap);

    RecordSetSpawnCommand command(m_mockEditorController.data(), m_newSpawnPoint);

    m_mockEditorController->m_tileChangedNotified = false; // Reset notification flag
    m_mockEditorController->m_notifiedPosition = QPoint(); // Reset position

    command.execute();

    QCOMPARE(m_mockMap->getSpawnPoint(), m_newSpawnPoint);
    QVERIFY(m_mockEditorController->m_tileChangedNotified);
    QCOMPARE(m_mockEditorController->m_notifiedPosition, m_newSpawnPoint);
}

void TestRecordSetSpawnCommand::testUndo_RestoresPreviousSpawnAndNotifies() {
    QVERIFY(m_mockEditorController);
    QVERIFY(m_mockMap);

    RecordSetSpawnCommand command(m_mockEditorController.data(), m_newSpawnPoint);
    command.execute(); // Set to new spawn point

    m_mockEditorController->m_tileChangedNotified = false; // Reset notification flag
    m_mockEditorController->m_notifiedPosition = QPoint(); // Reset position

    // The position that should be notified is the one that *was* the spawn point
    QPoint previouslyNotifiedSpawn = m_newSpawnPoint;

    command.undo();

    QCOMPARE(m_mockMap->getSpawnPoint(), m_initialSpawnPoint);
    QVERIFY(m_mockEditorController->m_tileChangedNotified);
    // Upon undo, the notification should be for the tile that *changed* state.
    // This is the tile that *was* the new spawn point and now is not (or is back to being a normal tile).
    QCOMPARE(m_mockEditorController->m_notifiedPosition, previouslyNotifiedSpawn);
}

void TestRecordSetSpawnCommand::testRedo_SetsSpawnAgainAndNotifies() {
    QVERIFY(m_mockEditorController);
    QVERIFY(m_mockMap);

    RecordSetSpawnCommand command(m_mockEditorController.data(), m_newSpawnPoint);
    command.execute();
    command.undo();

    m_mockEditorController->m_tileChangedNotified = false; // Reset notification flag
    m_mockEditorController->m_notifiedPosition = QPoint(); // Reset position

    command.redo();

    QCOMPARE(m_mockMap->getSpawnPoint(), m_newSpawnPoint);
    QVERIFY(m_mockEditorController->m_tileChangedNotified);
    QCOMPARE(m_mockEditorController->m_notifiedPosition, m_newSpawnPoint);
}

// QTEST_MAIN will be in a central test runner file or defined per test if run individually.
// For now, to make it self-contained for potential individual compilation:
// #include "TestRecordSetSpawnCommand.moc"
// However, if there's a main test runner, this TestRecordSetSpawnCommand.cpp
// would not have QTEST_MAIN. For now, I'll assume it might be compiled as part of a larger test suite.
// If this file needs to be a standalone main(), QTEST_MAIN and the .moc include are needed.
// For now, commenting out QTEST_MAIN and .moc to align with typical project structure
// where a main runner aggregates tests.
// QTEST_APPLESS_MAIN(TestRecordSetSpawnCommand)
// #include "TestRecordSetSpawnCommand.moc"
