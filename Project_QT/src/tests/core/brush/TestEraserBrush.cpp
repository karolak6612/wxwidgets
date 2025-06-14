#include <QtTest/QtTest>
#include <memory>
#include <vector>
#include <algorithm>

#include "core/brush/EraserBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/assets/ItemData.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/CreatureDatabase.h"
#include "core/settings/AppSettings.h" // Required for EraserBrush logic

#include "tests/core/brush/MockEditorController.h"
#include "editor_logic/commands/RecordModifyTileContentsCommand.h"

// Using declarations
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEItem = RME::core::Item;
using RMEItemData = RME::core::assets::ItemData;
using RMEItemDatabase = RME::core::assets::ItemDatabase;
using RMEAssetManager = RME::core::assets::AssetManager;
using RMEClientVersionManager = RME::core::assets::ClientVersionManager;
using RMEMaterialManager = RME::core::assets::MaterialManager;
using RMESpriteManager = RME::core::sprites::SpriteManager;
using RMECreatureDatabase = RME::core::assets::CreatureDatabase;
using RMEAppSettings = RME::core::AppSettings;

using RMEBrush = RME::core::Brush;
using RMEEraserBrush = RME::core::brush::EraserBrush;
using RMEMockEditorController = MockEditorController;

using RMECommandBase = QUndoCommand;
using RMERecordModifyTileContentsCommand = RME_COMMANDS::RecordModifyTileContentsCommand;


// Test Item IDs
const uint16_t ID_GROUND_NORMAL = 1;
const uint16_t ID_GROUND_COMPLEX = 2; // e.g., a quest tile
const uint16_t ID_ITEM_NORMAL = 101;
const uint16_t ID_ITEM_COMPLEX = 102; // e.g., a unique container
const uint16_t ID_ITEM_BORDER = 103;  // e.g., a fence part
const uint16_t ID_ITEM_STACKABLE = 104;


class TestEraserBrush : public QObject {
    Q_OBJECT

public:
    TestEraserBrush() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test methods
    void testCanApply();

    void testApply_NormalErase_LeaveUniques();
    void testApply_NormalErase_ClearUniques();

    void testApply_AggressiveErase_LeaveUniques();
    void testApply_AggressiveErase_ClearUniques();

    void testApply_EmptyTile_NoCommand();


private:
    std::unique_ptr<RMEEraserBrush> m_eraserBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    RMEMap* m_map = nullptr; // Points to map in m_mockController
    RMETile* m_testTile = nullptr;
    RMEPosition m_testPosition;
    std::unique_ptr<RMEBrushSettings> m_brushSettings;

    std::unique_ptr<RMEClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RMEItemDatabase> m_itemDatabase;
    std::unique_ptr<RMECreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RMESpriteManager> m_spriteManager;
    std::unique_ptr<RMEMaterialManager> m_materialManager;
    std::unique_ptr<RMEAssetManager> m_assetManager;
    // MockEditorController is assumed to have a way to set AppSettings values for testing.
    // For example, a public member `RMEAppSettings m_appSettings` or a method `setMockSetting(key, value)`.

    void setupTestItemDatabase();
    void populateTestTile_AllElements();
    void populateTestTile_MixedItems();
    void setEraserLeaveUniqueItems(bool leave);

    // Helper to check item existence by ID on m_testTile
    bool hasItemOnTile(uint16_t id) const {
        if (!m_testTile) return false;
        for (const auto& item_ptr : m_testTile->getItems()) {
            if (item_ptr->getID() == id) return true;
        }
        return false;
    }
};

void TestEraserBrush::initTestCase() {}
void TestEraserBrush::cleanupTestCase() {}

void TestEraserBrush::setupTestItemDatabase() {
    QVERIFY(m_itemDatabase);
    RMEItemData d; // Re-usable ItemData instance

    d = {}; d.id = ID_GROUND_NORMAL; d.name = "Grass"; d.isGround = true; d.isComplex = false; d.isBorder = false;
    m_itemDatabase->addItemData(d);
    d = {}; d.id = ID_GROUND_COMPLEX; d.name = "Quest Ground"; d.isGround = true; d.isComplex = true; d.isBorder = false;
    m_itemDatabase->addItemData(d);
    d = {}; d.id = ID_ITEM_NORMAL; d.name = "Flower"; d.isGround = false; d.isComplex = false; d.isBorder = false;
    m_itemDatabase->addItemData(d);
    d = {}; d.id = ID_ITEM_COMPLEX; d.name = "Magic Chest"; d.isGround = false; d.isComplex = true; d.isBorder = false;
    m_itemDatabase->addItemData(d);
    d = {}; d.id = ID_ITEM_BORDER; d.name = "Fence Post"; d.isGround = false; d.isComplex = false; d.isBorder = true;
    m_itemDatabase->addItemData(d);
    d = {}; d.id = ID_ITEM_STACKABLE; d.name = "Gold Coin"; d.isGround = false; d.isComplex = false; d.isBorder = false; d.isStackable = true;
    m_itemDatabase->addItemData(d);

    Item::setItemDatabase(m_itemDatabase.get());
}

void TestEraserBrush::init() {
    m_eraserBrush = std::make_unique<RMEEraserBrush>();

    m_clientVersionManager = std::make_unique<RMEClientVersionManager>();
    m_itemDatabase = std::make_unique<RMEItemDatabase>(*m_clientVersionManager);
    setupTestItemDatabase(); // Populate after creation, before AssetManager
    m_creatureDatabase = std::make_unique<RMECreatureDatabase>();
    m_spriteManager = std::make_unique<RMESpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RMEMaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RMEAssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    // ItemDatabase must be set for Item::create BEFORE MockEditorController constructor if it creates a MockMap that creates Items.
    m_mockController = std::make_unique<RMEMockEditorController>();
    m_map = m_mockController->getMap();
    QVERIFY(m_map);
    m_mockController->setMockAssetManager(m_assetManager.get()); // AssetManager for controller (e.g. for item names in commands)
    // Ensure the MockMap inside MockEditorController also uses the correct ItemDatabase if it creates items.
    // This is handled by Item::setItemDatabase().

    m_brushSettings = std::make_unique<RMEBrushSettings>();
    m_testPosition = RMEPosition(5, 5, 0);
    m_testTile = m_map->getOrCreateTile(m_testPosition);
    QVERIFY(m_testTile);

    m_mockController->reset();
}

void TestEraserBrush::cleanup() {
    m_eraserBrush.reset();
    m_testTile = nullptr;
    m_map = nullptr;
    m_mockController.reset();
    m_brushSettings.reset();
    m_assetManager.reset();
    m_itemDatabase.reset();
    m_creatureDatabase.reset();
    m_spriteManager.reset();
    m_materialManager.reset();
    m_clientVersionManager.reset();
    Item::setItemDatabase(nullptr);
}

void TestEraserBrush::setEraserLeaveUniqueItems(bool leave) {
    // This requires MockEditorController to have a way to influence its AppSettings.
    // For this test, we'll assume MockEditorController has a public AppSettings member
    // or a specific setter for test settings.
    // e.g. m_mockController->m_appSettings.setBool("ERASER_LEAVE_UNIQUE_ITEMS", leave);
    // If MockEditorController was updated with setMockSetting:
    m_mockController->setMockSetting("ERASER_LEAVE_UNIQUE_ITEMS", leave);
}

void TestEraserBrush::populateTestTile_AllElements() {
    m_testTile->setGround(Item::create(ID_GROUND_COMPLEX));
    m_testTile->addItem(Item::create(ID_ITEM_NORMAL));
    m_testTile->addItem(Item::create(ID_ITEM_COMPLEX));
    m_testTile->addItem(Item::create(ID_ITEM_BORDER));
    m_testTile->addItem(Item::create(ID_ITEM_STACKABLE));
    m_testTile->setSpawn(std::make_unique<RME::core::Spawn>(1, 60)); // radius 1, interval 60s
    m_testTile->setCreature(std::make_unique<RME::core::Creature>("TestEraserCreature"));
}

void TestEraserBrush::populateTestTile_MixedItems() {
    m_testTile->setGround(Item::create(ID_GROUND_NORMAL));
    m_testTile->addItem(Item::create(ID_ITEM_NORMAL));
    m_testTile->addItem(Item::create(ID_ITEM_COMPLEX));
    m_testTile->addItem(Item::create(ID_ITEM_BORDER));
}


void TestEraserBrush::testCanApply() {
    QVERIFY(m_eraserBrush->canApply(m_map, m_testPosition, *m_brushSettings));
    QVERIFY(!m_eraserBrush->canApply(nullptr, m_testPosition, *m_brushSettings));
    QVERIFY(!m_eraserBrush->canApply(m_map, RMEPosition(100,100,0), *m_brushSettings));
}

void TestEraserBrush::testApply_NormalErase_LeaveUniques() {
    setEraserLeaveUniqueItems(true);
    m_brushSettings->isEraseMode = false;
    populateTestTile_AllElements();

    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    auto* cmd = dynamic_cast<RMERecordModifyTileContentsCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    QVERIFY(cmd->getStoredOldGround() == nullptr);
    QVERIFY(cmd->getStoredOldSpawn() == nullptr);
    QVERIFY(cmd->getStoredOldCreature() == nullptr);

    const auto& capturedItems = cmd->getStoredOldItems();
    QCOMPARE(capturedItems.size(), 2);
    bool normalFound = false, stackableFound = false;
    for(const auto& item : capturedItems) {
        if(item->getID() == ID_ITEM_NORMAL) normalFound = true;
        if(item->getID() == ID_ITEM_STACKABLE) stackableFound = true;
    }
    QVERIFY(normalFound && stackableFound);

    cmd->redo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == ID_GROUND_COMPLEX);
    QVERIFY(hasItemOnTile(ID_ITEM_COMPLEX));
    QVERIFY(hasItemOnTile(ID_ITEM_BORDER));
    QVERIFY(!hasItemOnTile(ID_ITEM_NORMAL));
    QVERIFY(!hasItemOnTile(ID_ITEM_STACKABLE));
    QVERIFY(m_testTile->getSpawn() != nullptr);
    QVERIFY(m_testTile->getCreature() != nullptr);
}

void TestEraserBrush::testApply_NormalErase_ClearUniques() {
    setEraserLeaveUniqueItems(false);
    m_brushSettings->isEraseMode = false;
    populateTestTile_MixedItems();

    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    auto* cmd = dynamic_cast<RMERecordModifyTileContentsCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    QVERIFY(cmd->getStoredOldGround() == nullptr);
    QVERIFY(cmd->getStoredOldSpawn() == nullptr);
    QVERIFY(cmd->getStoredOldCreature() == nullptr);
    QCOMPARE(cmd->getStoredOldItems().size(), 3);

    cmd->redo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == ID_GROUND_NORMAL);
    QVERIFY(m_testTile->getItems().isEmpty());
}

void TestEraserBrush::testApply_AggressiveErase_LeaveUniques() {
    setEraserLeaveUniqueItems(true);
    m_brushSettings->isEraseMode = true;
    populateTestTile_AllElements();

    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    auto* cmd = dynamic_cast<RMERecordModifyTileContentsCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    QVERIFY(cmd->getStoredOldGround() == nullptr);
    const auto& capturedItems = cmd->getStoredOldItems();
    bool normalFound = false, stackableFound = false, complexFound = false, borderFound = false;
    for(const auto& item : capturedItems) {
        if(item->getID() == ID_ITEM_NORMAL) normalFound = true;
        if(item->getID() == ID_ITEM_STACKABLE) stackableFound = true;
        if(item->getID() == ID_ITEM_COMPLEX) complexFound = true; // Should not be here
        if(item->getID() == ID_ITEM_BORDER) borderFound = true;   // Should not be here
    }
    QVERIFY(normalFound && stackableFound);
    QVERIFY(!complexFound && !borderFound);
    QCOMPARE(capturedItems.size(), 2);

    QVERIFY(cmd->getStoredOldSpawn() != nullptr);
    QVERIFY(cmd->getStoredOldCreature() != nullptr);

    cmd->redo();
    QVERIFY(m_testTile->getGround() && m_testTile->getGround()->getID() == ID_GROUND_COMPLEX);
    QVERIFY(hasItemOnTile(ID_ITEM_COMPLEX));
    QVERIFY(hasItemOnTile(ID_ITEM_BORDER));
    QVERIFY(!hasItemOnTile(ID_ITEM_NORMAL));
    QVERIFY(!hasItemOnTile(ID_ITEM_STACKABLE));
    QVERIFY(m_testTile->getSpawn() == nullptr);
    QVERIFY(m_testTile->getCreature() == nullptr);
}

void TestEraserBrush::testApply_AggressiveErase_ClearUniques() {
    setEraserLeaveUniqueItems(false);
    m_brushSettings->isEraseMode = true;
    populateTestTile_AllElements();

    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    auto* cmd = dynamic_cast<RMERecordModifyTileContentsCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd);

    QVERIFY(cmd->getStoredOldGround() != nullptr && cmd->getStoredOldGround()->getID() == ID_GROUND_COMPLEX);
    QCOMPARE(cmd->getStoredOldItems().size(), 4);
    QVERIFY(cmd->getStoredOldSpawn() != nullptr);
    QVERIFY(cmd->getStoredOldCreature() != nullptr);

    cmd->redo();
    QVERIFY(m_testTile->getGround() == nullptr);
    QVERIFY(m_testTile->getItems().isEmpty());
    QVERIFY(m_testTile->getSpawn() == nullptr);
    QVERIFY(m_testTile->getCreature() == nullptr);
}

void TestEraserBrush::testApply_EmptyTile_NoCommand() {
    m_brushSettings->isEraseMode = false;
    m_testTile->setGround(nullptr);
    m_testTile->clearItems();
    m_testTile->setSpawn(nullptr);
    m_testTile->setCreature(nullptr);

    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);

    m_brushSettings->isEraseMode = true;
    m_eraserBrush->apply(m_mockController.get(), m_testPosition, *m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}


QTEST_MAIN(TestEraserBrush)
#include "TestEraserBrush.moc"
