#include <QtTest/QtTest>
#include <memory>
#include <vector>
#include <algorithm> // For std::find_if

#include "editor_logic/commands/RecordAddRemoveItemCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "tests/core/brush/MockEditorController.h"

// Using declarations
using RMEPosition = RME::core::Position;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEItem = RME::core::Item;
using RMEItemData = RME::core::assets::ItemData;
using RMEAssetManager = RME::core::assets::AssetManager;
using RMEItemDatabase = RME::core::assets::ItemDatabase;
using RMECreatureDatabase = RME::core::assets::CreatureDatabase;
using RMESpriteManager = RME::core::sprites::SpriteManager;
using RMEClientVersionManager = RME::core::assets::ClientVersionManager;
using RMEMaterialManager = RME::core::assets::MaterialManager;
using RMECommand = RME_COMMANDS::RecordAddRemoveItemCommand;
using RMEMockEditorController = MockEditorController;

const uint16_t TEST_ITEM_ID_1 = 201;
const uint16_t TEST_ITEM_ID_2 = 202;
const uint16_t TEST_GROUND_FOR_ITEMS = 1;

class TestRecordAddRemoveItemCommand : public QObject {
    Q_OBJECT

public:
    TestRecordAddRemoveItemCommand() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    // Test cases
    void testAddItem_UndoRedo();
    void testRemoveItem_UndoRedo();
    void testRemoveNonExistentItem_ShouldNotCrash();
    void testNotificationTriggered_Add();
    void testNotificationTriggered_Remove();

private:
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEAssetManager> m_assetManager;
    std::unique_ptr<RMEItemDatabase> m_itemDatabase;
    std::unique_ptr<RMECreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RMESpriteManager> m_spriteManager;
    std::unique_ptr<RMEClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RMEMaterialManager> m_materialManager;
    // RMEMap* m_map will point to the map inside m_mockController
    RMEMap* m_map = nullptr;
    RMETile* m_testTile = nullptr;
    RMEPosition m_testPosition;

    void setupMockItemDatabase() {
        RMEItemData item1, item2, groundItem;
        item1.id = TEST_ITEM_ID_1; item1.name = "Test Item Alpha"; item1.isGround = false;
        item2.id = TEST_ITEM_ID_2; item2.name = "Test Item Beta"; item2.isGround = false;
        groundItem.id = TEST_GROUND_FOR_ITEMS; groundItem.name = "Items Ground"; groundItem.isGround = true;
        m_itemDatabase->addItemData(item1);
        m_itemDatabase->addItemData(item2);
        m_itemDatabase->addItemData(groundItem);
        Item::setItemDatabase(m_itemDatabase.get());
    }

    // Helper to count items of a certain ID on the tile
    int countItems(uint16_t id) const {
        if (!m_testTile) return 0;
        int count = 0;
        for (const auto& item_ptr : m_testTile->getItems()) {
            if (item_ptr->getID() == id) {
                count++;
            }
        }
        return count;
    }
};

void TestRecordAddRemoveItemCommand::initTestCase() {}

void TestRecordAddRemoveItemCommand::init() {
    m_clientVersionManager = std::make_unique<RMEClientVersionManager>();
    m_itemDatabase = std::make_unique<RMEItemDatabase>(*m_clientVersionManager);
    setupMockItemDatabase();
    m_creatureDatabase = std::make_unique<RMECreatureDatabase>();
    m_spriteManager = std::make_unique<RMESpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RMEMaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RMEAssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    m_mockController = std::make_unique<RMEMockEditorController>();
    m_map = m_mockController->getMap(); // Use the map from MockEditorController
    QVERIFY(m_map);

    m_mockController->setMockAssetManager(m_assetManager.get());

    m_testPosition = RMEPosition(4, 4, 0);
    m_testTile = m_map->getOrCreateTile(m_testPosition);
    QVERIFY(m_testTile);
    m_testTile->setGround(Item::create(TEST_GROUND_FOR_ITEMS));
    QVERIFY(m_testTile->getGround());

    // This was MockEditorController::resetNotifications(), changed to match actual mock
    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
}

void TestRecordAddRemoveItemCommand::cleanup() {
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

void TestRecordAddRemoveItemCommand::cleanupTestCase() {}

void TestRecordAddRemoveItemCommand::testAddItem_UndoRedo() {
    QCOMPARE(countItems(TEST_ITEM_ID_1), 0);

    auto itemToAdd = Item::create(TEST_ITEM_ID_1);
    RMECommand cmd(m_testTile, std::move(itemToAdd), m_mockController.get());

    cmd.redo(); // Add item
    QCOMPARE(countItems(TEST_ITEM_ID_1), 1);

    cmd.undo(); // Remove item
    QCOMPARE(countItems(TEST_ITEM_ID_1), 0);

    cmd.redo(); // Add item again
    QCOMPARE(countItems(TEST_ITEM_ID_1), 1);
}

void TestRecordAddRemoveItemCommand::testRemoveItem_UndoRedo() {
    m_testTile->addItem(Item::create(TEST_ITEM_ID_1));
    QCOMPARE(countItems(TEST_ITEM_ID_1), 1);

    RME::core::Item* itemOnTile = nullptr;
    for (const auto& item_ptr : m_testTile->getItems()) {
        if (item_ptr->getID() == TEST_ITEM_ID_1) {
            itemOnTile = item_ptr.get();
            break;
        }
    }
    QVERIFY(itemOnTile != nullptr);

    RMECommand cmd(m_testTile, itemOnTile, m_mockController.get());

    cmd.redo(); // Remove item
    QCOMPARE(countItems(TEST_ITEM_ID_1), 0);

    cmd.undo(); // Add item back
    QCOMPARE(countItems(TEST_ITEM_ID_1), 1);

    cmd.redo(); // Remove item again
    QCOMPARE(countItems(TEST_ITEM_ID_1), 0);
}

void TestRecordAddRemoveItemCommand::testRemoveNonExistentItem_ShouldNotCrash() {
    QCOMPARE(countItems(TEST_ITEM_ID_2), 0);

    auto dummyItemForCtor = Item::create(TEST_ITEM_ID_2);
    RMECommand cmd(m_testTile, dummyItemForCtor.get(), m_mockController.get());

    cmd.redo();
    QCOMPARE(countItems(TEST_ITEM_ID_2), 0);

    cmd.undo();
    QCOMPARE(countItems(TEST_ITEM_ID_2), 1);
}

void TestRecordAddRemoveItemCommand::testNotificationTriggered_Add() {
    auto itemToAdd = Item::create(TEST_ITEM_ID_1);
    RMECommand cmd(m_testTile, std::move(itemToAdd), m_mockController.get());

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

void TestRecordAddRemoveItemCommand::testNotificationTriggered_Remove() {
    m_testTile->addItem(Item::create(TEST_ITEM_ID_1));
    RME::core::Item* itemOnTile = m_testTile->getTopItemByID(TEST_ITEM_ID_1);
    QVERIFY(itemOnTile);

    RMECommand cmd(m_testTile, itemOnTile, m_mockController.get());

    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
    cmd.redo(); // remove
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_testPosition);

    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
    cmd.undo(); // add back
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_testPosition);
}

QTEST_MAIN(TestRecordAddRemoveItemCommand)
#include "TestRecordAddRemoveItemCommand.moc"
