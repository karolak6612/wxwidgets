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
        return m_tilesInternal.value(pos).get();
    }
    RME::core::Tile* getOrCreateTile(const RME::core::Position& pos) override {
        // getTileForEditing already has create logic.
        return getTileForEditing(pos);
    }
    bool isPositionValid(const RME::core::Position& pos) const override {
        return pos.x >= 0 && pos.x < getWidth() && pos.y >= 0 && pos.y < getHeight() && pos.z >=0 && pos.z < getDepth();
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
    MockSelectionManager(RME::core::Map* map) : RME::core::selection::SelectionManager(map) {}
    bool isEmpty() const override { return m_selectedPositions.isEmpty(); }

    QList<RME::core::Position> m_selectedPositions;
    void clearSelection() { m_selectedPositions.clear(); }
    void addSelectedPosition(const RME::core::Position& pos) { m_selectedPositions.append(pos); }

    // This method is what DeleteSelectionCommand.cpp expects to use to populate its m_affectedPositions
    // This is a simplified assumption for the test.
    const QList<RME::core::Position>& getSelectedTilePositions() const { return m_selectedPositions; }
};

class MockAppSettings : public RME::core::settings::AppSettings {
public:
    bool getBool(const QString& key, bool defaultValue) const override {
        if (m_boolSettings.contains(key)) {
            return m_boolSettings.value(key);
        }
        return defaultValue;
    }
    void setBoolValue(const QString& key, bool value) {
        m_boolSettings[key] = value;
    }
    void resetMockState() {
        m_boolSettings.clear();
    }
private:
    QMap<QString, bool> m_boolSettings;
};


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

    void testDeleteSelection_NotEmpty();
    void testDeleteSelection_Empty();

private:
    std::unique_ptr<RME::editor_logic::EditorController> m_editorController;

    // Dependencies are now owned by unique_ptrs and raw pointers are passed to EditorController
    std::unique_ptr<MockMap> m_ownedMockMap;
    MockMap* m_mockMap; // Raw pointer for convenience, points to m_ownedMockMap's object

    std::unique_ptr<MockUndoStack> m_mockUndoStack;
    std::unique_ptr<MockBrushManager> m_mockBrushManager;
    std::unique_ptr<MockSelectionManager> m_mockSelectionManager;
    std::unique_ptr<MockAppSettings> m_mockAppSettings;

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
    m_mockMap = m_ownedMockMap.get();

    m_mockUndoStack = std::make_unique<MockUndoStack>();
    m_mockBrushManager = std::make_unique<MockBrushManager>();
    m_mockSelectionManager = std::make_unique<MockSelectionManager>(m_mockMap);
    m_mockAppSettings = std::make_unique<MockAppSettings>();

    // EditorController takes raw pointers, but the mocks are owned by unique_ptrs in this test class
    m_editorController = std::make_unique<RME::editor_logic::EditorController>(
        m_mockMap, // Pass raw pointer, ownership is with m_ownedMockMap
        m_mockUndoStack.get(),
        m_mockSelectionManager.get(),
        m_mockBrushManager.get(),
        m_mockAppSettings.get(),
        m_assetManager.get()
    );
}

void TestEditorController::cleanup() {
    m_editorController.reset();
    m_mockAppSettings.reset();
    m_mockSelectionManager.reset();
    m_mockBrushManager.reset();
    m_mockUndoStack.reset();
    m_ownedMockMap.reset(); // This owns the map
    m_mockMap = nullptr;

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
    auto* cmd = dynamic_cast<RME_COMMANDS::DeleteSelectionCommand*>(m_mockUndoStack->lastPushedCommandRaw);
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
    auto* cmd = dynamic_cast<RME_COMMANDS::DeleteSelectionCommand*>(m_mockUndoStack->lastPushedCommandRaw);
    QVERIFY(cmd != nullptr);
    // DeleteSelectionCommand's redo() is expected to handle the case of empty selection
    // (e.g., by doing nothing or setting appropriate text like "Delete Selection (nothing selected)").
}

QTEST_MAIN(TestEditorController)
#include "TestEditorController.moc"
