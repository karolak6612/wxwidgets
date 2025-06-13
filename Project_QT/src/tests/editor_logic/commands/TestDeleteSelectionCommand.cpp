#include <QtTest/QtTest>
#include <memory>
#include <QList>
#include <QMap>

#include "editor_logic/commands/DeleteSelectionCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/assets/ItemData.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "tests/core/brush/MockEditorController.h" // For EditorControllerInterface

// Using declarations
using RMEPosition = RME::core::Position;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEItem = RME::core::Item;
using RMESpawn = RME::core::Spawn;
using RMECreature = RME::core::Creature;
using RMEItemData = RME::core::assets::ItemData;
using RMEAssetManager = RME::core::assets::AssetManager;
using RMEItemDatabase = RME::core::assets::ItemDatabase;
using RMECreatureDatabase = RME::core::assets::CreatureDatabase;
using RMESpriteManager = RME::core::sprites::SpriteManager;
using RMEClientVersionManager = RME::core::assets::ClientVersionManager;
using RMEMaterialManager = RME::core::assets::MaterialManager;
using RMECommand = RME_COMMANDS::DeleteSelectionCommand;
using RMEMockEditorController = MockEditorController;

// Test Item IDs
const uint16_t DEL_CMD_GROUND_ID = 401;
const uint16_t DEL_CMD_ITEM_ID1 = 402;
const QString DEL_CMD_CREATURE_NAME = "TestGoblin";

class TestDeleteSelectionCommand : public QObject {
    Q_OBJECT

public:
    TestDeleteSelectionCommand() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testRedo_ClearsSelectedTiles_StoresState();
    void testUndo_RestoresTileStates();
    void testRedoUndoRedo_Cycle();
    void testRedo_NoSelection_DoesNothing();
    void testNotificationTriggered();

private:
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEAssetManager> m_assetManager;
    std::unique_ptr<RMEItemDatabase> m_itemDatabase;
    std::unique_ptr<RMECreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RMESpriteManager> m_spriteManager;
    std::unique_ptr<RMEClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RMEMaterialManager> m_materialManager;
    // Map is now owned by MockEditorController in this setup
    RMEMap* m_map = nullptr;

    void setupMockItemDatabase() {
        RMEItemData ground, item1;
        ground.id = DEL_CMD_GROUND_ID; ground.name = "Del Test Ground"; ground.isGround = true;
        item1.id = DEL_CMD_ITEM_ID1; item1.name = "Del Test Item 1"; item1.isGround = false;
        m_itemDatabase->addItemData(ground);
        m_itemDatabase->addItemData(item1);
        Item::setItemDatabase(m_itemDatabase.get());
    }
    void setupMockCreatureDatabase() {
        RME::core::assets::CreatureData creatureData;
        creatureData.name = DEL_CMD_CREATURE_NAME;
        m_creatureDatabase->addCreatureData(creatureData);
    }

    // Helper to populate a tile fully
    void populateTile(RMETile* tile) {
        if (!tile) return;
        tile->setGround(Item::create(DEL_CMD_GROUND_ID));
        tile->addItem(Item::create(DEL_CMD_ITEM_ID1));
        tile->setSpawn(std::make_unique<RMESpawn>(1)); // radius 1
        tile->setCreature(std::make_unique<RMECreature>(DEL_CMD_CREATURE_NAME));
    }

    bool verifyTileIsEmpty(const RMETile* tile) const {
        if (!tile) return true;
        return !tile->getGround() && tile->getItems().empty() &&
               !tile->getSpawn() && !tile->getCreature();
    }

    bool verifyTileIsPopulated(const RMETile* tile) const {
         if (!tile) return false;
         bool groundOk = tile->getGround() && tile->getGround()->getID() == DEL_CMD_GROUND_ID;
         bool itemsOk = !tile->getItems().empty() && tile->getItems()[0]->getID() == DEL_CMD_ITEM_ID1;
         bool spawnOk = tile->getSpawn() && tile->getSpawn()->getRadius() == 1;
         bool creatureOk = tile->getCreature() && tile->getCreature()->getName() == DEL_CMD_CREATURE_NAME;
         return groundOk && itemsOk && spawnOk && creatureOk;
    }
};

void TestDeleteSelectionCommand::initTestCase() {}

void TestDeleteSelectionCommand::init() {
    m_clientVersionManager = std::make_unique<RMEClientVersionManager>();
    m_itemDatabase = std::make_unique<RMEItemDatabase>(*m_clientVersionManager);
    setupMockItemDatabase();
    m_creatureDatabase = std::make_unique<RMECreatureDatabase>();
    setupMockCreatureDatabase();
    m_spriteManager = std::make_unique<RMESpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RMEMaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RMEAssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    m_mockController = std::make_unique<RMEMockEditorController>();
    m_map = m_mockController->getMap(); // Use map from MockEditorController
    QVERIFY(m_map);
    m_mockController->setMockAssetManager(m_assetManager.get());
}

void TestDeleteSelectionCommand::cleanup() {
    m_map = nullptr; // Was pointing to map inside m_mockController
    m_mockController.reset(); // Deletes MockMap

    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    Item::setItemDatabase(nullptr);
}

void TestDeleteSelectionCommand::cleanupTestCase() {}

void TestDeleteSelectionCommand::testRedo_ClearsSelectedTiles_StoresState() {
    RMEPosition pos1(1,1,0), pos2(1,2,0);
    populateTile(m_map->getOrCreateTile(pos1));
    populateTile(m_map->getOrCreateTile(pos2));

    QList<RMEPosition> selectedPositions = {pos1, pos2};
    RMECommand cmd(m_map, selectedPositions, m_mockController.get());

    cmd.redo();

    QVERIFY(verifyTileIsEmpty(m_map->getTile(pos1)));
    QVERIFY(verifyTileIsEmpty(m_map->getTile(pos2)));
    QCOMPARE(cmd.getUndoneTileStates().count(), 2);
    QVERIFY(cmd.getUndoneTileStates().contains(pos1));
    QVERIFY(cmd.getUndoneTileStates().contains(pos2));
    QVERIFY(verifyTileIsPopulated(cmd.getUndoneTileStates().value(pos1).get()));
    QVERIFY(verifyTileIsPopulated(cmd.getUndoneTileStates().value(pos2).get()));
}

void TestDeleteSelectionCommand::testUndo_RestoresTileStates() {
    RMEPosition pos1(1,1,0);
    populateTile(m_map->getOrCreateTile(pos1));

    QList<RMEPosition> selectedPositions = {pos1};
    RMECommand cmd(m_map, selectedPositions, m_mockController.get());

    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_map->getTile(pos1)));

    cmd.undo();
    QVERIFY(verifyTileIsPopulated(m_map->getTile(pos1)));
}

void TestDeleteSelectionCommand::testRedoUndoRedo_Cycle() {
    RMEPosition pos1(1,1,0);
    populateTile(m_map->getOrCreateTile(pos1));
    QList<RMEPosition> selectedPositions = {pos1};
    RMECommand cmd(m_map, selectedPositions, m_mockController.get());

    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_map->getTile(pos1)));

    cmd.undo();
    QVERIFY(verifyTileIsPopulated(m_map->getTile(pos1)));

    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_map->getTile(pos1)));
}

void TestDeleteSelectionCommand::testRedo_NoSelection_DoesNothing() {
    RMEPosition pos1(1,1,0);
    RMETile* tile1 = m_map->getOrCreateTile(pos1);
    populateTile(tile1);

    QList<RMEPosition> selectedPositions = {};
    RMECommand cmd(m_map, selectedPositions, m_mockController.get());

    cmd.redo();
    QVERIFY(verifyTileIsPopulated(m_map->getTile(pos1)));
    QVERIFY(cmd.getUndoneTileStates().isEmpty());
    // The command text might be set to indicate nothing was selected/done.
    // Based on current DeleteSelectionCommand.cpp, it sets text to "Delete Selection (0 tile(s))"
    // or "Delete Selection (nothing selected)"
    QVERIFY(cmd.text().contains("(0 tile(s))") || cmd.text().contains("(nothing selected)"));
}

void TestDeleteSelectionCommand::testNotificationTriggered() {
    RMEPosition pos1(1,1,0);
    populateTile(m_map->getOrCreateTile(pos1));
    QList<RMEPosition> selectedPositions = {pos1};
    RMECommand cmd(m_map, selectedPositions, m_mockController.get());

    m_mockController->resetNotifications(); // Assuming MockEditorController has this
    cmd.redo();
    QVERIFY(m_mockController->m_tileChangedNotified); // Check flag on mock
    QCOMPARE(m_mockController->m_notifiedPosition, pos1); // Check position

    m_mockController->resetNotifications();
    cmd.undo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, pos1);
}

QTEST_MAIN(TestDeleteSelectionCommand)
#include "TestDeleteSelectionCommand.moc"
