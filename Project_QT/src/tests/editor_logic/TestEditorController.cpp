#include <QtTest/QtTest>
#include <QUndoStack>
#include "editor_logic/EditorController.h"
#include "editor_logic/commands/BrushStrokeCommand.h" // For command ID and RME_COMMANDS namespace
#include "editor_logic/commands/DeleteCommand.h"    // For command ID and RME_COMMANDS namespace
#include "core/map/Map.h"
#include "core/selection/SelectionManager.h"
#include "core/brush/BrushManagerService.h"
#include "core/brush/BrushSettings.h"
#include "core/Tile.h"
#include "core/Item.h" // For creating items in tiles for tests
#include "tests/core/MockItemTypeProvider.h" // For item creation
#include "MockBrush.h" // Our new mock brush
#include "core/waypoints/WaypointManager.h"   // For EditorController constructor
#include "core/houses/HouseData.h"           // For adding houses to map
#include "editor_logic/commands/SetHouseExitCommand.h" // For command ID checks

class TestEditorController : public QObject {
    Q_OBJECT

private:
    RME::Map* m_map = nullptr;
    QUndoStack* m_undoStack = nullptr;
    RME::SelectionManager* m_selectionManager = nullptr;
    RME::BrushManagerService* m_brushManagerService = nullptr;
    RME::core::WaypointManager* m_waypointManager = nullptr; // Added
    RME::EditorController* m_editorController = nullptr;
    RME::MockItemTypeProvider* m_mockItemProvider = nullptr;
    RME::MockBrush* m_mockBrushInstance = nullptr;

private slots:
    void init() {
        m_mockItemProvider = new RME::MockItemTypeProvider();
        m_map = new RME::Map(100, 100, 8, nullptr /* AssetManager* */);
        m_undoStack = new QUndoStack(this);
        m_selectionManager = new RME::SelectionManager(m_map, m_undoStack, this);
        m_brushManagerService = new RME::BrushManagerService(this);
        m_waypointManager = new RME::core::WaypointManager(m_map); // Added

        auto mockBrushOwned = std::make_unique<RME::MockBrush>("TestBrush1");
        m_mockBrushInstance = mockBrushOwned.get();
        m_brushManagerService->addBrush(std::move(mockBrushOwned));
        m_brushManagerService->setActiveBrush("TestBrush1");

        m_editorController = new RME::EditorController(m_map, m_undoStack, m_selectionManager, m_brushManagerService, m_waypointManager, this); // Added m_waypointManager
    }

    void cleanup() {
        delete m_editorController; m_editorController = nullptr;
        m_mockBrushInstance = nullptr;
        delete m_brushManagerService; m_brushManagerService = nullptr;
        delete m_selectionManager; m_selectionManager = nullptr;
        delete m_waypointManager; m_waypointManager = nullptr; // Added
        delete m_undoStack; m_undoStack = nullptr;
        delete m_map; m_map = nullptr;
        delete m_mockItemProvider; m_mockItemProvider = nullptr;
    }

    void testApplyBrushStroke_Draw() {
        RME::Position pos1(10, 10, 7);
        RME::BrushSettings settings;
        settings.setName("TestBrush1"); // Name of the mock brush

        m_editorController->applyBrushStroke({pos1}, settings, false); // isErase = false
        QCOMPARE(m_undoStack->count(), 1);
        QVERIFY(m_mockBrushInstance != nullptr);
        QCOMPARE(m_mockBrushInstance->drawCallCount, 1);
        QCOMPARE(m_mockBrushInstance->lastDrawPositions.first(), pos1);

        RME::Tile* tile = m_map->getTile(pos1);
        QVERIFY(tile != nullptr);
        QVERIFY(tile->hasAttribute("mock_draw_attr"));
        QCOMPARE(tile->getAttribute("mock_draw_attr").toString(), QString("drawn_by_TestBrush1"));

        m_undoStack->undo();
        // Undo for a draw operation should revert the tile state, not call undraw on the brush.
        // The MockBrush::undraw would clear the attribute. Here we check if state is restored.
        QCOMPARE(m_mockBrushInstance->undrawCallCount, 0);

        tile = m_map->getTile(pos1);
        // Depending on how BrushStrokeCommand::undo handles created tiles:
        // If it was a new tile, it might be removed (become nullptr).
        // If it existed, its attribute should be gone.
        bool tileExistedAndAttributeGone = tile && !tile->hasAttribute("mock_draw_attr");
        bool tileRemoved = (m_map->getTile(pos1) == nullptr); // If getOrCreateTile made it, undo might remove it
        QVERIFY(tileExistedAndAttributeGone || tileRemoved);


        m_undoStack->redo();
        QCOMPARE(m_mockBrushInstance->drawCallCount, 2); // Redo calls draw again
        tile = m_map->getTile(pos1);
        QVERIFY(tile != nullptr);
        QVERIFY(tile->hasAttribute("mock_draw_attr"));
    }

    void testApplyBrushStroke_Erase() {
        RME::Position pos1(15, 15, 7);
        bool createdFlag = false; // Dummy variable for getOrCreateTile
        RME::Tile* tile = m_map->getOrCreateTile(pos1, createdFlag);
        QVERIFY(tile != nullptr);
        tile->setAttribute("mock_draw_attr", "pre_existing");

        m_mockBrushInstance->resetSpy();
        RME::BrushSettings settings;
        settings.setName("TestBrush1");

        m_editorController->applyBrushStroke({pos1}, settings, true); // isErase = true
        QCOMPARE(m_undoStack->count(), 1);
        QVERIFY(m_mockBrushInstance != nullptr);
        QCOMPARE(m_mockBrushInstance->undrawCallCount, 1); // Erase should call undraw
        QCOMPARE(m_mockBrushInstance->lastUndrawPositions.first(), pos1);

        tile = m_map->getTile(pos1); // Re-fetch, might be different instance after some ops
        QVERIFY(tile != nullptr);
        QVERIFY(!tile->hasAttribute("mock_draw_attr")); // Attribute should be cleared by mock undraw

        m_undoStack->undo();
        tile = m_map->getTile(pos1);
        QVERIFY(tile != nullptr);
        QVERIFY(tile->hasAttribute("mock_draw_attr")); // Attribute restored
        QCOMPARE(tile->getAttribute("mock_draw_attr").toString(), QString("pre_existing"));

        m_undoStack->redo();
        tile = m_map->getTile(pos1);
        QVERIFY(tile != nullptr);
        QVERIFY(!tile->hasAttribute("mock_draw_attr")); // Attribute cleared again by undraw
    }

    void testDeleteSelection() {
        RME::Position pos1(5, 5, 7);
        RME::Position pos2(5, 6, 7);
        bool created; // Dummy for getOrCreateTile
        RME::Tile* tile1 = m_map->getOrCreateTile(pos1, created);
        QVERIFY(tile1 != nullptr);
        // Add an item to make it non-empty for the check after undo
        tile1->addItem(RME::Item::create(1234, m_mockItemProvider));

        RME::Tile* tile2 = m_map->getOrCreateTile(pos2, created);
        QVERIFY(tile2 != nullptr);

        m_selectionManager->startSelectionChange();
        m_selectionManager->addSelectedTile(tile1); // Assuming addSelectedTile exists
        m_selectionManager->addSelectedTile(tile2);
        m_selectionManager->finishSelectionChange();

        QCOMPARE(m_selectionManager->getSelectedTiles().count(), 2);

        m_editorController->deleteSelection();
        QCOMPARE(m_undoStack->count(), 1);
        // After delete, tiles are set to nullptr in the map by DeleteCommand::redo()
        QVERIFY(m_map->getTile(pos1) == nullptr);
        QVERIFY(m_map->getTile(pos2) == nullptr);
        QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());

        m_undoStack->undo();
        RME::Tile* restoredTile1 = m_map->getTile(pos1);
        QVERIFY(restoredTile1 != nullptr);
        QVERIFY(!restoredTile1->getAllItems().isEmpty()); // Check if item is back

        RME::Tile* restoredTile2 = m_map->getTile(pos2); // Tile2 was empty
        QVERIFY(restoredTile2 != nullptr);

        // QVERIFY(m_selectionManager->getSelectedTiles().count() == 2); // Optional: test re-selection

        m_undoStack->redo();
        QVERIFY(m_map->getTile(pos1) == nullptr);
        QVERIFY(m_map->getTile(pos2) == nullptr);
        QVERIFY(m_selectionManager->getSelectedTiles().isEmpty());
    }

    // --- House Exit Tests ---
    void testSetHouseExit_NewExit() {
        uint32_t houseId = 1;
        // Map::addHouse takes HouseData by rvalue reference.
        // Create it, then move it.
        RME::HouseData newHouse(houseId, "TestHouse1");
        m_map->addHouse(std::move(newHouse));

        RME::Position newExitPos(5,5,7);
        bool created; // dummy
        m_map->getOrCreateTile(newExitPos, created)->addItem(RME::Item::create(1, m_mockItemProvider)); // Add ground

        m_editorController->setHouseExit(houseId, newExitPos);
        QCOMPARE(m_undoStack->count(), 1);
        const QUndoCommand* command = m_undoStack->command(0);
        QCOMPARE(command->id(), RME_COMMANDS::SetHouseExitCommandId);

        RME::HouseData* house = m_map->getHouse(houseId);
        QVERIFY(house);
        QCOMPARE(house->getEntryPoint(), newExitPos);
        RME::Tile* exitTile = m_map->getTile(newExitPos);
        QVERIFY(exitTile && exitTile->isHouseExit());

        m_undoStack->undo();
        QVERIFY(house->getEntryPoint() != newExitPos);
        QVERIFY(exitTile && !exitTile->isHouseExit());

        m_undoStack->redo();
        QCOMPARE(house->getEntryPoint(), newExitPos);
        QVERIFY(exitTile && exitTile->isHouseExit());
    }

    void testSetHouseExit_ChangeExit() {
        uint32_t houseId = 2;
        RME::Position oldExitPos(6,6,7);
        RME::Position newExitPos(7,7,7);
        bool created; // dummy
        m_map->getOrCreateTile(oldExitPos, created)->addItem(RME::Item::create(1, m_mockItemProvider));
        m_map->getOrCreateTile(newExitPos, created)->addItem(RME::Item::create(1, m_mockItemProvider));

        RME::HouseData houseSetup(houseId, "TestHouse2");
        // Set initial exit directly on HouseData *before* adding to map,
        // because addHouse moves it and we can't modify houseSetup after that.
        // Also, houseSetup.setEntryPoint needs the map context.
        m_map->addHouse(std::move(houseSetup));
        RME::HouseData* house = m_map->getHouse(houseId);
        QVERIFY(house);
        house->setEntryPoint(oldExitPos, m_map); // Set initial exit after adding to map

        m_editorController->setHouseExit(houseId, newExitPos);
        QCOMPARE(m_undoStack->count(), 1);
        QCOMPARE(house->getEntryPoint(), newExitPos);
        RME::Tile* oldTile = m_map->getTile(oldExitPos);
        QVERIFY(oldTile && !oldTile->isHouseExit());
        RME::Tile* newTile = m_map->getTile(newExitPos);
        QVERIFY(newTile && newTile->isHouseExit());

        m_undoStack->undo();
        QCOMPARE(house->getEntryPoint(), oldExitPos);
        QVERIFY(oldTile && oldTile->isHouseExit());
        QVERIFY(newTile && !newTile->isHouseExit());
    }

    void testSetHouseExit_ClearExit() {
        uint32_t houseId = 3;
        RME::Position oldExitPos(8,8,7);
        RME::Position invalidPos; // Default, to clear
        bool created; // dummy
        m_map->getOrCreateTile(oldExitPos, created)->addItem(RME::Item::create(1, m_mockItemProvider));

        RME::HouseData houseSetup(houseId, "TestHouse3");
        m_map->addHouse(std::move(houseSetup));
        RME::HouseData* house = m_map->getHouse(houseId);
        QVERIFY(house);
        house->setEntryPoint(oldExitPos, m_map);

        m_editorController->setHouseExit(houseId, invalidPos);
        QCOMPARE(m_undoStack->count(), 1);
        QCOMPARE(house->getEntryPoint(), invalidPos);
        RME::Tile* oldTile = m_map->getTile(oldExitPos);
        QVERIFY(oldTile && !oldTile->isHouseExit());

        m_undoStack->undo();
        QCOMPARE(house->getEntryPoint(), oldExitPos);
        QVERIFY(oldTile && oldTile->isHouseExit());
    }

    void testSetHouseExit_InvalidLocation() {
        uint32_t houseId = 4;
        RME::HouseData newHouse(houseId, "TestHouse4");
        m_map->addHouse(std::move(newHouse));
        RME::Position invalidTarget(9,9,7);
        bool created; // dummy
        m_map->getOrCreateTile(invalidTarget, created); // No ground added, so invalid exit location

        m_editorController->setHouseExit(houseId, invalidTarget);
        QCOMPARE(m_undoStack->count(), 0);
    }

    void testSetHouseExit_NoChange() {
        uint32_t houseId = 5;
        RME::Position currentExit(10,10,7);
        bool created; // dummy
        m_map->getOrCreateTile(currentExit, created)->addItem(RME::Item::create(1, m_mockItemProvider));

        RME::HouseData houseSetup(houseId, "TestHouse5");
        m_map->addHouse(std::move(houseSetup));
        RME::HouseData* house = m_map->getHouse(houseId);
        QVERIFY(house);
        house->setEntryPoint(currentExit, m_map);

        m_editorController->setHouseExit(houseId, currentExit); // Target is same as current
        QCOMPARE(m_undoStack->count(), 0);
    }
};
// QTEST_APPLESS_MAIN(TestEditorController)
// #include "TestEditorController.moc"
