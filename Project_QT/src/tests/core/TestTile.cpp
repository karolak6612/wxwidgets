#include <QtTest/QtTest>
#include "core/Tile.h"      // Class to test
#include "core/Item.h"      // For creating items
#include "core/Creature.h"  // For creating creatures
#include "core/Spawn.h"     // For creating spawns
#include "MockItemTypeProvider.h" // Mock provider for items

using namespace RME;

class TestTile : public QObject
{
    Q_OBJECT

private:
    MockItemTypeProvider mockProvider;
    Position testPos{10, 20, 7};

    // Item IDs for testing
    const uint16_t DIRT_ID = 1;    // Assumed to be ground
    const uint16_t STONE_ID = 2;   // Assumed to be stackable, blocking
    const uint16_t GRASS_ID = 3;   // Assumed to be ground, walkable
    const uint16_t SWORD_ID = 100; // Not ground, not blocking by itself

private slots:
    void initTestCase();
    void init(); // Called before each test function

    void testTileCreation();
    void testDeepCopy();
    void testAddItem_Ground();
    void testAddItem_Stacking(); // Basic stacking, not complex RME logic yet
    void testRemoveItem();
    void testPopItem();
    void testGetItems();
    void testCreatureManagement();
    void testSpawnManagement();
    void testFlags();
    void testUpdateAndBlocking(); // Test update() and isBlocking() interaction
};

void TestTile::initTestCase() {
    MockItemData dirtData;
    dirtData.name = "Dirt";
    dirtData.isGround = true;
    dirtData.isWalkable = true;
    mockProvider.setMockData(DIRT_ID, dirtData);

    MockItemData stoneData;
    stoneData.name = "Stone";
    stoneData.isStackable = true;
    stoneData.isBlocking = true; // Stones block
    stoneData.isWalkable = false;
    mockProvider.setMockData(STONE_ID, stoneData);

    MockItemData grassData;
    grassData.name = "Grass";
    grassData.isGround = true;
    grassData.isWalkable = true;
    mockProvider.setMockData(GRASS_ID, grassData);

    MockItemData swordData;
    swordData.name = "Sword";
    swordData.isBlocking = false; // Item itself doesn't block tile
    swordData.isWalkable = true; // Can walk over if on floor
    mockProvider.setMockData(SWORD_ID, swordData);
}

void TestTile::init() {
    // Reset or re-initialize members before each test if needed
}

void TestTile::testTileCreation()
{
    Tile tile(testPos, &mockProvider);
    QCOMPARE(tile.getPosition(), testPos);
    QVERIFY(tile.getGround() == nullptr);
    QVERIFY(tile.getItems().isEmpty());
    QVERIFY(tile.getCreature() == nullptr);
    QVERIFY(tile.getSpawn() == nullptr);
    QCOMPARE(tile.getHouseID(), 0u);
    QCOMPARE(tile.getMapFlags(), TileMapFlag::NO_FLAGS);
}

void TestTile::testDeepCopy()
{
    Tile original(testPos, &mockProvider);
    original.addItem(Item::create(DIRT_ID, &mockProvider));
    original.addItem(Item::create(SWORD_ID, &mockProvider));
    original.setCreature(std::make_unique<Creature>("Goblin"));
    original.setHouseID(123);
    original.addMapFlag(TileMapFlag::PROTECTION_ZONE);

    auto copy = original.deepCopy();
    QVERIFY(copy != nullptr);
    QVERIFY(copy.get() != &original);
    QCOMPARE(copy->getPosition(), original.getPosition());
    QCOMPARE(copy->getHouseID(), original.getHouseID());
    QCOMPARE(copy->getMapFlags(), original.getMapFlags());

    QVERIFY(copy->getGround() != nullptr);
    QVERIFY(copy->getGround() != original.getGround());
    QCOMPARE(copy->getGround()->getID(), DIRT_ID);

    QCOMPARE(copy->getItems().size(), 1);
    QVERIFY(copy->getItems().first().get() != original.getItems().first().get());
    QCOMPARE(copy->getItems().first()->getID(), SWORD_ID);

    QVERIFY(copy->getCreature() != nullptr);
    QVERIFY(copy->getCreature() != original.getCreature());
    QCOMPARE(copy->getCreature()->getName(), QString("Goblin"));

    // Modify copy, original should be unaffected
    copy->addItem(Item::create(STONE_ID, &mockProvider));
    QCOMPARE(original.getItems().size(), 1); // Original still has 1 top item
    QCOMPARE(copy->getItems().size(), 2);    // Copy has 2 top items
}


void TestTile::testAddItem_Ground()
{
    Tile tile(testPos, &mockProvider);
    auto dirt = Item::create(DIRT_ID, &mockProvider);
    Item* dirtPtr = dirt.get();
    Item* addedDirt = tile.addItem(std::move(dirt));

    QCOMPARE(addedDirt, dirtPtr);
    QVERIFY(tile.getGround() == dirtPtr);
    QVERIFY(tile.getItems().isEmpty());

    // Add another ground item, should replace
    auto grass = Item::create(GRASS_ID, &mockProvider);
    Item* grassPtr = grass.get();
    tile.addItem(std::move(grass));
    QVERIFY(tile.getGround() == grassPtr);
    QVERIFY(tile.getItems().isEmpty()); // Old ground should be gone
}

void TestTile::testAddItem_Stacking() // Simplified stacking
{
    Tile tile(testPos, &mockProvider);
    tile.addItem(Item::create(DIRT_ID, &mockProvider)); // Ground

    auto sword1 = Item::create(SWORD_ID, &mockProvider);
    Item* sword1Ptr = sword1.get();
    tile.addItem(std::move(sword1));
    QCOMPARE(tile.getItems().size(), 1);
    QVERIFY(tile.getItems().first().get() == sword1Ptr);

    auto stone = Item::create(STONE_ID, &mockProvider); // Not ground
    Item* stonePtr = stone.get();
    tile.addItem(std::move(stone));
    QCOMPARE(tile.getItems().size(), 2);
    QVERIFY(tile.getItems().last().get() == stonePtr); // Appends by default
}

void TestTile::testRemoveItem()
{
    Tile tile(testPos, &mockProvider);
    Item* ground = tile.addItem(Item::create(DIRT_ID, &mockProvider));
    Item* top1 = tile.addItem(Item::create(SWORD_ID, &mockProvider));
    Item* top2 = tile.addItem(Item::create(STONE_ID, &mockProvider));

    QCOMPARE(tile.getItemCount(), 3);
    tile.removeItem(top1);
    QCOMPARE(tile.getItemCount(), 2);
    QVERIFY(tile.getGround() == ground);
    QVERIFY(tile.getItems().first().get() == top2); // top1 removed

    tile.removeItem(ground);
    QCOMPARE(tile.getItemCount(), 1);
    QVERIFY(tile.getGround() == nullptr);
    QVERIFY(tile.getItems().first().get() == top2);
}

void TestTile::testPopItem()
{
    Tile tile(testPos, &mockProvider);
    Item* groundOrig = tile.addItem(Item::create(DIRT_ID, &mockProvider));
    Item* topOrig = tile.addItem(Item::create(SWORD_ID, &mockProvider));

    std::unique_ptr<Item> poppedTop = tile.popItem(topOrig);
    QVERIFY(poppedTop.get() == topOrig);
    QCOMPARE(tile.getItemCount(), 1); // Only ground left

    std::unique_ptr<Item> poppedGround = tile.popItem(groundOrig);
    QVERIFY(poppedGround.get() == groundOrig);
    QVERIFY(tile.getGround() == nullptr);
    QCOMPARE(tile.getItemCount(), 0);
}


void TestTile::testGetItems()
{
    Tile tile(testPos, &mockProvider);
    QVERIFY(tile.getTopItem() == nullptr);
    QCOMPARE(tile.getItemAtStackpos(0), nullptr);

    Item* ground = tile.addItem(Item::create(DIRT_ID, &mockProvider));
    QCOMPARE(tile.getTopItem(), ground);
    QCOMPARE(tile.getItemAtStackpos(0), ground);
    QCOMPARE(tile.getItemAtStackpos(1), nullptr);

    Item* top1 = tile.addItem(Item::create(SWORD_ID, &mockProvider));
    Item* top2 = tile.addItem(Item::create(STONE_ID, &mockProvider));
    QCOMPARE(tile.getTopItem(), top2); // Last added top item
    QCOMPARE(tile.getItemAtStackpos(0), ground);
    QCOMPARE(tile.getItemAtStackpos(1), top1);
    QCOMPARE(tile.getItemAtStackpos(2), top2);
    QCOMPARE(tile.getItemAtStackpos(3), nullptr);

    QList<Item*> allItems = tile.getAllItems();
    QCOMPARE(allItems.size(), 3);
    QVERIFY(allItems.contains(ground));
    QVERIFY(allItems.contains(top1));
    QVERIFY(allItems.contains(top2));
}

void TestTile::testCreatureManagement()
{
    Tile tile(testPos, &mockProvider);
    QVERIFY(tile.getCreature() == nullptr);

    auto creature = std::make_unique<Creature>("Dragon");
    Creature* creaturePtr = creature.get();
    tile.setCreature(std::move(creature));
    QVERIFY(tile.getCreature() == creaturePtr);

    std::unique_ptr<Creature> poppedCreature = tile.popCreature();
    QVERIFY(poppedCreature.get() == creaturePtr);
    QVERIFY(tile.getCreature() == nullptr);
}

void TestTile::testSpawnManagement()
{
    Tile tile(testPos, &mockProvider);
    QVERIFY(tile.getSpawn() == nullptr);

    auto spawn = std::make_unique<Spawn>(3); // Radius 3
    Spawn* spawnPtr = spawn.get();
    tile.setSpawn(std::move(spawn));
    QVERIFY(tile.getSpawn() == spawnPtr);
    QCOMPARE(tile.getSpawn()->getRadius(), 3);

    std::unique_ptr<Spawn> poppedSpawn = tile.popSpawn();
    QVERIFY(poppedSpawn.get() == spawnPtr);
    QVERIFY(tile.getSpawn() == nullptr);
}

void TestTile::testFlags()
{
    Tile tile(testPos, &mockProvider);
    QCOMPARE(tile.getMapFlags(), TileMapFlag::NO_FLAGS);
    QVERIFY(!tile.hasMapFlag(TileMapFlag::PROTECTION_ZONE));

    tile.addMapFlag(TileMapFlag::PROTECTION_ZONE);
    QVERIFY(tile.hasMapFlag(TileMapFlag::PROTECTION_ZONE));
    QVERIFY(tile.isPZ());

    tile.addMapFlag(TileMapFlag::NO_PVP_ZONE);
    QVERIFY(tile.hasMapFlag(TileMapFlag::PROTECTION_ZONE));
    QVERIFY(tile.hasMapFlag(TileMapFlag::NO_PVP_ZONE));
    QCOMPARE(tile.getMapFlags(), TileMapFlag::PROTECTION_ZONE | TileMapFlag::NO_PVP_ZONE);

    tile.removeMapFlag(TileMapFlag::PROTECTION_ZONE);
    QVERIFY(!tile.hasMapFlag(TileMapFlag::PROTECTION_ZONE));
    QVERIFY(tile.hasMapFlag(TileMapFlag::NO_PVP_ZONE));
    QVERIFY(!tile.isPZ());
}

void TestTile::testUpdateAndBlocking()
{
    Tile tile(testPos, &mockProvider);
    // Initially no items, should not be blocking based on items
    tile.update(); // Explicit call, though addItem etc. also call it
    QVERIFY(!tile.hasStateFlag(TileStateFlag::BLOCKING));
    QVERIFY(!tile.isBlocking());

    // Add a walkable ground item
    tile.addItem(Item::create(GRASS_ID, &mockProvider)); // Grass is ground, walkable
    // tile.update(); // addItem calls update
    QVERIFY(!tile.isBlocking());
    QVERIFY(tile.hasStateFlag(TileStateFlag::HAS_WALKABLE_GROUND));

    // Add a blocking item
    tile.addItem(Item::create(STONE_ID, &mockProvider)); // Stone is blocking
    // tile.update();
    QVERIFY(tile.isBlocking()); // Should now be blocking
    QVERIFY(tile.hasStateFlag(TileStateFlag::BLOCKING));

    // Remove blocking item
    tile.removeItem(tile.getItems().first().get()); // remove stone
    QVERIFY(!tile.isBlocking());
    QVERIFY(!tile.hasStateFlag(TileStateFlag::BLOCKING));
}


QTEST_MAIN(TestTile)
#include "TestTile.moc"
