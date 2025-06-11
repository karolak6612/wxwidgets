#include <QtTest/QtTest>
#include <QDebug>

#include "core/map/MapElements.h" // For WaypointData
#include "core/Position.h"
#include "core/map/Map.h"             // For Map integration tests
#include "core/assets/AssetManager.h" // For Map constructor
#include "core/settings/AppSettings.h"// For Map constructor (potentially)
#include "core/IItemTypeProvider.h"   // For MockItemTypeProvider
#include "core/assets/ItemDatabase.h" // For mock setup

// Using namespace for convenience in test file
using namespace RME::core;       // For Position
using namespace RME::core::map;  // For Map, WaypointData (MapElements.h is in RME namespace, not RME::core::map)
                                 // Correcting WaypointData namespace if it's just RME::WaypointData
using namespace RME;             // For WaypointData if it's directly in RME namespace

// Copied MockItemTypeProvider (ideally from a shared test utility)
class MockItemTypeProvider : public RME::core::IItemTypeProvider {
public:
    MockItemTypeProvider() {
        m_itemTypes[1] = std::make_unique<RME::core::assets::ItemType>(1, "Test Ground Item");
        m_itemTypes[1]->isGround = true;
    }
    QString getName(uint16_t id) const override { Q_UNUSED(id); return "Mock Item"; }
    QString getDescription(uint16_t id) const override { Q_UNUSED(id); return "Mock Description"; }
    double getWeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1.0; }
    bool isBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isProjectileBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isPathBlocking(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isWalkable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool isStackable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isGround(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isGround : false; }
    bool isAlwaysOnTop(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isReadable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isWriteable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isFluidContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isSplash(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isMoveable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool hasHeight(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isTeleport(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDoor(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isPodium(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDepot(uint16_t id) const override { Q_UNUSED(id); return false; }
    int getSpriteX(uint16_t id, uint16_t subtype, int frame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(frame); return 0; }
    int getSpriteY(uint16_t id, uint16_t subtype, int frame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(frame); return 0; }
    int getSpriteWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteOffsetX(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getSpriteOffsetY(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getAnimationFrames(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1;}
    const RME::core::SpriteSheet* getSpriteSheet(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return nullptr;}
    bool usesAlternativeSpriteSheet(uint16_t id, uint16_t subtype) const override {Q_UNUSED(id); Q_UNUSED(subtype); return false;}
    AssetManager& getAssetManager() override { Q_ASSERT(false); return *static_cast<AssetManager*>(nullptr); }
private:
    std::map<uint16_t, std::unique_ptr<RME::core::assets::ItemType>> m_itemTypes;
};


class TestWaypointData : public QObject
{
    Q_OBJECT

public:
    TestWaypointData();
    ~TestWaypointData() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testWaypointDataConstruction();
    void testWaypointDataConnections();
    void testMapWaypointManagement();

private:
    MockItemTypeProvider* m_mockItemProvider;
    RME::core::assets::AssetManager* m_assetManager;
    RME::core::settings::AppSettings* m_appSettings; // For Map constructor, if needed
    RME::core::map::Map* m_map; // For integration tests
};

TestWaypointData::TestWaypointData() :
    m_mockItemProvider(nullptr), m_assetManager(nullptr), m_appSettings(nullptr), m_map(nullptr)
{}

TestWaypointData::~TestWaypointData() {}

void TestWaypointData::initTestCase() {
    // One-time setup for all tests in this class
    m_appSettings = new RME::core::settings::AppSettings();
    m_mockItemProvider = new MockItemTypeProvider();
    auto itemDb = new RME::core::assets::ItemDatabase(m_mockItemProvider); // ItemDatabase takes ownership
    m_assetManager = new RME::core::assets::AssetManager(itemDb, nullptr, nullptr);
}

void TestWaypointData::cleanupTestCase() {
    // One-time cleanup
    delete m_assetManager; // Deletes itemDb, which deletes m_mockItemProvider
    m_assetManager = nullptr;
    m_mockItemProvider = nullptr;
    delete m_appSettings;
    m_appSettings = nullptr;
}

void TestWaypointData::init() {
    // Per-test setup
    // Map constructor requires IItemTypeProvider (via AssetManager is complex, direct is simpler if Map allows)
    // The Map constructor takes IItemTypeProvider*. ItemDatabase IS an IItemTypeProvider.
    m_map = new RME::core::map::Map(&(m_assetManager->getItemDatabase()));
    m_map->setWidth(100); // Default map size for tests
    m_map->setHeight(100);
    m_map->setDepth(8);
}

void TestWaypointData::cleanup() {
    // Per-test cleanup
    delete m_map;
    m_map = nullptr;
}

void TestWaypointData::testWaypointDataConstruction() {
    WaypointData wp("WP1", {10,20,7});
    QCOMPARE(wp.name, QString("WP1"));
    QCOMPARE(wp.position, Position(10,20,7));
    QVERIFY(wp.getConnections().isEmpty());
}

void TestWaypointData::testWaypointDataConnections() {
    WaypointData wp1("WP1", {10,20,7});
    wp1.addConnection("WP2");
    wp1.addConnection("WP3");

    QVERIFY(wp1.isConnectedTo("WP2"));
    QVERIFY(wp1.isConnectedTo("WP3"));
    QVERIFY(!wp1.isConnectedTo("WP4"));
    QCOMPARE(wp1.getConnections().size(), 2);

    wp1.addConnection("WP2"); // Add duplicate
    QCOMPARE(wp1.getConnections().size(), 2); // Size should not change

    wp1.removeConnection("WP4"); // Remove non-existent
    QCOMPARE(wp1.getConnections().size(), 2);

    wp1.removeConnection("WP2");
    QVERIFY(!wp1.isConnectedTo("WP2"));
    QCOMPARE(wp1.getConnections().size(), 1);
    QVERIFY(wp1.isConnectedTo("WP3")); // Verify WP3 is still there

    wp1.addConnection(""); // Try adding empty string
    QVERIFY(!wp1.isConnectedTo("")); // Should not be added
    QVERIFY(wp1.getConnections().size() == 1); // Size should not change

    wp1.addConnection("WP1"); // Try adding self-connection
    QVERIFY(!wp1.isConnectedTo("WP1")); // Should not be added
    QVERIFY(wp1.getConnections().size() == 1); // Size should not change
}

void TestWaypointData::testMapWaypointManagement() {
    // Map is created in init()
    QVERIFY(m_map != nullptr);

    WaypointData wd1("Start", {1,1,7});
    wd1.addConnection("Mid");

    WaypointData wd2("Mid", {5,5,7});
    wd2.addConnection("Start");
    wd2.addConnection("End");

    // Use QMap interface correctly
    m_map->addWaypoint(wd1); // Assumes addWaypoint copies or moves
    m_map->addWaypoint(wd2);

    QCOMPARE(m_map->getWaypoints().size(), 2);

    const WaypointData* pWd1 = m_map->getWaypoint("Start");
    QVERIFY(pWd1 != nullptr);
    if (pWd1) {
        QCOMPARE(pWd1->position, Position(1,1,7));
        QVERIFY(pWd1->isConnectedTo("Mid"));
        QVERIFY(!pWd1->isConnectedTo("End"));
    }

    const WaypointData* pWdMid = m_map->getWaypoint("Mid");
    QVERIFY(pWdMid != nullptr);
    if (pWdMid) {
        QCOMPARE(pWdMid->position, Position(5,5,7));
        QVERIFY(pWdMid->isConnectedTo("Start"));
        QVERIFY(pWdMid->isConnectedTo("End"));
    }

    QVERIFY(m_map->removeWaypoint("Start"));
    QCOMPARE(m_map->getWaypoints().size(), 1);
    QVERIFY(m_map->getWaypoint("Start") == nullptr);
    QVERIFY(m_map->getWaypoint("Mid") != nullptr); // Mid should still be there

    QVERIFY(!m_map->removeWaypoint("NonExistent")); // Try removing non-existent
}

// QTEST_MAIN(TestWaypointData) // Will be run by rme_core_map_tests
#include "TestWaypointData.moc" // Must be last line
