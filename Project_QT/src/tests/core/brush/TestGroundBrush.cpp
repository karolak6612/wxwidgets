#include <QtTest/QtTest>
#include <algorithm>

#include "core/brush/GroundBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/brush/BrushEnums.h"
#include "core/map/Map.h"
#include "core/assets/MaterialData.h"
#include "core/settings/AppSettings.h"
#include "core/Position.h"
#include "core/Tile.h"
#include "core/Item.h"

#include "tests/core/brush/MockEditorController.h"
#include "tests/core/assets/MockAssetManager.h"
#include "tests/core/assets/MockMaterialManager.h"
#include "tests/core/assets/MockCreatureDatabase.h"
#include "tests/core/MockItemTypeProvider.h"

// Using declarations (as before)
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMaterialData = RME::core::assets::MaterialData;
using RMEMaterialGroundSpecifics = RME::core::assets::MaterialGroundSpecifics;
using RMEMaterialBorderRule = RME::core::assets::MaterialBorderRule;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMEGroundBrush = RME::core::brush::GroundBrush;
using RMEMockAssetManager = RME::tests::MockAssetManager;
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEBorderType = RME::BorderType;


class TestGroundBrush : public QObject {
    Q_OBJECT
private:
    // ... (members as before, GENERIC_OUTER_BORDER_ID, etc.) ...
    std::unique_ptr<RMEGroundBrush> m_groundBrush;
    std::unique_ptr<MockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;

    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProvider;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgr;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgr;

    std::unique_ptr<RMEAppSettings> m_appSettings;

    RMEMaterialData m_grassMaterial;
    RMEMaterialData m_dirtMaterial;

    const uint16_t GRASS_ITEM_ID = 201;
    const uint16_t DIRT_ITEM_ID = 202;
    const uint16_t GRASS_BORDER_ITEM_ID = 203;
    const uint16_t DIRT_BORDER_ITEM_ID = 204;
    const uint16_t GENERIC_OUTER_BORDER_ID = 205;


    // ... (helper setupTileGround as before) ...
     void setupTileGround(const RMEPosition& pos, uint16_t groundItemId) {
        RMETile* tile = m_map->getTileForEditing(pos); QVERIFY(tile);
        const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(groundItemId); QVERIFY(itemData);
        // Clear previous ground if any for clean setup
        if(tile->getGround()) tile->setGround(nullptr);
        tile->setGround(std::make_unique<RME::core::Item>(groundItemId, itemData));
    }

private slots:
    void initTestCase() {
        // ... (material definitions as before) ...
        m_grassMaterial.id = "grass";
        m_grassMaterial.typeAttribute = "ground";
        RMEMaterialGroundSpecifics grassSpecifics;
        grassSpecifics.items.append({GRASS_ITEM_ID, 100});
        RMEMaterialBorderRule grassToNoneRule;
        grassToNoneRule.align = "outer";
        grassToNoneRule.toBrushName = "none";
        grassToNoneRule.borderItemId = GRASS_BORDER_ITEM_ID; // Grass border vs void
        grassSpecifics.borders.append(grassToNoneRule);
        RMEMaterialBorderRule grassToDirtRule;
        grassToDirtRule.align = "outer";
        grassToDirtRule.toBrushName = "dirt";
        grassToDirtRule.borderItemId = GENERIC_OUTER_BORDER_ID; // A different border for grass vs dirt
        grassSpecifics.borders.append(grassToDirtRule);
        m_grassMaterial.specificData = grassSpecifics;

        m_dirtMaterial.id = "dirt";
        // ... (dirtMaterial setup as before) ...
        m_dirtMaterial.typeAttribute = "ground";
        RMEMaterialGroundSpecifics dirtSpecifics;
        dirtSpecifics.items.append({DIRT_ITEM_ID, 100});
        // Add a rule for dirt bordering void, for testing neighbor updates
        RMEMaterialBorderRule dirtToNoneRule;
        dirtToNoneRule.align = "outer"; dirtToNoneRule.toBrushName = "none";
        dirtToNoneRule.borderItemId = DIRT_BORDER_ITEM_ID;
        dirtSpecifics.borders.append(dirtToNoneRule);
        m_dirtMaterial.specificData = dirtSpecifics;

        RMEGroundBrush::initializeStaticData(); // This now uses procedurally generated s_border_types
    }

    void init() {
        // ... (mock setups as before) ...
        m_groundBrush = std::make_unique<RMEGroundBrush>();
        m_mockController = std::make_unique<MockEditorController>();
        m_mockItemProvider = std::make_unique<RMEMockItemTypeProvider>();
        m_mockItemProvider->setMockData(GRASS_ITEM_ID, { "Grass Ground", GRASS_ITEM_ID, true, false, QString("grass") });
        m_mockItemProvider->setMockData(DIRT_ITEM_ID, { "Dirt Ground", DIRT_ITEM_ID, true, false, QString("dirt") });
        m_mockItemProvider->setMockData(GRASS_BORDER_ITEM_ID, { "Grass Border", GRASS_BORDER_ITEM_ID, false, true});
        m_mockItemProvider->setMockData(DIRT_BORDER_ITEM_ID, { "Dirt Border", DIRT_BORDER_ITEM_ID, false, true});
        m_mockItemProvider->setMockData(GENERIC_OUTER_BORDER_ID, { "Generic Outer Border", GENERIC_OUTER_BORDER_ID, false, true});


        m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();
        m_mockMaterialMgr = std::make_unique<RMEMockMaterialManager>();
        m_mockMaterialMgr->addMaterial(m_grassMaterial);
        m_mockMaterialMgr->addMaterial(m_dirtMaterial);
        m_mockAssetMgr = std::make_unique<RMEMockAssetManager>(m_mockItemProvider.get(), m_mockCreatureDb.get(), m_mockMaterialMgr.get());
        m_appSettings = std::make_unique<RMEAppSettings>();
        m_map = std::make_unique<RMEMap>(10, 10, 1, m_mockItemProvider.get());
        m_mockController->m_mockMap = m_map.get();
        m_mockController->m_mockAppSettings = m_appSettings.get();
        m_mockController->setMockAssetManager(m_mockAssetMgr.get());
        m_groundBrush->setMaterial(m_mockMaterialMgr->getMaterial("grass"));
        m_mockController->reset();
    }

    void cleanup() { }

    void testSetMaterial() {
        const RMEMaterialData* grass = m_mockMaterialMgr->getMaterial("grass");
        m_groundBrush->setMaterial(grass);
        QCOMPARE(m_groundBrush->getMaterial(), grass);
        QCOMPARE(m_groundBrush->getName(), QString("grass"));

        m_groundBrush->setMaterial(nullptr);
        QCOMPARE(m_groundBrush->getMaterial(), nullptr);
        QCOMPARE(m_groundBrush->getName(), QString("Ground Brush"));

        RMEMaterialData nonGroundMaterial;
        nonGroundMaterial.id = "wall_test";
        nonGroundMaterial.typeAttribute = "wall";
        m_groundBrush->setMaterial(&nonGroundMaterial);
        QVERIFY(m_groundBrush->getMaterial() == nullptr);
    }

    void testApply_DrawGround_CallsController() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition pos(1, 1, 0); // Drawing grass on empty tile
        m_groundBrush->apply(m_mockController.get(), pos, settings);

        bool setGroundCalled = false; bool setBordersCalled = false;
        int setBorderItemsCallCount = 0;
        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem") { setGroundCalled = true; QCOMPARE(call.newGroundId, GRASS_ITEM_ID); QCOMPARE(call.oldGroundId_field, static_cast<uint16_t>(0)); }
            if (call.method == "recordSetBorderItems" && call.pos == pos) {
                setBordersCalled = true;
                QVERIFY(call.newBorderIds.isEmpty());
                QVERIFY(call.oldBorderIds.isEmpty());
            }
             if (call.method == "recordSetBorderItems") { // Count all setBorderItems calls
                setBorderItemsCallCount++;
            }
        }
        QVERIFY(setGroundCalled);

        // If drawing on a completely empty tile, with empty neighbors, tiledata=0, s_border_types[0]=NONE.
        // So newBorderIds for target is empty. Old borders on target also empty. No call for target.
        // Neighbors are also empty, so no calls for them either.
        // This means setBordersCalled for targetPos will be false.
        // And setBorderItemsCallCount will be 0.
        QVERIFY(!setBordersCalled); // No change, no call for target
        QCOMPARE(setBorderItemsCallCount, 0); // No calls for any of the 9 tiles
    }


    void testApply_Draw_NorthNeighborIsVoid_PlacesNorthEdge() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition targetPos(2, 2, 0);
        // North neighbor (2,1) is void. All other surrounding neighbors also void.
        // tiledata for targetPos will be RME::TILE_N (0x02), as only North is "different" (void).
        // GroundBrush::s_border_types[RME::TILE_N] is packBorderTypes(BT::WX_NORTH_HORIZONTAL)
        // determineAlignString(WX_NORTH_HORIZONTAL,...) -> "outer"
        // determineToBrushName(WX_NORTH_HORIZONTAL,...) -> "none" (for North neighbor)
        // Rule for grass: align="outer", toBrushName="none" -> GRASS_BORDER_ITEM_ID

        m_groundBrush->apply(m_mockController.get(), targetPos, settings);

        bool groundSet = false;
        bool borderSetForTarget = false;
        QList<uint16_t> targetNewBorders;

        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem" && call.pos == targetPos) {
                groundSet = true; QCOMPARE(call.newGroundId, GRASS_ITEM_ID);
            }
            if (call.method == "recordSetBorderItems" && call.pos == targetPos) {
                borderSetForTarget = true; targetNewBorders = call.newBorderIds;
            }
        }
        QVERIFY(groundSet);
        QVERIFY(borderSetForTarget);

        QVERIFY2(!targetNewBorders.isEmpty(), "Expected a North edge border item, but no new borders were applied. Check s_border_types[0x02] and rule matching.");
        if (!targetNewBorders.isEmpty()) {
            QVERIFY(targetNewBorders.contains(GRASS_BORDER_ITEM_ID));
            QCOMPARE(targetNewBorders.size(), 1);
        }
    }

    void testApply_Draw_NorthNeighborIsDirt_PlacesGenericBorder() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition targetPos(3, 2, 0);
        RMEPosition northNeighborPos(3, 1, 0);

        setupTileGround(northNeighborPos, DIRT_ITEM_ID);

        m_groundBrush->apply(m_mockController.get(), targetPos, settings);

        bool groundSet = false; bool borderSetForTarget = false; QList<uint16_t> targetNewBorders;
        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem" && call.pos == targetPos) { groundSet = true; QCOMPARE(call.newGroundId, GRASS_ITEM_ID); }
            if (call.method == "recordSetBorderItems" && call.pos == targetPos) { borderSetForTarget = true; targetNewBorders = call.newBorderIds; }
        }
        QVERIFY(groundSet); QVERIFY(borderSetForTarget);
        QVERIFY2(!targetNewBorders.isEmpty(), "Expected a generic outer border item, but no new borders applied.");
        if (!targetNewBorders.isEmpty()) {
            QVERIFY(targetNewBorders.contains(GENERIC_OUTER_BORDER_ID));
            QCOMPARE(targetNewBorders.size(), 1);
        }
    }

    void testApply_Draw_NEandNWNeighborsDifferent_PlacesTwoCorners() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition targetPos(4, 2, 0);
        RMEPosition neNeighborPos(5, 1, 0); // NE of (4,2) is (5,1)
        RMEPosition nwNeighborPos(3, 1, 0); // NW of (4,2) is (3,1)

        setupTileGround(neNeighborPos, DIRT_ITEM_ID);
        setupTileGround(nwNeighborPos, DIRT_ITEM_ID);
        // N, W, E, S, SW, SE are void for targetPos(4,2)
        // tiledata for targetPos: NE bit (RME::TILE_NE = 0x04) and NW bit (RME::TILE_NW = 0x01) are set => 0x05
        // s_border_types[0x05] = packBorderTypes(BT::WX_NORTHWEST_CORNER, BT::WX_NORTHEAST_CORNER)

        m_groundBrush->apply(m_mockController.get(), targetPos, settings);

        bool groundSet = false; bool borderSetForTarget = false; QList<uint16_t> targetNewBorders;
        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem" && call.pos == targetPos) { groundSet = true; }
            if (call.method == "recordSetBorderItems" && call.pos == targetPos) { borderSetForTarget = true; targetNewBorders = call.newBorderIds; std::sort(targetNewBorders.begin(), targetNewBorders.end());}
        }
        QVERIFY(groundSet); QVERIFY(borderSetForTarget);

        // Expected: WX_NORTHWEST_CORNER -> determineAlignString="outer", determineToBrushName for NW (neighbor at index 0) is "dirt" -> matches grassToDirtRule -> GENERIC_OUTER_BORDER_ID
        //           WX_NORTHEAST_CORNER -> determineAlignString="outer", determineToBrushName for NE (neighbor at index 2) is "dirt" -> matches grassToDirtRule -> GENERIC_OUTER_BORDER_ID
        // Since newBorderItemIds avoids duplicates, it should contain one GENERIC_OUTER_BORDER_ID.
        QVERIFY2(!targetNewBorders.isEmpty(), "Expected border items for NE/NW different, but none applied.");
        if(!targetNewBorders.isEmpty()){
            QVERIFY(targetNewBorders.contains(GENERIC_OUTER_BORDER_ID));
            QCOMPARE(targetNewBorders.size(), 1);
        }
        qWarning("testApply_Draw_NEandNWNeighborsDifferent: Result depends on simplified determineToBrushName for corners. Current stub might pick N for both, leading to 'none', then GRASS_BORDER_ITEM_ID.");
    }

    void testApply_EraseGround_CallsController() { // Renamed from testApply_EraseGround_CallsControllerAndDoAutoBorders
        RMEBrushSettings settings; settings.isEraseMode = true;
        RMEPosition pos(2, 2, 0);
        setupTileGround(pos, GRASS_ITEM_ID);
        m_mockController->reset();
        m_groundBrush->apply(m_mockController.get(), pos, settings);

        bool setGroundCalledToNull = false; int setBorderItemsCallCount = 0;
        QList<uint16_t> targetNewBorders, targetOldBorders;

        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem") {
                setGroundCalledToNull = true;
                QCOMPARE(call.pos, pos);
                QCOMPARE(call.newGroundId, static_cast<uint16_t>(0));
                QCOMPARE(call.oldGroundId_field, GRASS_ITEM_ID);
            }
            else if (call.method == "recordSetBorderItems") {
                setBorderItemsCallCount++;
                if(call.pos == pos) { // Borders for the erased tile
                    targetNewBorders = call.newBorderIds;
                    targetOldBorders = call.oldBorderIds;
                }
            }
        }
        QVERIFY(setGroundCalledToNull);
        // When erasing, currentTileSpecifics becomes null. doAutoBorders for target tile
        // should result in newBorderIds being empty. If old borders were present, they'd be cleared.
        QVERIFY(targetNewBorders.isEmpty());
        // oldBorderIds for target tile depends on if it had items. For this test, it had none.
        QVERIFY(targetOldBorders.isEmpty());
        // Call to recordSetBorderItems might be skipped if old and new are both empty.
        // QVERIFY(setBorderItemsCallCount >= 1 && setBorderItemsCallCount <= 9); This check is too broad.
    }

     void testDoAutoBorders_WithMockedMaterialOnTileAndExistingBorders() {
        RMEPosition targetPos(1,1,0);
        RMEBrushSettings settings; settings.isEraseMode = false;

        setupTileGround(targetPos, GRASS_ITEM_ID);

        RMETile* tile = m_map->getTileForEditing(targetPos);
        const RME::core::assets::ItemData* borderItemData = m_mockItemProvider->getItemData(GRASS_BORDER_ITEM_ID);
        QVERIFY(borderItemData);
        tile->addItem(std::make_unique<RME::core::Item>(GRASS_BORDER_ITEM_ID, borderItemData));

        const RME::core::assets::ItemData* borderItemData2 = m_mockItemProvider->getItemData(DIRT_BORDER_ITEM_ID);
        QVERIFY(borderItemData2);
        tile->addItem(std::make_unique<RME::core::Item>(DIRT_BORDER_ITEM_ID, borderItemData2));

        QList<uint16_t> expectedOldBorderIds = {GRASS_BORDER_ITEM_ID, DIRT_BORDER_ITEM_ID};
        std::sort(expectedOldBorderIds.begin(), expectedOldBorderIds.end());

        m_mockController->reset();
        m_groundBrush->apply(m_mockController.get(), targetPos, settings);

        bool setGroundItemCalled = false;
        bool setBorderItemsCalledForTarget = false;

        for(const auto& call : m_mockController->calls) {
            if (call.method == "recordSetGroundItem" && call.pos == targetPos) {
                setGroundItemCalled = true; QCOMPARE(call.newGroundId, GRASS_ITEM_ID);
                QCOMPARE(call.oldGroundId_field, GRASS_ITEM_ID);
            }
            if (call.method == "recordSetBorderItems" && call.pos == targetPos) {
                setBorderItemsCalledForTarget = true;
                QVERIFY(call.newBorderIds.isEmpty());
                QCOMPARE(call.oldBorderIds, expectedOldBorderIds);
            }
        }
        QVERIFY(setGroundItemCalled);
        QVERIFY(setBorderItemsCalledForTarget);
    }

};

// ... (moc include) ...
