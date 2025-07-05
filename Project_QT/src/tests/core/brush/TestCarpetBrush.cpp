#include <QtTest/QtTest>
#include <algorithm>

#include "core/brush/CarpetBrush.h"
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

// Using declarations
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMaterialData = RME::core::assets::MaterialData;
using RMEMaterialCarpetSpecifics = RME::core::assets::MaterialCarpetSpecifics;
using RMEMaterialOrientedPart = RME::core::assets::MaterialOrientedPart;
using RMEMaterialItemEntry = RME::core::assets::MaterialItemEntry;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMECarpetBrush = RME::core::brush::CarpetBrush;
using RMEMockAssetManager = RME::tests::MockAssetManager;
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEBorderType = RME::BorderType;


class TestCarpetBrush : public QObject {
    Q_OBJECT
private:
    std::unique_ptr<RMECarpetBrush> m_carpetBrush;
    std::unique_ptr<MockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;

    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProvider;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgr;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgr;

    std::unique_ptr<RMEAppSettings> m_appSettings;

    RMEMaterialData m_testCarpetMaterial;
    const uint16_t CARPET_CENTER_ID = 301;
    const uint16_t CARPET_NORTH_EDGE_ID = 302;
    const uint16_t CARPET_NORTHEAST_CORNER_ID = 303;
    const uint16_t CARPET_SOUTH_EDGE_ID = 304;
    const uint16_t CARPET_EAST_EDGE_ID = 305;
    const uint16_t CARPET_WEST_EDGE_ID = 306;
    const uint16_t CARPET_SOUTHEAST_CORNER_ID = 307;
    const uint16_t CARPET_NORTHWEST_CORNER_ID = 308; // Added for completeness
    const uint16_t CARPET_SOUTHWEST_CORNER_ID = 309; // Added for completeness
    const uint16_t OTHER_MATERIAL_CARPET_ID = 401;
    const uint16_t NON_CARPET_ITEM_ID = 402;


    void setupTileWithItems(const RMEPosition& pos, const QList<uint16_t>& itemIds) {
        RMETile* tile = m_map->getTileForEditing(pos);
        QVERIFY(tile);
        tile->getItemsForWrite().clear(); // Clear previous items before setting up new ones
        for (uint16_t itemId : itemIds) {
            const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(itemId);
            QVERIFY(itemData); // Ensure item data is mocked
            tile->addItem(std::make_unique<RME::core::Item>(itemId, itemData));
        }
    }

    // Keep old setupTileWithCarpet for compatibility or simplify its usage
    void setupTileWithCarpet(const RMEPosition& pos, uint16_t itemId) {
        setupTileWithItems(pos, {itemId});
    }


private slots:
    void initTestCase();
    void init();
    void cleanup();
    void testSetMaterial();
    void testApply_Draw_EmptyTile_AlignsCorrectly(); // Renamed
    void testApply_Erase_RemovesCarpetItems();
    // testApply_Draw_NorthEdgePiece(); // To be removed
    // testApply_Draw_Layering_CorrectlyUpdatesToCenterOrDefault(); // To be removed
    void testApply_Draw_NoLayering();
    void testApply_Draw_WithLayering();
    void testGetRandomItemIdForAlignment(); // Assuming we enhance the existing one
    void testBorderTypeToAlignmentString();
    void testUpdateCarpetAppearance_AlignmentLogic();
    void initTestCase() {
        m_testCarpetMaterial.id = "test_carpet";
        m_testCarpetMaterial.typeAttribute = "carpet";
        RMEMaterialCarpetSpecifics carpetSpecifics;

        RMEMaterialOrientedPart centerPart; centerPart.align = "center"; centerPart.items.append({CARPET_CENTER_ID, 100}); carpetSpecifics.parts.append(centerPart);
        RMEMaterialOrientedPart northPart; northPart.align = "n"; northPart.items.append({CARPET_NORTH_EDGE_ID, 100}); carpetSpecifics.parts.append(northPart);
        RMEMaterialOrientedPart eastPart; eastPart.align = "e"; eastPart.items.append({CARPET_EAST_EDGE_ID, 100}); carpetSpecifics.parts.append(eastPart);
        RMEMaterialOrientedPart southPart; southPart.align = "s"; southPart.items.append({CARPET_SOUTH_EDGE_ID, 100}); carpetSpecifics.parts.append(southPart);
        RMEMaterialOrientedPart westPart; westPart.align = "w"; westPart.items.append({CARPET_WEST_EDGE_ID, 100}); carpetSpecifics.parts.append(westPart);

        // Corners - map to "cnw", "cne" etc. as per CarpetBrush::borderTypeToAlignmentString
        RMEMaterialOrientedPart nwPart; nwPart.align = "cnw"; nwPart.items.append({CARPET_NORTHWEST_CORNER_ID, 100}); carpetSpecifics.parts.append(nwPart);
        RMEMaterialOrientedPart nePart; nePart.align = "cne"; nePart.items.append({CARPET_NORTHEAST_CORNER_ID, 100}); carpetSpecifics.parts.append(nePart);
        RMEMaterialOrientedPart swPart; swPart.align = "csw"; swPart.items.append({CARPET_SOUTHWEST_CORNER_ID, 100}); carpetSpecifics.parts.append(swPart);
        RMEMaterialOrientedPart sePart; sePart.align = "cse"; sePart.items.append({CARPET_SOUTHEAST_CORNER_ID, 100}); carpetSpecifics.parts.append(sePart);

        m_testCarpetMaterial.specificData = carpetSpecifics;

        RMECarpetBrush::initializeStaticData();
    }

    void init() {
        m_carpetBrush = std::make_unique<RMECarpetBrush>();
        m_mockController = std::make_unique<MockEditorController>();

        m_mockItemProvider = std::make_unique<RMEMockItemTypeProvider>();
        m_mockItemProvider->setMockData(CARPET_CENTER_ID, { "Carpet Center", CARPET_CENTER_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_NORTH_EDGE_ID, { "Carpet N Edge", CARPET_NORTH_EDGE_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_EAST_EDGE_ID, { "Carpet E Edge", CARPET_EAST_EDGE_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_SOUTH_EDGE_ID, { "Carpet S Edge", CARPET_SOUTH_EDGE_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_WEST_EDGE_ID, { "Carpet W Edge", CARPET_WEST_EDGE_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_NORTHWEST_CORNER_ID, { "Carpet NW Corner", CARPET_NORTHWEST_CORNER_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_NORTHEAST_CORNER_ID, { "Carpet NE Corner", CARPET_NORTHEAST_CORNER_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_SOUTHWEST_CORNER_ID, { "Carpet SW Corner", CARPET_SOUTHWEST_CORNER_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(CARPET_SOUTHEAST_CORNER_ID, { "Carpet SE Corner", CARPET_SOUTHEAST_CORNER_ID, false, false, QString("test_carpet") });
        m_mockItemProvider->setMockData(999, { "Other Item", 999, false, false, QString("") }); // Kept as a generic other item
        m_mockItemProvider->setMockData(OTHER_MATERIAL_CARPET_ID, { "Other Carpet", OTHER_MATERIAL_CARPET_ID, false, false, QString("other_carpet_material") });
        m_mockItemProvider->setMockData(NON_CARPET_ITEM_ID, { "Non-Carpet Item", NON_CARPET_ITEM_ID, false, false, QString("") });


        m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();
        m_mockMaterialMgr = std::make_unique<RMEMockMaterialManager>();
        m_mockMaterialMgr->addMaterial(m_testCarpetMaterial);

        m_mockAssetMgr = std::make_unique<RMEMockAssetManager>(
            m_mockItemProvider.get(), m_mockCreatureDb.get(), m_mockMaterialMgr.get()
        );

        m_appSettings = std::make_unique<RMEAppSettings>();
        m_appSettings->setLayerCarpetsEnabled(false);

        m_map = std::make_unique<RMEMap>(10, 10, 1, m_mockItemProvider.get());

        m_mockController->m_mockMap = m_map.get();
        m_mockController->m_mockAppSettings = m_appSettings.get();
        m_mockController->setMockAssetManager(m_mockAssetMgr.get());

        m_carpetBrush->setMaterial(m_mockMaterialMgr->getMaterial("test_carpet"));
        m_mockController->reset();
    }

    void cleanup() { }

    void testSetMaterial() {
        const RMEMaterialData* carpetMat = m_mockMaterialMgr->getMaterial("test_carpet");
        m_carpetBrush->setMaterial(carpetMat);
        QCOMPARE(m_carpetBrush->getMaterial(), carpetMat);
        QCOMPARE(m_carpetBrush->getName(), QString("test_carpet"));

        m_carpetBrush->setMaterial(nullptr);
        QCOMPARE(m_carpetBrush->getMaterial(), nullptr);
        QCOMPARE(m_carpetBrush->getName(), QString("Carpet Brush"));

        RMEMaterialData nonCarpetMaterial;
        nonCarpetMaterial.id = "wall_test";
        nonCarpetMaterial.typeAttribute = "wall";
        m_carpetBrush->setMaterial(&nonCarpetMaterial);
        QVERIFY(m_carpetBrush->getMaterial() == nullptr);
    }


    void testApply_Draw_EmptyTile_AlignsCorrectly() { // Renamed
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition pos(1, 1, 0);
        m_map->clear(); // Ensure tile is empty

        // Test with layering disabled
        m_appSettings->setLayerCarpetsEnabled(false);
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), pos, settings);

        // Expected sequence for targetPos:
        // 1. apply() adds CARPET_CENTER_ID
        // 2. updateCarpetAppearance(targetPos) removes CARPET_CENTER_ID
        // 3. updateCarpetAppearance(targetPos) adds aligned item (CARPET_CENTER_ID for empty surroundings)
        // Plus 8 calls to updateCarpetAppearance for neighbors (not checked in detail here)
        QCOMPARE(m_mockController->getCallCountForPos(pos, "recordAddItem", CARPET_CENTER_ID), 2); // Initial add + re-add aligned
        QCOMPARE(m_mockController->getCallCountForPos(pos, "recordRemoveItem", CARPET_CENTER_ID), 1); // Removal by update

        // Verify the sequence more strictly if needed by checking full call list order.
        // For simplicity, we check counts here.

        // Test with layering enabled - initial placement should be the same for an empty tile
        m_map->clear();
        m_appSettings->setLayerCarpetsEnabled(true);
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), pos, settings);
        QCOMPARE(m_mockController->getCallCountForPos(pos, "recordAddItem", CARPET_CENTER_ID), 2);
        QCOMPARE(m_mockController->getCallCountForPos(pos, "recordRemoveItem", CARPET_CENTER_ID), 1);
    }

    void testApply_Erase_RemovesCarpetItems() {
        RMEBrushSettings settings; settings.isEraseMode = true;
        RMEPosition pos(2, 2, 0);

        // Setup tile with a mix of items
        QList<uint16_t> itemsOnTile = {CARPET_CENTER_ID, CARPET_NORTH_EDGE_ID, OTHER_MATERIAL_CARPET_ID, NON_CARPET_ITEM_ID};
        setupTileWithItems(pos, itemsOnTile);

        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), pos, settings);

        // Verify only items from m_testCarpetMaterial are removed
        QVERIFY(m_mockController->wasMethodCalledForPos(pos, "recordRemoveItem", CARPET_CENTER_ID));
        QVERIFY(m_mockController->wasMethodCalledForPos(pos, "recordRemoveItem", CARPET_NORTH_EDGE_ID));
        QVERIFY(!m_mockController->wasMethodCalledForPos(pos, "recordRemoveItem", OTHER_MATERIAL_CARPET_ID));
        QVERIFY(!m_mockController->wasMethodCalledForPos(pos, "recordRemoveItem", NON_CARPET_ITEM_ID));

        // Count total remove calls to ensure no unexpected removals
        int totalRemoveCalls = 0;
        for(const auto& call : m_mockController->calls) {
            if(call.method == "recordRemoveItem") totalRemoveCalls++;
        }
        QCOMPARE(totalRemoveCalls, 2); // Only the two items from m_testCarpetMaterial
    }

    // testApply_Draw_NorthEdgePiece - Removed as its core logic is better tested in testUpdateCarpetAppearance_AlignmentLogic
    // testApply_Draw_Layering_CorrectlyUpdatesToCenterOrDefault - Removed as it's superseded by new specific tests

    void testApply_Draw_NoLayering() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition targetPos(3,3,0);
        m_appSettings->setLayerCarpetsEnabled(false);
        m_carpetBrush->setMaterial(&m_testCarpetMaterial);

        // Scenario 3.1: Target tile has an existing carpet of the same material.
        m_map->clear();
        setupTileWithCarpet(targetPos, CARPET_NORTH_EDGE_ID);
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), targetPos, settings);

        // Expected calls for targetPos (simplified for empty neighbors -> aligned is CENTER_ID):
        // 1. remove CARPET_NORTH_EDGE_ID (apply specific)
        // 2. add CARPET_CENTER_ID (apply specific)
        // 3. remove CARPET_CENTER_ID (updateCarpetAppearance for targetPos)
        // 4. add CARPET_CENTER_ID (aligned, updateCarpetAppearance for targetPos)
        auto callsForTarget = m_mockController->getCallsForPos(targetPos);
        QCOMPARE(callsForTarget.size(), 4); // Expect 4 direct actions on the target tile
        if(callsForTarget.size() >= 1) QCOMPARE(callsForTarget[0], MockCall("recordRemoveItem", targetPos, CARPET_NORTH_EDGE_ID));
        if(callsForTarget.size() >= 2) QCOMPARE(callsForTarget[1], MockCall("recordAddItem", targetPos, CARPET_CENTER_ID));
        if(callsForTarget.size() >= 3) QCOMPARE(callsForTarget[2], MockCall("recordRemoveItem", targetPos, CARPET_CENTER_ID));
        if(callsForTarget.size() >= 4) QCOMPARE(callsForTarget[3], MockCall("recordAddItem", targetPos, CARPET_CENTER_ID));


        // Scenario 3.2: Target tile has an existing carpet of a *different* material.
        m_map->clear();
        setupTileWithCarpet(targetPos, OTHER_MATERIAL_CARPET_ID);
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), targetPos, settings);

        // Expected calls for targetPos (simplified for empty neighbors -> aligned is CENTER_ID):
        // 1. add CARPET_CENTER_ID (apply specific, other material not touched)
        // 2. remove CARPET_CENTER_ID (updateCarpetAppearance for targetPos)
        // 3. add CARPET_CENTER_ID (aligned, updateCarpetAppearance for targetPos)
        callsForTarget = m_mockController->getCallsForPos(targetPos);
        QCOMPARE(callsForTarget.size(), 3);
        QVERIFY(!m_mockController->wasMethodCalledForPos(targetPos, "recordRemoveItem", OTHER_MATERIAL_CARPET_ID));
        if(callsForTarget.size() >= 1) QCOMPARE(callsForTarget[0], MockCall("recordAddItem", targetPos, CARPET_CENTER_ID));
        if(callsForTarget.size() >= 2) QCOMPARE(callsForTarget[1], MockCall("recordRemoveItem", targetPos, CARPET_CENTER_ID));
        if(callsForTarget.size() >= 3) QCOMPARE(callsForTarget[2], MockCall("recordAddItem", targetPos, CARPET_CENTER_ID));
        RMETile* tile = m_map->getTile(targetPos);
        QVERIFY(tile && tile->hasItemWithID(OTHER_MATERIAL_CARPET_ID)); // Other carpet should still be there
    }

    void testApply_Draw_WithLayering() {
        RMEBrushSettings settings; settings.isEraseMode = false;
        RMEPosition targetPos(4,4,0);
        m_appSettings->setLayerCarpetsEnabled(true);
        m_carpetBrush->setMaterial(&m_testCarpetMaterial);

        // Scenario 4.1: Target tile is empty (covered by testApply_Draw_EmptyTile_AlignsCorrectly, just double check layering setting)
        m_map->clear();
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), targetPos, settings);
        // Expect sequence: add CENTER, remove CENTER, add ALIGNED_CENTER (for targetPos)
        QVERIFY(m_mockController->getCallCountForPos(targetPos, "recordAddItem", CARPET_CENTER_ID) >= 1); // Initial add at least
        // Further checks similar to testApply_Draw_EmptyTile_AlignsCorrectly if needed.

        // Scenario 4.2: Target tile has an existing carpet of the *same* material.
        m_map->clear();
        setupTileWithItems(targetPos, {CARPET_NORTH_EDGE_ID}); // Use setupTileWithItems to avoid clear
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), targetPos, settings);

        // Expected calls for targetPos (simplified for empty neighbors -> new item aligned to CENTER_ID):
        // 1. add CARPET_CENTER_ID (apply specific, new item layered)
        // The updateCarpetAppearance for targetPos will act on one of the items.
        // If it acts on the newly added CARPET_CENTER_ID:
        // 2. remove CARPET_CENTER_ID
        // 3. add CARPET_CENTER_ID (aligned)
        // If it acts on the original CARPET_NORTH_EDGE_ID, it would try to align that.
        // This depends on item iteration order in updateCarpetAppearance.
        // For this test, we mainly care that a new item is added and the old one is NOT removed by apply().
        QVERIFY(!m_mockController->wasMethodCalledForPos(targetPos, "recordRemoveItem", CARPET_NORTH_EDGE_ID)); // Original not removed by apply()
        QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordAddItem", CARPET_CENTER_ID)); // New item added

        RMETile* tile = m_map->getTile(targetPos);
        QVERIFY(tile);
        bool foundOriginal = false;
        bool foundNew = false;
        int countOfTestMaterial = 0;
        for(const auto& item : tile->getItems()){
            if(item->getID() == CARPET_NORTH_EDGE_ID) foundOriginal = true;
            if(item->getID() == CARPET_CENTER_ID) foundNew = true; // Or whatever it was aligned to
             for(const auto& part : std::get<RMEMaterialCarpetSpecifics>(m_testCarpetMaterial.specificData).parts) {
                for(const auto& entry : part.items) {
                    if(entry.itemId == item->getID()) {
                        countOfTestMaterial++;
                        break;
                    }
                }
             }
        }
        QVERIFY(foundOriginal); // Original should still be there.
        QVERIFY(countOfTestMaterial >= 2); // Original + new (possibly aligned)


        // Scenario 4.3: Target tile has an existing carpet of a *different* material.
        m_map->clear();
        setupTileWithItems(targetPos, {OTHER_MATERIAL_CARPET_ID});
        m_mockController->reset();
        m_carpetBrush->apply(m_mockController.get(), targetPos, settings);
        // Expected: new carpet added, other material untouched by apply's add/remove logic.
        // updateCarpetAppearance will act on the new carpet.
        QVERIFY(!m_mockController->wasMethodCalledForPos(targetPos, "recordRemoveItem", OTHER_MATERIAL_CARPET_ID));
        QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordAddItem", CARPET_CENTER_ID));
        tile = m_map->getTile(targetPos);
        QVERIFY(tile && tile->hasItemWithID(OTHER_MATERIAL_CARPET_ID));
        QVERIFY(tile && tile->hasItemWithID(CARPET_CENTER_ID)); // Or its aligned version
    }


    void testGetRandomItemIdForAlignment() { // Assuming we enhance the existing one
        QVERIFY(m_carpetBrush);
        const auto* material = m_carpetBrush->getMaterial();
        QVERIFY(material);
        const auto* specifics = std::get_if<RMEMaterialCarpetSpecifics>(&material->specificData);
        QVERIFY(specifics);

        // Test defined alignments from initTestCase's m_testCarpetMaterial
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("center", specifics), CARPET_CENTER_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("n", specifics), CARPET_NORTH_EDGE_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("e", specifics), CARPET_EAST_EDGE_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("s", specifics), CARPET_SOUTH_EDGE_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("w", specifics), CARPET_WEST_EDGE_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("cnw", specifics), CARPET_NORTHWEST_CORNER_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("cne", specifics), CARPET_NORTHEAST_CORNER_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("csw", specifics), CARPET_SOUTHWEST_CORNER_ID);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("cse", specifics), CARPET_SOUTHEAST_CORNER_ID);

        // Test fallback for undefined alignment
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("undefined_alignment", specifics), CARPET_CENTER_ID);

        // Test case sensitivity (should be case insensitive as per implementation)
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("CeNtEr", specifics), CARPET_CENTER_ID);

        // Test with null specifics
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("center", nullptr), static_cast<uint16_t>(0));

        // Test alignment part exists but has no items
        RMEMaterialCarpetSpecifics tempSpecificsNoItems = *specifics;
        bool found_n_part = false;
        for (auto& part : tempSpecificsNoItems.parts) {
            if (part.align == "n") {
                part.items.clear(); // Remove items from "n" part
                found_n_part = true;
                break;
            }
        }
        QVERIFY(found_n_part); // Ensure we actually modified the intended part
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("n", &tempSpecificsNoItems), CARPET_CENTER_ID); // Should fall back to center

        // Test alignment part for "center" exists but has no items
        RMEMaterialCarpetSpecifics tempSpecificsNoCenterItems = *specifics;
        found_n_part = false; // reuse var
        for (auto& part : tempSpecificsNoCenterItems.parts) {
            if (part.align == "center") {
                part.items.clear();
                found_n_part = true;
                break;
            }
        }
        QVERIFY(found_n_part);
        // Fallback from "center" (if it's empty) to the first available item in other parts is complex.
        // The current getRandomItemIdForAlignment first tries the requested align, then "center".
        // If "center" itself is requested and empty, it returns 0.
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("center", &tempSpecificsNoCenterItems), static_cast<uint16_t>(0));
        // If requesting "n", and "n" is empty, and "center" is also empty:
        RMEMaterialCarpetSpecifics tempSpecificsNoNAndNoCenterItems = tempSpecificsNoItems; // "n" is already empty
         for (auto& part : tempSpecificsNoNAndNoCenterItems.parts) { // Now empty "center" too
            if (part.align == "center") {
                part.items.clear();
                break;
            }
        }
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("n", &tempSpecificsNoNAndNoCenterItems), static_cast<uint16_t>(0));


        // Test with multiple items and chances for one alignment part
        RMEMaterialCarpetSpecifics tempSpecificsWithChances = *specifics;
        const uint16_t ITEM_CHANCE_1 = 401;
        const uint16_t ITEM_CHANCE_2 = 402;
        for (auto& part : tempSpecificsWithChances.parts) {
            if (part.align == "test_chance") { // Should not exist, add new
                part.items.clear(); // Should be new anyway
            }
        }
        RMEMaterialOrientedPart chancePart;
        chancePart.align = "test_chance";
        chancePart.items.append({ITEM_CHANCE_1, 25});
        chancePart.items.append({ITEM_CHANCE_2, 75});
        tempSpecificsWithChances.parts.append(chancePart);

        // Simple check: ensure one of the items is returned.
        // Robust chance testing is harder in unit tests.
        bool item1Returned = false;
        bool item2Returned = false;
        for (int i = 0; i < 200; ++i) { // Call multiple times
            uint16_t selectedId = m_carpetBrush->getRandomItemIdForAlignment("test_chance", &tempSpecificsWithChances);
            QVERIFY(selectedId == ITEM_CHANCE_1 || selectedId == ITEM_CHANCE_2);
            if (selectedId == ITEM_CHANCE_1) item1Returned = true;
            if (selectedId == ITEM_CHANCE_2) item2Returned = true;
        }
        QVERIFY(item1Returned); // Check that both items are possible to select
        QVERIFY(item2Returned);

        // Test case where totalChance is 0 but items exist (should pick first)
        RMEMaterialCarpetSpecifics tempSpecificsZeroChance = *specifics;
        RMEMaterialOrientedPart zeroChancePart;
        zeroChancePart.align = "zero_chance_test";
        zeroChancePart.items.append({ITEM_CHANCE_1, 0});
        zeroChancePart.items.append({ITEM_CHANCE_2, 0});
        tempSpecificsZeroChance.parts.append(zeroChancePart);
        QCOMPARE(m_carpetBrush->getRandomItemIdForAlignment("zero_chance_test", &tempSpecificsZeroChance), ITEM_CHANCE_1);
    }

    void testBorderTypeToAlignmentString() {
        // Ensure m_carpetBrush is initialized (it should be by init())
        QVERIFY(m_carpetBrush);

        // Test common WX border types
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_NORTH_HORIZONTAL), QString("n"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_EAST_HORIZONTAL), QString("e"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_SOUTH_HORIZONTAL), QString("s"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_WEST_HORIZONTAL), QString("w"));

        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_NORTHWEST_CORNER), QString("cnw"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_NORTHEAST_CORNER), QString("cne"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_SOUTHWEST_CORNER), QString("csw"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_SOUTHEAST_CORNER), QString("cse"));

        // Test CARPET_CENTER
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::CARPET_CENTER), QString("center"));

        // Test BorderType::NONE (should map to "center" as per current implementation)
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::NONE), QString("center"));

        // Test Diagonals (should map to "center" as per current implementation comments)
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_NORTHWEST_DIAGONAL), QString("center"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_NORTHEAST_DIAGONAL), QString("center"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_SOUTHWEST_DIAGONAL), QString("center"));
        QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(RME::BorderType::WX_SOUTHEAST_DIAGONAL), QString("center"));

        // Test an out-of-range or unexpected value, if BorderType can hold it.
        // The current implementation has a default case that returns "center" and logs a warning.
        // Casting an arbitrary integer to BorderType for testing this can be problematic if it's not a valid enum value.
        // However, we can test a high enum value if one existed or rely on the default case of the switch.
        // For now, the existing enum values cover the main paths.
        // Example: (uint8_t)14 - if BorderType had more values
        // QCOMPARE(m_carpetBrush->borderTypeToAlignmentString(static_cast<RME::BorderType>(14)), QString("center")); // Assuming 14 is undefined and hits default
    }

    void TestCarpetBrush::testUpdateCarpetAppearance_AlignmentLogic() {
        QVERIFY(m_carpetBrush);
        QVERIFY(m_mockController);
        QVERIFY(m_map);
        QVERIFY(m_mockAssetMgr); // Ensure asset manager is available for item data
        m_carpetBrush->setMaterial(&m_testCarpetMaterial); // Ensure brush has material

        RMEBrushSettings settings; // Not directly used by updateCarpetAppearance, but good for context
        const auto* carpetSpecifics = std::get_if<RMEMaterialCarpetSpecifics>(&m_testCarpetMaterial.specificData);
        QVERIFY(specifics);

        // Helper lambda to run a test case for a given tiledata configuration
        auto runAlignmentTest = [&](const RMEPosition& targetPos, uint8_t tiledata, uint16_t initialItemIdOnTarget, const QMap<RMEPosition, uint16_t>& neighborItems) {
            m_map->clear(); // Clear map for each sub-test
            if (initialItemIdOnTarget != 0) {
                setupTileWithCarpet(targetPos, initialItemIdOnTarget);
            }
            for (auto it = neighborItems.constBegin(); it != neighborItems.constEnd(); ++it) {
                setupTileWithCarpet(it.key(), it.value());
            }

            m_mockController->reset();
            m_carpetBrush->updateCarpetAppearance(m_mockController.get(), targetPos, m_map.get(), &m_testCarpetMaterial);

            // Determine expected outcome based on tiledata
            RMEBorderType expectedBorderType = static_cast<RMEBorderType>(RMECarpetBrush::s_carpet_types[tiledata]);
            QString expectedAlignStr = m_carpetBrush->borderTypeToAlignmentString(expectedBorderType);
            uint16_t expectedNewItemId = m_carpetBrush->getRandomItemIdForAlignment(expectedAlignStr, carpetSpecifics);

            if (expectedNewItemId != 0 && initialItemIdOnTarget != expectedNewItemId) {
                bool removedOld = false;
                bool addedNew = false;
                for (const auto& call : m_mockController->calls) {
                    if (call.pos == targetPos) {
                        if (call.method == "recordRemoveItem" && call.itemId == initialItemIdOnTarget) removedOld = true;
                        if (call.method == "recordAddItem" && call.itemId == expectedNewItemId) addedNew = true;
                    }
                }
                QVERIFY2(removedOld, qPrintable(QString("Expected recordRemoveItem for oldId %1 not called for tiledata 0x%2 (align: %3, newId: %4)")
                                                .arg(initialItemIdOnTarget).arg(tiledata, 2, 16, QChar('0')).arg(expectedAlignStr).arg(expectedNewItemId)));
                QVERIFY2(addedNew, qPrintable(QString("Expected recordAddItem for newId %1 not called for tiledata 0x%2 (align: %3)")
                                              .arg(expectedNewItemId).arg(tiledata, 2, 16, QChar('0')).arg(expectedAlignStr)));
            } else if (expectedNewItemId == 0 && initialItemIdOnTarget != 0) { // Should remove if no valid new item
                bool removedOld = false;
                 for (const auto& call : m_mockController->calls) {
                    if (call.pos == targetPos && call.method == "recordRemoveItem" && call.itemId == initialItemIdOnTarget) removedOld = true;
                }
                QVERIFY2(removedOld, qPrintable(QString("Expected recordRemoveItem (newId 0) for oldId %1 not called for tiledata 0x%2 (align: %3)")
                                                .arg(initialItemIdOnTarget).arg(tiledata, 2, 16, QChar('0')).arg(expectedAlignStr)));
            } else { // No change expected or new item is same as old
                for (const auto& call : m_mockController->calls) {
                     if(call.pos == targetPos) { // Should be no calls for targetPos if no change
                        QVERIFY2(false, qPrintable(QString("Unexpected controller call for tiledata 0x%2 (align: %3, oldId: %4, newId: %5)")
                                                   .arg(tiledata, 2, 16, QChar('0')).arg(expectedAlignStr).arg(initialItemIdOnTarget).arg(expectedNewItemId)));
                     }
                }
            }
        };

        RMEPosition target(5,5,0);
        const uint16_t CENTER_ID = CARPET_CENTER_ID; // Initial item on target

        // Test Case 1: tiledata = 0x00 (no neighbors of same type) -> center
        runAlignmentTest(target, 0x00, CENTER_ID, {});

        // Test Case 2: tiledata = TILE_N (0x02) -> s_carpet_types[0x02] should map to "s" (CARPET_SOUTH_EDGE_ID)
        QMap<RMEPosition, uint16_t> neighbors_N_only;
        neighbors_N_only[target.translated(0, -1)] = CENTER_ID; // North neighbor
        runAlignmentTest(target, RME::TILE_N, CENTER_ID, neighbors_N_only);

        // Test Case 3: tiledata = TILE_E (0x10) -> s_carpet_types[0x10] should map to "w" (CARPET_WEST_EDGE_ID)
        QMap<RMEPosition, uint16_t> neighbors_E_only;
        neighbors_E_only[target.translated(1, 0)] = CENTER_ID; // East neighbor
        runAlignmentTest(target, RME::TILE_E, CENTER_ID, neighbors_E_only);

        // Test Case 4: tiledata = TILE_N | TILE_W (0x0A)
        // s_carpet_types[0x0A] = WX_NORTHWEST_CORNER -> "cnw" (CARPET_NORTHWEST_CORNER_ID)
        QMap<RMEPosition, uint16_t> neighbors_NW_corner;
        neighbors_NW_corner[target.translated(0, -1)] = CENTER_ID; // N
        neighbors_NW_corner[target.translated(-1, 0)] = CENTER_ID; // W
        runAlignmentTest(target, RME::TILE_N | RME::TILE_W, CENTER_ID, neighbors_NW_corner);

        // Test Case 5: tiledata = 0xFF (all neighbors same) -> s_carpet_types[0xFF] WX_NORTH_HORIZONTAL -> "n" (CARPET_NORTH_EDGE_ID)
        QMap<RMEPosition, uint16_t> neighbors_all;
        neighbors_all[target.translated(0, -1)] = CENTER_ID; // N
        neighbors_all[target.translated(1, -1)] = CENTER_ID; // NE
        neighbors_all[target.translated(-1,-1)] = CENTER_ID; // NW
        neighbors_all[target.translated(1, 0)] = CENTER_ID;  // E
        neighbors_all[target.translated(-1, 0)] = CENTER_ID; // W
        neighbors_all[target.translated(0, 1)] = CENTER_ID;  // S
        neighbors_all[target.translated(1, 1)] = CENTER_ID;  // SE
        neighbors_all[target.translated(-1, 1)] = CENTER_ID; // SW
        runAlignmentTest(target, 0xFF, CENTER_ID, neighbors_all);

        // Test Case 6: TILE_E | TILE_W | TILE_NE | TILE_N | TILE_NW (0x1F) -> WX_NORTH_HORIZONTAL -> "n"
        QMap<RMEPosition, uint16_t> neighbors_0x1F;
        neighbors_0x1F[target.translated(0,-1)] = CENTER_ID; // N
        neighbors_0x1F[target.translated(1,-1)] = CENTER_ID; // NE
        neighbors_0x1F[target.translated(-1,-1)] = CENTER_ID; // NW
        neighbors_0x1F[target.translated(1,0)] = CENTER_ID;  // E
        neighbors_0x1F[target.translated(-1,0)] = CENTER_ID; // W
        runAlignmentTest(target, RME::TILE_E | RME::TILE_W | RME::TILE_NE | RME::TILE_N | RME::TILE_NW, CENTER_ID, neighbors_0x1F);

        // Test Case 7: Target tile initially has no carpet item.
        // updateCarpetAppearance should do nothing if no target item.
        QMap<RMEPosition, uint16_t> neighbors_S_only;
        neighbors_S_only[target.translated(0, 1)] = CENTER_ID; // South neighbor
        m_map->clear();
        for (auto it = neighbors_S_only.constBegin(); it != neighbors_S_only.constEnd(); ++it) {
            setupTileWithCarpet(it.key(), it.value());
        }
        m_mockController->reset();
        m_carpetBrush->updateCarpetAppearance(m_mockController.get(), target, m_map.get(), &m_testCarpetMaterial);
        QVERIFY(m_mockController->calls.isEmpty()); // Should do nothing as target has no carpet
    }
};

// QTEST_APPLESS_MAIN(TestCarpetBrush)
// #include "TestCarpetBrush.moc"
