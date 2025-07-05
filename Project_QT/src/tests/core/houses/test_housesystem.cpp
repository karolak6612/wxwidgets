#include <QtTest>
#include "Project_QT/src/core/houses/HouseData.h"
#include "Project_QT/src/core/map/Map.h"
#include "Project_QT/src/tests/core/mocks/MockMapElements.h"
#include "Project_QT/src/core/assets/AssetManager.h"
#include "Project_QT/src/core/sprites/SpriteManager.h"
#include "Project_QT/src/core/assets/ItemDatabase.h"
#include "Project_QT/src/core/assets/CreatureDatabase.h"
#include "Project_QT/src/core/assets/ClientVersionManager.h"
#include "Project_QT/src/core/assets/MaterialManager.h"
#include "Project_QT/src/core/Tile.h" // Added for Tile operations
#include "Project_QT/src/core/Item.h"  // Added for Item operations
#include "Project_QT/src/core/assets/ItemData.h" // Added for ItemData

// Define placeholder for actual TileMapFlag if not easily accessible
#ifndef TILESTATE_PROTECTIONZONE
#define TILESTATE_PROTECTIONZONE 0x00000001
#endif


class TestHouseSystem : public QObject {
    Q_OBJECT

public:
    TestHouseSystem();
    ~TestHouseSystem();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // HouseData Tests
    void testHouseData_Construction();
    void testHouseData_Properties();
    void testHouseData_ExitsManagement();
    void testHouseData_TilesManagement();
    void testHouseData_Description();

    // Map House Management Tests
    void testMap_AddGetHouse();
    void testMap_AddHouse_ExistingId();
    void testMap_RemoveHouse();
    void testMap_RemoveHouse_UpdatesTiles();
    void testMap_GetUnusedHouseId();
    void testMap_ChangeHouseId();
    void testMap_ChangeHouseId_UpdatesTiles();
    void testMap_ChangeHouseId_EdgeCases();
    void testSetEntryPoint_TileFlags();
    void testMap_IsValidHouseExitLocation(); // New test slot


private:
    RME::Map* m_map;
    RME::core::assets::AssetManager* m_assetManager;
    RME::core::sprites::SpriteManager* m_spriteManager;
    RME::core::assets::ItemDatabase* m_itemDatabase; // Will add mock items here
    RME::core::assets::CreatureDatabase* m_creatureDatabase;
    RME::core::assets::ClientVersionManager* m_clientVersionManager;
    RME::core::assets::MaterialManager* m_materialManager;

    // Item IDs for testing isValidHouseExitLocation
    const uint16_t VALID_GROUND_ID = 1001;
    const uint16_t OTHER_GROUND_ID = 1002;
    const uint16_t BLOCKING_ITEM_ID = 1003;
};

TestHouseSystem::TestHouseSystem() : m_map(nullptr), m_assetManager(nullptr),
    m_spriteManager(nullptr), m_itemDatabase(nullptr), m_creatureDatabase(nullptr),
    m_clientVersionManager(nullptr), m_materialManager(nullptr)
{}
TestHouseSystem::~TestHouseSystem() {
}

void TestHouseSystem::initTestCase() {
    // Global setup if any
}
void TestHouseSystem::cleanupTestCase() {
    // Global cleanup if any
}

void TestHouseSystem::init() {
    m_clientVersionManager = new RME::core::assets::ClientVersionManager();
    m_itemDatabase = new RME::core::assets::ItemDatabase(*m_clientVersionManager); // Real ItemDatabase
    m_creatureDatabase = new RME::core::assets::CreatureDatabase();
    m_spriteManager = new RME::core::sprites::SpriteManager(*m_clientVersionManager);
    m_materialManager = new RME::core::assets::MaterialManager(*m_clientVersionManager);

    // Setup mock items for exit location tests
    RME::core::assets::ItemData validGround;
    validGround.id = VALID_GROUND_ID; validGround.name = "Valid Ground"; validGround.isGround = true; validGround.isBlocking = false;
    m_itemDatabase->addItemData(validGround); // Assuming addItemData or similar exists

    RME::core::assets::ItemData otherGround;
    otherGround.id = OTHER_GROUND_ID; otherGround.name = "Other Ground"; otherGround.isGround = true; otherGround.isBlocking = false;
    m_itemDatabase->addItemData(otherGround);

    RME::core::assets::ItemData blockingItem;
    blockingItem.id = BLOCKING_ITEM_ID; blockingItem.name = "Blocking Item"; blockingItem.isGround = false; blockingItem.isBlocking = true;
    m_itemDatabase->addItemData(blockingItem);


    m_assetManager = new RME::core::assets::AssetManager(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = new RME::Map(10, 10, 8, m_assetManager); // Map of 10x10x8 for testing positions
}
void TestHouseSystem::cleanup() {
    delete m_map;
    m_map = nullptr;
    delete m_assetManager; // AssetManager owns its dependencies in this setup
    m_assetManager = nullptr;
    delete m_materialManager;
    m_materialManager = nullptr;
    delete m_spriteManager;
    m_spriteManager = nullptr;
    delete m_creatureDatabase;
    m_creatureDatabase = nullptr;
    delete m_itemDatabase;
    m_itemDatabase = nullptr;
    delete m_clientVersionManager;
    m_clientVersionManager = nullptr;
}

// --- HouseData Tests ---
void TestHouseSystem::testHouseData_Construction() {
    RME::HouseData house1;
    QCOMPARE(house1.getId(), (uint32_t)0);
    QVERIFY(house1.getName().isEmpty());
    QCOMPARE(house1.getRent(), (uint32_t)0);
    QVERIFY(house1.getExits().isEmpty());
    QVERIFY(house1.getTilePositions().isEmpty());

    RME::HouseData house2(123, "Test House");
    QCOMPARE(house2.getId(), (uint32_t)123);
    QCOMPARE(house2.getName(), QString("Test House"));
}

void TestHouseSystem::testHouseData_Properties() {
    RME::HouseData house;
    house.setId(1);
    QCOMPARE(house.getId(), (uint32_t)1);

    house.setName("Grand Villa");
    QCOMPARE(house.getName(), QString("Grand Villa"));

    house.setTownId(5);
    QCOMPARE(house.getTownId(), (uint32_t)5);

    RME::Position entry(100, 200, 7); // This position will be out of bounds for the small test map
                                      // but HouseData itself doesn't validate against map.
    house.setEntryPoint(entry);
    QCOMPARE(house.getEntryPoint(), entry);

    house.setRent(1000);
    QCOMPARE(house.getRent(), (uint32_t)1000);

    house.setSizeInSqms(150);
    QCOMPARE(house.getSizeInSqms(), 150);

    house.setIsGuildhall(true);
    QVERIFY(house.isGuildhall());
}

void TestHouseSystem::testHouseData_ExitsManagement() {
    RME::HouseData house;
    RME::Position exit1(1, 1, 7); // Adjusted to be within map bounds
    RME::Position exit2(2, 1, 7);

    house.addExit(exit1);
    QCOMPARE(house.getExits().count(), 1);
    QVERIFY(house.getExits().contains(exit1));

    house.addExit(exit1); // Should not add duplicate
    QCOMPARE(house.getExits().count(), 1);

    house.addExit(exit2);
    QCOMPARE(house.getExits().count(), 2);
    QVERIFY(house.getExits().contains(exit2));

    QVERIFY(house.removeExit(exit1));
    QCOMPARE(house.getExits().count(), 1);
    QVERIFY(!house.getExits().contains(exit1));

    QVERIFY(!house.removeExit(exit1)); // Already removed

    house.clearExits();
    QVERIFY(house.getExits().isEmpty());
}

void TestHouseSystem::testHouseData_TilesManagement() {
    RME::HouseData house;
    RME::Position tile1(3, 3, 7); // Adjusted
    RME::Position tile2(4, 3, 7);

    house.addTilePosition(tile1);
    QCOMPARE(house.getTilePositions().count(), 1);
    QVERIFY(house.containsTile(tile1));

    house.addTilePosition(tile1); // No duplicates
    QCOMPARE(house.getTilePositions().count(), 1);

    house.addTilePosition(tile2);
    QCOMPARE(house.getTilePositions().count(), 2);
    QVERIFY(house.containsTile(tile2));

    QVERIFY(house.removeTilePosition(tile1));
    QCOMPARE(house.getTilePositions().count(), 1);
    QVERIFY(!house.containsTile(tile1));

    QVERIFY(!house.removeTilePosition(tile1));

    house.clearTilePositions();
    QVERIFY(house.getTilePositions().isEmpty());
}

void TestHouseSystem::testHouseData_Description() {
    RME::HouseData house(7, "Adventurer's Guild");
    house.setRent(500);
    house.setIsGuildhall(true);
    QString desc = house.getDescription();
    QVERIFY(desc.contains("Adventurer's Guild"));
    QVERIFY(desc.contains("ID:7"));
    QVERIFY(desc.contains("Rent: 500"));
    QVERIFY(desc.contains("Guildhall"));
}

// --- Map House Management Tests ---
void TestHouseSystem::testMap_AddGetHouse() {
    RME::HouseData house1_data(1, "House One");
    RME::Position entry1(1,1,7); // Adjusted
    house1_data.setEntryPoint(entry1);

    m_map->addHouse(std::move(house1_data));

    QCOMPARE(m_map->getHouses().count(), 1);

    const RME::HouseData* retrieved_const = m_map->getHouse(1);
    QVERIFY(retrieved_const != nullptr);
    QCOMPARE(retrieved_const->getName(), QString("House One"));
    QCOMPARE(retrieved_const->getEntryPoint(), entry1);

    RME::HouseData* retrieved_non_const = m_map->getHouse(1);
    QVERIFY(retrieved_non_const != nullptr);
    retrieved_non_const->setName("House One Modified");

    const RME::HouseData* retrieved_again = m_map->getHouse(1);
    QCOMPARE(retrieved_again->getName(), QString("House One Modified"));

    QVERIFY(m_map->getHouse(2) == nullptr);
}

void TestHouseSystem::testMap_AddHouse_ExistingId() {
    m_map->addHouse(RME::HouseData(1, "First House"));
    m_map->addHouse(RME::HouseData(1, "Second House Overwrite"));

    QCOMPARE(m_map->getHouses().count(), 1);
    const RME::HouseData* house = m_map->getHouse(1);
    QVERIFY(house != nullptr);
    QCOMPARE(house->getName(), QString("Second House Overwrite"));
}


void TestHouseSystem::testMap_RemoveHouse() {
    m_map->addHouse(RME::HouseData(1, "To Be Removed"));
    QVERIFY(m_map->getHouse(1) != nullptr);

    QVERIFY(m_map->removeHouse(1));
    QVERIFY(m_map->getHouse(1) == nullptr);
    QCOMPARE(m_map->getHouses().count(), 0);

    QVERIFY(!m_map->removeHouse(1));
}

void TestHouseSystem::testMap_RemoveHouse_UpdatesTiles() {
    RME::Position pos1(3,3,7); // Adjusted
    RME::Position pos2(4,3,7);

    RME::Tile* tile1 = m_map->getOrCreateTile(pos1);
    RME::Tile* tile2 = m_map->getOrCreateTile(pos2);
    QVERIFY(tile1 && tile2);

    tile1->setHouseId(1);
    tile2->setHouseId(1);

    RME::HouseData house_data(1, "House With Tiles");
    house_data.addTilePosition(pos1);
    house_data.addTilePosition(pos2);
    m_map->addHouse(std::move(house_data));

    QCOMPARE(m_map->getTile(pos1)->getHouseId(), (uint32_t)1);
    QCOMPARE(m_map->getTile(pos2)->getHouseId(), (uint32_t)1);

    m_map->removeHouse(1);

    QCOMPARE(m_map->getTile(pos1)->getHouseId(), (uint32_t)0);
    QCOMPARE(m_map->getTile(pos2)->getHouseId(), (uint32_t)0);
}

void TestHouseSystem::testMap_GetUnusedHouseId() {
    QCOMPARE(m_map->getUnusedHouseId(), (uint32_t)1);
    m_map->addHouse(RME::HouseData(1, "H1"));
    QCOMPARE(m_map->getUnusedHouseId(), (uint32_t)2);
    m_map->addHouse(RME::HouseData(3, "H3"));
    QCOMPARE(m_map->getUnusedHouseId(), (uint32_t)4);
    m_map->addHouse(RME::HouseData(2, "H2"));
    QCOMPARE(m_map->getUnusedHouseId(), (uint32_t)4);

    m_map->removeHouse(3);
    QCOMPARE(m_map->getUnusedHouseId(), (uint32_t)3);
}

void TestHouseSystem::testMap_ChangeHouseId() {
    RME::HouseData house_data(10, "Old ID House");
    m_map->addHouse(std::move(house_data));

    QVERIFY(m_map->getHouse(10) != nullptr);
    QVERIFY(m_map->getHouse(20) == nullptr);

    QVERIFY(m_map->changeHouseId(10, 20));

    QVERIFY(m_map->getHouse(10) == nullptr);
    const RME::HouseData* new_house = m_map->getHouse(20);
    QVERIFY(new_house != nullptr);
    QCOMPARE(new_house->getId(), (uint32_t)20);
    QCOMPARE(new_house->getName(), QString("Old ID House"));
}

void TestHouseSystem::testMap_ChangeHouseId_UpdatesTiles() {
    RME::Position pos1(4,4,7); // Adjusted
    RME::Tile* tile1 = m_map->getOrCreateTile(pos1);
    QVERIFY(tile1);
    tile1->setHouseId(10);

    RME::HouseData house_data(10, "House With Tile For ID Change");
    house_data.addTilePosition(pos1);
    m_map->addHouse(std::move(house_data));

    QCOMPARE(m_map->getTile(pos1)->getHouseId(), (uint32_t)10);

    m_map->changeHouseId(10, 20);

    QCOMPARE(m_map->getTile(pos1)->getHouseId(), (uint32_t)20);
    const RME::HouseData* house = m_map->getHouse(20);
    QVERIFY(house != nullptr && house->containsTile(pos1));
}

void TestHouseSystem::testMap_ChangeHouseId_EdgeCases() {
    m_map->addHouse(RME::HouseData(1, "H1"));
    m_map->addHouse(RME::HouseData(2, "H2"));

    QVERIFY(!m_map->changeHouseId(1, 2));
    QVERIFY(!m_map->changeHouseId(1, 0));
    QVERIFY(!m_map->changeHouseId(3, 4));
    QVERIFY(m_map->changeHouseId(1, 1));

    QVERIFY(m_map->getHouse(1) != nullptr);
    QVERIFY(m_map->getHouse(2) != nullptr);
}

void TestHouseSystem::testSetEntryPoint_TileFlags() {
    RME::HouseData house(1, "Test House");
    RME::core::Position pos1(5,5,7);
    RME::core::Position pos2(6,6,7);
    RME::core::Position invalidPos;

    house.setEntryPoint(pos1, m_map);
    QCOMPARE(house.getEntryPoint(), pos1);
    RME::Tile* tile1 = m_map->getTile(pos1);
    QVERIFY(tile1);
    QVERIFY(tile1->isHouseExit());

    house.setEntryPoint(pos2, m_map);
    QCOMPARE(house.getEntryPoint(), pos2);
    tile1 = m_map->getTile(pos1);
    QVERIFY(tile1);
    QVERIFY(!tile1->isHouseExit());
    RME::Tile* tile2 = m_map->getTile(pos2);
    QVERIFY(tile2);
    QVERIFY(tile2->isHouseExit());

    house.setEntryPoint(invalidPos, m_map);
    QCOMPARE(house.getEntryPoint(), invalidPos);
    tile2 = m_map->getTile(pos2);
    QVERIFY(tile2);
    QVERIFY(!tile2->isHouseExit());

    RME::HouseData houseNullMap(2, "Null Map House");
    RME::core::Position posForNullMap(1,1,7); // Adjusted
    houseNullMap.setEntryPoint(posForNullMap, nullptr);
    QCOMPARE(houseNullMap.getEntryPoint(), posForNullMap);
}

void TestHouseSystem::testMap_IsValidHouseExitLocation() {
    QVERIFY(m_map);
    RME::Position pos(5,5,7);

    // Scenario 1: Valid exit location
    RME::Tile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->setGround(std::make_unique<RME::Item>(VALID_GROUND_ID, m_itemDatabase->getItemData(VALID_GROUND_ID)));
    tile->setHouseId(0);
    // Make sure tile is not blocking by clearing items and updating
    tile->getItemsForWrite().clear();
    tile->update();
    QVERIFY(m_map->isValidHouseExitLocation(pos));

    // Scenario 2: No tile at position
    RME::Position nonExistentPosFar(500, 500, 7);
    QVERIFY(!m_map->isValidHouseExitLocation(nonExistentPosFar));

    RME::Position nonExistentPosNear(5,6,7);
    m_map->clearTile(nonExistentPosNear);
    QVERIFY(!m_map->isValidHouseExitLocation(nonExistentPosNear));

    // Scenario 3: Tile has no ground
    tile = m_map->getOrCreateTile(pos);
    tile->setGround(nullptr);
    tile->setHouseId(0);
    tile->getItemsForWrite().clear();
    tile->update();
    QVERIFY(!m_map->isValidHouseExitLocation(pos));

    // Scenario 4: Tile is already part of a house
    tile->setGround(std::make_unique<RME::Item>(OTHER_GROUND_ID, m_itemDatabase->getItemData(OTHER_GROUND_ID)));
    tile->setHouseId(99);
    tile->update();
    QVERIFY(!m_map->isValidHouseExitLocation(pos));
    tile->setHouseId(0);

    // Scenario 5: Tile is blocking
    tile->setGround(std::make_unique<RME::Item>(OTHER_GROUND_ID, m_itemDatabase->getItemData(OTHER_GROUND_ID)));
    tile->addItem(std::make_unique<RME::Item>(BLOCKING_ITEM_ID, m_itemDatabase->getItemData(BLOCKING_ITEM_ID)));
    tile->update();
    QVERIFY(tile->isBlocking());
    QVERIFY(!m_map->isValidHouseExitLocation(pos));

    // Cleanup for next test if 'pos' is reused (it is if init() doesn't fully reset map state)
    // This is handled by init() creating a new map.
}


QTEST_APPLESS_MAIN(TestHouseSystem)
#include "test_housesystem.moc"
