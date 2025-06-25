#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr
#include <vector> // For std::vector, if needed
#include <algorithm> // For std::sort, if needed

// Forward declare or include necessary classes
#include "core/brush/WallBrush.h"
#include "core/brush/BrushSettings.h"
#include "core/map/Map.h"
#include "core/assets/MaterialData.h"
#include "core/settings/AppSettings.h"
#include "core/Position.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h" // For ItemData flags like isWall

#include "tests/core/brush/MockEditorController.h"
#include "tests/core/assets/MockAssetManager.h"
#include "tests/core/assets/MockMaterialManager.h"
#include "tests/core/assets/MockCreatureDatabase.h"
#include "tests/core/MockItemTypeProvider.h"

// Using declarations for brevity
using RMEPosition = RME::core::Position;
using RMEBrushSettings = RME::core::BrushSettings;
using RMEMaterialData = RME::core::assets::MaterialData;
using RMEMaterialWallSpecifics = RME::core::assets::MaterialWallSpecifics;
using RMEMaterialWallPart = RME::core::assets::MaterialWallPart;
using RMEMaterialItemEntry = RME::core::assets::MaterialItemEntry;
using RMEMaterialDoorDefinition = RME::core::assets::MaterialDoorDefinition;
using RMEMap = RME::core::map::Map;
using RMETile = RME::core::Tile;
using RMEAppSettings = RME::core::AppSettings;
using RMEWallBrush = RME::core::WallBrush;
using RMEMockEditorController = MockEditorController;
using RMEMockAssetManager = RME::tests::MockAssetManager;
using RMEMockMaterialManager = RME::tests::MockMaterialManager;
using RMEMockItemTypeProvider = RME::tests::MockItemTypeProvider;
using RMEMockCreatureDatabase = RME::tests::MockCreatureDatabase;
using RMEBorderType = RME::BorderType;
using RMEItem = RME::core::Item;


// Wall tile data bit constants (mirroring conceptual WALLTILE_N_BIT etc.)
static constexpr uint8_t TEST_WALL_N_BIT = (1 << 0);
static constexpr uint8_t TEST_WALL_W_BIT = (1 << 1);
static constexpr uint8_t TEST_WALL_E_BIT = (1 << 2);
static constexpr uint8_t TEST_WALL_S_BIT = (1 << 3);


class TestWallBrush : public QObject {
    Q_OBJECT

public:
    TestWallBrush() = default;
    ~TestWallBrush() = default;

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testStaticWallTypesInitialization();
    void testWallSegmentTypeToOrientationString();
    void testGetItemIdForSegment_Solid();
    void testGetItemIdForSegment_DoorWindow();
    void testApply_Draw_EmptyTile_PlacesPole();
    void testApply_Draw_WithNeighbors_AlignsWalls();
    void testApply_Draw_PlacesDoorOrWindow();
    void testApply_Erase_RemovesWallAndUpdatesNeighbors();

private:
    std::unique_ptr<RMEWallBrush> m_wallBrush;
    std::unique_ptr<RMEMockEditorController> m_mockController;
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEMockItemTypeProvider> m_mockItemProvider;
    std::unique_ptr<RMEMockCreatureDatabase> m_mockCreatureDb;
    std::unique_ptr<RMEMockMaterialManager> m_mockMaterialMgr;
    std::unique_ptr<RMEMockAssetManager> m_mockAssetMgr;
    std::unique_ptr<RMEAppSettings> m_appSettings;
    std::unique_ptr<RMEBrushSettings> m_brushSettings;


    RMEMaterialData m_testWallMaterial;
    // Define item IDs for wall segments, doors, windows
    const uint16_t WALL_POLE_ID = 501;
    const uint16_t WALL_HORIZONTAL_ID = 502;
    const uint16_t WALL_VERTICAL_ID = 503;
    const uint16_t WALL_N_END_ID = 504;
    const uint16_t WALL_E_END_ID = 505;
    const uint16_t WALL_S_END_ID = 506;
    const uint16_t WALL_W_END_ID = 507;
    const uint16_t WALL_N_T_ID = 508;
    const uint16_t WALL_E_T_ID = 509;
    const uint16_t WALL_S_T_ID = 510;
    const uint16_t WALL_W_T_ID = 511;
    const uint16_t WALL_INTERSECTION_ID = 512;
    const uint16_t WALL_NW_DIAG_ID = 513;
    const uint16_t WALL_NE_DIAG_ID = 514;
    const uint16_t WALL_SW_DIAG_ID = 515;
    const uint16_t WALL_SE_DIAG_ID = 516;
    const uint16_t WALL_UNTOUCHABLE_ID = 517;


    const uint16_t WALL_NORMAL_DOOR_ID = 550;
    const uint16_t WALL_WINDOW_ID = 551;
    const uint16_t WALL_QUEST_DOOR_ID = 552;

    void setupTileWithWall(const RMEPosition& pos, uint16_t itemId, const QString& materialId = QStringLiteral("test_wall")) {
        RMETile* tile = m_map->getTileForEditing(pos);
        QVERIFY(tile);

        const RME::core::assets::ItemData* itemData = m_mockItemProvider->getItemData(itemId);
        if (!itemData) {
            RME::tests::MockItemTypeProvider::MockItemDetails details;
            details.name = QString("TestWallItem %1").arg(itemId);
            details.id = itemId;
            details.isGround = false;
            details.isStackable = false;
            details.materialId = materialId;
            details.isWall = true;
            details.isDoor = (itemId == WALL_NORMAL_DOOR_ID || itemId == WALL_QUEST_DOOR_ID);
            details.isWindow = (itemId == WALL_WINDOW_ID);
            m_mockItemProvider->setMockData(itemId, details);
            itemData = m_mockItemProvider->getItemData(itemId);
        }
        QVERIFY(itemData);

        tile->addItem(std::make_unique<RME::core::Item>(itemId, itemData));
    }
};

void TestWallBrush::initTestCase() {
    m_testWallMaterial.id = "test_wall";
    m_testWallMaterial.typeAttribute = "wall";
    RMEMaterialWallSpecifics wallSpecifics;

    auto addPart = [&](const QString& orient, uint16_t id, const QList<RMEMaterialDoorDefinition>& doors = {}) {
        RMEMaterialWallPart part;
        part.orientationType = orient;
        part.items.append({id, 100});
        part.doors = doors;
        wallSpecifics.parts.append(part);
    };

    addPart("pole", WALL_POLE_ID);
    addPart("horizontal", WALL_HORIZONTAL_ID, {{WALL_NORMAL_DOOR_ID, "normal", true, false}, {WALL_WINDOW_ID, "window", true, false}, {WALL_QUEST_DOOR_ID, "quest", false, true}});
    addPart("vertical", WALL_VERTICAL_ID);
    addPart("north_end", WALL_N_END_ID);
    addPart("east_end", WALL_E_END_ID);
    addPart("south_end", WALL_S_END_ID);
    addPart("west_end", WALL_W_END_ID);
    addPart("north_t", WALL_N_T_ID);
    addPart("east_t", WALL_E_T_ID);
    addPart("south_t", WALL_S_T_ID);
    addPart("west_t", WALL_W_T_ID);
    addPart("intersection", WALL_INTERSECTION_ID);
    addPart("northwest_diagonal", WALL_NW_DIAG_ID);
    addPart("northeast_diagonal", WALL_NE_DIAG_ID);
    addPart("southwest_diagonal", WALL_SW_DIAG_ID);
    addPart("southeast_diagonal", WALL_SE_DIAG_ID);
    addPart("untouchable", WALL_UNTOUCHABLE_ID);
    addPart("corner", WALL_NW_DIAG_ID); // Fallback for diagonals if specific not found

    m_testWallMaterial.specificData = wallSpecifics;

    RMEWallBrush::initializeStaticData();
}

void TestWallBrush::init() {
    m_wallBrush = std::make_unique<RMEWallBrush>();
    m_mockController = std::make_unique<RMEMockEditorController>();

    m_mockItemProvider = std::make_unique<RMEMockItemTypeProvider>();
    auto mockWallItem = [&](uint16_t id, const QString& name, bool isDoor = false, bool isWindow = false){
        m_mockItemProvider->setMockData(id, { name, id, false, false, QString("test_wall"), true, isDoor, isWindow });
    };

    mockWallItem(WALL_POLE_ID, "Wall Pole");
    mockWallItem(WALL_HORIZONTAL_ID, "Wall Horizontal");
    mockWallItem(WALL_VERTICAL_ID, "Wall Vertical");
    mockWallItem(WALL_N_END_ID, "Wall N-End");
    mockWallItem(WALL_E_END_ID, "Wall E-End");
    mockWallItem(WALL_S_END_ID, "Wall S-End");
    mockWallItem(WALL_W_END_ID, "Wall W-End");
    mockWallItem(WALL_N_T_ID, "Wall N-T");
    mockWallItem(WALL_E_T_ID, "Wall E-T");
    mockWallItem(WALL_S_T_ID, "Wall S-T");
    mockWallItem(WALL_W_T_ID, "Wall W-T");
    mockWallItem(WALL_INTERSECTION_ID, "Wall Intersection");
    mockWallItem(WALL_NW_DIAG_ID, "Wall NW-Diag");
    mockWallItem(WALL_NE_DIAG_ID, "Wall NE-Diag");
    mockWallItem(WALL_SW_DIAG_ID, "Wall SW-Diag");
    mockWallItem(WALL_SE_DIAG_ID, "Wall SE-Diag");
    mockWallItem(WALL_UNTOUCHABLE_ID, "Wall Untouchable");
    mockWallItem(WALL_NORMAL_DOOR_ID, "Normal Door", true);
    mockWallItem(WALL_WINDOW_ID, "Window", false, true);
    mockWallItem(WALL_QUEST_DOOR_ID, "Quest Door", true);

    m_mockCreatureDb = std::make_unique<RMEMockCreatureDatabase>();
    m_mockMaterialMgr = std::make_unique<RMEMockMaterialManager>();
    m_mockMaterialMgr->addMaterial(m_testWallMaterial);

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

    m_wallBrush->setMaterial(m_mockMaterialMgr->getMaterial("test_wall"));
    m_mockController->reset();
}

void TestWallBrush::cleanup() {}
void TestWallBrush::cleanupTestCase() {}

void TestWallBrush::testStaticWallTypesInitialization() {
    QCOMPARE(RMEWallBrush::s_full_wall_types[0], static_cast<uint32_t>(RME::BorderType::WALL_POLE));
    QCOMPARE(RMEWallBrush::s_full_wall_types[TEST_WALL_N_BIT], static_cast<uint32_t>(RME::BorderType::WALL_SOUTH_END));
    QCOMPARE(RMEWallBrush::s_full_wall_types[TEST_WALL_E_BIT | TEST_WALL_W_BIT], static_cast<uint32_t>(RME::BorderType::WALL_HORIZONTAL));
    QCOMPARE(RMEWallBrush::s_full_wall_types[TEST_WALL_S_BIT | TEST_WALL_E_BIT | TEST_WALL_W_BIT | TEST_WALL_N_BIT], static_cast<uint32_t>(RME::BorderType::WALL_INTERSECTION));

    QCOMPARE(RMEWallBrush::s_half_wall_types[0], static_cast<uint32_t>(RME::BorderType::WALL_POLE));
    QCOMPARE(RMEWallBrush::s_half_wall_types[TEST_WALL_N_BIT], static_cast<uint32_t>(RME::BorderType::WALL_VERTICAL));
    QCOMPARE(RMEWallBrush::s_half_wall_types[TEST_WALL_E_BIT | TEST_WALL_W_BIT], static_cast<uint32_t>(RME::BorderType::WALL_HORIZONTAL));
}

void TestWallBrush::testWallSegmentTypeToOrientationString() {
    QVERIFY(m_wallBrush);
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_POLE), QStringLiteral("pole"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_VERTICAL), QStringLiteral("vertical"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_HORIZONTAL), QStringLiteral("horizontal"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_NORTH_END), QStringLiteral("north_end"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_EAST_END), QStringLiteral("east_end"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_SOUTH_END), QStringLiteral("south_end"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_WEST_END), QStringLiteral("west_end"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_NORTH_T), QStringLiteral("north_t"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_EAST_T), QStringLiteral("east_t"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_SOUTH_T), QStringLiteral("south_t"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_WEST_T), QStringLiteral("west_t"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_INTERSECTION), QStringLiteral("intersection"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_NORTHWEST_DIAGONAL), QStringLiteral("northwest_diagonal"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_NORTHEAST_DIAGONAL), QStringLiteral("northeast_diagonal"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_SOUTHWEST_DIAGONAL), QStringLiteral("southwest_diagonal"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_SOUTHEAST_DIAGONAL), QStringLiteral("southeast_diagonal"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::WALL_UNTOUCHABLE), QStringLiteral("untouchable"));
    QCOMPARE(m_wallBrush->wallSegmentTypeToOrientationString(RME::BorderType::NONE), QStringLiteral("pole"));
}

void TestWallBrush::testGetItemIdForSegment_Solid() {
    QVERIFY(m_wallBrush);
    const auto* specifics = m_wallBrush->getCurrentWallSpecifics();
    QVERIFY(specifics);
    RMEBrushSettings settings;

    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_POLE, settings, specifics), WALL_POLE_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_HORIZONTAL, settings, specifics), WALL_HORIZONTAL_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_VERTICAL, settings, specifics), WALL_VERTICAL_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_NORTH_END, settings, specifics), WALL_N_END_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_EAST_END, settings, specifics), WALL_E_END_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_SOUTH_END, settings, specifics), WALL_S_END_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_WEST_END, settings, specifics), WALL_W_END_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_NORTH_T, settings, specifics), WALL_N_T_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_EAST_T, settings, specifics), WALL_E_T_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_SOUTH_T, settings, specifics), WALL_S_T_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_WEST_T, settings, specifics), WALL_W_T_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_INTERSECTION, settings, specifics), WALL_INTERSECTION_ID);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_NORTHWEST_DIAGONAL, settings, specifics), WALL_NW_DIAG_ID);

    QCOMPARE(m_wallBrush->getItemIdForSegment(static_cast<RME::BorderType>(99), settings, specifics), static_cast<uint16_t>(0)); // Undefined segment, no "corner" fallback defined for this
}

void TestWallBrush::testGetItemIdForSegment_DoorWindow() {
    QVERIFY(m_wallBrush);
    const auto* specifics = m_wallBrush->getCurrentWallSpecifics();
    QVERIFY(specifics);

    RMEBrushSettings settings_place_door;
    settings_place_door.setGenericBrushParameter("placeDoor", true);
    settings_place_door.setGenericBrushParameter("doorType", "normal");

    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_HORIZONTAL, settings_place_door, specifics), WALL_NORMAL_DOOR_ID);

    RMEBrushSettings settings_place_window;
    settings_place_window.setGenericBrushParameter("placeWindow", true);
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_HORIZONTAL, settings_place_window, specifics), WALL_WINDOW_ID);

    RMEBrushSettings settings_place_quest_door;
    settings_place_quest_door.setGenericBrushParameter("placeDoor", true);
    settings_place_quest_door.setGenericBrushParameter("doorType", "quest");
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_HORIZONTAL, settings_place_quest_door, specifics), WALL_QUEST_DOOR_ID);

    // Test asking for a door in a part that has no doors defined (e.g., "pole" part)
    QCOMPARE(m_wallBrush->getItemIdForSegment(RME::BorderType::WALL_POLE, settings_place_door, specifics), WALL_POLE_ID);
}

void TestWallBrush::testApply_Draw_EmptyTile_PlacesPole() {
    RMEBrushSettings settings; settings.isEraseMode = false;
    RMEPosition pos(1, 1, 0);
    m_map->clear();
    m_wallBrush->setMaterial(&m_testWallMaterial);
    m_mockController->reset();

    m_wallBrush->apply(m_mockController.get(), pos, settings);

    // Expected sequence on targetPos (pos):
    // 1. apply() places WALL_POLE_ID
    // 2. updateWallAppearance(pos) called. Tiledata=0 (empty neighbors). Segment=WALL_POLE. NewID=WALL_POLE_ID.
    //    Since old (initial) is WALL_POLE_ID and new is WALL_POLE_ID, no recordRemove/recordAdd from update itself.

    QVERIFY(m_mockController->wasMethodCalledForPos(pos, "recordAddItem", WALL_POLE_ID));

    int addItemCountForTarget = 0;
    int removeItemCountForTarget = 0;
    bool initialPoleAddVerified = false;

    for(const auto& call : m_mockController->calls) {
        if (call.pos == pos) {
            if(call.method == "recordAddItem") {
                if (addItemCountForTarget == 0 && call.itemId == WALL_POLE_ID) {
                    initialPoleAddVerified = true;
                }
                addItemCountForTarget++;
            }
            if(call.method == "recordRemoveItem") removeItemCountForTarget++;
        }
    }
    QVERIFY(initialPoleAddVerified);
    QCOMPARE(addItemCountForTarget, 1); // Only the initial add by apply()
    QCOMPARE(removeItemCountForTarget, 0); // No change by updateWallAppearance on the target tile
}

void TestWallBrush::testApply_Draw_WithNeighbors_AlignsWalls() {
    QVERIFY(m_wallBrush);
    QVERIFY(m_mockController);
    QVERIFY(m_map);
    m_wallBrush->setMaterial(&m_testWallMaterial);

    const auto* wallSpecifics = m_wallBrush->getCurrentWallSpecifics();
    QVERIFY(wallSpecifics);

    auto runAlignmentTest = [&](const RMEPosition& targetPos,
                                const QMap<RMEPosition, uint16_t>& neighborItemIds, // Item IDs for neighbors (of same material)
                                uint8_t expectedTileData, // Expected tiledata for target based on neighbors
                                RME::BorderType expectedSegmentTypeOnTarget) {
        m_map->clear();
        for (auto it = neighborItemIds.constBegin(); it != neighborItemIds.constEnd(); ++it) {
            setupTileWithWall(it.key(), it.value());
        }

        m_mockController->reset();
        RMEBrushSettings settings; settings.isEraseMode = false;
        m_wallBrush->apply(m_mockController.get(), targetPos, settings);

        QString expectedAlignStr = m_wallBrush->wallSegmentTypeToOrientationString(expectedSegmentTypeOnTarget);
        uint16_t expectedFinalItemId = m_wallBrush->getItemIdForSegment(expectedSegmentTypeOnTarget, settings, wallSpecifics);
        QVERIFY(expectedFinalItemId != 0);

        bool initialAdd = false;
        bool initialRemove = false;
        bool finalAdd = false;
        int targetRelevantCallIndex = 0;

        for(const auto& call : m_mockController->calls) {
            if (call.pos == targetPos) {
                targetRelevantCallIndex++;
                if (targetRelevantCallIndex == 1 && call.method == "recordAddItem" && call.itemId == WALL_POLE_ID) initialAdd = true;
                // If expected is POLE, then initialRemove and finalAdd won't happen for target tile update.
                if (WALL_POLE_ID != expectedFinalItemId) {
                    if (targetRelevantCallIndex == 2 && call.method == "recordRemoveItem" && call.itemId == WALL_POLE_ID) initialRemove = true;
                    if (targetRelevantCallIndex == 3 && call.method == "recordAddItem" && call.itemId == expectedFinalItemId) finalAdd = true;
                }
            }
        }

        QString msg = QString("Test for target segment %1 (item %2), tiledata 0x%3").arg(expectedAlignStr).arg(expectedFinalItemId).arg(expectedTileData, 1, 16);
        QVERIFY2(initialAdd, qPrintable(msg + " - Failed: Initial Add of POLE piece not recorded."));
        if (WALL_POLE_ID != expectedFinalItemId) {
             QVERIFY2(initialRemove, qPrintable(msg + " - Failed: Removal of initial POLE piece not recorded."));
             QVERIFY2(finalAdd, qPrintable(msg + " - Failed: Final Add of expected aligned piece not recorded."));
        } else {
             QVERIFY2(targetRelevantCallIndex == 1, qPrintable(msg + " - Failed: Expected only initial add for POLE."));
        }
    };

    RMEPosition target(5,5,0);

    runAlignmentTest(target, {}, 0, RME::BorderType::WALL_POLE); // Pole

    QMap<RMEPosition, uint16_t> n_neighbor; n_neighbor[target.translated(0,-1)] = WALL_POLE_ID;
    runAlignmentTest(target, n_neighbor, TEST_WALL_N_BIT, RME::BorderType::WALL_SOUTH_END);

    QMap<RMEPosition, uint16_t> ew_neighbors; ew_neighbors[target.translated(1,0)] = WALL_POLE_ID; ew_neighbors[target.translated(-1,0)] = WALL_POLE_ID;
    runAlignmentTest(target, ew_neighbors, TEST_WALL_E_BIT | TEST_WALL_W_BIT, RME::BorderType::WALL_HORIZONTAL);

    QMap<RMEPosition, uint16_t> all_cardinal_neighbors;
    all_cardinal_neighbors[target.translated(0,-1)] = WALL_POLE_ID; // N
    all_cardinal_neighbors[target.translated(0,1)] = WALL_POLE_ID;  // S
    all_cardinal_neighbors[target.translated(1,0)] = WALL_POLE_ID;  // E
    all_cardinal_neighbors[target.translated(-1,0)] = WALL_POLE_ID; // W
    runAlignmentTest(target, all_cardinal_neighbors, TEST_WALL_N_BIT | TEST_WALL_S_BIT | TEST_WALL_E_BIT | TEST_WALL_W_BIT, RME::BorderType::WALL_INTERSECTION);

    QMap<RMEPosition, uint16_t> n_e_w_neighbors; // South T
    n_e_w_neighbors[target.translated(0,-1)] = WALL_POLE_ID; // N
    n_e_w_neighbors[target.translated(1,0)] = WALL_POLE_ID;  // E
    n_e_w_neighbors[target.translated(-1,0)] = WALL_POLE_ID; // W
    runAlignmentTest(target, n_e_w_neighbors, TEST_WALL_N_BIT | TEST_WALL_E_BIT | TEST_WALL_W_BIT, RME::BorderType::WALL_SOUTH_T);

    QMap<RMEPosition, uint16_t> n_w_neighbors; // SE Diagonal (Corner)
    n_w_neighbors[target.translated(0,-1)] = WALL_POLE_ID; // N
    n_w_neighbors[target.translated(-1,0)] = WALL_POLE_ID; // W
    runAlignmentTest(target, n_w_neighbors, TEST_WALL_N_BIT | TEST_WALL_W_BIT, RME::BorderType::WALL_SOUTHEAST_DIAGONAL);
}

void TestWallBrush::testApply_Draw_PlacesDoorOrWindow() {
    QVERIFY(m_wallBrush);
    const auto* specifics = m_wallBrush->getCurrentWallSpecifics();
    QVERIFY(specifics);
    RMEPosition targetPos(5,5,0);
    m_map->clear();
    m_wallBrush->setMaterial(&m_testWallMaterial);

    // Setup neighbors to form a horizontal wall segment at targetPos
    QMap<RMEPosition, uint16_t> ew_neighbors;
    ew_neighbors[targetPos.translated(1,0)] = WALL_POLE_ID;  // East neighbor
    ew_neighbors[targetPos.translated(-1,0)] = WALL_POLE_ID; // West neighbor
    for (auto it = ew_neighbors.constBegin(); it != ew_neighbors.constEnd(); ++it) {
        setupTileWithWall(it.key(), it.value());
    }

    // Scenario 4.1: Place Door
    m_brushSettings->setGenericBrushParameter("placeDoor", true);
    m_brushSettings->setGenericBrushParameter("doorType", "normal");
    m_mockController->reset();
    m_wallBrush->apply(m_mockController.get(), targetPos, *m_brushSettings.get());

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordAddItem", WALL_NORMAL_DOOR_ID));
    // Verify that initial pole was added, then removed, then door added
    QVERIFY(m_mockController->findCallSequence(targetPos, {"recordAddItem", "recordRemoveItem", "recordAddItem"}, {WALL_POLE_ID, WALL_POLE_ID, WALL_NORMAL_DOOR_ID}));


    // Scenario 4.2: Place Window
    m_brushSettings->setGenericBrushParameter("placeDoor", false);
    m_brushSettings->setGenericBrushParameter("placeWindow", true);
    m_map->clearTile(targetPos); // Clear target tile from previous test part
    for (auto it = ew_neighbors.constBegin(); it != ew_neighbors.constEnd(); ++it) { // Re-setup neighbors
        setupTileWithWall(it.key(), it.value());
    }
    m_mockController->reset();
    m_wallBrush->apply(m_mockController.get(), targetPos, *m_brushSettings.get());

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordAddItem", WALL_WINDOW_ID));
    QVERIFY(m_mockController->findCallSequence(targetPos, {"recordAddItem", "recordRemoveItem", "recordAddItem"}, {WALL_POLE_ID, WALL_POLE_ID, WALL_WINDOW_ID}));

    // Reset brush settings
    m_brushSettings->setGenericBrushParameter("placeWindow", false);
}

void TestWallBrush::testApply_Erase_RemovesWallAndUpdatesNeighbors() {
    RMEBrushSettings settings; settings.isEraseMode = true;
    RMEPosition targetPos(2,2,0);
    RMEPosition westNeighborPos(1,2,0);
    m_wallBrush->setMaterial(&m_testWallMaterial);
    m_map->clear();

    // Setup: Target has WALL_HORIZONTAL_ID. West neighbor has WALL_POLE_ID.
    setupTileWithWall(targetPos, WALL_HORIZONTAL_ID);
    setupTileWithWall(westNeighborPos, WALL_POLE_ID);

    m_mockController->reset();
    m_wallBrush->apply(m_mockController.get(), targetPos, settings);

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "recordRemoveItem", WALL_HORIZONTAL_ID));

    // West neighbor (1,2,0) was WALL_POLE_ID. Its East neighbor (targetPos) is now void.
    // Tiledata for West neighbor becomes TEST_WALL_E_BIT (0x04).
    // s_full_wall_types[0x04] = WALL_WEST_END.
    QVERIFY(m_mockController->wasMethodCalledForPos(westNeighborPos, "recordRemoveItem", WALL_POLE_ID));
    QVERIFY(m_mockController->wasMethodCalledForPos(westNeighborPos, "recordAddItem", WALL_W_END_ID));

    QVERIFY(m_mockController->wasMethodCalledForPos(targetPos, "notifyTileChanged"));
    QVERIFY(m_mockController->wasMethodCalledForPos(westNeighborPos, "notifyTileChanged"));
}


QTEST_MAIN(TestWallBrush)
#include "TestWallBrush.moc" // Will be generated by AUTOMOC
