#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr
#include <QList>
#include <QString>

#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Position.h"
// Minimal includes for Map constructor context (AssetManager and its dependencies)
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"

// Using declarations
using RMEWpMgr = RME::core::waypoints::WaypointManager;
using RMEWp = RME::core::waypoints::Waypoint;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEPos = RME::core::Position;

class TestWaypointManager : public QObject {
    Q_OBJECT

public:
    TestWaypointManager() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testAddWaypoint_New();
    void testAddWaypoint_ReplaceExisting();
    void testAddWaypoint_EmptyName_Fails();
    void testGetWaypointByName_Found_CaseInsensitive();
    void testGetWaypointByName_NotFound();
    void testGetWaypointsAt_FoundOne();
    void testGetWaypointsAt_FoundMultiple();
    void testGetWaypointsAt_NotFound();
    void testRemoveWaypoint_Existing();
    void testRemoveWaypoint_NonExisting();
    void testGetAllWaypoints_EmptyAndPopulated();
    void testClearAllWaypoints();
    void testTileWaypointCount_AddRemoveClear();

private:
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEWpMgr> m_waypointManager;

    // Asset related members for Map construction
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestWaypointManager::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    // No specific items needed for waypoint tests, empty ItemDatabase is fine.
    // However, Item::setItemDatabase should be called if Tile objects create default ground items.
    // For these tests, assume Tiles are created empty or map doesn't auto-populate them with items.
    // RME::core::Item::setItemDatabase(m_itemDatabase.get()); // If needed for Tile construction

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(50, 50, 1, m_assetManager.get());
    m_waypointManager = std::make_unique<RMEWpMgr>(m_map.get());

    // Ensure map has some tiles for testing waypoint counts
    // Tile constructor requires IItemTypeProvider (passed via AssetManager to Map, then to Tile)
    // This is okay as AssetManager is provided to Map.
    m_map->getOrCreateTile(RMEPos(10,10,7));
    m_map->getOrCreateTile(RMEPos(10,11,7));
}

void TestWaypointManager::cleanup() {
    m_waypointManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    // RME::core::Item::setItemDatabase(nullptr); // If set in init()
}

void TestWaypointManager::testAddWaypoint_New() {
    RMEPos pos(10,10,7);
    bool added = m_waypointManager->addWaypoint("TestWp1", pos);
    QVERIFY(added);
    RMEWp* wp = m_waypointManager->getWaypointByName("testwp1");
    QVERIFY(wp != nullptr);
    if(wp) { // QtTest already checks for null, but for safety in direct comparisons
        QCOMPARE(wp->m_name, QString("TestWp1"));
        QCOMPARE(wp->m_position, pos);
    }
}

void TestWaypointManager::testAddWaypoint_ReplaceExisting() {
    RMEPos pos1(10,10,7);
    RMEPos pos2(10,11,7);
    m_waypointManager->addWaypoint("TestWp_Replace", pos1);
    RMEWp* wp1 = m_waypointManager->getWaypointByName("testwp_replace");
    QVERIFY(wp1 && wp1->m_position == pos1);

    bool replaced = m_waypointManager->addWaypoint("TESTWP_REPLACE", pos2); // Same normalized name, different pos
    QVERIFY(replaced);

    QCOMPARE(m_waypointManager->getAllWaypoints().size(), 1);
    RMEWp* wp2 = m_waypointManager->getWaypointByName("testwp_replace");
    QVERIFY(wp2 != nullptr);
    if(wp2) {
        QCOMPARE(wp2->m_name, QString("TESTWP_REPLACE"));
        QCOMPARE(wp2->m_position, pos2);
    }
}

void TestWaypointManager::testAddWaypoint_EmptyName_Fails() {
    bool added = m_waypointManager->addWaypoint("   ", RMEPos(10,10,7));
    QVERIFY(!added);
    QVERIFY(m_waypointManager->getAllWaypoints().isEmpty());
}

void TestWaypointManager::testGetWaypointByName_Found_CaseInsensitive() {
    m_waypointManager->addWaypoint("MyWaypoint", RMEPos(10,10,7));
    RMEWp* wp = m_waypointManager->getWaypointByName("mywaypoint");
    QVERIFY(wp != nullptr);
    RMEWp* wp_caps = m_waypointManager->getWaypointByName("MYWAYPOINT");
    QVERIFY(wp_caps != nullptr);
    QCOMPARE(wp, wp_caps);
}

void TestWaypointManager::testGetWaypointByName_NotFound() {
    QVERIFY(m_waypointManager->getWaypointByName("NonExistent") == nullptr);
}

void TestWaypointManager::testGetWaypointsAt_FoundOne() {
    RMEPos pos(10,10,7);
    m_waypointManager->addWaypoint("WpAtPos1", pos);
    QList<RMEWp*> wps = m_waypointManager->getWaypointsAt(pos);
    QCOMPARE(wps.size(), 1);
    QVERIFY(wps.contains(m_waypointManager->getWaypointByName("WpAtPos1")));
}

void TestWaypointManager::testGetWaypointsAt_FoundMultiple() {
    RMEPos pos(10,10,7);
    m_waypointManager->addWaypoint("WpMulti1", pos);
    m_waypointManager->addWaypoint("WpMulti2", pos);
    QList<RMEWp*> wps = m_waypointManager->getWaypointsAt(pos);
    QCOMPARE(wps.size(), 2);
    QVERIFY(wps.contains(m_waypointManager->getWaypointByName("WpMulti1")));
    QVERIFY(wps.contains(m_waypointManager->getWaypointByName("WpMulti2")));
}

void TestWaypointManager::testGetWaypointsAt_NotFound() {
    QVERIFY(m_waypointManager->getWaypointsAt(RMEPos(20,20,7)).isEmpty());
}

void TestWaypointManager::testRemoveWaypoint_Existing() {
    m_waypointManager->addWaypoint("ToRemove", RMEPos(10,10,7));
    QVERIFY(m_waypointManager->getWaypointByName("toremove") != nullptr);
    bool removed = m_waypointManager->removeWaypoint("TORemove");
    QVERIFY(removed);
    QVERIFY(m_waypointManager->getWaypointByName("toremove") == nullptr);
    QVERIFY(m_waypointManager->getAllWaypoints().isEmpty());
}

void TestWaypointManager::testRemoveWaypoint_NonExisting() {
    bool removed = m_waypointManager->removeWaypoint("DoesNotExist");
    QVERIFY(!removed);
}

void TestWaypointManager::testGetAllWaypoints_EmptyAndPopulated() {
    QVERIFY(m_waypointManager->getAllWaypoints().isEmpty());
    m_waypointManager->addWaypoint("Wp1", RMEPos(10,10,7));
    m_waypointManager->addWaypoint("Wp2", RMEPos(10,11,7));
    QList<RMEWp*> allWps = m_waypointManager->getAllWaypoints();
    QCOMPARE(allWps.size(), 2);
    bool foundWp1 = false, foundWp2 = false;
    for(RMEWp* wp : allWps) {
        if (wp->m_name == "Wp1") foundWp1 = true;
        if (wp->m_name == "Wp2") foundWp2 = true;
    }
    QVERIFY(foundWp1 && foundWp2);
}

void TestWaypointManager::testClearAllWaypoints() {
    m_waypointManager->addWaypoint("WpToClear1", RMEPos(10,10,7));
    m_waypointManager->addWaypoint("WpToClear2", RMEPos(10,11,7));
    QCOMPARE(m_waypointManager->getAllWaypoints().size(), 2);
    m_waypointManager->clearAllWaypoints();
    QVERIFY(m_waypointManager->getAllWaypoints().isEmpty());
}

void TestWaypointManager::testTileWaypointCount_AddRemoveClear() {
    RMEPos pos1(10,10,7);
    RMEPos pos2(10,11,7);
    RMETile* tile1 = m_map->getTile(pos1); // Tiles created in init()
    RMETile* tile2 = m_map->getTile(pos2);
    QVERIFY(tile1 && tile2);
    QCOMPARE(tile1->getWaypointCount(), 0);
    QCOMPARE(tile2->getWaypointCount(), 0);

    m_waypointManager->addWaypoint("Wp_T1_1", pos1);
    QCOMPARE(tile1->getWaypointCount(), 1);
    QCOMPARE(tile2->getWaypointCount(), 0);

    m_waypointManager->addWaypoint("Wp_T1_2", pos1);
    QCOMPARE(tile1->getWaypointCount(), 2);

    m_waypointManager->addWaypoint("Wp_T2_1", pos2);
    QCOMPARE(tile1->getWaypointCount(), 2);
    QCOMPARE(tile2->getWaypointCount(), 1);

    m_waypointManager->addWaypoint("Wp_T1_1", pos2); // This replaces old Wp_T1_1 on pos1
    QCOMPARE(tile1->getWaypointCount(), 1); // Wp_T1_2 still on tile1
    QCOMPARE(tile2->getWaypointCount(), 2); // Wp_T2_1 and new Wp_T1_1 now on tile2

    m_waypointManager->removeWaypoint("Wp_T1_2");
    QCOMPARE(tile1->getWaypointCount(), 0);
    QCOMPARE(tile2->getWaypointCount(), 2);

    m_waypointManager->clearAllWaypoints();
    QCOMPARE(tile1->getWaypointCount(), 0);
    QCOMPARE(tile2->getWaypointCount(), 0);
}

QTEST_MAIN(TestWaypointManager)
#include "TestWaypointManager.moc"
