#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr

#include "core/houses/Houses.h"
#include "core/houses/House.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Position.h"
// Minimal includes for Map constructor context
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "core/Item.h" // Required for Item::setItemDatabase

// Using declarations
using RMEHouses = RME::core::houses::Houses;
using RMEHouse = RME::core::houses::House;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEPos = RME::core::Position;

class TestHouses : public QObject {
    Q_OBJECT

public:
    TestHouses() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testCreateNewHouse_EmptyManager_AssignsID1();
    void testCreateNewHouse_SequentialIDs();
    void testCreateNewHouse_WithDesiredID_Available();
    void testCreateNewHouse_WithDesiredID_Taken();
    void testAddExistingHouse_Success();
    void testAddExistingHouse_IDCollision();
    void testAddExistingHouse_NullPtr();
    void testGetHouse_FoundAndNotFound();
    void testRemoveHouse_Existing();
    void testRemoveHouse_NonExisting();
    void testRemoveHouse_CleansTileLinks();
    void testGetAllHouses();
    void testGetNextAvailableHouseID_Empty();
    void testGetNextAvailableHouseID_Sequential();
    void testGetNextAvailableHouseID_WithGaps();
    void testChangeHouseID_Success();
    void testChangeHouseID_OldIdNotFound();
    void testChangeHouseID_NewIdTaken();
    void testChangeHouseID_NewIdIsZero();
    void testClearAllHouses_Populated();
    void testClearAllHouses_Empty();
    void testClearAllHouses_CleansTileLinks();

private:
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RMEHouses> m_housesManager;

    // Asset related members for Map construction
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestHouses::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    RME::core::Item::setItemDatabase(m_itemDatabase.get()); // For Tiles creating items if needed

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_assetManager.get());
    m_housesManager = std::make_unique<RMEHouses>(m_map.get());
}

void TestHouses::cleanup() {
    m_housesManager.reset();
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    RME::core::Item::setItemDatabase(nullptr); // Clear static pointer
}

void TestHouses::testCreateNewHouse_EmptyManager_AssignsID1() {
    RMEHouse* house = m_housesManager->createNewHouse();
    QVERIFY(house != nullptr);
    if(house) QCOMPARE(house->getId(), quint32(1));
    QCOMPARE(m_housesManager->getHouseCount(), 1);
}

void TestHouses::testCreateNewHouse_SequentialIDs() {
    RMEHouse* h1 = m_housesManager->createNewHouse();
    RMEHouse* h2 = m_housesManager->createNewHouse();
    QVERIFY(h1 && h2);
    if(h1) QCOMPARE(h1->getId(), quint32(1));
    if(h2) QCOMPARE(h2->getId(), quint32(2));
    QCOMPARE(m_housesManager->getHouseCount(), 2);
}

void TestHouses::testCreateNewHouse_WithDesiredID_Available() {
    RMEHouse* house = m_housesManager->createNewHouse(100);
    QVERIFY(house != nullptr);
    if(house) QCOMPARE(house->getId(), quint32(100));
    QCOMPARE(m_housesManager->getHouseCount(), 1);
}

void TestHouses::testCreateNewHouse_WithDesiredID_Taken() {
    m_housesManager->createNewHouse(100);
    RMEHouse* house = m_housesManager->createNewHouse(100);
    QVERIFY(house != nullptr);
    if(house) QCOMPARE(house->getId(), quint32(101));
    QCOMPARE(m_housesManager->getHouseCount(), 2);
}

void TestHouses::testAddExistingHouse_Success() {
    auto house_ptr = std::make_unique<RMEHouse>(200, m_map.get());
    house_ptr->setName("Loaded House");
    bool added = m_housesManager->addExistingHouse(std::move(house_ptr));
    QVERIFY(added);
    QCOMPARE(m_housesManager->getHouseCount(), 1);
    RMEHouse* retrieved = m_housesManager->getHouse(200);
    QVERIFY(retrieved != nullptr);
    if(retrieved) QCOMPARE(retrieved->getName(), QString("Loaded House"));
}

void TestHouses::testAddExistingHouse_IDCollision() {
    m_housesManager->createNewHouse(200);
    auto house_ptr = std::make_unique<RMEHouse>(200, m_map.get());
    RMEHouse* rawPtrBeforeAdd = house_ptr.get();
    bool added = m_housesManager->addExistingHouse(std::move(house_ptr));
    QVERIFY(!added);
    QVERIFY(house_ptr.get() == rawPtrBeforeAdd);
    QCOMPARE(m_housesManager->getHouseCount(), 1);
}

void TestHouses::testAddExistingHouse_NullPtr() {
    bool added = m_housesManager->addExistingHouse(nullptr);
    QVERIFY(!added);
    QCOMPARE(m_housesManager->getHouseCount(), 0);
}

void TestHouses::testGetHouse_FoundAndNotFound() {
    RMEHouse* h1 = m_housesManager->createNewHouse(50);
    QVERIFY(h1 != nullptr);
    QCOMPARE(m_housesManager->getHouse(50), h1);
    QVERIFY(m_housesManager->getHouse(51) == nullptr);
}

void TestHouses::testRemoveHouse_Existing() {
    RMEHouse* house = m_housesManager->createNewHouse(55);
    QVERIFY(house != nullptr);
    QCOMPARE(m_housesManager->getHouseCount(), 1);
    bool removed = m_housesManager->removeHouse(55);
    QVERIFY(removed);
    QCOMPARE(m_housesManager->getHouseCount(), 0);
    QVERIFY(m_housesManager->getHouse(55) == nullptr);
}

void TestHouses::testRemoveHouse_NonExisting() {
    bool removed = m_housesManager->removeHouse(999);
    QVERIFY(!removed);
}

void TestHouses::testRemoveHouse_CleansTileLinks() {
    RMEPos pos(5,5,7);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    RMEHouse* house = m_housesManager->createNewHouse(60);
    QVERIFY(house);
    house->linkTile(tile);
    QCOMPARE(tile->getHouseId(), quint32(60));

    m_housesManager->removeHouse(60);
    QCOMPARE(tile->getHouseId(), quint32(0));
}

void TestHouses::testGetAllHouses() {
    QVERIFY(m_housesManager->getAllHouses().isEmpty());
    RMEHouse* h1 = m_housesManager->createNewHouse(1);
    RMEHouse* h2 = m_housesManager->createNewHouse(2);
    QList<RMEHouse*> allHouses = m_housesManager->getAllHouses();
    QCOMPARE(allHouses.size(), 2);
    QVERIFY(allHouses.contains(h1));
    QVERIFY(allHouses.contains(h2));
}

void TestHouses::testGetNextAvailableHouseID_Empty() {
    QCOMPARE(m_housesManager->getNextAvailableHouseID(), quint32(1));
}

void TestHouses::testGetNextAvailableHouseID_Sequential() {
    m_housesManager->createNewHouse(1);
    m_housesManager->createNewHouse(2);
    QCOMPARE(m_housesManager->getNextAvailableHouseID(), quint32(3));
}

void TestHouses::testGetNextAvailableHouseID_WithGaps() {
    m_housesManager->createNewHouse(1);
    m_housesManager->createNewHouse(3);
    QCOMPARE(m_housesManager->getNextAvailableHouseID(), quint32(4));
}

void TestHouses::testChangeHouseID_Success() {
    RMEHouse* house = m_housesManager->createNewHouse(70);
    QVERIFY(house);
    house->setName("Original House");
    bool changed = m_housesManager->changeHouseID(70, 700);
    QVERIFY(changed);
    QVERIFY(m_housesManager->getHouse(70) == nullptr);
    RMEHouse* newH = m_housesManager->getHouse(700);
    QVERIFY(newH != nullptr);
    if(newH) {
        QCOMPARE(newH->getId(), quint32(700));
        QCOMPARE(newH->getName(), QString("Original House"));
        QCOMPARE(newH, house);
    }
}

void TestHouses::testChangeHouseID_OldIdNotFound() {
    bool changed = m_housesManager->changeHouseID(999, 1000);
    QVERIFY(!changed);
}

void TestHouses::testChangeHouseID_NewIdTaken() {
    m_housesManager->createNewHouse(80);
    m_housesManager->createNewHouse(800);
    bool changed = m_housesManager->changeHouseID(80, 800);
    QVERIFY(!changed);
    QVERIFY(m_housesManager->getHouse(80) != nullptr);
}

void TestHouses::testChangeHouseID_NewIdIsZero() {
    m_housesManager->createNewHouse(90);
    bool changed = m_housesManager->changeHouseID(90, 0);
    QVERIFY(!changed);
    QVERIFY(m_housesManager->getHouse(90) != nullptr);
}

void TestHouses::testClearAllHouses_Populated() {
    m_housesManager->createNewHouse(1);
    m_housesManager->createNewHouse(2);
    QCOMPARE(m_housesManager->getHouseCount(), 2);
    m_housesManager->clearAllHouses();
    QCOMPARE(m_housesManager->getHouseCount(), 0);
    QVERIFY(m_housesManager->getAllHouses().isEmpty());
}

void TestHouses::testClearAllHouses_Empty() {
    QCOMPARE(m_housesManager->getHouseCount(), 0);
    m_housesManager->clearAllHouses();
    QCOMPARE(m_housesManager->getHouseCount(), 0);
}

void TestHouses::testClearAllHouses_CleansTileLinks() {
    RMEPos pos(6,6,7);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    RMEHouse* house = m_housesManager->createNewHouse(90);
    QVERIFY(house);
    house->linkTile(tile);
    QCOMPARE(tile->getHouseId(), quint32(90));

    m_housesManager->clearAllHouses();
    QCOMPARE(tile->getHouseId(), quint32(0));
}

QTEST_MAIN(TestHouses)
#include "TestHouses.moc"
