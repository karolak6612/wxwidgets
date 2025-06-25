#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr

// Classes to be tested or used
#include "core/brush/RawBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h" // Though not directly used by RawBrush, Tile might interact
#include "core/assets/ItemData.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ClientVersionManager.h" // For AssetManager
#include "core/assets/MaterialManager.h"    // For AssetManager
#include "core/sprites/SpriteManager.h"       // For AssetManager
#include "core/assets/CreatureDatabase.h"   // For AssetManager

// Mocks and test utilities
#include "tests/core/brush/MockEditorController.h" // Already should have pushCommand etc.
#include "editor_logic/commands/RecordSetGroundCommand.h"
#include "editor_logic/commands/RecordAddRemoveItemCommand.h"


// Using declarations for convenience
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEItem = RME::core::Item;
using RMEItemData = RME::core::ItemData;
using RMEItemDatabase = RME::core::assets::ItemDatabase;
using RMEAssetManager = RME::core::assets::AssetManager;
using RMEClientVersionManager = RME::core::assets::ClientVersionManager;
using RMEMaterialManager = RME::core::assets::MaterialManager;
using RMESpriteManager = RME::core::sprites::SpriteManager;
using RMECreatureDatabase = RME::core::assets::CreatureDatabase;

using RMEBrush = RME::core::Brush; // Base class
using RMERawBrush = RME::core::RawBrush;
using RMEMockEditorController = MockEditorController; // From tests/core/brush/

using RMECommandBase = QUndoCommand; // Base for casting
using RMERecordSetGroundCommand = RME::core::actions::RecordSetGroundCommand;
using RMERecordAddRemoveItemCommand = RME::core::actions::RecordAddRemoveItemCommand;
using RMEItemChangeOperation = RME::core::actions::ItemChangeOperation;


const uint16_t GROUND_ITEM_ID_1 = 101;
const uint16_t GROUND_ITEM_ID_2 = 102;
const uint16_t STACKABLE_ITEM_ID_1 = 201;
const uint16_t NON_STACKABLE_ITEM_ID_1 = 301;


class TestRawBrush : public QObject {
    Q_OBJECT

public:
    TestRawBrush() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test cases
    void testSetGetItemId();
    void testCanApply_InvalidConditions();
    void testCanApply_ValidConditions();

    void testApply_DrawGround_New();
    void testApply_DrawGround_Replace();
    void testApply_DrawNonGround_OnGround();
    void testApply_DrawNonGround_NoGroundError(); // Should not place if tile has no ground

    void testApply_EraseGround_Exists();
    void testApply_EraseGround_NotMatching();
    void testApply_EraseGround_NoGround();

    void testApply_EraseNonGround_Exists();
    void testApply_EraseNonGround_NotExists();
    void testApply_EraseNonGround_NoGround(); // Should not matter if item isn't ground

    void testApply_InvalidItemId(); // Item ID 0 or not in DB
    void testApply_NoItemIdSelected();


private:
    std::unique_ptr<RMERawBrush> m_rawBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    // RMEMap* m_map will point to the map inside m_mockController
    // No need for separate m_map unique_ptr if MockEditorController owns its map.
    RMEMap* m_map = nullptr;
    std::unique_ptr<RMEBrushSettings> m_brushSettings;

    // Real AssetManager setup for Map and Item::create
    std::unique_ptr<RMEClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RMEItemDatabase> m_itemDatabase;
    std::unique_ptr<RMECreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RMESpriteManager> m_spriteManager;
    std::unique_ptr<RMEMaterialManager> m_materialManager;
    std::unique_ptr<RMEAssetManager> m_assetManager;

    void setupMockItemDatabase();
};

void TestRawBrush::initTestCase() {
    // One-time setup for the entire test class
}

void TestRawBrush::cleanupTestCase() {
    // One-time cleanup for the entire test class
}

void TestRawBrush::setupMockItemDatabase() {
    QVERIFY(m_itemDatabase);
    RMEItemData groundData1;
    groundData1.id = GROUND_ITEM_ID_1;
    groundData1.name = "Grass";
    groundData1.isGround = true;
    m_itemDatabase->addItemData(groundData1);

    RMEItemData groundData2;
    groundData2.id = GROUND_ITEM_ID_2;
    groundData2.name = "Dirt";
    groundData2.isGround = true;
    m_itemDatabase->addItemData(groundData2);

    RMEItemData stackableData1;
    stackableData1.id = STACKABLE_ITEM_ID_1;
    stackableData1.name = "Magic Stone";
    stackableData1.isStackable = true;
    m_itemDatabase->addItemData(stackableData1);

    RMEItemData nonStackableData1;
    nonStackableData1.id = NON_STACKABLE_ITEM_ID_1;
    nonStackableData1.name = "Sword";
    m_itemDatabase->addItemData(nonStackableData1);
}


void TestRawBrush::init() {
    m_rawBrush = std::make_unique<RMERawBrush>();

    // AssetManager and its components
    m_clientVersionManager = std::make_unique<RMEClientVersionManager>();
    m_itemDatabase = std::make_unique<RMEItemDatabase>(*m_clientVersionManager);
    m_creatureDatabase = std::make_unique<RMECreatureDatabase>();
    m_spriteManager = std::make_unique<RMESpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RMEMaterialManager>(*m_clientVersionManager);
    setupMockItemDatabase();
    m_assetManager = std::make_unique<RMEAssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    // MockEditorController AFTER AssetManager, because MockEditorController's constructor might use it.
    // MockEditorController creates its own MockMap, which uses the global itemDb for Item::create.
    // So, ItemDatabase must be populated before MockMap is used by MockEditorController.
    RME::core::Item::setItemDatabase(&m_itemDatabase.value()); // Set global static item DB for Item::create
    m_mockController = std::make_unique<RMEMockEditorController>();

    m_map = m_mockController->getMap(); // Get the map from controller
    QVERIFY(m_map);
    // Ensure the map inside MockEditorController also uses our AssetManager if it needs to create items
    // For now, we assume MockMap or Tile methods use the global ItemDatabase for Item::create,
    // or AssetManager is passed down if needed. The MockEditorController was given one below.

    m_brushSettings = std::make_unique<RMEBrushSettings>();
    m_mockController->setMockAssetManager(m_assetManager.get());
    m_mockController->reset();
}

void TestRawBrush::cleanup() {
    m_rawBrush.reset();
    m_mockController.reset();
    m_map = nullptr; // Was pointing to map inside m_mockController
    m_brushSettings.reset();

    m_assetManager.reset();
    m_itemDatabase.reset();
    m_creatureDatabase.reset();
    m_spriteManager.reset();
    m_materialManager.reset();
    m_clientVersionManager.reset();
    RME::core::Item::setItemDatabase(nullptr); // Clear global static item DB
}

void TestRawBrush::testSetGetItemId() {
    QCOMPARE(m_rawBrush->getItemId(), static_cast<uint16_t>(0)); // Default
    m_rawBrush->setItemId(1234);
    QCOMPARE(m_rawBrush->getItemId(), static_cast<uint16_t>(1234));
}

void TestRawBrush::testCanApply_InvalidConditions() {
    RMEPosition pos(5,5,0);
    m_rawBrush->setItemId(0); // No item selected
    QVERIFY(!m_rawBrush->canApply(m_map, pos, *m_brushSettings));

    m_rawBrush->setItemId(GROUND_ITEM_ID_1);
    QVERIFY(!m_rawBrush->canApply(nullptr, pos, *m_brushSettings)); // Null map
    QVERIFY(!m_rawBrush->canApply(m_map, RMEPosition(100,100,0), *m_brushSettings)); // Invalid pos

    RMETile* tileNoGround = m_map->getOrCreateTile(pos);
    tileNoGround->setGround(nullptr);
    m_brushSettings->isEraseMode = true;
    QVERIFY(!m_rawBrush->canApply(m_map, pos, *m_brushSettings)); // Cannot erase from tile without ground
}

void TestRawBrush::testCanApply_ValidConditions() {
    RMEPosition pos(5,5,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(GROUND_ITEM_ID_1);
    m_brushSettings->isEraseMode = false;
    QVERIFY(m_rawBrush->canApply(m_map, pos, *m_brushSettings));

    m_brushSettings->isEraseMode = true;
    QVERIFY(m_rawBrush->canApply(m_map, pos, *m_brushSettings));

    RMETile* tileNoGround = m_map->getOrCreateTile(RMEPosition(6,6,0));
    tileNoGround->setGround(nullptr); // Intentionally no ground
    m_rawBrush->setItemId(GROUND_ITEM_ID_1); // A ground item
    m_brushSettings->isEraseMode = false;
    QVERIFY(m_rawBrush->canApply(m_map, RMEPosition(6,6,0), *m_brushSettings)); // Can place ground if no ground
}


void TestRawBrush::testApply_DrawGround_New() {
    RMEPosition pos(1,1,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(nullptr);

    m_rawBrush->setItemId(GROUND_ITEM_ID_1);
    m_brushSettings->isEraseMode = false;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);

    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand);
    auto* cmd = dynamic_cast<RMERecordSetGroundCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    // Verify command's internal state (requires getters on command if not testing via redo)
    // const RMEItem* undoCmdState = cmd->getGroundForUndoState(); // Assuming getter
    // const RMEItem* redoCmdState = cmd->getGroundForRedoState(); // Assuming getter
    // QVERIFY(undoCmdState == nullptr);
    // QVERIFY(redoCmdState != nullptr && redoCmdState->getID() == GROUND_ITEM_ID_1);

    cmd->redo();
    QVERIFY(tile->getGround() != nullptr);
    QCOMPARE(tile->getGround()->getID(), GROUND_ITEM_ID_1);
}

void TestRawBrush::testApply_DrawGround_Replace() {
    RMEPosition pos(1,2,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(GROUND_ITEM_ID_2);
    m_brushSettings->isEraseMode = false;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand);
    auto* cmd = dynamic_cast<RMERecordSetGroundCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    cmd->redo();
    QVERIFY(tile->getGround() != nullptr && tile->getGround()->getID() == GROUND_ITEM_ID_2);
}

void TestRawBrush::testApply_DrawNonGround_OnGround() {
    RMEPosition pos(1,3,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(STACKABLE_ITEM_ID_1);
    m_brushSettings->isEraseMode = false;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand);
    auto* cmd = dynamic_cast<RMERecordAddRemoveItemCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);
    QCOMPARE(cmd->getOperation(), RMEItemChangeOperation::Add);
    QCOMPARE(cmd->getItemIdForOperation(), STACKABLE_ITEM_ID_1);

    cmd->redo();
    QVERIFY(tile->getTopItemByID(STACKABLE_ITEM_ID_1) != nullptr);
}

void TestRawBrush::testApply_DrawNonGround_NoGroundError() {
    RMEPosition pos(1,4,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(nullptr);

    m_rawBrush->setItemId(STACKABLE_ITEM_ID_1); // Non-ground item
    m_brushSettings->isEraseMode = false;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
    QVERIFY(tile->getItems().isEmpty());
}

void TestRawBrush::testApply_EraseGround_Exists() {
    RMEPosition pos(2,1,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(GROUND_ITEM_ID_1);
    m_brushSettings->isEraseMode = true;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand);
    auto* cmd = dynamic_cast<RMERecordSetGroundCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    cmd->redo();
    QVERIFY(tile->getGround() == nullptr);
}

void TestRawBrush::testApply_EraseGround_NotMatching() {
    RMEPosition pos(2,2,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(GROUND_ITEM_ID_2);
    m_brushSettings->isEraseMode = true;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
    QVERIFY(tile->getGround() != nullptr && tile->getGround()->getID() == GROUND_ITEM_ID_1);
}

void TestRawBrush::testApply_EraseGround_NoGround() {
    RMEPosition pos(2,3,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(nullptr);

    m_rawBrush->setItemId(GROUND_ITEM_ID_1);
    m_brushSettings->isEraseMode = true;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

void TestRawBrush::testApply_EraseNonGround_Exists() {
    RMEPosition pos(3,1,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));
    tile->addItem(RMEItem::create(STACKABLE_ITEM_ID_1));

    m_rawBrush->setItemId(STACKABLE_ITEM_ID_1);
    m_brushSettings->isEraseMode = true;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand);
    auto* cmd = dynamic_cast<RMERecordAddRemoveItemCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);
    QCOMPARE(cmd->getOperation(), RMEItemChangeOperation::Remove);
    QCOMPARE(cmd->getItemIdForOperation(), STACKABLE_ITEM_ID_1);

    cmd->redo();
    QVERIFY(tile->getTopItemByID(STACKABLE_ITEM_ID_1) == nullptr);
}

void TestRawBrush::testApply_EraseNonGround_NotExists() {
    RMEPosition pos(3,2,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(STACKABLE_ITEM_ID_1); // This item is not on tile
    m_brushSettings->isEraseMode = true;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

void TestRawBrush::testApply_EraseNonGround_NoGround() {
    RMEPosition pos(3,3,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(nullptr);

    m_rawBrush->setItemId(STACKABLE_ITEM_ID_1);
    m_brushSettings->isEraseMode = true;

    // RawBrush::canApply checks for ground if eraseMode is true.
    // If it passed canApply, it would attempt to remove item (which isn't there).
    // RawBrush.cpp: canApply returns false if isEraseMode and no ground.
    QVERIFY(!m_rawBrush->canApply(m_map, pos, *m_brushSettings));
    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

void TestRawBrush::testApply_InvalidItemId() {
    RMEPosition pos(4,1,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(9999); // Non-existent ID
    m_brushSettings->isEraseMode = false;

    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled); // apply should find ID invalid
}

void TestRawBrush::testApply_NoItemIdSelected() {
    RMEPosition pos(4,2,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    tile->setGround(RMEItem::create(GROUND_ITEM_ID_1));

    m_rawBrush->setItemId(0); // ID 0
    m_brushSettings->isEraseMode = false;

    // canApply should return false for itemID 0
    QVERIFY(!m_rawBrush->canApply(m_map, pos, *m_brushSettings));
    m_rawBrush->apply(m_mockController.get(), pos, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled); // apply should not proceed
}

QTEST_MAIN(TestRawBrush)
#include "TestRawBrush.moc"
