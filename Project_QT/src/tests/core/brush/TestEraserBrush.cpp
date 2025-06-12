#include <QtTest/QtTest>
#include "core/brush/EraserBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/IItemTypeProvider.h"
#include "tests/core/MockItemTypeProvider.h"
#include "MockEditorController.h"
#include "core/Position.h" // Ensure Position is included for RME::core::Position

// Using declarations for RME::core types to simplify test code
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMap = RME::core::map::Map;

class TestEraserBrush : public QObject {
    Q_OBJECT
private:
    RME::core::brush::EraserBrush m_eraserBrush;
    RME::MockItemTypeProvider m_mockItemProvider;
    MockEditorController m_mockController;
    RMEMap* m_map = nullptr;

private slots:
    void initTestCase() {
        // No specific item types needed for EraserBrush tests as it doesn't query item properties itself
    }

    void init() {
        m_mockController.reset();
        if (m_map) delete m_map;
        m_map = new RMEMap(10, 10, 1, nullptr /* assetManager for map */);
    }
    void cleanup() {
        delete m_map;
        m_map = nullptr;
    }

    void testGetNameAndLookID() {
        // Pass the mock provider to BrushSettings constructor
        RMEBrushSettings settings(&m_mockItemProvider);
        QCOMPARE(m_eraserBrush.getName(), QString("Eraser"));
        QCOMPARE(m_eraserBrush.getLookID(settings), RME::core::brush::EDITOR_SPRITE_ERASER_LOOKID);
    }

    void testProperties() {
        QVERIFY(m_eraserBrush.isEraser());
        QVERIFY(m_eraserBrush.canDrag());
        QVERIFY(m_eraserBrush.needsBorders());
    }

    void testCanApply() {
        RMEBrushSettings settings(&m_mockItemProvider);
        RMEPosition validPos(5,5,0);
        RMEPosition invalidPos(100,100,0); // Assuming map is 10x10

        QVERIFY(m_eraserBrush.canApply(m_map, validPos, settings));
        QVERIFY(!m_eraserBrush.canApply(m_map, invalidPos, settings)); // Invalid position
    }

    void testApply_NormalErase() {
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setAggressiveEraseMode(false); // Normal erase

        RMEPosition pos(1,1,0);
        m_eraserBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("clearTileNormally"));
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.leaveUnique, false); // Current hardcoded value in EraserBrush::apply
    }

    void testApply_AggressiveErase() {
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setAggressiveEraseMode(true); // Aggressive erase

        RMEPosition pos(2,2,0);
        m_eraserBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("clearTileAggressively"));
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.leaveUnique, false); // Current hardcoded value in EraserBrush::apply
    }

    void testApply_NullController() {
        RMEBrushSettings settings(&m_mockItemProvider);
        RMEPosition pos(1,1,0);
        m_eraserBrush.apply(nullptr, pos, settings);
        QCOMPARE(m_mockController.calls.size(), 0); // No calls should be made
    }
};
// QTEST_APPLESS_MAIN(TestEraserBrush)
// #include "TestEraserBrush.moc"
