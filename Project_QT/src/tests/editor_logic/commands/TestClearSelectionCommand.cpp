#include <QtTest/QtTest>
#include <memory>
#include <QList>

#include "editor_logic/commands/ClearSelectionCommand.h"
#include "core/selection/SelectionManager.h"
#include "core/Tile.h" // For RME::core::Tile
#include "core/map/Map.h" // For MockSelectionManager context
#include "core/assets/AssetManager.h" // For Map context for SelectionManager
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"

// --- Mock SelectionManager ---
// (If a more generic one isn't already in tests/mocks, define a minimal one here)
class MockSelectionManagerForCmdTest : public RME::core::selection::SelectionManager {
public:
    // Provide Map and QUndoStack (even if null for some tests if not used by these commands directly)
    MockSelectionManagerForCmdTest(RME::core::Map* map)
        : RME::core::selection::SelectionManager(map, nullptr /*no undostack needed for internal calls*/) {
    }

    QList<RME::core::Tile*> m_currentSelectedTiles_mock;
    int clearSelectionInternalCalled = 0;
    int setSelectedTilesInternalCalled = 0;

    // --- Overrides of internal methods used by ClearSelectionCommand ---
    void clearSelectionInternal() override {
        clearSelectionInternalCalled++;
        bool changed = !m_currentSelectedTiles_mock.isEmpty();
        m_currentSelectedTiles_mock.clear();
        if (changed) emit selectionChanged();
    }

    void setSelectedTilesInternal(const QList<RME::core::Tile*>& tiles) override {
        setSelectedTilesInternalCalled++;
        // Simple assignment for mock; a real one might compare for actual change.
        bool changed = (m_currentSelectedTiles_mock != tiles);
        m_currentSelectedTiles_mock = tiles;
        if (changed) emit selectionChanged();
    }

    // --- Overrides of public methods used by ClearSelectionCommand ---
    QList<RME::core::Tile*> getCurrentSelectedTilesList() const override {
        return m_currentSelectedTiles_mock;
    }
    // getCurrentSelectedTiles() from base returns QSet, but command uses getCurrentSelectedTilesList()
    // const QSet<RME::core::Tile*>& getSelectedTiles() const override {
    //     // This is tricky if base class returns its own m_selectedTiles.
    //     // For this mock, let's assume this method isn't directly used by command logic being tested,
    //     // or that getCurrentSelectedTilesList() is sufficient.
    //     // If it IS used, this mock needs to manage a QSet member too.
    //     static QSet<RME::core::Tile*> tempSet; // Not ideal
    //     tempSet = QSet<RME::core::Tile*>(m_currentSelectedTiles_mock.begin(), m_currentSelectedTiles_mock.end());
    //     return tempSet;
    // }

    bool isEmpty() const override { return m_currentSelectedTiles_mock.isEmpty(); }


    // Helper for test setup
    void MOCK_addTileToSelection(RME::core::Tile* tile) {
        if (tile && !m_currentSelectedTiles_mock.contains(tile)) {
            m_currentSelectedTiles_mock.append(tile);
        }
    }
    void resetMockStats() {
        clearSelectionInternalCalled = 0;
        setSelectedTilesInternalCalled = 0;
        // m_currentSelectedTiles_mock.clear(); // Don't clear here, test setup will do it.
    }
};

class TestClearSelectionCommand : public QObject {
    Q_OBJECT

public:
    TestClearSelectionCommand() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testRedo_ClearsSelection_StoresOld();
    void testUndo_RestoresOldSelection();
    void testRedo_NoInitialSelection();
    void testUndo_NoOldSelectionToRestore();

private:
    std::unique_ptr<RME::core::Map> m_map;
    std::unique_ptr<MockSelectionManagerForCmdTest> m_mockSelectionManager;
    // Dummy tiles for testing (not added to map, just need pointers)
    // These are stack allocated, their addresses are stable for a test.
    RME::core::Tile m_tile1, m_tile2;

    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestClearSelectionCommand::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    // No items needed for this specific command test, but AssetManager needs ItemDatabase
    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RME::core::Map>(10,10,1, m_assetManager.get());
    m_mockSelectionManager = std::make_unique<MockSelectionManagerForCmdTest>(m_map.get());

    // Re-initialize tiles to ensure they are fresh for each test
    m_tile1 = RME::core::Tile(RME::core::Position(0,0,0));
    m_tile2 = RME::core::Tile(RME::core::Position(1,0,0));
    m_mockSelectionManager->resetMockStats();
    m_mockSelectionManager->m_currentSelectedTiles_mock.clear(); // Ensure selection is clear at start of each test
}

void TestClearSelectionCommand::cleanup() {
    m_mockSelectionManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
}

void TestClearSelectionCommand::testRedo_ClearsSelection_StoresOld() {
    m_mockSelectionManager->MOCK_addTileToSelection(&m_tile1);
    m_mockSelectionManager->MOCK_addTileToSelection(&m_tile2);
    QCOMPARE(m_mockSelectionManager->getCurrentSelectedTilesList().size(), 2);

    RME::editor_logic::commands::ClearSelectionCommand cmd(m_mockSelectionManager.get());
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->clearSelectionInternalCalled, 1);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    QCOMPARE(cmd.getOldSelectedTiles().size(), 2);
    QVERIFY(cmd.getOldSelectedTiles().contains(&m_tile1));
    QVERIFY(cmd.getOldSelectedTiles().contains(&m_tile2));
    QVERIFY(cmd.text().contains("Clear Selection (2 tiles)"));
}

void TestClearSelectionCommand::testUndo_RestoresOldSelection() {
    m_mockSelectionManager->MOCK_addTileToSelection(&m_tile1);
    RME::editor_logic::commands::ClearSelectionCommand cmd(m_mockSelectionManager.get());
    cmd.redo(); // Captures [&m_tile1], then clears selection

    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    cmd.undo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QCOMPARE(m_mockSelectionManager->getCurrentSelectedTilesList().size(), 1);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().contains(&m_tile1));
    QVERIFY(cmd.text().contains("Undo Clear Selection (restored 1 tiles)"));
}

void TestClearSelectionCommand::testRedo_NoInitialSelection() {
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    RME::editor_logic::commands::ClearSelectionCommand cmd(m_mockSelectionManager.get());
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->clearSelectionInternalCalled, 0);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    QVERIFY(cmd.getOldSelectedTiles().isEmpty());
    QVERIFY(cmd.text().contains("Clear Selection (nothing selected)") || cmd.text().contains("already cleared"));
}

void TestClearSelectionCommand::testUndo_NoOldSelectionToRestore() {
    RME::editor_logic::commands::ClearSelectionCommand cmd(m_mockSelectionManager.get());
    cmd.redo();

    m_mockSelectionManager->resetMockStats(); // Reset call counters for this check
    cmd.undo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 0);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    QVERIFY(cmd.text().contains("Undo Clear Selection (nothing to restore)"));
}

QTEST_MAIN(TestClearSelectionCommand)
#include "TestClearSelectionCommand.moc"
