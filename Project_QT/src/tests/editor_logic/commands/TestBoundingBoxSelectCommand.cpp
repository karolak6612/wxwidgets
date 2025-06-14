#include <QtTest/QtTest>
#include <memory>
#include <QList>
#include <QSet>

#include "editor_logic/commands/BoundingBoxSelectCommand.h"
#include "core/selection/SelectionManager.h"
#include "core/Tile.h"
#include "core/map/Map.h" // For MockSelectionManager context
#include "core/assets/AssetManager.h" // For Map context for SelectionManager
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"

// --- Mock SelectionManager (similar to TestClearSelectionCommand's) ---
class MockSelectionManagerForBBTest : public RME::core::selection::SelectionManager {
public:
    MockSelectionManagerForBBTest(RME::core::Map* map)
        : RME::core::selection::SelectionManager(map, nullptr) {
    }

    QList<RME::core::Tile*> m_currentSelectedTiles_mock;
    int setSelectedTilesInternalCalled = 0;
    QList<RME::core::Tile*> lastSetSelectedTilesInternal_arg;

    void setSelectedTilesInternal(const QList<RME::core::Tile*>& tiles) override {
        setSelectedTilesInternalCalled++;
        lastSetSelectedTilesInternal_arg = tiles;
        m_currentSelectedTiles_mock = tiles;
        // emit selectionChanged(); // Base class would handle if Q_OBJECT
    }

    QList<RME::core::Tile*> getCurrentSelectedTilesList() const override {
        return m_currentSelectedTiles_mock;
    }
    // Override isEmpty if needed by command logic, though BoundingBoxSelectCommand doesn't directly use it.
    bool isEmpty() const override { return m_currentSelectedTiles_mock.isEmpty(); }

    void resetMockStats() {
        setSelectedTilesInternalCalled = 0;
        lastSetSelectedTilesInternal_arg.clear();
        m_currentSelectedTiles_mock.clear();
    }
};

class TestBoundingBoxSelectCommand : public QObject {
    Q_OBJECT

public:
    TestBoundingBoxSelectCommand() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testRedo_NonAdditive_ReplacesSelection();
    void testUndo_NonAdditive_RestoresPrevious();
    void testRedo_Additive_AddsToSelection();
    void testUndo_Additive_RestoresPrevious();
    void testRedo_Additive_NoNewTiles_NoChange();
    void testRedo_NonAdditive_EmptyBox_ClearsSelection();

private:
    std::unique_ptr<RME::core::Map> m_map;
    std::unique_ptr<MockSelectionManagerForBBTest> m_mockSelectionManager;
    RME::core::Tile m_tile1, m_tile2, m_tile3, m_tile4; // Dummy tiles

    // Asset related members for Map construction
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestBoundingBoxSelectCommand::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RME::core::Map>(10,10,1, m_assetManager.get());
    m_mockSelectionManager = std::make_unique<MockSelectionManagerForBBTest>(m_map.get());

    // Initialize dummy tiles with distinct positions for testing
    m_tile1 = RME::core::Tile(RME::core::Position(0,0,0));
    m_tile2 = RME::core::Tile(RME::core::Position(1,0,0));
    m_tile3 = RME::core::Tile(RME::core::Position(0,1,0));
    m_tile4 = RME::core::Tile(RME::core::Position(1,1,0));
    m_mockSelectionManager->resetMockStats();
}

void TestBoundingBoxSelectCommand::cleanup() {
    m_mockSelectionManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
}

void TestBoundingBoxSelectCommand::testRedo_NonAdditive_ReplacesSelection() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    QList<RME::core::Tile*> boxTiles = {&m_tile2, &m_tile3};
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, false /*non-additive*/, initialSelection);
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    // Convert to QSet for comparison as order doesn't matter for selection state
    QSet<RME::core::Tile*> finalSelectionSet = QSet<RME::core::Tile*>::fromList(m_mockSelectionManager->lastSetSelectedTilesInternal_arg);
    QCOMPARE(finalSelectionSet.size(), 2);
    QVERIFY(finalSelectionSet.contains(&m_tile2));
    QVERIFY(finalSelectionSet.contains(&m_tile3));

    QSet<RME::core::Tile*> cmdAfterState = QSet<RME::core::Tile*>::fromList(cmd.getSelectionStateAfter());
    QCOMPARE(cmdAfterState.size(), 2);
}

void TestBoundingBoxSelectCommand::testUndo_NonAdditive_RestoresPrevious() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);

    QList<RME::core::Tile*> boxTiles = {&m_tile2, &m_tile3};
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, false, initialSelection);
    cmd.redo();
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    cmd.undo();
    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QSet<RME::core::Tile*> finalSelectionSet = QSet<RME::core::Tile*>::fromList(m_mockSelectionManager->lastSetSelectedTilesInternal_arg);
    QCOMPARE(finalSelectionSet.size(), 1);
    QVERIFY(finalSelectionSet.contains(&m_tile1));
}

void TestBoundingBoxSelectCommand::testRedo_Additive_AddsToSelection() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    QList<RME::core::Tile*> boxTiles = {&m_tile2, &m_tile1}; // tile1 is already selected
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, true /*additive*/, initialSelection);
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QSet<RME::core::Tile*> finalSelectionSet = QSet<RME::core::Tile*>::fromList(m_mockSelectionManager->lastSetSelectedTilesInternal_arg);
    QCOMPARE(finalSelectionSet.size(), 2);
    QVERIFY(finalSelectionSet.contains(&m_tile1));
    QVERIFY(finalSelectionSet.contains(&m_tile2));

    QSet<RME::core::Tile*> cmdAfterState = QSet<RME::core::Tile*>::fromList(cmd.getSelectionStateAfter());
    QCOMPARE(cmdAfterState.size(), 2);
}

void TestBoundingBoxSelectCommand::testUndo_Additive_RestoresPrevious() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);

    QList<RME::core::Tile*> boxTiles = {&m_tile2};
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, true, initialSelection);
    cmd.redo();
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    cmd.undo();
    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QSet<RME::core::Tile*> finalSelectionSet = QSet<RME::core::Tile*>::fromList(m_mockSelectionManager->lastSetSelectedTilesInternal_arg);
    QCOMPARE(finalSelectionSet.size(), 1);
    QVERIFY(finalSelectionSet.contains(&m_tile1));
}

void TestBoundingBoxSelectCommand::testRedo_Additive_NoNewTiles_NoChange() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1, &m_tile2};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    QList<RME::core::Tile*> boxTiles = {&m_tile1};
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, true, initialSelection);
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QSet<RME::core::Tile*> finalSelectionSet = QSet<RME::core::Tile*>::fromList(m_mockSelectionManager->lastSetSelectedTilesInternal_arg);
    QCOMPARE(finalSelectionSet.size(), 2);
    QVERIFY(finalSelectionSet.contains(&m_tile1));
    QVERIFY(finalSelectionSet.contains(&m_tile2));
    QVERIFY(cmd.text().contains("(no change)"));
}

void TestBoundingBoxSelectCommand::testRedo_NonAdditive_EmptyBox_ClearsSelection() {
    QList<RME::core::Tile*> initialSelection = {&m_tile1, &m_tile2};
    m_mockSelectionManager->setSelectedTilesInternal(initialSelection);
    m_mockSelectionManager->setSelectedTilesInternalCalled = 0;

    QList<RME::core::Tile*> boxTiles = {}; // Empty box
    RME::editor_logic::commands::BoundingBoxSelectCommand cmd(m_mockSelectionManager.get(), boxTiles, false /*non-additive*/, initialSelection);
    cmd.redo();

    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QVERIFY(m_mockSelectionManager->lastSetSelectedTilesInternal_arg.isEmpty());

    QSet<RME::core::Tile*> cmdAfterState = QSet<RME::core::Tile*>::fromList(cmd.getSelectionStateAfter());
    QVERIFY(cmdAfterState.isEmpty());
}

QTEST_MAIN(TestBoundingBoxSelectCommand)
#include "TestBoundingBoxSelectCommand.moc"
