#include <QtTest/QtTest>
#include <memory>

#include "core/brush/SpawnBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Spawn.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"
// #include "editor_logic/commands/RecordSetSpawnCommand.h" // Not directly tested here, but brush uses it

#include "tests/core/brush/MockEditorController.h"
#include "tests/core/assets/MockAssetManager.h"
#include "tests/core/assets/MockMaterialManager.h"
#include "tests/core/assets/MockCreatureDatabase.h"
#include "tests/core/MockItemTypeProvider.h"
#include "core/assets/AssetManager.h" // Real AssetManager for Map
#include "core/sprites/SpriteManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"


// Using declarations
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMESpawn = RME::core::Spawn;
using RMESpawnBrush = RME::core::brush::SpawnBrush;
using RMEMockEditorController = MockEditorController;
// using RMEMockAssetManager = RME::tests::MockAssetManager; // Controller will use one, map another
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEItem = RME::core::Item;

// Defined in SpawnBrush.cpp, make it accessible or re-define for test clarity if needed.
extern const int DEFAULT_SPAWN_INTERVAL_SECONDS;

class TestSpawnBrush : public QObject {
    Q_OBJECT

public:
    TestSpawnBrush() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testCanApply();
    void testApply_DrawNewSpawn();
    void testApply_DrawUpdateExistingSpawn();
    void testApply_EraseSpawn();
    void testApply_EraseEmpty();


private:
    std::unique_ptr<RMESpawnBrush> m_spawnBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;

    // For real AssetManager used by Map
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
    std::unique_ptr<RME::core::assets::AssetManager> m_realAssetManagerForMap;

    // For Mock AssetManager used by Controller (if controller's needs differ)
    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProviderForController;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDbForController;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgrForController;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgrForController;


    std::unique_ptr<RMEBrushSettings> m_brushSettings;

    const uint16_t GROUND_ITEM_ID = 701;
};

void TestSpawnBrush::initTestCase() {
    // One-time setup
}

void TestSpawnBrush::init() {
    m_spawnBrush = std::make_unique<RMESpawnBrush>();
    m_mockController = std::make_unique<RMEMockEditorController>();

    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    RME::core::assets::ItemData groundItemData;
    groundItemData.id = GROUND_ITEM_ID;
    groundItemData.name = "Test Ground";
    groundItemData.isGround = true;
    m_itemDatabase->addItemData(groundItemData);

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);

    m_realAssetManagerForMap = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    m_map = std::make_unique<RMEMap>(20, 20, 1, m_realAssetManagerForMap.get());

    m_mockItemProviderForController = std::make_unique<RMEMockItemTypeProvider>();
    m_mockCreatureDbForController = std::make_unique<RMEMockCreatureDatabase>();
    m_mockMaterialMgrForController = std::make_unique<RMEMockMaterialManager>(*m_clientVersionManager);
    m_mockAssetMgrForController = std::make_unique<RMEMockAssetManager>(
        m_mockItemProviderForController.get(), m_mockCreatureDbForController.get(), m_mockMaterialMgrForController.get()
    );

    m_brushSettings = std::make_unique<RMEBrushSettings>();
    m_brushSettings->setSize(3);

    m_mockController->m_mockMap = m_map.get();
    m_mockController->m_brushSettings = m_brushSettings.get();
    m_mockController->setMockAssetManager(m_mockAssetMgrForController.get());
    m_mockController->reset();
}

void TestSpawnBrush::cleanup() {
    m_spawnBrush.reset();
    m_mockController.reset();
    m_map.reset();
    m_realAssetManagerForMap.reset(); // Order matters: reset map users first
    m_mockAssetMgrForController.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    m_brushSettings.reset();
    m_mockItemProviderForController.reset();
    m_mockCreatureDbForController.reset();
    m_mockMaterialMgrForController.reset();
}
void TestSpawnBrush::cleanupTestCase() {}

void TestSpawnBrush::testCanApply() {
    QVERIFY(m_spawnBrush);
    RMEPosition validPos(5,5,0);
    RMEPosition invalidPos(100,100,0);

    RMETile* tileWithGround = m_map->getOrCreateTile(validPos);
    QVERIFY(tileWithGround);
    tileWithGround->setGround(std::make_unique<RMEItem>(GROUND_ITEM_ID, m_itemDatabase->getItemData(GROUND_ITEM_ID)));
    QVERIFY(m_spawnBrush->canApply(m_map.get(), validPos, *m_brushSettings));

    RMETile* tileNoGround = m_map->getOrCreateTile(RMEPosition(6,6,0));
    QVERIFY(tileNoGround);
    tileNoGround->setGround(nullptr);
    QVERIFY(!m_spawnBrush->canApply(m_map.get(), RMEPosition(6,6,0), *m_brushSettings));

    QVERIFY(!m_spawnBrush->canApply(m_map.get(), invalidPos, *m_brushSettings));

    QVERIFY(!m_spawnBrush->canApply(nullptr, validPos, *m_brushSettings));
}

void TestSpawnBrush::testApply_DrawNewSpawn() {
    QVERIFY(m_spawnBrush);
    RMEPosition pos(5,5,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->setGround(std::make_unique<RMEItem>(GROUND_ITEM_ID, m_itemDatabase->getItemData(GROUND_ITEM_ID)));
    QVERIFY(!tile->getSpawn());

    m_brushSettings->setSize(5);

    m_mockController->reset();
    m_spawnBrush->apply(m_mockController.get(), pos, *m_brushSettings.get());

    // The command applies the change. We check the command recorded.
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommandText.contains("Set Spawn"));
    // To check tile state, we'd need to execute the command or inspect its contents.
    // For now, assume command construction implies intent.
    // If RecordSetSpawnCommand was used, we could try to cast lastPushedCommand if it's stored.
}

void TestSpawnBrush::testApply_DrawUpdateExistingSpawn() {
    RMEPosition pos(6,6,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->setGround(std::make_unique<RMEItem>(GROUND_ITEM_ID, m_itemDatabase->getItemData(GROUND_ITEM_ID)));

    auto initialSpawn = std::make_unique<RMESpawn>(2, 30);
    initialSpawn->addCreatureType("Dragon");
    tile->setSpawn(std::move(initialSpawn)); // Directly set for pre-condition
    QVERIFY(tile->getSpawn());

    m_brushSettings->setSize(4);

    m_mockController->reset();
    m_spawnBrush->apply(m_mockController.get(), pos, *m_brushSettings.get());

    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommandText.contains("Set Spawn"));
    // Further checks would involve executing the command and then verifying tile state.
}


void TestSpawnBrush::testApply_EraseSpawn() {
    RMEPosition pos(7,7,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->setGround(std::make_unique<RMEItem>(GROUND_ITEM_ID, m_itemDatabase->getItemData(GROUND_ITEM_ID)));
    tile->setSpawn(std::make_unique<RMESpawn>(3));
    QVERIFY(tile->getSpawn());

    m_brushSettings->isEraseMode = true;
    m_mockController->reset();
    m_spawnBrush->apply(m_mockController.get(), pos, *m_brushSettings.get());

    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommandText.contains("Erase Spawn"));
}

void TestSpawnBrush::testApply_EraseEmpty() {
    RMEPosition pos(8,8,0);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->setGround(std::make_unique<RMEItem>(GROUND_ITEM_ID, m_itemDatabase->getItemData(GROUND_ITEM_ID)));
    QVERIFY(!tile->getSpawn());

    m_brushSettings->isEraseMode = true;
    m_mockController->reset();
    m_spawnBrush->apply(m_mockController.get(), pos, *m_brushSettings.get());

    QVERIFY(!m_mockController->pushCommandCalled);
}

// #include "TestSpawnBrush.moc" // For AUTOMOC
