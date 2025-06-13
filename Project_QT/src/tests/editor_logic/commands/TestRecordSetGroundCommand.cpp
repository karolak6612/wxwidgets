#include <QtTest/QtTest>
#include <memory>

#include "editor_logic/commands/RecordSetGroundCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
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
using RMEItemData = RME::core::assets::ItemData;
using RMEAssetManager = RME::core::assets::AssetManager;
using RMEItemDatabase = RME::core::assets::ItemDatabase;
using RMECreatureDatabase = RME::core::assets::CreatureDatabase;
using RMESpriteManager = RME::core::sprites::SpriteManager;
using RMEClientVersionManager = RME::core::assets::ClientVersionManager;
using RMEMaterialManager = RME::core::assets::MaterialManager;
using RMECommand = RME_COMMANDS::RecordSetGroundCommand;
using RMEMockEditorController = MockEditorController;

const uint16_t TEST_GROUND_ID_1 = 101; // Example ground item ID
const uint16_t TEST_GROUND_ID_2 = 102; // Another example ground item ID
const uint16_t TEST_REGULAR_ITEM_ID = 1; // For initial tile ground

class TestRecordSetGroundCommand : public QObject {
    Q_OBJECT

public:
    TestRecordSetGroundCommand() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    // Test cases
    void testSetNewGround_UndoRedo();
    void testReplaceGround_UndoRedo();
    void testClearGround_UndoRedo();
    void testUndoClearGround_UndoRedo(); // Set ground, clear it, undo clear, redo clear
    void testNotificationTriggered();

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
        RMEItemData item1, item2, regularItem;
        item1.id = TEST_GROUND_ID_1; item1.name = "Test Ground Alpha"; item1.isGround = true;
        item2.id = TEST_GROUND_ID_2; item2.name = "Test Ground Beta"; item2.isGround = true;
        regularItem.id = TEST_REGULAR_ITEM_ID; regularItem.name = "Regular Ground"; regularItem.isGround = true;
        m_itemDatabase->addItemData(item1);
        m_itemDatabase->addItemData(item2);
        m_itemDatabase->addItemData(regularItem);
        Item::setItemDatabase(m_itemDatabase.get()); // For Item::create to work if it uses static DB
    }
};

void TestRecordSetGroundCommand::initTestCase() {}

void TestRecordSetGroundCommand::init() {
    m_clientVersionManager = std::make_unique<RMEClientVersionManager>();
    m_itemDatabase = std::make_unique<RMEItemDatabase>(*m_clientVersionManager);
    setupMockItemDatabase();
    m_creatureDatabase = std::make_unique<RMECreatureDatabase>();
    m_spriteManager = std::make_unique<RMESpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RMEMaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RMEAssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    // MockEditorController creates its own MockMap.
    // We need to ensure Item::setItemDatabase is called BEFORE MockEditorController constructor
    // if MockMap's constructor or methods (like getOrCreateTile) might create Items.
    m_mockController = std::make_unique<RMEMockEditorController>();
    m_map = m_mockController->getMap(); // Use the map from MockEditorController
    QVERIFY(m_map);

    // It's crucial that any map operations in MockEditorController or its MockMap
    // that might create items (e.g. getOrCreateTile if it adds a default ground)
    // can access the ItemDatabase. Item::create uses a static pointer to ItemDatabase.

    m_mockController->setMockAssetManager(m_assetManager.get()); // For command text generation

    m_testPosition = RMEPosition(3, 3, 0);
    m_testTile = m_map->getOrCreateTile(m_testPosition);
    QVERIFY(m_testTile);

    m_testTile->setGround(Item::create(TEST_REGULAR_ITEM_ID));
    QVERIFY(m_testTile->getGround());

    // This was MockEditorController::resetNotifications(), changed to match actual mock
    m_mockController->m_tileChangedNotified = false;
    m_mockController->m_notifiedPosition = RMEPosition();
}

void TestRecordSetGroundCommand::cleanup() {
    m_testTile = nullptr;
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

void TestRecordSetGroundCommand::cleanupTestCase() {}

void TestRecordSetGroundCommand::testSetNewGround_UndoRedo() {
    m_testTile->setGround(nullptr);
    QVERIFY(!m_testTile->getGround());

    auto newGround = Item::create(TEST_GROUND_ID_1);
    uint16_t newGroundId = newGround->getID();

    RMECommand cmd(m_testTile, std::move(newGround), nullptr, m_mockController.get());
    cmd.redo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), newGroundId);

    cmd.undo();
    QVERIFY(!m_testTile->getGround());

    cmd.redo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), newGroundId);
}

void TestRecordSetGroundCommand::testReplaceGround_UndoRedo() {
    uint16_t initialGroundId = m_testTile->getGround()->getID();

    auto newGround = Item::create(TEST_GROUND_ID_1);
    uint16_t newGroundId = newGround->getID();
    auto oldGroundCopy = m_testTile->getGround()->deepCopy();

    RMECommand cmd(m_testTile, std::move(newGround), std::move(oldGroundCopy), m_mockController.get());
    cmd.redo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), newGroundId);

    cmd.undo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), initialGroundId);

    cmd.redo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), newGroundId);
}

void TestRecordSetGroundCommand::testClearGround_UndoRedo() {
    uint16_t initialGroundId = m_testTile->getGround()->getID();
    auto oldGroundCopy = m_testTile->getGround()->deepCopy();

    RMECommand cmd(m_testTile, nullptr, std::move(oldGroundCopy), m_mockController.get());
    cmd.redo();
    QVERIFY(!m_testTile->getGround());

    cmd.undo();
    QVERIFY(m_testTile->getGround());
    QCOMPARE(m_testTile->getGround()->getID(), initialGroundId);

    cmd.redo();
    QVERIFY(!m_testTile->getGround());
}

void TestRecordSetGroundCommand::testUndoClearGround_UndoRedo() {
    auto oldGroundRegularCopy = m_testTile->getGround()->deepCopy();
    auto groundId1Item = Item::create(TEST_GROUND_ID_1);
    uint16_t ground1Id = groundId1Item->getID();
    RMECommand setCmd(m_testTile, std::move(groundId1Item), std::move(oldGroundRegularCopy), m_mockController.get());
    setCmd.redo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == ground1Id);

    auto oldGroundId1Copy = m_testTile->getGround()->deepCopy();
    RMECommand clearCmd(m_testTile, nullptr, std::move(oldGroundId1Copy), m_mockController.get());
    clearCmd.redo();
    QVERIFY(!m_testTile->getGround());

    clearCmd.undo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == ground1Id);

    clearCmd.redo();
    QVERIFY(!m_testTile->getGround());
}

void TestRecordSetGroundCommand::testNotificationTriggered() {
    auto newGround = Item::create(TEST_GROUND_ID_1);
    RMECommand cmd(m_testTile, std::move(newGround), nullptr, m_mockController.get());

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

QTEST_MAIN(TestRecordSetGroundCommand)
#include "TestRecordSetGroundCommand.moc"
