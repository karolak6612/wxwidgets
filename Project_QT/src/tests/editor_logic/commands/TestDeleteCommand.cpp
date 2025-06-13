#include <QtTest/QtTest>
#include <memory>
#include <QList>
#include <QMap>

#include "editor_logic/commands/DeleteCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/data_transfer/TileData.h" // For TileData struct
#include "core/selection/SelectionManager.h" // For MockSelectionManager base and interface
#include "core/assets/ItemData.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "tests/core/brush/MockEditorController.h" // For EditorControllerInterface

// --- Mock SelectionManager (similar to TestClearSelectionCommand's) ---
class MockSelectionManagerForDelCmdTest : public RME::core::selection::SelectionManager {
public:
    MockSelectionManagerForDelCmdTest(RME::core::Map* map)
        : RME::core::selection::SelectionManager(map, nullptr) {
    }

    QList<RME::core::Tile*> m_currentSelectedTiles_mock_list;
    int clearSelectionInternalCalled = 0;
    int setSelectedTilesInternalCalled = 0;
    QList<RME::core::Tile*> lastSetSelectedTilesInternal_arg;

    void clearSelectionInternal() override {
        clearSelectionInternalCalled++;
        bool changed = !m_currentSelectedTiles_mock_list.isEmpty();
        m_currentSelectedTiles_mock_list.clear();
        if (changed) emit selectionChanged();
    }

    void setSelectedTilesInternal(const QList<RME::core::Tile*>& tiles) override {
        setSelectedTilesInternalCalled++;
        lastSetSelectedTilesInternal_arg = tiles;
        // Simulate the selection actually changing in the mock
        bool changed = (QSet<RME::core::Tile*>::fromList(m_currentSelectedTiles_mock_list) != QSet<RME::core::Tile*>::fromList(tiles));
        m_currentSelectedTiles_mock_list = tiles;
        if (changed) emit selectionChanged();
    }

    QList<RME::core::Tile*> getCurrentSelectedTilesList() const override {
        return m_currentSelectedTiles_mock_list;
    }
    bool isEmpty() const override { return m_currentSelectedTiles_mock_list.isEmpty(); }

    void MOCK_setCurrentSelection(const QList<RME::core::Tile*>& tiles) {
        m_currentSelectedTiles_mock_list = tiles;
    }
    void resetMockStats() {
        clearSelectionInternalCalled = 0;
        setSelectedTilesInternalCalled = 0;
        lastSetSelectedTilesInternal_arg.clear();
        // m_currentSelectedTiles_mock_list.clear(); // Test should set initial state explicitly
    }
};

// Test Item IDs
const uint16_t GEN_DEL_CMD_GROUND_ID = 501;
const uint16_t GEN_DEL_CMD_ITEM_ID1 = 502;
const QString GEN_DEL_CMD_CREATURE_NAME = "TestSpider";

class TestDeleteCommand : public QObject {
    Q_OBJECT

public:
    TestDeleteCommand() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testRedo_ClearsTiles_StoresData_ClearsSelection();
    void testUndo_RestoresTiles_RestoresSelection();
    void testRedo_NoSelection_DoesNothing();
    void testNotifications();

private:
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
    std::unique_ptr<RME::core::Map> m_map; // Map owned by test fixture for direct tile manipulation
    std::unique_ptr<MockSelectionManagerForDelCmdTest> m_mockSelectionManager;

    RME::core::Tile* m_tile1_ptr = nullptr;
    RME::core::Tile* m_tile2_ptr = nullptr;

    void setupMockItemDatabase() {
        RME::core::assets::ItemData ground, item1;
        ground.id = GEN_DEL_CMD_GROUND_ID; ground.name = "Gen Del Ground"; ground.isGround = true;
        item1.id = GEN_DEL_CMD_ITEM_ID1; item1.name = "Gen Del Item 1"; item1.isGround = false;
        m_itemDatabase->addItemData(ground);
        m_itemDatabase->addItemData(item1);
        RME::core::Item::setItemDatabase(m_itemDatabase.get());
    }
    void setupMockCreatureDatabase() {
        RME::core::assets::CreatureData creatureData;
        creatureData.name = GEN_DEL_CMD_CREATURE_NAME;
        m_creatureDatabase->addCreatureData(creatureData);
    }

    void populateTile(RME::core::Tile* tile) {
        if (!tile) return;
        tile->setGround(RME::core::Item::create(GEN_DEL_CMD_GROUND_ID));
        tile->addItem(RME::core::Item::create(GEN_DEL_CMD_ITEM_ID1));
        tile->setSpawn(std::make_unique<RME::core::Spawn>(1));
        tile->setCreature(std::make_unique<RME::core::Creature>(GEN_DEL_CMD_CREATURE_NAME));
    }

    bool verifyTileIsEmpty(const RME::core::Tile* tile) const {
        if (!tile) return true;
        return !tile->getGround() && tile->getItems().empty() &&
               !tile->getSpawn() && !tile->getCreature();
    }

    bool verifyTileIsPopulated(const RME::core::Tile* tile) const {
         if (!tile) return false;
         bool groundOk = tile->getGround() && tile->getGround()->getID() == GEN_DEL_CMD_GROUND_ID;
         bool itemsOk = !tile->getItems().empty() && tile->getItems()[0]->getID() == GEN_DEL_CMD_ITEM_ID1;
         bool spawnOk = tile->getSpawn() && tile->getSpawn()->getRadius() == 1;
         bool creatureOk = tile->getCreature() && tile->getCreature()->getName() == GEN_DEL_CMD_CREATURE_NAME;
         return groundOk && itemsOk && spawnOk && creatureOk;
    }
     // Overload for TileData check
    bool verifyTileIsPopulated(const RME::core::data_transfer::TileData* tileData) const {
        if(!tileData) return false;
        bool groundOk = tileData->ground && tileData->ground->getID() == GEN_DEL_CMD_GROUND_ID;
        bool itemsOk = !tileData->items.isEmpty() && tileData->items[0]->getID() == GEN_DEL_CMD_ITEM_ID1;
        bool spawnOk = tileData->spawn && tileData->spawn->getRadius() == 1;
        bool creatureOk = tileData->creature && tileData->creature->getName() == GEN_DEL_CMD_CREATURE_NAME;
        return groundOk && itemsOk && spawnOk && creatureOk;
    }
};

void TestDeleteCommand::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    setupMockItemDatabase();
    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    setupMockCreatureDatabase();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RME::core::Map>(10,10,1, m_assetManager.get());
    m_mockSelectionManager = std::make_unique<MockSelectionManagerForDelCmdTest>(m_map.get());
    m_mockController = std::make_unique<RMEMockEditorController>();
    m_mockController->m_mockMap = m_map.get();
    m_mockController->setMockAssetManager(m_assetManager.get());


    m_tile1_ptr = m_map->getOrCreateTile(RME::core::Position(1,1,0));
    m_tile2_ptr = m_map->getOrCreateTile(RME::core::Position(1,2,0));
    populateTile(m_tile1_ptr);
    populateTile(m_tile2_ptr);
    m_mockSelectionManager->resetMockStats();
    m_mockSelectionManager->m_currentSelectedTiles_mock_list.clear();
    m_mockController->resetNotifications();
}

void TestDeleteCommand::cleanup() {
    m_tile1_ptr = nullptr; m_tile2_ptr = nullptr;
    m_mockSelectionManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    m_mockController.reset();
    RME::core::Item::setItemDatabase(nullptr);
}

void TestDeleteCommand::testRedo_ClearsTiles_StoresData_ClearsSelection() {
    m_mockSelectionManager->MOCK_setCurrentSelection({m_tile1_ptr, m_tile2_ptr});

    RME_COMMANDS::DeleteCommand cmd(m_map.get(), m_mockSelectionManager.get(), m_mockController.get());
    cmd.redo();

    QVERIFY(verifyTileIsEmpty(m_tile1_ptr));
    QVERIFY(verifyTileIsEmpty(m_tile2_ptr));
    QCOMPARE(cmd.getOriginalTileData().count(), 2);
    QVERIFY(cmd.getOriginalTileData().contains(m_tile1_ptr->getPosition()));
    QVERIFY(cmd.getOriginalTileData().contains(m_tile2_ptr->getPosition()));
    const RME::core::data_transfer::TileData& td1 = cmd.getOriginalTileData().value(m_tile1_ptr->getPosition());
    QVERIFY(verifyTileIsPopulated(&td1));

    QCOMPARE(m_mockSelectionManager->clearSelectionInternalCalled, 1);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());
    QVERIFY(cmd.text().contains("Delete Selection (2 tile(s))"));
}

void TestDeleteCommand::testUndo_RestoresTiles_RestoresSelection() {
    m_mockSelectionManager->MOCK_setCurrentSelection({m_tile1_ptr});
    RME_COMMANDS::DeleteCommand cmd(m_map.get(), m_mockSelectionManager.get(), m_mockController.get());

    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_tile1_ptr));
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().isEmpty());

    cmd.undo();
    QVERIFY(verifyTileIsPopulated(m_tile1_ptr));
    QCOMPARE(m_mockSelectionManager->setSelectedTilesInternalCalled, 1);
    QCOMPARE(m_mockSelectionManager->getCurrentSelectedTilesList().size(), 1);
    QVERIFY(m_mockSelectionManager->getCurrentSelectedTilesList().contains(m_tile1_ptr));
    QVERIFY(cmd.getPreviouslySelectedTiles().contains(m_tile1_ptr));
}

void TestDeleteCommand::testRedo_NoSelection_DoesNothing() {
    m_mockSelectionManager->MOCK_setCurrentSelection({});

    RME_COMMANDS::DeleteCommand cmd(m_map.get(), m_mockSelectionManager.get(), m_mockController.get());
    cmd.redo();

    QVERIFY(verifyTileIsPopulated(m_tile1_ptr));
    QVERIFY(verifyTileIsPopulated(m_tile2_ptr));
    QVERIFY(cmd.getOriginalTileData().isEmpty());
    QCOMPARE(m_mockSelectionManager->clearSelectionInternalCalled, 0);
    QVERIFY(cmd.text().contains("Delete (nothing selected)"));
}

void TestDeleteCommand::testNotifications() {
    m_mockSelectionManager->MOCK_setCurrentSelection({m_tile1_ptr});
    RME_COMMANDS::DeleteCommand cmd(m_map.get(), m_mockSelectionManager.get(), m_mockController.get());

    m_mockController->resetNotifications();
    cmd.redo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_tile1_ptr->getPosition());

    m_mockController->resetNotifications();
    cmd.undo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_tile1_ptr->getPosition());
}

QTEST_MAIN(TestDeleteCommand)
#include "TestDeleteCommand.moc"
