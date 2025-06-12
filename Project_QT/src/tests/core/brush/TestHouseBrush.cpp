#include <QtTest/QtTest>
#include "core/brush/HouseBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/IItemTypeProvider.h"
#include "tests/core/MockItemTypeProvider.h"
#include "MockEditorController.h"
#include "core/Tile.h" // For RME::TileMapFlag
#include "core/Position.h" // For RME::core::Position

// Using declarations for RME::core types to simplify test code
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMap = RME::core::map::Map;
using RMETileMapFlag = RME::TileMapFlag; // Note: TileMapFlag is in RME, not RME::core

class TestHouseBrush : public QObject {
    Q_OBJECT
private:
    RME::core::brush::HouseBrush m_houseBrush;
    RME::MockItemTypeProvider m_mockItemProvider;
    MockEditorController m_mockController;
    RMEMap* m_map = nullptr;

private slots:
    void init() {
        m_mockController.reset();
        m_houseBrush.setCurrentHouseId(0);
        if (m_map) delete m_map;
        m_map = new RMEMap(10, 10, 1, nullptr);
    }
    void cleanup() {
        delete m_map;
        m_map = nullptr;
    }

    void testPropertiesAndName() {
        RMEBrushSettings settings(&m_mockItemProvider);
        QCOMPARE(m_houseBrush.getName(), QObject::tr("House Brush (Eraser)"));
        QCOMPARE(m_houseBrush.getLookID(settings), RME::core::brush::EDITOR_SPRITE_HOUSE_BRUSH_LOOKID);
        QVERIFY(m_houseBrush.isHouse());
        QVERIFY(m_houseBrush.canDrag());

        m_houseBrush.setCurrentHouseId(123);
        QCOMPARE(m_houseBrush.getCurrentHouseId(), static_cast<uint32_t>(123));
        QCOMPARE(m_houseBrush.getName(), QObject::tr("House Brush (ID: %1)").arg(123));
    }

    void testCanApply() {
        RMEBrushSettings settings(&m_mockItemProvider);
        RMEPosition validPos(5,5,0);
        RMEPosition invalidPos(100,100,0); // Assuming map is 10x10

        QVERIFY(m_houseBrush.canApply(m_map, validPos, settings));
        QVERIFY(!m_houseBrush.canApply(m_map, invalidPos, settings)); // Invalid position
    }

    void testApply_AssignNewHouse() {
        RMEPosition pos(1,1,0);
        m_houseBrush.setCurrentHouseId(10);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);

        m_mockController.mock_current_tile_house_id = 0; // Tile currently has no house

        m_houseBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 4);
        QCOMPARE(m_mockController.calls[0].method, QString("getTileHouseId"));
        QCOMPARE(m_mockController.calls[1].method, QString("setTileHouseId"));
        QCOMPARE(m_mockController.calls[1].houseId, static_cast<uint32_t>(10));
        QCOMPARE(m_mockController.calls[2].method, QString("addTilePositionToHouse"));
        QCOMPARE(m_mockController.calls[2].houseId, static_cast<uint32_t>(10));
        QCOMPARE(m_mockController.calls[3].method, QString("setTileMapFlag"));
        QCOMPARE(m_mockController.calls[3].mapFlag, RMETileMapFlag::PROTECTION_ZONE);
        QVERIFY(m_mockController.calls[3].flagSet);
    }

    void testApply_ReassignHouse() {
        RMEPosition pos(2,2,0);
        m_houseBrush.setCurrentHouseId(20); // New house ID
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);

        m_mockController.mock_current_tile_house_id = 5; // Tile currently belongs to house 5

        m_houseBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 5);
        QCOMPARE(m_mockController.calls[0].method, QString("getTileHouseId"));
        QCOMPARE(m_mockController.calls[1].method, QString("removeTilePositionFromHouse"));
        QCOMPARE(m_mockController.calls[1].houseId, static_cast<uint32_t>(5));
        QCOMPARE(m_mockController.calls[2].method, QString("setTileHouseId"));
        QCOMPARE(m_mockController.calls[2].houseId, static_cast<uint32_t>(20));
        QCOMPARE(m_mockController.calls[3].method, QString("addTilePositionToHouse"));
        QCOMPARE(m_mockController.calls[3].houseId, static_cast<uint32_t>(20));
        QCOMPARE(m_mockController.calls[4].method, QString("setTileMapFlag"));
        QVERIFY(m_mockController.calls[4].flagSet);
    }

    void testApply_AssignToSameHouse() {
        RMEPosition pos(2,2,0);
        m_houseBrush.setCurrentHouseId(20);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);

        m_mockController.mock_current_tile_house_id = 20; // Tile already belongs to this house

        m_houseBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 2);
        QCOMPARE(m_mockController.calls[0].method, QString("getTileHouseId"));
        QCOMPARE(m_mockController.calls[1].method, QString("setTileMapFlag"));
        QVERIFY(m_mockController.calls[1].flagSet);
    }

    void testApply_EraseHouse() {
        RMEPosition pos(3,3,0);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(true);
        // m_houseBrush.setCurrentHouseId(0); // Eraser mode logic in apply doesn't use m_currentHouseId

        m_mockController.mock_current_tile_house_id = 30; // Tile currently belongs to house 30

        m_houseBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 4);
        QCOMPARE(m_mockController.calls[0].method, QString("getTileHouseId"));
        QCOMPARE(m_mockController.calls[1].method, QString("setTileHouseId"));
        QCOMPARE(m_mockController.calls[1].houseId, static_cast<uint32_t>(0));
        QCOMPARE(m_mockController.calls[2].method, QString("removeTilePositionFromHouse"));
        QCOMPARE(m_mockController.calls[2].houseId, static_cast<uint32_t>(30));
        QCOMPARE(m_mockController.calls[3].method, QString("setTileMapFlag"));
        QVERIFY(!m_mockController.calls[3].flagSet); // PZ false
    }

    void testApply_EraseEmptyTile() {
        RMEPosition pos(4,4,0);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(true);
        m_mockController.mock_current_tile_house_id = 0; // Tile has no house

        m_houseBrush.apply(&m_mockController, pos, settings);
        QCOMPARE(m_mockController.calls.size(), 1);
        QCOMPARE(m_mockController.calls[0].method, QString("getTileHouseId"));
    }

    void testApply_DrawWithHouseIdZero() {
        RMEPosition pos(1,1,0);
        m_houseBrush.setCurrentHouseId(0); // Trying to draw "no house"
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);

        m_houseBrush.apply(&m_mockController, pos, settings);
        QVERIFY(m_mockController.calls.isEmpty()); // Should do nothing
    }

    void testApply_NullController() {
        RMEPosition pos(1,1,0);
        m_houseBrush.setCurrentHouseId(10);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);
        m_houseBrush.apply(nullptr, pos, settings);
        QVERIFY(m_mockController.calls.isEmpty()); // No calls if controller is null
    }

};
// QTEST_APPLESS_MAIN(TestHouseBrush)
// #include "TestHouseBrush.moc"
