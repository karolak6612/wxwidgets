#include <QtTest/QtTest>
#include <memory>
#include <QList>
#include <QMap>

// Class to be tested
#include "editor_logic/EditorController.h"

// Core classes used by EditorController and tests
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/MaterialManager.h"
#include "core/selection/SelectionManager.h"
#include "core/brush/BrushManager.h"
#include "core/brush/Brush.h"
#include "core/settings/BrushSettings.h"
#include "core/settings/AppSettings.h"

// Commands used
#include "editor_logic/commands/DeleteSelectionCommand.h"
#include "editor_logic/commands/ClearSelectionCommand.h"
#include "editor_logic/commands/BoundingBoxSelectCommand.h"
#include "editor_logic/commands/SetHouseExitCommand.h" // Added
#include "core/map_constants.h"
#include "core/houses/Houses.h" // Added
#include "core/houses/House.h"  // Added

// Qt classes
#include <QUndoStack>
#include <QUndoCommand>


// --- Minimal Mock Implementations ---

class MockMap : public RME::core::Map {
public:
    MockMap(RME::core::assets::AssetManager* assetMgr) : RME::core::Map(10, 10, 1, assetMgr) {
        // Ensure ItemDatabase is set for Item::create if map/tile ops use it.
        // This is typically handled by test setup (Item::setItemDatabase).
    }

    QMap<RME::core::Position, std::unique_ptr<RME::core::Tile>> m_tilesInternal;
    bool tileChangedNotified = false;
    RME::core::Position lastNotifiedPos;


    RME::core::Tile* getTileForEditing(const RME::core::Position& pos) override {
        if (!m_tilesInternal.contains(pos)) {
            // Ensure ItemDatabase is available if Tile constructor or subsequent ops create items
            m_tilesInternal.insert(pos, std::make_unique<RME::core::Tile>(pos));
        }
        return m_tilesInternal[pos].get();
    }
    RME::core::Tile* getTile(const RME::core::Position& pos) const override {
        // Test specific: return from a predefined set of tiles if pos matches.
        // This requires TestEditorController to own some tiles and MockMap to have access.
        // For now, this mock will return tiles from m_tilesInternal if they exist via getTileForEditing.
        // If a test needs specific tiles at specific locations without calling getOrCreateTile first,
        // m_tilesInternal needs to be populated by the test setup.
        if (m_testTilesOwner && m_testTilesOwner->contains(pos)) {
            return m_testTilesOwner->value(pos);
        }
        return m_tilesInternal.value(pos).get(); // Fallback to existing logic
    }
    RME::core::Tile* getOrCreateTile(const RME::core::Position& pos) override {
        if (m_testTilesOwner && m_testTilesOwner->contains(pos)) {
            // If test framework provides a specific tile for this position, use it.
            // This part of the logic might be complex depending on how TestEditorController manages its test tiles.
            // For simplicity, getTileForEditing handles creation if not found.
        }
        return getTileForEditing(pos);
    }
    bool isPositionValid(const RME::core::Position& pos) const override {
        // Allow a slightly larger range for bounding box tests that might go slightly out of 10x10
        return pos.x >= 0 && pos.x < 20 && pos.y >= 0 && pos.y < 20 && pos.z >=0 && pos.z < RME::core::MAP_MAX_Z_VALUE + 1;
    }
    void notifyTileChanged(const RME::core::Position& pos) override {
        tileChangedNotified = true;
        lastNotifiedPos = pos;
    }
    void resetNotifications() {
        tileChangedNotified = false;
        lastNotifiedPos = RME::core::Position();
    }
};

class MockUndoStack : public QUndoStack {
    Q_OBJECT
public:
    MockUndoStack(QObject* parent = nullptr) : QUndoStack(parent) {}
    ~MockUndoStack() override {
        // QUndoStack handles deletion of commands pushed as raw pointers.
        // If lastPushedCommand was std::unique_ptr and released, QUndoStack owns it.
    }

    void push(QUndoCommand *cmd) override {
        pushCalled = true;
        // Store the raw pointer. QUndoStack takes ownership.
        // Be careful if trying to delete this pointer elsewhere.
        lastPushedCommandRaw = cmd;
        QUndoStack::push(cmd);
    }
    void beginMacro(const QString& text) override {
        beginMacroCalled = true;
        macroText = text;
        QUndoStack::beginMacro(text);
    }
    void endMacro() override {
        endMacroCalled = true;
        QUndoStack::endMacro();
    }
    void resetMockState() {
        pushCalled = false;
        beginMacroCalled = false;
        endMacroCalled = false;
        macroText.clear();
        lastPushedCommandRaw = nullptr; // Don't delete, QUndoStack owns or it was never pushed.
                                        // If clear is called, QUndoStack handles it.
        // Clear the actual stack if needed for stricter tests
        // while (canUndo()) { QUndoStack::undo(); } // This might be too much / noisy
        // QUndoStack::clear(); // This deletes commands
    }

    bool pushCalled = false;
    bool beginMacroCalled = false;
    bool endMacroCalled = false;
    QString macroText;
    QUndoCommand* lastPushedCommandRaw = nullptr;
};

class MockBrush : public RME::core::brush::Brush {
public:
    void apply(RME::core::editor::EditorControllerInterface* /*controller*/,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& /*settings*/) override {
        applyCalled = true;
        applyCallCount++;
        lastApplyPos = pos;
    }
    QString getName() const override { return m_name; }
    int getLookID(const RME::core::BrushSettings& /*settings*/) const override { return 0; }
    bool canApply(const RME::core::map::Map* /*map*/, const RME::core::Position& /*pos*/, const RME::core::BrushSettings& /*settings*/) const override { return m_canApply; }
    bool canBeErasingTool() const override { return m_isEraser; }

    void resetMockState() {
        applyCalled = false;
        applyCallCount = 0;
        lastApplyPos = RME::core::Position();
    }

    bool applyCalled = false;
    int applyCallCount = 0;
    RME::core::Position lastApplyPos;
    bool m_isEraser = false;
    bool m_canApply = true;
    QString m_name = "MockBrush";
};

class MockBrushManager : public RME::core::brush::BrushManager {
public:
    MockBrushManager() : m_activeBrush(std::make_unique<MockBrush>()) {}
    RME::core::brush::Brush* getActiveBrush() override {
        getActiveBrushCalled = true;
        return m_activeBrush.get();
    }
    MockBrush* getMockActiveBrush() {
        if (m_activeBrush) return m_activeBrush.get();
        return nullptr;
    }

    void resetMockState() {
        getActiveBrushCalled = false;
        if (m_activeBrush) m_activeBrush->resetMockState();
    }

    bool getActiveBrushCalled = false;
    std::unique_ptr<MockBrush> m_activeBrush;
};

class MockSelectionManager : public RME::core::selection::SelectionManager {
public:
    MockSelectionManager(RME::core::Map* map) : RME::core::selection::SelectionManager(map, nullptr) {} // Pass nullptr for QUndoStack

    QList<RME::core::Tile*> m_currentSelectedTiles_mock_list;

    bool isEmpty() const override { return m_currentSelectedTiles_mock_list.isEmpty(); }

    void MOCK_setSelectedTiles(const QList<RME::core::Tile*>& tiles) {
        m_currentSelectedTiles_mock_list = tiles;
    }

    void MOCK_addTileToSelection(RME::core::Tile* tile) {
        if (tile && !m_currentSelectedTiles_mock_list.contains(tile)) {
            m_currentSelectedTiles_mock_list.append(tile);
        }
    }

    void clearSelectionInternal() override {
        bool changed = !m_currentSelectedTiles_mock_list.isEmpty();
        m_currentSelectedTiles_mock_list.clear();
        if (changed) { /*emit selectionChanged();*/ }
    }
    void addTilesToSelectionInternal(const QList<RME::core::Tile*>& tilesToAdd) override {
        bool changed = false;
        for (RME::core::Tile* tile : tilesToAdd) {
            if (tile && !m_currentSelectedTiles_mock_list.contains(tile)) {
                m_currentSelectedTiles_mock_list.append(tile);
                changed = true;
            }
        }
        if (changed) { /*emit selectionChanged();*/ }
    }
    void removeTilesFromSelectionInternal(const QList<RME::core::Tile*>& tilesToDeselect) override {
        bool changed = false;
        for (RME::core::Tile* tile : tilesToDeselect) {
            if (m_currentSelectedTiles_mock_list.removeAll(tile) > 0) {
                changed = true;
            }
        }
        if (changed) { /*emit selectionChanged();*/ }
    }
    void setSelectedTilesInternal(const QList<RME::core::Tile*>& tilesToSelect) override {
        bool changed = (QSet<RME::core::Tile*>::fromList(m_currentSelectedTiles_mock_list) != QSet<RME::core::Tile*>::fromList(tilesToSelect));
        m_currentSelectedTiles_mock_list = tilesToSelect;
        if (changed) { /*emit selectionChanged();*/ }
    }
    QList<RME::core::Tile*> getCurrentSelectedTilesList() const override {
        return m_currentSelectedTiles_mock_list;
    }

    void resetMockState() {
        m_currentSelectedTiles_mock_list.clear();
    }
};

class MockAppSettings : public RME::core::settings::AppSettings {
public:
    bool getBool(const QString& key, bool defaultValue) const override {
        if (m_boolSettings.contains(key)) return m_boolSettings.value(key);
        return defaultValue;
    }
    QString getString(const QString& key, const QString& defaultValue) const override {
        if (m_stringSettings.contains(key)) return m_stringSettings.value(key);
        return defaultValue;
    }

    // For test setup
    void setBoolValue(const QString& key, bool value) { m_boolSettings[key] = value; }
    void setStringValue(const QString& key, const QString& value) { m_stringSettings[key] = value; }

    void resetMockState() {
        m_boolSettings.clear();
        m_stringSettings.clear();
    }
private:
    QMap<QString, bool> m_boolSettings;
    QMap<QString, QString> m_stringSettings;
};

// Make MockMap aware of TestEditorController's tiles for getTile
QMap<RME::core::Position, RME::core::Tile*>* MockMap::m_testTilesOwner = nullptr;


class TestEditorController : public QObject {
    Q_OBJECT

public:
    TestEditorController() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testApplyBrushStroke_SingleTile();
    void testApplyBrushStroke_MultiTile_Macro();
    void testApplyBrushStroke_NoActiveBrush();
    void testApplyBrushStroke_EraseModeMacroText();

    // Old testDeleteSelection_NotEmpty and _Empty are removed / replaced by testHandleDeleteSelection_*

    // New tests
    void testHandleDeleteSelection_WithSelection();  // Added
    void testHandleDeleteSelection_WithoutSelection(); // Added
    void testClearSelection_NotEmpty();
    void testClearSelection_Empty();
    void testPerformBoundingBoxSelection_NonAdditive_CurrentFloor();
    void testPerformBoundingBoxSelection_Additive_CurrentFloor();
    // void testPerformBoundingBoxSelection_AllFloors(); // Optional, more complex map setup

    // Tests for setHouseExit
    void testSetHouseExit_ValidHouseAndPosition();
    void testSetHouseExit_InvalidHouseId();
    void testSetHouseExit_SamePosition_NoCommand();

private:
    std::unique_ptr<RME::editor_logic::EditorController> m_editorController;

    std::unique_ptr<MockMap> m_ownedMockMap;
    MockMap* m_mockMapRawPtr;

    // Tiles owned by the test fixture for MockMap to serve
    QMap<RME::core::Position, RME::core::Tile> m_fixtureTiles;
    // Helper to populate m_fixtureTiles and set MockMap::m_testTilesOwner
    void setupFixtureTiles();


    std::unique_ptr<MockUndoStack> m_mockUndoStack;
    std::unique_ptr<MockBrushManager> m_mockBrushManager;
    std::unique_ptr<MockSelectionManager> m_mockSelectionManager;
    std::unique_ptr<MockAppSettings> m_mockAppSettings;
    std::unique_ptr<RME::core::houses::Houses> m_realHousesManager; // Added

    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
};

void TestEditorController::initTestCase() {
    RME::core::Item::setItemDatabase(nullptr);
}

void TestEditorController::cleanupTestCase() {
    RME::core::Item::setItemDatabase(nullptr);
}

void TestEditorController::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    RME::core::Item::setItemDatabase(m_itemDatabase.get()); // Set static pointer for Item::create

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );

    m_ownedMockMap = std::make_unique<MockMap>(m_assetManager.get());
    m_mockMapRawPtr = m_ownedMockMap.get();
    setupFixtureTiles(); // Initialize fixture tiles and inform MockMap

    m_mockUndoStack = std::make_unique<MockUndoStack>();
    m_mockBrushManager = std::make_unique<MockBrushManager>();
    m_mockSelectionManager = std::make_unique<MockSelectionManager>(m_mockMapRawPtr);
    m_mockAppSettings = std::make_unique<MockAppSettings>();

    // EditorController constructor was updated to include HousesManager
    m_mockSelectionManager = std::make_unique<MockSelectionManager>(m_mockMapRawPtr);
    m_mockAppSettings = std::make_unique<MockAppSettings>();
    m_realHousesManager = std::make_unique<RME::core::houses::Houses>(m_mockMapRawPtr); // Initialize HousesManager

    m_editorController = std::make_unique<RME::editor_logic::EditorController>(
        m_mockMapRawPtr,
        m_mockUndoStack.get(),
        m_mockSelectionManager.get(),
        m_mockBrushManager.get(),
        m_mockAppSettings.get(),
        m_assetManager.get(),
        m_realHousesManager.get() // Pass real HousesManager
    );

    // Reset all mocks
    m_mockUndoStack->resetMockState();
    m_mockSelectionManager->resetMockState(); // Added
    m_mockAppSettings->resetMockState();
    if (m_mockBrushManager->getMockActiveBrush()) {
         m_mockBrushManager->getMockActiveBrush()->resetMockState();
    }
    m_mockBrushManager->resetMockState();
    m_mockMapRawPtr->resetNotifications();
}

void TestEditorController::cleanup() {
    m_editorController.reset();
    m_mockAppSettings.reset();
    m_mockSelectionManager.reset();
    m_mockBrushManager.reset();
    m_mockUndoStack.reset();
    m_ownedMockMap.reset();
    m_mockMapRawPtr = nullptr;
    MockMap::m_testTilesOwner = nullptr;
    m_fixtureTiles.clear();
    m_realHousesManager.reset(); // Added cleanup

    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    RME::core::Item::setItemDatabase(nullptr);
}

void TestEditorController::testApplyBrushStroke_SingleTile() {
    RME::core::Position pos(1,1,0);
    QList<RME::core::Position> positions = {pos};
    RME::core::BrushSettings settings;

    m_mockBrushManager->getMockActiveBrush()->resetMockState();
    m_mockUndoStack->resetMockState();

    m_editorController->applyBrushStroke(positions, settings);

    QVERIFY(m_mockBrushManager->getActiveBrushCalled);
    QVERIFY(m_mockBrushManager->getMockActiveBrush()->applyCalled);
    QCOMPARE(m_mockBrushManager->getMockActiveBrush()->lastApplyPos, pos);
    QVERIFY(m_mockUndoStack->beginMacroCalled);
    QVERIFY(m_mockUndoStack->endMacroCalled);
    QVERIFY(m_mockUndoStack->macroText.contains("MockBrush Stroke"));
}

void TestEditorController::testApplyBrushStroke_MultiTile_Macro() {
    QList<RME::core::Position> positions = {RME::core::Position(1,1,0), RME::core::Position(1,2,0)};
    RME::core::BrushSettings settings;
    m_mockBrushManager->getMockActiveBrush()->resetMockState();
    m_mockUndoStack->resetMockState();

    m_editorController->applyBrushStroke(positions, settings);

    QVERIFY(m_mockUndoStack->beginMacroCalled);
    QVERIFY(m_mockUndoStack->endMacroCalled);
    QCOMPARE(m_mockBrushManager->getMockActiveBrush()->applyCallCount, 2);
    QCOMPARE(m_mockBrushManager->getMockActiveBrush()->lastApplyPos, RME::core::Position(1,2,0));
}

void TestEditorController::testApplyBrushStroke_NoActiveBrush() {
    m_mockBrushManager->m_activeBrush.reset();
    QList<RME::core::Position> positions = {RME::core::Position(1,1,0)};
    RME::core::BrushSettings settings;
    m_mockUndoStack->resetMockState();

    m_editorController->applyBrushStroke(positions, settings);

    QVERIFY(m_mockBrushManager->getActiveBrushCalled); // Still called to check for brush
    QVERIFY(!m_mockUndoStack->beginMacroCalled);
    QVERIFY(!m_mockUndoStack->pushCalled);
}

void TestEditorController::testApplyBrushStroke_EraseModeMacroText() {
    RME::core::Position pos(1,1,0);
    QList<RME::core::Position> positions = {pos};
    RME::core::BrushSettings settings;
    settings.isEraseMode = true;
    QVERIFY(m_mockBrushManager->getMockActiveBrush()); // Ensure brush exists
    m_mockBrushManager->getMockActiveBrush()->m_isEraser = true;
    m_mockUndoStack->resetMockState();

    m_editorController->applyBrushStroke(positions, settings);
    QVERIFY(m_mockUndoStack->beginMacroCalled); // Macro should still begin
    QVERIFY(m_mockUndoStack->macroText.contains("Erase Stroke (MockBrush)"));
}


void TestEditorController::testDeleteSelection_NotEmpty() {
    m_mockSelectionManager->addSelectedPosition(RME::core::Position(1,1,0));
    m_mockUndoStack->resetMockState();

    m_editorController->deleteSelection();

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::DeleteSelectionCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);
    // The actual text/behavior of DeleteSelectionCommand is tested in its own test suite.
    // Here, we just verify that EditorController correctly creates and pushes it.
}

void TestEditorController::testDeleteSelection_Empty() {
    m_mockSelectionManager->clearSelection();
    m_mockUndoStack->resetMockState();

    m_editorController->deleteSelection();

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::DeleteSelectionCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    // DeleteSelectionCommand's redo() is expected to handle the case of empty selection.
}

// Test methods for handleDeleteSelection (replaces old testDeleteSelection_*)
void TestEditorController::testHandleDeleteSelection_WithSelection() {
    // Setup: MockSelectionManager has a selection
    RME::core::Tile testTile(RME::core::Position(1,1,7)); // Dummy tile for selection list
    m_mockSelectionManager->MOCK_setSelectedTiles({&testTile});
    QVERIFY(!m_mockSelectionManager->isEmpty());

    m_mockUndoStack->resetMockState();
    m_editorController->handleDeleteSelection();

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::DeleteCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);
    // Further checks on cmd's internal state (like captured positions) could be done,
    // but that's more for TestDeleteCommand. Here we check that EC pushed the right type of command.
}

void TestEditorController::testHandleDeleteSelection_WithoutSelection() {
    m_mockSelectionManager->MOCK_setSelectedTiles({}); // Ensure selection is empty
    QVERIFY(m_mockSelectionManager->isEmpty());

    m_mockUndoStack->resetMockState();
    m_editorController->handleDeleteSelection();

    QVERIFY(!m_mockUndoStack->pushCalled); // No command should be pushed if selection is empty
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw == nullptr);
}

void TestEditorController::setupFixtureTiles() {
    m_fixtureTiles.clear();
    // Create a few tiles for specific positions
    // Example: 3x3 area on current floor (e.g. Z=7)
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            RME::core::Position pos(x, y, RME::core::GROUND_LAYER); // Assuming GROUND_LAYER = 7
            m_fixtureTiles.insert(pos, RME::core::Tile(pos));
        }
    }
    // Make MockMap aware of these tiles
    // This requires MockMap to have a static or settable pointer to this map.
    // For simplicity, using a static member in MockMap (defined outside class).
    static QMap<RME::core::Position, RME::core::Tile*> tilePtrMap;
    tilePtrMap.clear();
    for(auto it = m_fixtureTiles.begin(); it != m_fixtureTiles.end(); ++it) {
        tilePtrMap.insert(it.key(), &it.value());
    }
    MockMap::m_testTilesOwner = &tilePtrMap;
}


void TestEditorController::testClearSelection_NotEmpty() {
    RME::core::Tile testTile1(RME::core::Position(1,1,7));
    RME::core::Tile testTile2(RME::core::Position(1,2,7));
    QList<RME::core::Tile*> initialSelection = {&testTile1, &testTile2};
    m_mockSelectionManager->MOCK_setSelectedTiles(initialSelection);

    QVERIFY(!m_mockSelectionManager->isEmpty());
    m_mockUndoStack->resetMockState();

    m_editorController->clearCurrentSelection();

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::ClearSelectionCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);

    // Verify the command captured the correct old selection
    QCOMPARE(cmd->getOldSelectedTiles().size(), 2);
    QVERIFY(cmd->getOldSelectedTiles().contains(&testTile1));
    QVERIFY(cmd->getOldSelectedTiles().contains(&testTile2));
}

void TestEditorController::testClearSelection_Empty() {
    m_mockSelectionManager->MOCK_setSelectedTiles({}); // Empty selection
    QVERIFY(m_mockSelectionManager->isEmpty());
    m_mockUndoStack->resetMockState();

    m_editorController->clearCurrentSelection();

    QVERIFY(!m_mockUndoStack->pushCalled); // Should not push a command if selection is already empty
}

void TestEditorController::testPerformBoundingBoxSelection_NonAdditive_CurrentFloor() {
    // Setup fixture tiles (e.g. 0,0,7 to 2,2,7) by calling setupFixtureTiles() in init()
    // Initial selection (e.g., one tile outside the coming box)
    RME::core::Tile initialSelTile(RME::core::Position(5,5,7));
    QList<RME::core::Tile*> initialSelection = {&initialSelTile};
    m_mockSelectionManager->MOCK_setSelectedTiles(initialSelection);

    m_mockAppSettings->setMockStringValue("SELECTION_TYPE", "CurrentFloor");
    m_mockAppSettings->setBoolValue("COMPENSATED_SELECT", false);

    RME::core::BrushSettings currentBrushSettings;
    currentBrushSettings.setActiveZ(RME::core::GROUND_LAYER); // e.g. 7

    RME::core::Position p1(0,0, RME::core::GROUND_LAYER);
    RME::core::Position p2(1,1, RME::core::GROUND_LAYER); // Selects (0,0,7), (1,0,7), (0,1,7), (1,1,7)

    m_mockUndoStack->resetMockState();
    m_editorController->performBoundingBoxSelection(p1, p2, Qt::NoModifier, currentBrushSettings);

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::BoundingBoxSelectCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);

    QVERIFY(!cmd->getIsAdditive());
    QCOMPARE(cmd->getSelectionStateBefore().size(), 1); // Contained initialSelTile
    QVERIFY(cmd->getSelectionStateBefore().contains(&initialSelTile));

    QList<RME::core::Tile*> calculated = cmd->getCalculatedTilesInBox();
    QCOMPARE(calculated.size(), 4); // (0,0,7), (1,0,7), (0,1,7), (1,1,7) from m_fixtureTiles
    // Verify specific tiles are present (requires m_fixtureTiles to be accessible or check by position)
    QVERIFY(calculated.contains(m_mockMapRawPtr->getTile(RME::core::Position(0,0,RME::core::GROUND_LAYER))));
    QVERIFY(calculated.contains(m_mockMapRawPtr->getTile(RME::core::Position(1,1,RME::core::GROUND_LAYER))));
}

void TestEditorController::testPerformBoundingBoxSelection_Additive_CurrentFloor() {
    RME::core::Tile initialSelTile(RME::core::Position(5,5,7));
    QList<RME::core::Tile*> initialSelection = {&initialSelTile};
    m_mockSelectionManager->MOCK_setSelectedTiles(initialSelection);

    m_mockAppSettings->setMockStringValue("SELECTION_TYPE", "CurrentFloor");
    m_mockAppSettings->setBoolValue("COMPENSATED_SELECT", false);
    RME::core::BrushSettings currentBrushSettings;
    currentBrushSettings.setActiveZ(RME::core::GROUND_LAYER);

    RME::core::Position p1(0,0, RME::core::GROUND_LAYER);
    RME::core::Position p2(0,0, RME::core::GROUND_LAYER); // Selects only (0,0,7)

    m_mockUndoStack->resetMockState();
    m_editorController->performBoundingBoxSelection(p1, p2, Qt::ControlModifier, currentBrushSettings);

    QVERIFY(m_mockUndoStack->pushCalled);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::BoundingBoxSelectCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);

    QVERIFY(cmd->getIsAdditive());
    QList<RME::core::Tile*> stateAfter = cmd->getSelectionStateAfter();
    QCOMPARE(stateAfter.size(), 2); // initialSelTile + (0,0,7)
    QVERIFY(stateAfter.contains(&initialSelTile));
    QVERIFY(stateAfter.contains(m_mockMapRawPtr->getTile(RME::core::Position(0,0,RME::core::GROUND_LAYER))));
}

// --- Tests for setHouseExit ---
void TestEditorController::testSetHouseExit_ValidHouseAndPosition() {
    QVERIFY(m_realHousesManager);
    RME::core::houses::House* testHouse = m_realHousesManager->createNewHouse(1);
    QVERIFY(testHouse);
    quint32 houseId = testHouse->getId();
    RME::core::Position exitPos(5,5,7);
    m_mockMapRawPtr->getOrCreateTile(exitPos); // Ensure tile exists for House::setExit to flag

    m_mockUndoStack->resetMockState();
    m_editorController->setHouseExit(houseId, exitPos);

    QVERIFY(m_mockUndoStack->pushCalled);
    QVERIFY(m_mockUndoStack->lastPushedCommandRaw != nullptr);
    auto* cmd = dynamic_cast<RME::editor_logic::commands::SetHouseExitCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);
    if (cmd) {
        QCOMPARE(cmd->getHouse(), testHouse);
        QCOMPARE(cmd->getNewExitPosition(), exitPos);
    }
}

void TestEditorController::testSetHouseExit_InvalidHouseId() {
    RME::core::Position exitPos(5,5,7);
    m_mockUndoStack->resetMockState();

    m_editorController->setHouseExit(999 /*non-existent ID*/, exitPos);

    QVERIFY(!m_mockUndoStack->pushCalled);
}

void TestEditorController::testSetHouseExit_SamePosition_NoCommand() {
    QVERIFY(m_realHousesManager);
    RME::core::houses::House* testHouse = m_realHousesManager->createNewHouse(1);
    QVERIFY(testHouse);
    quint32 houseId = testHouse->getId();
    RME::core::Position initialExitPos(5,5,7);

    m_mockMapRawPtr->getOrCreateTile(initialExitPos); // Ensure tile exists
    testHouse->setExit(initialExitPos); // Directly set initial exit
    QCOMPARE(testHouse->getExitPos(), initialExitPos);

    m_mockUndoStack->resetMockState();
    m_editorController->setHouseExit(houseId, initialExitPos); // Attempt to set to same pos

    QVERIFY(!m_mockUndoStack->pushCalled); // No command should be pushed
}


QTEST_MAIN(TestEditorController)
#include "TestEditorController.moc"

// Static member definition for MockMap's test tile access
QMap<RME::core::Position, RME::core::Tile*>* MockMap::m_testTilesOwner = nullptr;
