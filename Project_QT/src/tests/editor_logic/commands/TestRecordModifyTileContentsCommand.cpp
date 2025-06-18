#include <QtTest/QtTest>
#include <memory>
#include <vector>
#include <algorithm> // For std::find_if

#include "editor_logic/commands/RecordModifyTileContentsCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/assets/ItemData.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h" // For AssetManager constructor
#include "core/sprites/SpriteManager.h"   // For AssetManager constructor
#include "core/assets/ClientVersionManager.h" // For AssetManager constructor
#include "core/assets/MaterialManager.h"  // For AssetManager constructor
#include "tests/core/brush/MockEditorController.h" // Re-use existing mock controller

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
using RMECommand = RME::core::actions::RecordModifyTileContentsCommand;
using RMEMockEditorController = MockEditorController;

// Test Item IDs
const uint16_t TEST_CMD_GROUND_ID = 301;
const uint16_t TEST_CMD_ITEM_ID1 = 302;
const uint16_t TEST_CMD_ITEM_ID2 = 303;
const uint16_t TEST_CMD_CREATURE_ID = 1; // Assuming creature types are identified by a name/ID
const QString TEST_CMD_CREATURE_NAME = "TestRat";

class TestRecordModifyTileContentsCommand : public QObject {
    Q_OBJECT

public:
    TestRecordModifyTileContentsCommand() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    // Test cases
    void testUndoRedo_ClearAll();
    void testUndoRedo_ClearOnlyGround();
    void testUndoRedo_ClearOnlyItems();
    void testUndoRedo_ClearOnlySpawn();
    void testUndoRedo_ClearOnlyCreature();
    // void testUndoRedo_ClearSomeItems(); // Covered by ClearOnlyItems more or less
    void testUndoRedo_EmptyClear_DoesNothing(); // Command created with no actual cleared elements
    void testNotificationTriggered();

private:
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEAssetManager> m_assetManager;
    std::unique_ptr<RMEItemDatabase> m_itemDatabase;
    std::unique_ptr<RMECreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RMESpriteManager> m_spriteManager;
    std::unique_ptr<RMEClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RMEMaterialManager> m_materialManager;
    RMEMap* m_map = nullptr; // Points to map in m_mockController
    RMETile* m_testTile = nullptr;
    RMEPosition m_testPosition;

    void setupMockItemDatabase() {
        RMEItemData ground, item1, item2;
        ground.id = TEST_CMD_GROUND_ID; ground.name = "Cmd Test Ground"; ground.isGround = true;
        item1.id = TEST_CMD_ITEM_ID1; item1.name = "Cmd Test Item 1"; item1.isGround = false;
        item2.id = TEST_CMD_ITEM_ID2; item2.name = "Cmd Test Item 2"; item2.isGround = false;
        m_itemDatabase->addItemData(ground);
        m_itemDatabase->addItemData(item1);
        m_itemDatabase->addItemData(item2);
        Item::setItemDatabase(m_itemDatabase.get());
    }
    void setupMockCreatureDatabase() {
        RME::core::assets::CreatureData creatureData;
        creatureData.name = TEST_CMD_CREATURE_NAME;
        // creatureData.id = TEST_CMD_CREATURE_ID; // If CreatureData has an ID field
        // ... other creature properties ...
        m_creatureDatabase->addCreatureData(creatureData);
        // Creature::setCreatureDatabase(m_creatureDatabase.get()); // If needed
    }

    // Helper to check tile contents
    bool verifyTileIsEmpty(const RMETile* tile) const {
        return !tile->getGround() && tile->getItems().empty() && !tile->getSpawn() && !tile->getCreature();
    }
};

void TestRecordModifyTileContentsCommand::initTestCase() {}

void TestRecordModifyTileContentsCommand::init() {
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

    m_testPosition = RMEPosition(2, 2, 0);
    m_testTile = m_map->getOrCreateTile(m_testPosition);
    QVERIFY(m_testTile);

    m_testTile->setGround(nullptr);
    m_testTile->clearItems();
    m_testTile->setSpawn(nullptr);
    m_testTile->setCreature(nullptr);

    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
}

void TestRecordModifyTileContentsCommand::cleanup() {
    m_testTile = nullptr;
    m_map = nullptr;
    m_mockController.reset();

    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    Item::setItemDatabase(nullptr);
}

void TestRecordModifyTileContentsCommand::cleanupTestCase() {}

void TestRecordModifyTileContentsCommand::testUndoRedo_ClearAll() {
    // 1. Setup tile with all elements
    auto oldGround = Item::create(TEST_CMD_GROUND_ID);
    std::vector<std::unique_ptr<Item>> oldItems;
    oldItems.push_back(Item::create(TEST_CMD_ITEM_ID1));
    oldItems.push_back(Item::create(TEST_CMD_ITEM_ID2));
    auto oldSpawn = std::make_unique<RMESpawn>(2); // radius 2
    auto oldCreature = std::make_unique<RMECreature>(TEST_CMD_CREATURE_NAME);

    m_testTile->setGround(oldGround->deepCopy());
    for(const auto& item : oldItems) m_testTile->addItem(item->deepCopy());
    m_testTile->setSpawn(oldSpawn->deepCopy());
    m_testTile->setCreature(oldCreature->deepCopy());

    // 2. Create command with copies of what was cleared
    RMECommand cmd(m_testTile, m_mockController.get(),
                   std::move(oldGround), std::move(oldItems),
                   std::move(oldSpawn), std::move(oldCreature));

    // 3. Redo (simulates brush having cleared the tile, then command is pushed)
    m_testTile->setGround(nullptr);
    m_testTile->clearItems();
    m_testTile->setSpawn(nullptr);
    m_testTile->setCreature(nullptr);
    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_testTile));

    // 4. Undo
    cmd.undo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == TEST_CMD_GROUND_ID);
    QCOMPARE(m_testTile->getItems().size(), size_t(2));
    QVERIFY(m_testTile->getSpawn() && m_testTile->getSpawn()->getRadius() == 2);
    QVERIFY(m_testTile->getCreature() && m_testTile->getCreature()->getName() == TEST_CMD_CREATURE_NAME);

    // 5. Redo again
    cmd.redo();
    QVERIFY(verifyTileIsEmpty(m_testTile));
}

void TestRecordModifyTileContentsCommand::testUndoRedo_ClearOnlyGround() {
    auto oldGround = Item::create(TEST_CMD_GROUND_ID);
    m_testTile->setGround(oldGround->deepCopy());
    m_testTile->addItem(Item::create(TEST_CMD_ITEM_ID1));

    RMECommand cmd(m_testTile, m_mockController.get(), std::move(oldGround), {}, nullptr, nullptr);

    m_testTile->setGround(nullptr);
    cmd.redo();
    QVERIFY(!m_testTile->getGround());
    QCOMPARE(m_testTile->getItems().size(), size_t(1));

    cmd.undo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == TEST_CMD_GROUND_ID);
    QCOMPARE(m_testTile->getItems().size(), size_t(1));
}

void TestRecordModifyTileContentsCommand::testUndoRedo_ClearOnlyItems() {
    std::vector<std::unique_ptr<Item>> oldItems;
    oldItems.push_back(Item::create(TEST_CMD_ITEM_ID1));
    oldItems.push_back(Item::create(TEST_CMD_ITEM_ID2));
    for(const auto& item : oldItems) m_testTile->addItem(item->deepCopy());
    m_testTile->setGround(Item::create(TEST_CMD_GROUND_ID));

    RMECommand cmd(m_testTile, m_mockController.get(), nullptr, std::move(oldItems), nullptr, nullptr);

    m_testTile->clearItems();
    cmd.redo();
    QVERIFY(m_testTile->getItems().empty());
    QVERIFY(m_testTile->getGround());

    cmd.undo();
    QCOMPARE(m_testTile->getItems().size(), size_t(2));
    QVERIFY(m_testTile->getGround());
}

void TestRecordModifyTileContentsCommand::testUndoRedo_ClearOnlySpawn() {
    auto oldSpawn = std::make_unique<RMESpawn>(3); // radius 3
    m_testTile->setSpawn(oldSpawn->deepCopy());
    m_testTile->setGround(Item::create(TEST_CMD_GROUND_ID));

    RMECommand cmd(m_testTile, m_mockController.get(), nullptr, {}, std::move(oldSpawn), nullptr);

    m_testTile->setSpawn(nullptr);
    cmd.redo();
    QVERIFY(!m_testTile->getSpawn());
    QVERIFY(m_testTile->getGround());

    cmd.undo();
    QVERIFY(m_testTile->getSpawn() && m_testTile->getSpawn()->getRadius() == 3);
}

void TestRecordModifyTileContentsCommand::testUndoRedo_ClearOnlyCreature() {
    auto oldCreature = std::make_unique<RMECreature>(TEST_CMD_CREATURE_NAME);
    m_testTile->setCreature(oldCreature->deepCopy());
    m_testTile->setGround(Item::create(TEST_CMD_GROUND_ID));

    RMECommand cmd(m_testTile, m_mockController.get(), nullptr, {}, nullptr, std::move(oldCreature));

    m_testTile->setCreature(nullptr);
    cmd.redo();
    QVERIFY(!m_testTile->getCreature());
    QVERIFY(m_testTile->getGround());

    cmd.undo();
    QVERIFY(m_testTile->getCreature() && m_testTile->getCreature()->getName() == TEST_CMD_CREATURE_NAME);
}

void TestRecordModifyTileContentsCommand::testUndoRedo_EmptyClear_DoesNothing() {
    m_testTile->setGround(Item::create(TEST_CMD_GROUND_ID));
    m_testTile->addItem(Item::create(TEST_CMD_ITEM_ID1));
    size_t initialItemCount = m_testTile->getItems().size();
    uint16_t initialGroundId = m_testTile->getGround()->getID();

    RMECommand cmd(m_testTile, m_mockController.get(), nullptr, {}, nullptr, nullptr);
    cmd.redo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == initialGroundId);
    QCOMPARE(m_testTile->getItems().size(), initialItemCount);

    cmd.undo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == initialGroundId);
    QCOMPARE(m_testTile->getItems().size(), initialItemCount);
}

void TestRecordModifyTileContentsCommand::testNotificationTriggered() {
    auto oldGround = Item::create(TEST_CMD_GROUND_ID);
    RMECommand cmd(m_testTile, m_mockController.get(), std::move(oldGround), {}, nullptr, nullptr);

    m_testTile->setGround(nullptr);

    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
    cmd.redo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_testPosition);

    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
    cmd.undo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_testPosition);
}

QTEST_MAIN(TestRecordModifyTileContentsCommand)
#include "TestRecordModifyTileContentsCommand.moc"
