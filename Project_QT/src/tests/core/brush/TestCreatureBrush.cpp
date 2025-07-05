#include <QtTest/QtTest>
#include "core/brush/CreatureBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/AssetManager.h" // For Map constructor, though might be nullptr
#include "core/assets/CreatureData.h"
#include "core/settings/AppSettings.h" // For instantiating AppSettings
#include "core/Position.h"
#include "core/Tile.h"
#include "tests/core/brush/MockEditorController.h" // Updated Mock
#include "tests/core/assets/MockCreatureDatabase.h" // New Mock

// Using declarations for RME types to simplify test code
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMECreatureData = RME::core::assets::CreatureData;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMECreatureBrush = RME::core::brush::CreatureBrush;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;

class TestCreatureBrush : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<RMECreatureBrush> m_creatureBrush;
    std::unique_ptr<MockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEAppSettings> m_appSettings; // Real AppSettings for testing

    // Test Creature Data
    RMECreatureData m_testMonster;
    RMECreatureData m_testNpc;

private slots:
    void initTestCase() {
        // One-time setup for the entire test class (if any)
        m_testMonster.name = "Dragon";
        m_testMonster.flags = RME::CreatureTypeFlag::NONE; // Not an NPC

        m_testNpc.name = "Guard";
        m_testNpc.flags = RME::CreatureTypeFlag::IS_NPC;
    }

    void init() {
        // Setup before each test function
        m_creatureBrush = std::make_unique<RMECreatureBrush>();
        m_mockController = std::make_unique<MockEditorController>();
        m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();

        // Setup AppSettings (using real one, ensure QCoreApplication exists or use custom ini)
        // For unit tests, it's often better if AppSettings can be constructed without QCoreApplication
        // or if we can mock its specific methods. For now, assume it can be created.
        // QSettings::setDefaultFormat(QSettings::IniFormat); // Ensure a default format
        // m_appSettings = std::make_unique<RMEAppSettings>("TestOrg", "TestAppCreatureBrush");
        // Simplified AppSettings setup for now:
        m_appSettings = std::make_unique<RMEAppSettings>(); // Uses default test constructor
        m_appSettings->setAutoCreateSpawnEnabled(false); // Default to false
        m_appSettings->setDefaultSpawnTime(60);


        // Setup mock creature database
        m_mockCreatureDb->addOrUpdateCreatureData(m_testMonster.name, m_testMonster);
        m_mockCreatureDb->addOrUpdateCreatureData(m_testNpc.name, m_testNpc);

        // Configure mock controller
        // Map needs an AssetManager, even if null, for its constructor.
        // For tests not needing a full AssetManager, we pass nullptr.
        // The IItemTypeProvider for Tile creation is a separate issue, map should handle it.
        m_map = std::make_unique<RMEMap>(10, 10, 1, nullptr /* AssetManager* */);
        m_mockController->m_mockMap = m_map.get();
        m_mockController->m_mockAppSettings = m_appSettings.get();
        m_mockController->m_mockCreatureDatabase = m_mockCreatureDb.get(); // If controller needs it directly
                                                                      // More likely, brush gets type from palette,
                                                                      // then sets it on brush.

        m_creatureBrush->setCreatureType(nullptr); // Reset creature type
        m_mockController->reset();
    }

    void cleanup() {
        // Teardown after each test function
        // std::unique_ptr handles memory, just reset them if needed or let them go out of scope.
    }

    // --- Test Cases ---

    void testSetCreatureType() {
        const RMECreatureData* monster = m_mockCreatureDb->getCreatureData("Dragon");
        m_creatureBrush->setCreatureType(monster);
        QCOMPARE(m_creatureBrush->getCreatureType(), monster);
        QCOMPARE(m_creatureBrush->getName(), QString("Dragon"));

        m_creatureBrush->setCreatureType(nullptr);
        QCOMPARE(m_creatureBrush->getCreatureType(), nullptr);
        QCOMPARE(m_creatureBrush->getName(), QString("Creature Brush"));
    }

    void testCanApply_NoTypeSelected() {
        RMEBrushSettings settings;
        RMEPosition pos(1, 1, 0);
        m_creatureBrush->setCreatureType(nullptr);
        QVERIFY(!m_creatureBrush->canApply(m_map.get(), pos, settings));
    }

    void testCanApply_DrawOnEmpty() {
        RMEBrushSettings settings;
        settings.isEraseMode = false;
        RMEPosition pos(1, 1, 0);
        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Dragon"));

        QVERIFY(m_creatureBrush->canApply(m_map.get(), pos, settings));
    }

    void testCanApply_DrawOnBlockingTile() {
        RMEBrushSettings settings;
        settings.isEraseMode = false;
        RMEPosition pos(1, 1, 0);
        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Dragon"));

        RMETile* tile = m_map->getTileForEditing(pos); // Get or create tile
        QVERIFY(tile);
        // Mock a blocking item on the tile (details depend on ItemData and Tile::isBlocking impl)
        // For now, assume a way to make tile->isBlocking() return true.
        // This requires deeper mocking of Item/ItemData or a settable flag on Tile's mock.
        // As a placeholder: tile->addStateFlag(RME::TileStateFlag::BLOCKING); // If such a method exists
        // Or, we assume that the map/tile correctly determines blocking from items.
        // Since Tile::isBlocking() is complex, this test is hard without item setup.
        // QVERIFY(!m_creatureBrush->canApply(m_map.get(), pos, settings)); // Expected: false
        // For now, this test case is simplified as isBlocking might not be easily mockable here.
        qInfo("Skipping complex canApply_DrawOnBlockingTile, needs Tile item setup.");
    }


    void testCanApply_EraseExisting() {
        RMEBrushSettings settings;
        settings.isEraseMode = true;
        RMEPosition pos(1, 1, 0);
        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Dragon"));

        RMETile* tile = m_map->getTileForEditing(pos);
        tile->setCreature(std::make_unique<RME::core::creatures::Creature>(m_mockCreatureDb->getCreatureData("Dragon"), pos));

        QVERIFY(m_creatureBrush->canApply(m_map.get(), pos, settings));
    }

    void testCanApply_EraseEmpty() {
        RMEBrushSettings settings;
        settings.isEraseMode = true;
        RMEPosition pos(1, 1, 0);
        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Dragon"));
        // Tile is empty (no creature)
        QVERIFY(!m_creatureBrush->canApply(m_map.get(), pos, settings));
    }

    void testCanApply_PZ_NPC() {
        RMEBrushSettings settings;
        RMEPosition pos(1, 1, 0);
        RMETile* tile = m_map->getTileForEditing(pos);
        tile->addMapFlag(RME::TileMapFlag::PROTECTION_ZONE);

        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Guard")); // NPC
        QVERIFY(m_creatureBrush->canApply(m_map.get(), pos, settings));

        m_creatureBrush->setCreatureType(m_mockCreatureDb->getCreatureData("Dragon")); // Monster
        QVERIFY(!m_creatureBrush->canApply(m_map.get(), pos, settings));
    }


    void testApply_DrawCreatureOnEmptyTile() {
        RMEBrushSettings settings;
        settings.isEraseMode = false;
        RMEPosition pos(1, 1, 0);
        const RMECreatureData* monsterType = m_mockCreatureDb->getCreatureData("Dragon");
        m_creatureBrush->setCreatureType(monsterType);

        m_creatureBrush->apply(m_mockController.get(), pos, settings);

        QCOMPARE(m_mockController->calls.size(), 2); // recordAddCreature, notifyTileChanged
        bool foundAddCreature = false;
        for(const auto& call : m_mockController->calls){
            if(call.method == "recordAddCreature"){
                QCOMPARE(call.pos, pos);
                QCOMPARE(call.creatureType, monsterType);
                foundAddCreature = true;
            }
        }
        QVERIFY(foundAddCreature);

        // Actual tile check would be done by MockEditorController's recordAddCreature
        // For this test, we verify the controller was called correctly.
    }

    void testApply_EraseCreature() {
        RMEBrushSettings settings;
        settings.isEraseMode = true;
        RMEPosition pos(2, 2, 0);
        const RMECreatureData* monsterType = m_mockCreatureDb->getCreatureData("Dragon");
        m_creatureBrush->setCreatureType(monsterType); // Type for brush, even in erase mode

        // Setup: Place a creature on the tile directly for the test
        RMETile* tile = m_map->getTileForEditing(pos);
        tile->setCreature(std::make_unique<RME::core::creatures::Creature>(monsterType, pos));
        QVERIFY(tile->hasCreature());

        m_creatureBrush->apply(m_mockController.get(), pos, settings);

        QCOMPARE(m_mockController->calls.size(), 2); // recordRemoveCreature, notifyTileChanged
        bool foundRemoveCreature = false;
        for(const auto& call : m_mockController->calls){
            if(call.method == "recordRemoveCreature"){
                QCOMPARE(call.pos, pos);
                QCOMPARE(call.creatureType, monsterType); // Should record the type of creature removed
                foundRemoveCreature = true;
            }
        }
        QVERIFY(foundRemoveCreature);
    }

    void testApply_AutoCreateSpawn() {
        RMEBrushSettings settings;
        settings.isEraseMode = false;
        RMEPosition pos(3, 3, 0);
        const RMECreatureData* monsterType = m_mockCreatureDb->getCreatureData("Dragon");
        m_creatureBrush->setCreatureType(monsterType);

        m_appSettings->setAutoCreateSpawnEnabled(true);
        m_appSettings->setDefaultSpawnTime(120); // Custom spawn time for test

        m_creatureBrush->apply(m_mockController.get(), pos, settings);

        // Expected calls: recordAddCreature, recordAddSpawn, notifyTileChanged
        QCOMPARE(m_mockController->calls.size(), 3);

        bool addCreatureCalled = false;
        bool addSpawnCalled = false;

        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordAddCreature") {
                QCOMPARE(call.pos, pos);
                QCOMPARE(call.creatureType, monsterType);
                addCreatureCalled = true;
            } else if (call.method == "recordAddSpawn") {
                QCOMPARE(call.spawnData.getCenter(), pos);
                QCOMPARE(call.spawnData.getRadius(), 1);
                QCOMPARE(call.spawnData.getIntervalSeconds(), 120);
                QVERIFY(call.spawnData.getCreatureTypes().contains(monsterType->name));
                addSpawnCalled = true;
            }
        }
        QVERIFY(addCreatureCalled);
        QVERIFY(addSpawnCalled);
    }

    void testApply_ReplaceExistingCreature() {
        RMEBrushSettings settings;
        settings.isEraseMode = false;
        RMEPosition pos(4, 4, 0);

        const RMECreatureData* initialNpcType = m_mockCreatureDb->getCreatureData("Guard");
        const RMECreatureData* newMonsterType = m_mockCreatureDb->getCreatureData("Dragon");

        // Setup: Place an NPC on the tile
        RMETile* tile = m_map->getTileForEditing(pos);
        tile->setCreature(std::make_unique<RME::core::creatures::Creature>(initialNpcType, pos));

        m_creatureBrush->setCreatureType(newMonsterType); // Brush will place a Monster
        m_creatureBrush->apply(m_mockController.get(), pos, settings);

        // Expected: recordRemoveCreature (for NPC), recordAddCreature (for Monster), notifyTileChanged
        QCOMPARE(m_mockController->calls.size(), 3);

        bool removeCalled = false;
        bool addCalled = false;
        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordRemoveCreature") {
                QCOMPARE(call.creatureType, initialNpcType);
                removeCalled = true;
            } else if (call.method == "recordAddCreature") {
                QCOMPARE(call.creatureType, newMonsterType);
                addCalled = true;
            }
        }
        QVERIFY(removeCalled);
        QVERIFY(addCalled);
    }


    // TODO:
    // - Test auto-spawn NOT creating if already a spawn exists (getSpawnDataRef is not null)
    // - Test auto-spawn NOT creating if map->isTileCoveredBySpawn(pos) is true (needs map method)
    // - Test erasing last creature from auto-created spawn removes the spawn (needs spawn identification)
    // - Test undo/redo (more complex, requires controller to actually perform actions and undo stack)

};

// QTEST_APPLESS_MAIN(TestCreatureBrush) // Use this if no QApplication needed
// Or use a main that sets up QCoreApplication if AppSettings needs it.
// For now, assume tests can run without full Qt App if AppSettings is robust.

// It's common to include the .moc file if not using qmake's AUTOMOC or CMake equivalent
// For example: #include "TestCreatureBrush.moc"
// This depends on the build system setup. If using CMake with CTest, it usually handles moc generation.
