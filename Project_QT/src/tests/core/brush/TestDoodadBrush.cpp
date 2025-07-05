#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr
#include <vector> // For std::vector, if needed
#include <algorithm> // For std::sort, if needed

// Forward declare or include necessary classes
#include "core/brush/DoodadBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/MaterialData.h"
#include "core/settings/AppSettings.h"
#include "core/Position.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"

#include "tests/core/brush/MockEditorController.h"
#include "tests/core/assets/MockAssetManager.h"
#include "tests/core/assets/MockMaterialManager.h"
#include "tests/core/assets/MockCreatureDatabase.h"
#include "tests/core/MockItemTypeProvider.h"

// Using declarations
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMaterialData = RME::core::assets::MaterialData;
using RMEMaterialDoodadSpecifics = RME::core::assets::MaterialDoodadSpecifics;
using RMEMaterialAlternate = RME::core::assets::MaterialAlternate;
using RMEMaterialCompositeTile = RME::core::assets::MaterialCompositeTile;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMEDoodadBrush = RME::core::brush::DoodadBrush;
using RMEMockEditorController = MockEditorController;
using RMEMockAssetManager = RME::tests::MockAssetManager;
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEItem = RME::core::Item;


class TestDoodadBrush : public QObject {
    Q_OBJECT

public:
    TestDoodadBrush() = default;
    ~TestDoodadBrush() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testSelectAlternate();
    void testGetLookID();
    void testCanApply();
    void testApply_Draw_SingleItemDoodad();
    void testApply_Draw_CompositeDoodad();
    void testApply_Erase_SingleItemDoodad();
    void testApply_Erase_CompositeDoodad();

private:
    std::unique_ptr<RMEDoodadBrush> m_doodadBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProvider;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgr;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgr;
    std::unique_ptr<RMEAppSettings> m_appSettings;
    std::unique_ptr<RMEBrushSettings> m_brushSettings;

    RMEMaterialData m_singleItemDoodadMaterial;
    RMEMaterialData m_compositeDoodadMaterial;
    RMEMaterialData m_multiAlternateDoodadMaterial;
    RMEMaterialData m_emptyDoodadMaterial;


    // Item IDs
    const uint16_t SINGLE_DOODAD_ITEM_ID = 601;
    const uint16_t COMPOSITE_ITEM_A_ID = 602;
    const uint16_t COMPOSITE_ITEM_B_ID = 603;
    const uint16_t ALT1_ITEM_ID = 604;
    const uint16_t ALT2_ITEM_ID = 605;
};

void TestDoodadBrush::initTestCase() {
    // Single Item Doodad
    m_singleItemDoodadMaterial.id = "single_doodad";
    m_singleItemDoodadMaterial.typeAttribute = "doodad";
    RMEMaterialDoodadSpecifics singleSpecifics;
    RMEMaterialAlternate singleAlt;
    singleAlt.singleItemIds.append(SINGLE_DOODAD_ITEM_ID);
    singleSpecifics.alternates.append(singleAlt);
    m_singleItemDoodadMaterial.specificData = singleSpecifics;

    // Composite Doodad
    m_compositeDoodadMaterial.id = "composite_doodad";
    m_compositeDoodadMaterial.typeAttribute = "doodad";
    RMEMaterialDoodadSpecifics compositeSpecifics;
    RMEMaterialAlternate compositeAlt;
    RMEMaterialCompositeTile tile1; tile1.x = 0; tile1.y = 0; tile1.itemIds.append(COMPOSITE_ITEM_A_ID);
    RMEMaterialCompositeTile tile2; tile2.x = 1; tile2.y = 0; tile2.itemIds.append(COMPOSITE_ITEM_B_ID);
    compositeAlt.compositeTiles.append(tile1);
    compositeAlt.compositeTiles.append(tile2);
    compositeSpecifics.alternates.append(compositeAlt);
    m_compositeDoodadMaterial.specificData = compositeSpecifics;

    // Multi-Alternate Doodad
    m_multiAlternateDoodadMaterial.id = "multi_alt_doodad";
    m_multiAlternateDoodadMaterial.typeAttribute = "doodad";
    RMEMaterialDoodadSpecifics multiSpecifics;
    RMEMaterialAlternate alt1; alt1.singleItemIds.append(ALT1_ITEM_ID); multiSpecifics.alternates.append(alt1);
    RMEMaterialAlternate alt2; alt2.singleItemIds.append(ALT2_ITEM_ID); multiSpecifics.alternates.append(alt2);
    m_multiAlternateDoodadMaterial.specificData = multiSpecifics;

    // Empty Doodad (no alternates)
    m_emptyDoodadMaterial.id = "empty_doodad";
    m_emptyDoodadMaterial.typeAttribute = "doodad";
    m_emptyDoodadMaterial.specificData = RMEMaterialDoodadSpecifics(); // Empty alternates
}

void TestDoodadBrush::init() {
    m_doodadBrush = std::make_unique<RMEDoodadBrush>();
    m_mockController = std::make_unique<RMEMockEditorController>();

    m_mockItemProvider = std::make_unique<RMEMockItemTypeProvider>();
    m_mockItemProvider->setMockData(SINGLE_DOODAD_ITEM_ID, { "Single Doodad Item", SINGLE_DOODAD_ITEM_ID, false, false, QString("single_doodad") });
    m_mockItemProvider->setMockData(COMPOSITE_ITEM_A_ID, { "Composite A", COMPOSITE_ITEM_A_ID, false, false, QString("composite_doodad") });
    m_mockItemProvider->setMockData(COMPOSITE_ITEM_B_ID, { "Composite B", COMPOSITE_ITEM_B_ID, false, false, QString("composite_doodad") });
    m_mockItemProvider->setMockData(ALT1_ITEM_ID, { "Alt1 Item", ALT1_ITEM_ID, false, false, QString("multi_alt_doodad") });
    m_mockItemProvider->setMockData(ALT2_ITEM_ID, { "Alt2 Item", ALT2_ITEM_ID, false, false, QString("multi_alt_doodad") });

    m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();
    m_mockMaterialMgr = std::make_unique<RMEMockMaterialManager>();
    m_mockMaterialMgr->addMaterial(m_singleItemDoodadMaterial);
    m_mockMaterialMgr->addMaterial(m_compositeDoodadMaterial);
    m_mockMaterialMgr->addMaterial(m_multiAlternateDoodadMaterial);
    m_mockMaterialMgr->addMaterial(m_emptyDoodadMaterial);


    m_mockAssetMgr = std::make_unique<RMEMockAssetManager>(
        m_mockItemProvider.get(), m_mockCreatureDb.get(), m_mockMaterialMgr.get()
    );

    m_appSettings = std::make_unique<RMEAppSettings>();
    m_brushSettings = std::make_unique<RMEBrushSettings>();
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_mockItemProvider.get());

    m_mockController->m_mockMap = m_map.get();
    m_mockController->m_mockAppSettings = m_appSettings.get();
    m_mockController->m_brushSettings = m_brushSettings.get();
    m_mockController->setMockAssetManager(m_mockAssetMgr.get());

    m_mockController->reset();
}

void TestDoodadBrush::cleanup() {}
void TestDoodadBrush::cleanupTestCase() {}

void TestDoodadBrush::testSelectAlternate() {
    QVERIFY(m_doodadBrush);
    const auto* specifics_single = std::get_if<RMEMaterialDoodadSpecifics>(&m_singleItemDoodadMaterial.specificData);
    QVERIFY(specifics_single && !specifics_single->alternates.isEmpty());

    QCOMPARE(m_doodadBrush->selectAlternate(specifics_single, 0), &specifics_single->alternates.first());
    QCOMPARE(m_doodadBrush->selectAlternate(specifics_single, 1), &specifics_single->alternates.first());
    QCOMPARE(m_doodadBrush->selectAlternate(specifics_single, -1), &specifics_single->alternates.first());

    const auto* specifics_multi = std::get_if<RMEMaterialDoodadSpecifics>(&m_multiAlternateDoodadMaterial.specificData);
    QVERIFY(specifics_multi && specifics_multi->alternates.size() == 2);
    QCOMPARE(m_doodadBrush->selectAlternate(specifics_multi, 0), &specifics_multi->alternates[0]);
    QCOMPARE(m_doodadBrush->selectAlternate(specifics_multi, 1), &specifics_multi->alternates[1]);
    QCOMPARE(m_doodadBrush->selectAlternate(specifics_multi, 2), &specifics_multi->alternates[0]);

    RMEMaterialDoodadSpecifics emptySpecifics;
    QVERIFY(m_doodadBrush->selectAlternate(&emptySpecifics, 0) == nullptr);
    QVERIFY(m_doodadBrush->selectAlternate(nullptr, 0) == nullptr);
}

void TestDoodadBrush::testGetLookID() {
    QVERIFY(m_doodadBrush);
    RMEBrushSettings settings;

    m_doodadBrush->setMaterial(&m_singleItemDoodadMaterial);
    QCOMPARE(m_doodadBrush->getLookID(settings), 0);

    RMEMaterialData materialWithLookID = m_singleItemDoodadMaterial;
    materialWithLookID.lookId = 1234;
    m_doodadBrush->setMaterial(&materialWithLookID);
    QCOMPARE(m_doodadBrush->getLookID(settings), 1234);

    RMEMaterialData materialWithServerLookID = m_singleItemDoodadMaterial;
    materialWithServerLookID.lookId = 0; // Ensure client lookId is not set
    materialWithServerLookID.serverLookId = 5678;
    m_doodadBrush->setMaterial(&materialWithServerLookID);
    QCOMPARE(m_doodadBrush->getLookID(settings), 0);
}

void TestDoodadBrush::testCanApply() {
    QVERIFY(m_doodadBrush);
    RMEBrushSettings settings;
    RMEPosition validPos(1,1,0);

    m_doodadBrush->setMaterial(nullptr);
    QVERIFY(!m_doodadBrush->canApply(m_map.get(), validPos, settings));

    m_doodadBrush->setMaterial(&m_emptyDoodadMaterial); // Material with no alternates
    QVERIFY(!m_doodadBrush->canApply(m_map.get(), validPos, settings));

    m_doodadBrush->setMaterial(&m_singleItemDoodadMaterial);
    QVERIFY(m_doodadBrush->canApply(m_map.get(), validPos, settings));
    QVERIFY(!m_doodadBrush->canApply(nullptr, validPos, settings));
}

void TestDoodadBrush::testApply_Draw_SingleItemDoodad() {
    m_doodadBrush->setMaterial(&m_singleItemDoodadMaterial);
    RMEPosition pos(2,2,0);
    RMEBrushSettings settings; settings.isEraseMode = false;
    m_brushSettings->setGenericBrushParameter("variationIndex", 0);

    m_mockController->reset();
    m_doodadBrush->apply(m_mockController.get(), pos, *m_brushSettings.get());

    QCOMPARE(m_mockController->calls.size(), 1);
    const auto& call = m_mockController->calls.first();
    QCOMPARE(call.method, QStringLiteral("recordAddItem"));
    QCOMPARE(call.pos, pos);
    QCOMPARE(call.itemId, SINGLE_DOODAD_ITEM_ID);
    QCOMPARE(m_mockController->notifiedTiles.size(), 1);
    QVERIFY(m_mockController->notifiedTiles.contains(pos));
}

void TestDoodadBrush::testApply_Draw_CompositeDoodad() {
    m_doodadBrush->setMaterial(&m_compositeDoodadMaterial);
    RMEPosition clickPos(3,3,0);
    RMEBrushSettings settings; settings.isEraseMode = false;
    m_brushSettings->setGenericBrushParameter("variationIndex", 0);

    m_mockController->reset();
    m_doodadBrush->apply(m_mockController.get(), clickPos, *m_brushSettings.get());

    QCOMPARE(m_mockController->calls.size(), 2);

    bool itemA_found = false;
    bool itemB_found = false;
    RMEPosition posA = clickPos.translated(0,0,0);
    RMEPosition posB = clickPos.translated(1,0,0);

    for(const auto& call : m_mockController->calls) {
        QCOMPARE(call.method, QStringLiteral("recordAddItem"));
        if (call.pos == posA && call.itemId == COMPOSITE_ITEM_A_ID) itemA_found = true;
        if (call.pos == posB && call.itemId == COMPOSITE_ITEM_B_ID) itemB_found = true;
    }
    QVERIFY(itemA_found);
    QVERIFY(itemB_found);

    QCOMPARE(m_mockController->notifiedTiles.size(), 2);
    QVERIFY(m_mockController->notifiedTiles.contains(posA));
    QVERIFY(m_mockController->notifiedTiles.contains(posB));
}

void TestDoodadBrush::testApply_Erase_SingleItemDoodad() {
    m_doodadBrush->setMaterial(&m_singleItemDoodadMaterial);
    RMEPosition pos(2,2,0);
    RMEBrushSettings settings; settings.isEraseMode = true;
    // Ensure variationIndex is set in m_brushSettings, as apply reads from it directly
    m_brushSettings->setGenericBrushParameter("variationIndex", 0);

    // Setup: Place the target doodad item and an unrelated item
    RMETile* tile = m_map->getTileForEditing(pos);
    QVERIFY(tile);
    const RME::core::assets::ItemData* doodadItemData = m_mockItemProvider->getItemData(SINGLE_DOODAD_ITEM_ID);
    const RME::core::assets::ItemData* otherItemData = m_mockItemProvider->getItemData(ALT1_ITEM_ID); // Using ALT1_ITEM_ID as unrelated
    QVERIFY(doodadItemData); QVERIFY(otherItemData);
    tile->addItem(std::make_unique<RME::core::Item>(SINGLE_DOODAD_ITEM_ID, doodadItemData));
    tile->addItem(std::make_unique<RME::core::Item>(ALT1_ITEM_ID, otherItemData));

    m_mockController->reset();
    m_doodadBrush->apply(m_mockController.get(), pos, *m_brushSettings.get()); // Pass the member m_brushSettings

    bool removedTargetDoodad = false;
    bool removedOtherItem = false;
    for(const auto& call : m_mockController->calls) {
        if (call.method == QStringLiteral("recordRemoveItem") && call.pos == pos) {
            if (call.itemId == SINGLE_DOODAD_ITEM_ID) removedTargetDoodad = true;
            if (call.itemId == ALT1_ITEM_ID) removedOtherItem = true;
        }
    }
    QVERIFY(removedTargetDoodad);
    QVERIFY(!removedOtherItem);

    QVERIFY(m_mockController->notifiedTiles.contains(pos));
    // If only one item was removed, only one notification might be expected for the tile.
    // If erase logic clears more broadly or if other items were on the tile, this might need adjustment.
    // Based on current apply logic, only items from chosenAlternate are removed.
    // Notification logic affects all tiles where items were removed.
    QCOMPARE(m_mockController->notifiedTiles.size(), 1);
}

void TestDoodadBrush::testApply_Erase_CompositeDoodad() {
    m_doodadBrush->setMaterial(&m_compositeDoodadMaterial);
    RMEPosition clickPos(3,3,0);
    RMEPosition posA = clickPos.translated(0,0,0);
    RMEPosition posB = clickPos.translated(1,0,0);
    RMEBrushSettings settings; settings.isEraseMode = true;
    m_brushSettings->setGenericBrushParameter("variationIndex", 0);

    RMETile* tileA = m_map->getTileForEditing(posA); QVERIFY(tileA);
    RMETile* tileB = m_map->getTileForEditing(posB); QVERIFY(tileB);
    const RME::core::assets::ItemData* itemAData = m_mockItemProvider->getItemData(COMPOSITE_ITEM_A_ID);
    const RME::core::assets::ItemData* itemBData = m_mockItemProvider->getItemData(COMPOSITE_ITEM_B_ID);
    const RME::core::assets::ItemData* otherItemData = m_mockItemProvider->getItemData(ALT1_ITEM_ID);
    QVERIFY(itemAData); QVERIFY(itemBData); QVERIFY(otherItemData);

    tileA->addItem(std::make_unique<RME::core::Item>(COMPOSITE_ITEM_A_ID, itemAData));
    tileA->addItem(std::make_unique<RME::core::Item>(ALT1_ITEM_ID, otherItemData));
    tileB->addItem(std::make_unique<RME::core::Item>(COMPOSITE_ITEM_B_ID, itemBData));

    m_mockController->reset();
    m_doodadBrush->apply(m_mockController.get(), clickPos, *m_brushSettings.get()); // Pass member m_brushSettings

    bool removedItemA = false;
    bool removedItemB = false;
    bool removedOtherItemOnTileA = false;

    for(const auto& call : m_mockController->calls) {
        if (call.method == QStringLiteral("recordRemoveItem")) {
            if (call.pos == posA && call.itemId == COMPOSITE_ITEM_A_ID) removedItemA = true;
            if (call.pos == posB && call.itemId == COMPOSITE_ITEM_B_ID) removedItemB = true;
            if (call.pos == posA && call.itemId == ALT1_ITEM_ID) removedOtherItemOnTileA = true;
        }
    }
    QVERIFY(removedItemA);
    QVERIFY(removedItemB);
    QVERIFY(!removedOtherItemOnTileA);

    QVERIFY(m_mockController->notifiedTiles.contains(posA));
    QVERIFY(m_mockController->notifiedTiles.contains(posB));
    QVERIFY(m_mockController->notifiedTiles.size() == 2); // Tiles A and B should be notified.
}

// #include "TestDoodadBrush.moc" // Will be generated by AUTOMOC
