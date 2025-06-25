#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr
#include <vector> // For std::vector, if needed
#include <algorithm> // For std::sort, if needed

// Forward declare or include necessary classes
#include "core/brush/TableBrush.h"
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

// Using declarations for brevity
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMaterialData = RME::core::assets::MaterialData;
using RMEMaterialTableSpecifics = RME::core::assets::MaterialTableSpecifics;
using RMEMaterialOrientedPart = RME::core::assets::MaterialOrientedPart;
using RMEMaterialItemEntry = RME::core::assets::MaterialItemEntry;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMETableBrush = RME::core::TableBrush;
using RMEMockEditorController = MockEditorController;
using RMEMockAssetManager = RME::tests::MockAssetManager;
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEBorderType = RME::BorderType;
using RMEItem = RME::core::Item;


class TestTableBrush : public QObject {
    Q_OBJECT

public:
    TestTableBrush() = default;
    ~TestTableBrush() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testStaticTableTypesInitialization();
    void testTableSegmentTypeToAlignString();
    void testGetRandomItemIdForAlignString();
    void testApply_Draw_EmptyTile_PlacesAlone();
    void testApply_Draw_WithNeighbors_AlignsCorrectly();
    void testApply_Erase_RemovesTableAndUpdatesNeighbors();

private:
    std::unique_ptr<RMETableBrush> m_tableBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProvider;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgr;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgr;
    std::unique_ptr<RMEAppSettings> m_appSettings;

    RMEMaterialData m_testTableMaterial;
    // Define some item IDs for testing
    const uint16_t TABLE_ALONE_ID = 401;
    const uint16_t TABLE_VERTICAL_ID = 402;
    const uint16_t TABLE_HORIZONTAL_ID = 403;
    const uint16_t TABLE_N_END_ID = 404;
    const uint16_t TABLE_E_END_ID = 405;
    const uint16_t TABLE_S_END_ID = 406;
    const uint16_t TABLE_W_END_ID = 407;
    // For chance testing
    const uint16_t ITEM_CHANCE_1 = 501;
    const uint16_t ITEM_CHANCE_2 = 502;

    // Helper to set up a tile with items (ground or non-ground)
    void setupTileWithItems(const RMEPosition& pos, const QList<uint16_t>& itemIds, bool isGround = false) {
        RMETile* tile = m_map->getTileForEditing(pos);
        QVERIFY(tile);
        if (isGround) {
            QVERIFY(itemIds.size() <= 1); // Max 1 ground item
            if (itemIds.isEmpty()) {
                tile->setGround(nullptr);
            } else {
                const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(itemIds.first());
                QVERIFY(itemData && itemData->isGround);
                tile->setGround(std::make_unique<RMEItem>(itemIds.first(), itemData));
            }
        } else {
            tile->getItemsForWrite().clear(); // Clear previous non-ground items
            for (uint16_t itemId : itemIds) {
                const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(itemId);
                QVERIFY(itemData && !itemData->isGround);
                tile->addItem(std::make_unique<RMEItem>(itemId, itemData));
            }
        }
    }
};

void TestTableBrush::initTestCase() {
    m_testTableMaterial.id = "test_table";
    m_testTableMaterial.typeAttribute = "table";
    RMEMaterialTableSpecifics tableSpecifics;

    tableSpecifics.parts.append({QStringLiteral("alone"), {{TABLE_ALONE_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("vertical"), {{TABLE_VERTICAL_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("horizontal"), {{TABLE_HORIZONTAL_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("north"), {{TABLE_N_END_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("east"), {{TABLE_E_END_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("south"), {{TABLE_S_END_ID, 100}}});
    tableSpecifics.parts.append({QStringLiteral("west"), {{TABLE_W_END_ID, 100}}});

    m_testTableMaterial.specificData = tableSpecifics;

    RMETableBrush::initializeStaticData();
}

void TestTableBrush::init() {
    m_tableBrush = std::make_unique<RMETableBrush>();
    m_mockController = std::make_unique<RMEMockEditorController>();

    m_mockItemProvider = std::make_unique<RMEMockItemTypeProvider>();
    m_mockItemProvider->setMockData(TABLE_ALONE_ID, { "Table Alone", TABLE_ALONE_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_VERTICAL_ID, { "Table Vertical", TABLE_VERTICAL_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_HORIZONTAL_ID, { "Table Horizontal", TABLE_HORIZONTAL_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_N_END_ID, { "Table N-End", TABLE_N_END_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_E_END_ID, { "Table E-End", TABLE_E_END_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_S_END_ID, { "Table S-End", TABLE_S_END_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(TABLE_W_END_ID, { "Table W-End", TABLE_W_END_ID, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(ITEM_CHANCE_1, { "Table Chance 1", ITEM_CHANCE_1, false, false, QString("test_table") });
    m_mockItemProvider->setMockData(ITEM_CHANCE_2, { "Table Chance 2", ITEM_CHANCE_2, false, false, QString("test_table") });

    m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();
    m_mockMaterialMgr = std::make_unique<RMEMockMaterialManager>();
    m_mockMaterialMgr->addMaterial(m_testTableMaterial);

    m_mockAssetMgr = std::make_unique<RMEMockAssetManager>(
        m_mockItemProvider.get(), m_mockCreatureDb.get(), m_mockMaterialMgr.get()
    );

    m_appSettings = std::make_unique<RMEAppSettings>();
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_mockItemProvider.get());

    m_mockController->m_mockMap = m_map.get();
    m_mockController->m_mockAppSettings = m_appSettings.get();
    m_mockController->setMockAssetManager(m_mockAssetMgr.get());

    m_tableBrush->setMaterial(m_mockMaterialMgr->getMaterial("test_table"));
    m_mockController->reset();
}

void TestTableBrush::cleanup() {}
void TestTableBrush::cleanupTestCase() {}

void TestTableBrush::testStaticTableTypesInitialization() {
    QCOMPARE(RMETableBrush::s_table_types[0], static_cast<uint32_t>(RME::BorderType::TABLE_ALONE));
    QCOMPARE(RMETableBrush::s_table_types[RME::TILE_N], static_cast<uint32_t>(RME::BorderType::TABLE_SOUTH_END));
    QCOMPARE(RMETableBrush::s_table_types[RME::TILE_E | RME::TILE_W], static_cast<uint32_t>(RME::BorderType::TABLE_HORIZONTAL));
    QCOMPARE(RMETableBrush::s_table_types[RME::TILE_S | RME::TILE_E | RME::TILE_W | RME::TILE_N], static_cast<uint32_t>(RME::BorderType::TABLE_HORIZONTAL));
    QCOMPARE(RMETableBrush::s_table_types[RME::TILE_S | RME::TILE_N], static_cast<uint32_t>(RME::BorderType::TABLE_VERTICAL));
}

void TestTableBrush::testTableSegmentTypeToAlignString() {
    QVERIFY(m_tableBrush);
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_ALONE), QStringLiteral("alone"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_VERTICAL), QStringLiteral("vertical"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_HORIZONTAL), QStringLiteral("horizontal"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_SOUTH_END), QStringLiteral("south"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_EAST_END), QStringLiteral("east"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_NORTH_END), QStringLiteral("north"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::TABLE_WEST_END), QStringLiteral("west"));
    QCOMPARE(m_tableBrush->tableSegmentTypeToAlignString(RME::BorderType::NONE), QStringLiteral("alone"));
}

void TestTableBrush::testGetRandomItemIdForAlignString() {
    QVERIFY(m_tableBrush);
    const auto* specifics = m_tableBrush->getCurrentTableSpecifics();
    QVERIFY(specifics);

    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("alone"), specifics), TABLE_ALONE_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("vertical"), specifics), TABLE_VERTICAL_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("horizontal"), specifics), TABLE_HORIZONTAL_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("north"), specifics), TABLE_N_END_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("east"), specifics), TABLE_E_END_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("south"), specifics), TABLE_S_END_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("west"), specifics), TABLE_W_END_ID);

    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("undefined_alignment"), specifics), TABLE_ALONE_ID);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("CeNtEr"), specifics), TABLE_ALONE_ID);

    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("alone"), nullptr), static_cast<uint16_t>(0));

    RMEMaterialTableSpecifics tempSpecificsNoItems = *specifics;
    bool found_vertical_part = false;
    for (auto& part : tempSpecificsNoItems.parts) {
        if (part.align == QLatin1String("vertical")) {
            part.items.clear();
            found_vertical_part = true;
            break;
        }
    }
    QVERIFY(found_vertical_part);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("vertical"), &tempSpecificsNoItems), TABLE_ALONE_ID);

    RMEMaterialTableSpecifics tempSpecificsNoAloneItems = *specifics;
    bool found_alone_part = false;
    for (auto& part : tempSpecificsNoAloneItems.parts) {
        if (part.align == QLatin1String("alone")) {
            part.items.clear();
            found_alone_part = true;
            break;
        }
    }
    QVERIFY(found_alone_part);
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("alone"), &tempSpecificsNoAloneItems), static_cast<uint16_t>(0));

    RMEMaterialTableSpecifics tempSpecificsNoVerticalAndNoAlone = tempSpecificsNoItems;
    for (auto& part : tempSpecificsNoVerticalAndNoAlone.parts) {
        if (part.align == QLatin1String("alone")) {
            part.items.clear();
            break;
        }
    }
    QCOMPARE(m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("vertical"), &tempSpecificsNoVerticalAndNoAlone), static_cast<uint16_t>(0));

    RMEMaterialTableSpecifics tempSpecificsWithChances = *specifics;
    RMEMaterialOrientedPart chancePart;
    chancePart.align = "chance_test";
    chancePart.items.append({ITEM_CHANCE_1, 25});
    chancePart.items.append({ITEM_CHANCE_2, 75});
    tempSpecificsWithChances.parts.append(chancePart);

    bool item1Returned = false;
    bool item2Returned = false;
    for (int i = 0; i < 200; ++i) {
        uint16_t selectedId = m_tableBrush->getRandomItemIdForAlignString(QStringLiteral("chance_test"), &tempSpecificsWithChances);
        QVERIFY(selectedId == ITEM_CHANCE_1 || selectedId == ITEM_CHANCE_2);
        if (selectedId == ITEM_CHANCE_1) item1Returned = true;
        if (selectedId == ITEM_CHANCE_2) item2Returned = true;
    }
    QVERIFY(item1Returned);
    QVERIFY(item2Returned);
}

void TestTableBrush::testApply_Draw_EmptyTile_PlacesAlone() {
    RMEBrushSettings settings; settings.isEraseMode = false;
    RMEPosition pos(1, 1, 0);
    m_map->clear();
    m_tableBrush->setMaterial(&m_testTableMaterial);
    m_mockController->reset();

    m_tableBrush->apply(m_mockController.get(), pos, settings);

    // Sequence:
    // 1. apply() -> recordAddItem(pos, TABLE_ALONE_ID)
    // 2. updateTableAppearance(pos) -> tiledata=0, segment=TABLE_ALONE, newItemId=TABLE_ALONE_ID. Old was TABLE_ALONE_ID. No change. No controller calls from here.
    // 3. updateTableAppearance for neighbors -> no items, no calls.

    QVERIFY(m_mockController->wasMethodCalledForPos(pos, "recordAddItem", TABLE_ALONE_ID));
    int addItemCount = 0;
    int removeItemCount = 0;
     for(const auto& call : m_mockController->calls) {
        if (call.pos == pos) {
            if(call.method == "recordAddItem") addItemCount++;
            if(call.method == "recordRemoveItem") removeItemCount++;
        }
    }
    QCOMPARE(addItemCount, 1); // Only the initial add
    QCOMPARE(removeItemCount, 0); // No removal as the item is already correct
}

void TestTableBrush::testApply_Draw_WithNeighbors_AlignsCorrectly() {
    QVERIFY(m_tableBrush);
    QVERIFY(m_mockController);
    QVERIFY(m_map);
    m_tableBrush->setMaterial(&m_testTableMaterial);

    const auto* tableSpecifics = m_tableBrush->getCurrentTableSpecifics();
    QVERIFY(tableSpecifics);

    auto runAlignmentTest = [&](const RMEPosition& targetPos,
                                const QMap<RMEPosition, uint16_t>& neighborItemIds,
                                RME::BorderType expectedSegmentTypeOnTarget) {
        m_map->clear();
        for (auto it = neighborItemIds.constBegin(); it != neighborItemIds.constEnd(); ++it) {
            const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(it.value());
            QVERIFY(itemData && itemData->materialId == m_testTableMaterial.id);
            RMETile* tile = m_map->getTileForEditing(it.key());
            QVERIFY(tile);
            // For tables, we only care about items of the same material for adjacency
            tile->addItem(std::make_unique<RME::core::Item>(it.value(), itemData));
        }

        m_mockController->reset();
        RMEBrushSettings settings; settings.isEraseMode = false;
        m_tableBrush->apply(m_mockController.get(), targetPos, settings);

        QString expectedAlignStr = m_tableBrush->tableSegmentTypeToAlignString(expectedSegmentTypeOnTarget);
        uint16_t expectedFinalItemId = m_tableBrush->getRandomItemIdForAlignString(expectedAlignStr, tableSpecifics);
        QVERIFY(expectedFinalItemId != 0);

        bool initialAdd = false;
        bool initialRemove = false;
        bool finalAdd = false;
        int targetRelevantCallIndex = 0;

        for(const auto& call : m_mockController->calls) {
            if (call.pos == targetPos) {
                targetRelevantCallIndex++;
                if (targetRelevantCallIndex == 1 && call.method == "recordAddItem" && call.itemId == TABLE_ALONE_ID) initialAdd = true;
                if (targetRelevantCallIndex == 2 && call.method == "recordRemoveItem" && call.itemId == TABLE_ALONE_ID) initialRemove = true;
                if (targetRelevantCallIndex == 3 && call.method == "recordAddItem" && call.itemId == expectedFinalItemId) finalAdd = true;
            }
        }

        QString msg = QString("Test for target segment %1 (item %2)").arg(expectedAlignStr).arg(expectedFinalItemId);
        QVERIFY2(initialAdd, qPrintable(msg + " - Failed: Initial Add of ALONE piece not recorded."));
        if (TABLE_ALONE_ID != expectedFinalItemId) { // Only expect remove/add if target is not already ALONE
             QVERIFY2(initialRemove, qPrintable(msg + " - Failed: Removal of initial ALONE piece not recorded."));
             QVERIFY2(finalAdd, qPrintable(msg + " - Failed: Final Add of expected aligned piece not recorded."));
        } else { // If expected is ALONE, then only initialAdd, no remove/add cycle for update.
             QVERIFY2(!initialRemove, qPrintable(msg + " - Failed: Unexpected Removal of ALONE piece."));
             QVERIFY2(!finalAdd, qPrintable(msg + " - Failed: Unexpected Final Add of ALONE piece."));
        }
    };

    RMEPosition target(5,5,0);

    QMap<RMEPosition, uint16_t> northNeighbor;
    northNeighbor[target.translated(0,-1)] = TABLE_ALONE_ID;
    runAlignmentTest(target, northNeighbor, RME::BorderType::TABLE_SOUTH_END);

    QMap<RMEPosition, uint16_t> eastWestNeighbors;
    eastWestNeighbors[target.translated(1,0)] = TABLE_ALONE_ID;
    eastWestNeighbors[target.translated(-1,0)] = TABLE_ALONE_ID;
    runAlignmentTest(target, eastWestNeighbors, RME::BorderType::TABLE_HORIZONTAL);

    QMap<RMEPosition, uint16_t> allCardinalNeighbors;
    allCardinalNeighbors[target.translated(0,-1)] = TABLE_ALONE_ID;
    allCardinalNeighbors[target.translated(0,1)] = TABLE_ALONE_ID;
    allCardinalNeighbors[target.translated(1,0)] = TABLE_ALONE_ID;
    allCardinalNeighbors[target.translated(-1,0)] = TABLE_ALONE_ID;
    // wx: TILE_S | TILE_E | TILE_W | TILE_N (0x5A) maps to TABLE_HORIZONTAL, not VERTICAL.
    // My s_table_types has TILE_S | TILE_E | TILE_W | TILE_N = TABLE_HORIZONTAL
    runAlignmentTest(target, allCardinalNeighbors, RME::BorderType::TABLE_HORIZONTAL);
}

void TestTableBrush::testApply_Erase_RemovesTableAndUpdatesNeighbors() {
    RMEBrushSettings settings; settings.isEraseMode = true;
    RMEPosition targetPos(2,2,0);
    RMEPosition southNeighborPos(2,3,0);
    m_tableBrush->setMaterial(&m_testTableMaterial);

    // Setup: target has a horizontal table, south neighbor has an alone table
    setupTileWithItems(targetPos, 0, {TABLE_HORIZONTAL_ID});
    setupTileWithItems(southNeighborPos, 0, {TABLE_ALONE_ID});

    m_mockController->reset();
    m_tableBrush->apply(m_mockController.get(), targetPos, settings);

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordRemoveItem", TABLE_HORIZONTAL_ID));

    // South neighbor (2,3,0) was TABLE_ALONE. Its North neighbor (target) is now void.
    // Tiledata for south neighbor: TILE_N (0x02). s_table_types[0x02] = TABLE_SOUTH_END.
    // So, TABLE_ALONE should be removed, TABLE_S_END_ID should be added.
    QVERIFY(m_mockController->wasMethodCalledForPos(southNeighborPos, "recordRemoveItem", TABLE_ALONE_ID));
    QVERIFY(m_mockController->wasMethodCalledForPos(southNeighborPos, "recordAddItem", TABLE_S_END_ID));

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "notifyTileChanged"));
    QVERIFY(m_mockController->wasMethodCalledForPos(southNeighborPos, "notifyTileChanged"));
}


QTEST_MAIN(TestTableBrush)
#include "TestTableBrush.moc"
