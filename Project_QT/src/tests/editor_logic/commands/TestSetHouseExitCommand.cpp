#include <QtTest/QtTest>
#include <memory>

#include "editor_logic/commands/SetHouseExitCommand.h"
#include "core/houses/House.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Position.h"
#include "core/editor/EditorControllerInterface.h" // For mock controller
#include "tests/core/brush/MockEditorController.h" // The actual mock controller
// Minimal includes for Map/AssetManager context
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "core/Item.h" // Required for Item::setItemDatabase

// Using declarations
using RMESetExitCmd = RME::editor_logic::commands::SetHouseExitCommand;
using RMEHouse = RME::core::houses::House;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEPos = RME::core::Position;
using RMEMockEditorController = MockEditorController;

class TestSetHouseExitCommand : public QObject {
    Q_OBJECT

public:
    TestSetHouseExitCommand() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testSetNewExit_RedoUndo();
    void testChangeExit_RedoUndo();
    void testClearExit_RedoUndo(); // By setting an invalid position
    void testSetExit_SamePosition_NoChange();
    void testNotifications_OldAndNewExits();
    void testNotifications_OnlyNewExit();
    void testNotifications_OnlyOldExit_Clear();

private:
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEHouse> m_house;
    RMETile* m_tile_exit1_ptr = nullptr; // Owned by map
    RMETile* m_tile_exit2_ptr = nullptr; // Owned by map
    std::unique_ptr<RMEMockEditorController> m_mockController;

    RMEPos m_posExit1, m_posExit2, m_posInvalid;

    // Asset related members
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestSetHouseExitCommand::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    RME::core::Item::setItemDatabase(m_itemDatabase.get()); // For Tiles creating items if needed

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_assetManager.get());
    m_house = std::make_unique<RMEHouse>(1, m_map.get());

    m_posExit1 = RMEPos(5,5,7);
    m_posExit2 = RMEPos(6,6,7);
    // m_posInvalid is default constructed (invalid)

    m_tile_exit1_ptr = m_map->getOrCreateTile(m_posExit1);
    m_tile_exit2_ptr = m_map->getOrCreateTile(m_posExit2);
    QVERIFY(m_tile_exit1_ptr && m_tile_exit2_ptr);
    m_tile_exit1_ptr->setIsHouseExit(false);
    m_tile_exit2_ptr->setIsHouseExit(false);

    m_mockController = std::make_unique<RMEMockEditorController>();
    m_mockController->m_mockMap = m_map.get();
    m_mockController->setMockAssetManager(m_assetManager.get()); // For text generation if any
    m_mockController->resetNotifications();
}

void TestSetHouseExitCommand::cleanup() {
    m_tile_exit1_ptr = nullptr; m_tile_exit2_ptr = nullptr;
    m_house.reset();
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

void TestSetHouseExitCommand::testSetNewExit_RedoUndo() {
    QVERIFY(!m_house->getExitPos().isValid());
    QVERIFY(!m_tile_exit1_ptr->isHouseExit());

    RMESetExitCmd cmd(m_house.get(), m_posExit1, m_mockController.get());
    cmd.redo();

    QCOMPARE(m_house->getExitPos(), m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());

    cmd.undo();
    QVERIFY(!m_house->getExitPos().isValid());
    QVERIFY(!m_tile_exit1_ptr->isHouseExit());
}

void TestSetHouseExitCommand::testChangeExit_RedoUndo() {
    m_house->setExit(m_posExit1); // Set initial exit directly
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
    QVERIFY(!m_tile_exit2_ptr->isHouseExit());
    m_mockController->resetNotifications(); // Reset after setup

    RMESetExitCmd cmd(m_house.get(), m_posExit2, m_mockController.get());
    cmd.redo();

    QCOMPARE(m_house->getExitPos(), m_posExit2);
    QVERIFY(!m_tile_exit1_ptr->isHouseExit());
    QVERIFY(m_tile_exit2_ptr->isHouseExit());

    cmd.undo();
    QCOMPARE(m_house->getExitPos(), m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
    QVERIFY(!m_tile_exit2_ptr->isHouseExit());
}

void TestSetHouseExitCommand::testClearExit_RedoUndo() {
    m_house->setExit(m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
    m_mockController->resetNotifications();

    RMESetExitCmd cmd(m_house.get(), m_posInvalid /*new invalid pos*/, m_mockController.get());
    cmd.redo();

    QVERIFY(!m_house->getExitPos().isValid());
    QVERIFY(!m_tile_exit1_ptr->isHouseExit());

    cmd.undo();
    QCOMPARE(m_house->getExitPos(), m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
}

void TestSetHouseExitCommand::testSetExit_SamePosition_NoChange() {
    m_house->setExit(m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
    m_mockController->resetNotifications();

    RMESetExitCmd cmd(m_house.get(), m_posExit1, m_mockController.get());
    cmd.redo();

    QCOMPARE(m_house->getExitPos(), m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());

    if(m_posExit1.isValid()) {
        QVERIFY(m_mockController->m_tileChangedNotified);
        QCOMPARE(m_mockController->m_notifiedPosition, m_posExit1);
    } else {
        QVERIFY(!m_mockController->m_tileChangedNotified);
    }


    m_mockController->resetNotifications();
    cmd.undo();
    QCOMPARE(m_house->getExitPos(), m_posExit1);
    QVERIFY(m_tile_exit1_ptr->isHouseExit());
    if(m_posExit1.isValid()) {
        QVERIFY(m_mockController->m_tileChangedNotified);
        QCOMPARE(m_mockController->m_notifiedPosition, m_posExit1);
    } else {
        QVERIFY(!m_mockController->m_tileChangedNotified);
    }
}

void TestSetHouseExitCommand::testNotifications_OldAndNewExits() {
    m_house->setExit(m_posExit1);
    m_mockController->resetNotifications();

    RMESetExitCmd cmd(m_house.get(), m_posExit2, m_mockController.get());
    cmd.redo();

    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_posExit2);

    m_mockController->resetNotifications();
    cmd.undo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_posExit1);
}

void TestSetHouseExitCommand::testNotifications_OnlyNewExit() {
    m_mockController->resetNotifications();
    RMESetExitCmd cmd(m_house.get(), m_posExit1, m_mockController.get());
    cmd.redo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_posExit1);
}

void TestSetHouseExitCommand::testNotifications_OnlyOldExit_Clear() {
    m_house->setExit(m_posExit1);
    m_mockController->resetNotifications();
    RMESetExitCmd cmd(m_house.get(), m_posInvalid, m_mockController.get());
    cmd.redo();
    QVERIFY(m_mockController->m_tileChangedNotified);
    QCOMPARE(m_mockController->m_notifiedPosition, m_posExit1);
}

QTEST_MAIN(TestSetHouseExitCommand)
#include "TestSetHouseExitCommand.moc"
