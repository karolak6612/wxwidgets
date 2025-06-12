#include <QtTest/QtTest>
#include "core/brush/RawBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemData.h"
#include "core/assets/IItemTypeProvider.h" // For direct use in BrushSettings
#include "core/Position.h"
#include "tests/core/MockItemTypeProvider.h"
#include "MockEditorController.h"

// Using declarations for RME::core types to simplify test code
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEItemType = RME::core::assets::ItemType;
using RMEMap = RME::core::map::Map;


class TestRawBrush : public QObject {
    Q_OBJECT
private:
    RME::core::brush::RawBrush m_rawBrush;
    RME::MockItemTypeProvider m_mockItemProvider; // Owned by this test class
    MockEditorController m_mockController;
    RMEMap* m_map = nullptr;

    // Test Item IDs
    const uint16_t GROUND_ITEM_ID = 101;
    const uint16_t STACKABLE_ITEM_ID = 102;
    const uint16_t NON_EXISTENT_ITEM_ID = 9999;

private slots:
    void initTestCase() {
        // Setup ItemType data in mock provider
        MockItemData groundData; // This is from tests/core/MockItemTypeProvider.h
        groundData.name = "Test Ground";
        groundData.isGround = true;
        groundData.clientID = 123;
        m_mockItemProvider.setMockData(GROUND_ITEM_ID, groundData);

        MockItemData stackableData;
        stackableData.name = "Test Stackable";
        stackableData.isGround = false; // Explicitly not ground
        stackableData.clientID = 456;
        m_mockItemProvider.setMockData(STACKABLE_ITEM_ID, stackableData);
    }

    void init() {
        m_rawBrush.setItemId(0);
        m_mockController.reset();

        if (m_map) delete m_map;
        // For tests needing a map, AssetManager is not directly used by RawBrush,
        // but Map needs it. BrushSettings will carry the IItemTypeProvider.
        // We pass nullptr for AssetManager to Map constructor for simplicity if Map allows.
        m_map = new RMEMap(10, 10, 1, nullptr /* RME::core::assets::AssetManager* */);
    }
    void cleanup() {
        delete m_map;
        m_map = nullptr;
    }

    void testSetItemId() {
        m_rawBrush.setItemId(GROUND_ITEM_ID);
        QCOMPARE(m_rawBrush.getItemId(), GROUND_ITEM_ID);
    }

    void testGetNameAndLookID() {
        // Pass the mock provider to BrushSettings constructor
        RMEBrushSettings settings(&m_mockItemProvider);

        m_rawBrush.setItemId(GROUND_ITEM_ID);
        // Current getName() is a placeholder, doesn't use provider from settings yet.
        QCOMPARE(m_rawBrush.getName(), QObject::tr("RAW Brush (ID: %1)").arg(GROUND_ITEM_ID));
        QCOMPARE(m_rawBrush.getLookID(settings), static_cast<int>(123));

        m_rawBrush.setItemId(STACKABLE_ITEM_ID);
        QCOMPARE(m_rawBrush.getName(), QObject::tr("RAW Brush (ID: %1)").arg(STACKABLE_ITEM_ID));
        QCOMPARE(m_rawBrush.getLookID(settings), static_cast<int>(456));

        m_rawBrush.setItemId(NON_EXISTENT_ITEM_ID);
        QCOMPARE(m_rawBrush.getLookID(settings), 0); // Default if item type not found
    }

    void testCanApply() {
        RMEBrushSettings settings(&m_mockItemProvider);
        RMEPosition validPos(5,5,0);
        RMEPosition invalidPos(100,100,0); // Assuming map is 10x10

        m_rawBrush.setItemId(GROUND_ITEM_ID);
        QVERIFY(m_rawBrush.canApply(m_map, validPos, settings));
        QVERIFY(!m_rawBrush.canApply(m_map, invalidPos, settings)); // Invalid position

        m_rawBrush.setItemId(0); // No item set
        QVERIFY(!m_rawBrush.canApply(m_map, validPos, settings)); // No item means cannot apply
    }

    void testApply_DrawGround() {
        m_rawBrush.setItemId(GROUND_ITEM_ID);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);
        settings.setActionIdEnabled(true); // Assume these exist on BrushSettings
        settings.setActionId(123);
        settings.setItemSubtype(1);

        RMEPosition pos(1,1,0);
        m_rawBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("setTileGround"));
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.itemId, GROUND_ITEM_ID);
    }

    void testApply_DrawStackable() {
        m_rawBrush.setItemId(STACKABLE_ITEM_ID);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(false);

        RMEPosition pos(2,2,0);
        m_rawBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("addStackedItemToTile"));
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.itemId, STACKABLE_ITEM_ID);
    }

    void testApply_EraseGround() {
        m_rawBrush.setItemId(GROUND_ITEM_ID);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(true);

        RMEPosition pos(3,3,0);
        m_rawBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("setTileGround")); // Erasing ground calls setTileGround with nullptr
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.itemId, static_cast<uint16_t>(0)); // ItemID should be 0 for nullptr item
    }

    void testApply_EraseStackable() {
        m_rawBrush.setItemId(STACKABLE_ITEM_ID);
        RMEBrushSettings settings(&m_mockItemProvider);
        settings.setEraseMode(true);

        RMEPosition pos(4,4,0);
        m_rawBrush.apply(&m_mockController, pos, settings);

        QCOMPARE(m_mockController.calls.size(), 1);
        const auto& call = m_mockController.calls.first();
        QCOMPARE(call.method, QString("removeStackedItemFromTile"));
        QCOMPARE(call.pos, pos);
        QCOMPARE(call.itemId, STACKABLE_ITEM_ID);
    }

    void testApply_InvalidItem() {
        m_rawBrush.setItemId(NON_EXISTENT_ITEM_ID);
        RMEBrushSettings settings(&m_mockItemProvider);
        RMEPosition pos(1,1,0);
        m_rawBrush.apply(&m_mockController, pos, settings);
        QVERIFY(m_mockController.calls.isEmpty()); // No controller calls if item type is not found
    }

};
// QTEST_APPLESS_MAIN(TestRawBrush)
// #include "TestRawBrush.moc"
