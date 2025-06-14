#include <QtTest/QtTest>
#include <memory>

#include "core/brush/HouseBrush.h"
#include "core/houses/House.h"         // For RME::core::houses::House
#include "core/houses/Houses.h"        // For RME::core::houses::Houses
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Position.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h"     // For AppSettings
#include "core/editor/EditorControllerInterface.h"
#include "tests/core/brush/MockEditorController.h" // The actual mock controller
#include "editor_logic/commands/SetHouseTileCommand.h" // To check command type
#include "core/Item.h" // Required for Item::setItemDatabase

// Minimal includes for Map/AssetManager context for HousesManager
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"

// Using declarations
using RMEHouseBrush = RME::core::brush::HouseBrush;
using RMEHouse = RME::core::houses::House;
using RMEHouses = RME::core::houses::Houses;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEPos = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMockEditorController = MockEditorController; // This is tests/core/brush/MockEditorController.h
using RMESetHouseTileCommand = RME_COMMANDS::SetHouseTileCommand;

// Forward declare MockUndoStack from TestEditorController.cpp (or ensure MockEditorController handles it)
// For this test, we will rely on MockEditorController's lastPushedCommand mechanism.
// RMEMockEditorController from "tests/core/brush/MockEditorController.h"
// already has `std::unique_ptr<QUndoCommand> lastPushedCommand;`

class TestHouseBrush : public QObject {
    Q_OBJECT

public:
    TestHouseBrush() = default;

private slots:
    void initTestCase() {}
    void init();
    void cleanup();
    void cleanupTestCase() {}

    void testSettersAndGetters();
    void testCanApply();
    void testApply_AssignToHouse_NewAssignment();
    void testApply_AssignToHouse_ReassignFromOtherHouse();
    void testApply_AssignToHouse_NoHouseSelected_Fails();
    void testApply_AssignToHouse_InvalidHouseId_Fails();
    void testApply_EraseMode_SpecificHouse_CorrectTile();
    void testApply_EraseMode_SpecificHouse_WrongTile();
    void testApply_EraseMode_GenericErase_AssignedTile();
    void testApply_EraseMode_GenericErase_UnassignedTile();

private:
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEHouses> m_housesManager; // Real Houses manager
    RMEHouse* m_house1_ptr = nullptr;
    RMEHouse* m_house2_ptr = nullptr;
    RMETile* m_tile_ptr = nullptr;
    RMEPos m_testPos;

    std::unique_ptr<RMEHouseBrush> m_houseBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    RMEBrushSettings m_brushSettings;
    std::unique_ptr<RME::core::settings::AppSettings> m_appSettings;

    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestHouseBrush::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    RME::core::Item::setItemDatabase(m_itemDatabase.get());

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_assetManager.get());
    m_housesManager = std::make_unique<RMEHouses>(m_map.get());
    m_house1_ptr = m_housesManager->createNewHouse(1);
    m_house2_ptr = m_housesManager->createNewHouse(2);
    QVERIFY(m_house1_ptr && m_house2_ptr);

    m_testPos = RMEPos(5,5,7);
    m_tile_ptr = m_map->getOrCreateTile(m_testPos);
    QVERIFY(m_tile_ptr);
    m_tile_ptr->setHouseId(0);
    m_tile_ptr->setIsProtectionZone(false);

    m_houseBrush = std::make_unique<RMEHouseBrush>();
    m_appSettings = std::make_unique<RME::core::settings::AppSettings>();

    m_mockController = std::make_unique<RMEMockEditorController>();
    // Configure the mock controller with real/mocked sub-managers
    // m_mockController->m_mockMap is already initialized in its constructor.
    // For these tests, HouseBrush calls controller->getMap() which should return our m_map.
    // So, the MockEditorController's internal MockMap needs to be replaced or made to use our m_map.
    // The existing MockEditorController creates its own MockMap.
    // For this test, we need the HouseBrush to operate on *our* m_map and m_housesManager.
    // So, MockEditorController needs to return these specific instances.
    // Let's assume MockEditorController's m_mockMap is public for tests.
    // And we add setters for HousesManager and AppSettings.
    m_mockController->m_mockMap = m_map.get(); // Point to our map
    m_mockController->setMockAssetManager(m_assetManager.get());
    m_mockController->setMockHousesManager(m_housesManager.get());
    m_mockController->setMockAppSettings(m_appSettings.get());
    m_mockController->reset();

    m_brushSettings = RMEBrushSettings();
}

void TestHouseBrush::cleanup() {
    m_tile_ptr = nullptr;
    m_houseBrush.reset();
    m_mockController.reset();
    m_appSettings.reset();
    m_house1_ptr = nullptr; m_house2_ptr = nullptr;
    m_housesManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    RME::core::Item::setItemDatabase(nullptr);
}

void TestHouseBrush::testSettersAndGetters() {
    QCOMPARE(m_houseBrush->getCurrentHouseId(), quint32(0));
    m_houseBrush->setCurrentHouseId(123);
    QCOMPARE(m_houseBrush->getCurrentHouseId(), quint32(123));
}

void TestHouseBrush::testCanApply() {
    m_brushSettings.isEraseMode = false;
    m_houseBrush->setCurrentHouseId(0);
    QVERIFY(!m_houseBrush->canApply(m_map.get(), m_testPos, m_brushSettings));

    m_brushSettings.isEraseMode = true;
    QVERIFY(m_houseBrush->canApply(m_map.get(), m_testPos, m_brushSettings));
    m_brushSettings.isEraseMode = false;

    m_houseBrush->setCurrentHouseId(m_house1_ptr->getId());
    QVERIFY(m_houseBrush->canApply(m_map.get(), m_testPos, m_brushSettings));
    QVERIFY(!m_houseBrush->canApply(m_map.get(), RMEPos(100,100,7), m_brushSettings));
    QVERIFY(!m_houseBrush->canApply(nullptr, m_testPos, m_brushSettings));

    RMETile* noTile = m_map->getTile(RMEPos(9,9,0));
    QVERIFY(noTile == nullptr); // Tile doesn't exist
    QVERIFY(!m_houseBrush->canApply(m_map.get(), RMEPos(9,9,0), m_brushSettings));
}

void TestHouseBrush::testApply_AssignToHouse_NewAssignment() {
    m_houseBrush->setCurrentHouseId(m_house1_ptr->getId());
    m_brushSettings.isEraseMode = false;

    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    QVERIFY(m_mockController->lastPushedCommand.get() != nullptr);
    RMESetHouseTileCommand* cmd = dynamic_cast<RMESetHouseTileCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd != nullptr);

    cmd->redo();
    QCOMPARE(m_tile_ptr->getHouseId(), m_house1_ptr->getId());
    QVERIFY(m_tile_ptr->isProtectionZone());
    QVERIFY(m_house1_ptr->hasTilePosition(m_testPos));
}

void TestHouseBrush::testApply_AssignToHouse_ReassignFromOtherHouse() {
    m_tile_ptr->setHouseId(m_house2_ptr->getId());
    m_house2_ptr->addTilePosition(m_testPos);

    m_houseBrush->setCurrentHouseId(m_house1_ptr->getId());
    m_brushSettings.isEraseMode = false;
    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);

    QVERIFY(m_mockController->pushCommandCalled);
    RMESetHouseTileCommand* cmd = dynamic_cast<RMESetHouseTileCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd != nullptr);
    cmd->redo();
    QCOMPARE(m_tile_ptr->getHouseId(), m_house1_ptr->getId());
    QVERIFY(m_house1_ptr->hasTilePosition(m_testPos));
}

void TestHouseBrush::testApply_AssignToHouse_NoHouseSelected_Fails() {
    m_houseBrush->setCurrentHouseId(0);
    m_brushSettings.isEraseMode = false;
    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

void TestHouseBrush::testApply_AssignToHouse_InvalidHouseId_Fails() {
    m_houseBrush->setCurrentHouseId(999);
    m_brushSettings.isEraseMode = false;
    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

void TestHouseBrush::testApply_EraseMode_SpecificHouse_CorrectTile() {
    m_house1_ptr->linkTile(m_tile_ptr);
    m_houseBrush->setCurrentHouseId(m_house1_ptr->getId());
    m_brushSettings.isEraseMode = true;

    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    RMESetHouseTileCommand* cmd = dynamic_cast<RMESetHouseTileCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd != nullptr);
    cmd->redo();
    QCOMPARE(m_tile_ptr->getHouseId(), quint32(0));
    QVERIFY(!m_tile_ptr->isProtectionZone());
    QVERIFY(!m_house1_ptr->hasTilePosition(m_testPos));
}

void TestHouseBrush::testApply_EraseMode_SpecificHouse_WrongTile() {
    m_house2_ptr->linkTile(m_tile_ptr);
    m_houseBrush->setCurrentHouseId(m_house1_ptr->getId());
    m_brushSettings.isEraseMode = true;

    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
    QCOMPARE(m_tile_ptr->getHouseId(), m_house2_ptr->getId());
}

void TestHouseBrush::testApply_EraseMode_GenericErase_AssignedTile() {
    m_house1_ptr->linkTile(m_tile_ptr);
    m_houseBrush->setCurrentHouseId(0);
    m_brushSettings.isEraseMode = true;

    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(m_mockController->pushCommandCalled);
    RMESetHouseTileCommand* cmd = dynamic_cast<RMESetHouseTileCommand*>(m_mockController->lastPushedCommand.get());
    QVERIFY(cmd != nullptr);
    cmd->redo();
    QCOMPARE(m_tile_ptr->getHouseId(), quint32(0));
    QVERIFY(!m_house1_ptr->hasTilePosition(m_testPos));
}

void TestHouseBrush::testApply_EraseMode_GenericErase_UnassignedTile() {
    QCOMPARE(m_tile_ptr->getHouseId(), quint32(0));
    m_houseBrush->setCurrentHouseId(0);
    m_brushSettings.isEraseMode = true;

    m_houseBrush->apply(m_mockController.get(), m_testPos, m_brushSettings);
    QVERIFY(!m_mockController->pushCommandCalled);
}

QTEST_MAIN(TestHouseBrush)
#include "TestHouseBrush.moc"
