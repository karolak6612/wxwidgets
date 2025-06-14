#include <QtTest/QtTest>
#include <memory> // For std::unique_ptr

#include "core/houses/House.h"
#include "core/map/Map.h" // For RME::core::Map
#include "core/Tile.h"    // For RME::core::Tile (if testing link/unlink)
#include "core/Position.h"
// Minimal includes for Map constructor context
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"
#include "core/Item.h" // Required for Item::setItemDatabase and Tile interaction

// Using declarations
using RMEHouse = RME::core::houses::House;
using RMEMap = RME::core::Map;
using RMETile = RME::core::Tile;
using RMEPos = RME::core::Position;

class TestHouse : public QObject {
    Q_OBJECT

public:
    TestHouse() = default;

private slots:
    void initTestCase() {} // Added for completeness
    void init();
    void cleanup();
    void cleanupTestCase() {} // Added for completeness

    void testConstructorAndGetters();
    void testSetters();
    void testTilePositionManagement();
    void testLinkUnlinkTile_Basic();
    void testSetExit_Basic();
    void testCleanAllTileLinks_Basic();

private:
    std::unique_ptr<RMEMap> m_map;

    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
};

void TestHouse::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);
    // Set static item database for Item::create used in populateTile or by Tile itself
    RME::core::Item::setItemDatabase(m_itemDatabase.get());

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(10, 10, 1, m_assetManager.get());
}

void TestHouse::cleanup() {
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
    RME::core::Item::setItemDatabase(nullptr); // Clear static pointer
}

void TestHouse::testConstructorAndGetters() {
    RMEHouse house(123, m_map.get());
    QCOMPARE(house.getId(), quint32(123));
    QCOMPARE(house.getMap(), m_map.get());
    QVERIFY(house.getName().isEmpty());
    QCOMPARE(house.getRent(), 0);
    QCOMPARE(house.getTownId(), quint32(0));
    QVERIFY(!house.isGuildhall());
    QVERIFY(!house.getExitPos().isValid());
    QVERIFY(house.getTilePositions().isEmpty());
}

void TestHouse::testSetters() {
    RMEHouse house(1, m_map.get());

    house.setName("Test Villa");
    QCOMPARE(house.getName(), QString("Test Villa"));

    house.setRent(1000);
    QCOMPARE(house.getRent(), 1000);

    house.setTownId(5);
    QCOMPARE(house.getTownId(), quint32(5));

    house.setIsGuildhall(true);
    QVERIFY(house.isGuildhall());

    RMEPos exitPos(5,5,7);
    house.setExitPosInternal(exitPos);
    QCOMPARE(house.getExitPos(), exitPos);
}

void TestHouse::testTilePositionManagement() {
    RMEHouse house(1, m_map.get());
    RMEPos pos1(1,1,7);
    RMEPos pos2(1,2,7);
    RMEPos invalidPos;

    QVERIFY(house.getTilePositions().isEmpty());
    QCOMPARE(house.getTileCount(), 0);

    house.addTilePosition(pos1);
    QCOMPARE(house.getTileCount(), 1);
    QVERIFY(house.hasTilePosition(pos1));
    QVERIFY(!house.hasTilePosition(pos2));

    house.addTilePosition(pos2);
    QCOMPARE(house.getTileCount(), 2);
    QVERIFY(house.hasTilePosition(pos1));
    QVERIFY(house.hasTilePosition(pos2));

    house.addTilePosition(pos1);
    QCOMPARE(house.getTileCount(), 2);

    house.addTilePosition(invalidPos);
    QCOMPARE(house.getTileCount(), 2);

    house.removeTilePosition(pos1);
    QCOMPARE(house.getTileCount(), 1);
    QVERIFY(!house.hasTilePosition(pos1));
    QVERIFY(house.hasTilePosition(pos2));

    house.removeTilePosition(pos1);
    QCOMPARE(house.getTileCount(), 1);

    house.clearTilePositions();
    QVERIFY(house.getTilePositions().isEmpty());
    QCOMPARE(house.getTileCount(), 0);
}

void TestHouse::testLinkUnlinkTile_Basic() {
    RMEHouse house(77, m_map.get());
    RMEPos pos(2,2,7);
    RMETile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    QCOMPARE(tile->getHouseId(), quint32(0));

    house.linkTile(tile);
    QVERIFY(house.hasTilePosition(pos));
    QCOMPARE(tile->getHouseId(), quint32(77));

    house.unlinkTile(tile);
    // Note: The current House::unlinkTile also removes from m_tilePositions.
    // If it didn't, this test might need to check hasTilePosition(pos) differently or call removeTilePosition.
    QVERIFY(!house.hasTilePosition(pos));
    QCOMPARE(tile->getHouseId(), quint32(0));
}

void TestHouse::testSetExit_Basic() {
    RMEHouse house(88, m_map.get());
    RMEPos exitPos1(3,3,7);
    RMEPos exitPos2(3,4,7);
    RMETile* tile1 = m_map->getOrCreateTile(exitPos1);
    RMETile* tile2 = m_map->getOrCreateTile(exitPos2);
    QVERIFY(tile1 && tile2);

    house.setExit(exitPos1);
    QCOMPARE(house.getExitPos(), exitPos1);
    QVERIFY(tile1->isHouseExit());
    QVERIFY(!tile2->isHouseExit());

    house.setExit(exitPos2);
    QCOMPARE(house.getExitPos(), exitPos2);
    QVERIFY(!tile1->isHouseExit());
    QVERIFY(tile2->isHouseExit());

    house.setExit(RMEPos());
    QVERIFY(!house.getExitPos().isValid());
    QVERIFY(!tile2->isHouseExit());
}

void TestHouse::testCleanAllTileLinks_Basic() {
    RMEHouse house(99, m_map.get());
    RMEPos pos_h_tile(4,4,7);
    RMEPos pos_exit(4,5,7);

    RMETile* houseTile = m_map->getOrCreateTile(pos_h_tile);
    RMETile* exitTile = m_map->getOrCreateTile(pos_exit);
    QVERIFY(houseTile && exitTile);

    house.linkTile(houseTile);
    houseTile->setIsProtectionZone(true); // Manually set PZ for test
    house.setExit(pos_exit);

    QVERIFY(house.hasTilePosition(pos_h_tile));
    QCOMPARE(houseTile->getHouseId(), house.getId());
    QVERIFY(houseTile->isProtectionZone());
    QCOMPARE(house.getExitPos(), pos_exit);
    QVERIFY(exitTile->isHouseExit());

    house.cleanAllTileLinks();

    QVERIFY(house.getTilePositions().isEmpty());
    QVERIFY(!house.getExitPos().isValid());
    QCOMPARE(houseTile->getHouseId(), quint32(0));
    QVERIFY(!houseTile->isProtectionZone());
    QVERIFY(!exitTile->isHouseExit());
}

QTEST_MAIN(TestHouse)
#include "TestHouse.moc"
