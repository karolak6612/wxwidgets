#include <QtTest/QtTest>
#include "core/Tile.h"      // Class to test
#include "core/Item.h"      // For creating items
#include "core/creatures/Creature.h"  // For creating creatures (updated path from subtask)
// #include "core/Spawn.h"  // Removed old Spawn include
#include "core/spawns/SpawnData.h" // Added new SpawnData include
#include "MockItemTypeProvider.h" // Mock provider for items

using namespace RME;
// Forward declare Creature if its definition isn't strictly needed for these tests
// For now, assuming Creature.h provides enough, or use RME::core::creatures::Creature

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
    // void testSpawnManagement(); // Will be replaced by testSpawnDataReference
    void testSpawnDataReference(); // New name for spawn test
    void testFlags();
    void testUpdateAndBlocking(); // Test update() and isBlocking() interaction
    void testSelectionStateOnTile();
    void testHasSelectedElements();
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
    QVERIFY(tile.getSpawnDataRef() == nullptr); // Updated for new spawn handling
    QCOMPARE(tile.getHouseId(), 0u); // Updated getter name from Tile.h
    QCOMPARE(tile.getMapFlags(), TileMapFlag::NO_FLAGS);
}

void TestTile::testDeepCopy()
{
    Tile original(testPos, &mockProvider);
    original.addItem(Item::create(DIRT_ID, &mockProvider));
    original.addItem(Item::create(SWORD_ID, &mockProvider));
    original.setCreature(std::make_unique<RME::core::creatures::Creature>("Goblin")); // Use full namespace
    original.setHouseId(123); // Updated setter name
    original.addMapFlag(TileMapFlag::PROTECTION_ZONE);

    auto copy = original.deepCopy();
    QVERIFY(copy != nullptr);
    QVERIFY(copy.get() != &original);
    QCOMPARE(copy->getPosition(), original.getPosition());
    QCOMPARE(copy->getHouseId(), original.getHouseId()); // Updated getter name
    QCOMPARE(copy->getMapFlags(), original.getMapFlags());
    QCOMPARE(copy->getSpawnDataRef(), original.getSpawnDataRef()); // Check spawn ref copy

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

    // Test selection state copying
    original.setSelected(true);
    auto selectedCopy = original.deepCopy();
    QVERIFY(selectedCopy->isSelected()); // Selected state should be copied
    QVERIFY(original.isSelected());      // Original remains selected

    original.setSelected(false); // Reset original
    QVERIFY(!original.isSelected());
    // selectedCopy should still be selected because it's a copy of the state when original was selected
    QVERIFY(selectedCopy->isSelected());

    selectedCopy->setSelected(false); // Now change selectedCopy
    QVERIFY(!selectedCopy->isSelected());
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

    auto creature = std::make_unique<RME::core::creatures::Creature>("Dragon"); // Use full namespace
    RME::core::creatures::Creature* creaturePtr = creature.get(); // Use full namespace
    tile.setCreature(std::move(creature));
    QVERIFY(tile.getCreature() == creaturePtr);

    std::unique_ptr<RME::core::creatures::Creature> poppedCreature = tile.popCreature();
    QVERIFY(poppedCreature.get() == creaturePtr);
    QVERIFY(tile.getCreature() == nullptr);
}

void TestTile::testSpawnDataReference()
{
    Tile tile(testPos, &mockProvider);
    QVERIFY(tile.getSpawnDataRef() == nullptr);
    QVERIFY(!tile.isSpawnTile());
    QCOMPARE(tile.getSpawnRadius(), 0);
    QVERIFY(tile.getSpawnCreatureList().isEmpty());
    QCOMPARE(tile.getSpawnIntervalSeconds(), 0);

    // Using Position from testPos for SpawnData constructor
    RME::SpawnData spawnData1(testPos, 3, 60, {"Dragon"});
    tile.setSpawnDataRef(&spawnData1);
    QVERIFY(tile.getSpawnDataRef() == &spawnData1);
    QVERIFY(tile.isSpawnTile());
    QCOMPARE(tile.getSpawnRadius(), 3);
    QCOMPARE(tile.getSpawnCreatureList(), QStringList({"Dragon"}));
    QCOMPARE(tile.getSpawnIntervalSeconds(), 60);

    // Test with a different SpawnData object
    RME::SpawnData spawnData2(testPos, 1, 30, {"Cyclops", "Orc"});
    tile.setSpawnDataRef(&spawnData2);
    QVERIFY(tile.getSpawnDataRef() == &spawnData2);
    QCOMPARE(tile.getSpawnRadius(), 1);
    QCOMPARE(tile.getSpawnCreatureList(), QStringList({"Cyclops", "Orc"}));
    QCOMPARE(tile.getSpawnIntervalSeconds(), 30);

    tile.clearSpawnDataRef();
    QVERIFY(tile.getSpawnDataRef() == nullptr);
    QVERIFY(!tile.isSpawnTile());
    QCOMPARE(tile.getSpawnRadius(), 0); // Check defaults after clearing
    QVERIFY(tile.getSpawnCreatureList().isEmpty());
    QCOMPARE(tile.getSpawnIntervalSeconds(), 0);
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

void TestTile::testSelectionStateOnTile()
{
    Tile tile(testPos, &mockProvider);
    QVERIFY(!tile.isSelected()); // Test default
    QVERIFY(!tile.hasStateFlag(TileStateFlag::SELECTED));

    tile.setSelected(true);
    QVERIFY(tile.isSelected());
    QVERIFY(tile.hasStateFlag(TileStateFlag::SELECTED));

    tile.setSelected(false);
    QVERIFY(!tile.isSelected());
    QVERIFY(!tile.hasStateFlag(TileStateFlag::SELECTED));
}

void TestTile::testHasSelectedElements()
{
    Tile tile(testPos, &mockProvider);
    auto itemData = Item::create(SWORD_ID, &mockProvider); // SWORD_ID from initTestCase
    Item* item1Ptr = itemData.get();
    tile.addItem(std::move(itemData));

    // Case 1: Nothing selected
    QVERIFY(!tile.hasSelectedElements());

    // Case 2: Tile itself selected
    tile.setSelected(true);
    QVERIFY(tile.hasSelectedElements());
    tile.setSelected(false); // Reset tile selection

    // Case 3: An item on the tile is selected
    QVERIFY(item1Ptr != nullptr);
    item1Ptr->setSelected(true);
    QVERIFY(tile.hasSelectedElements());
    item1Ptr->setSelected(false); // Reset item selection

    // Case 4: Ground item selected
    auto groundItemData = Item::create(DIRT_ID, &mockProvider);
    Item* groundItemPtr = groundItemData.get();
    tile.setGround(std::move(groundItemData));
    QVERIFY(groundItemPtr != nullptr);
    groundItemPtr->setSelected(true);
    QVERIFY(tile.hasSelectedElements());
    groundItemPtr->setSelected(false); // Reset ground item selection
    tile.setGround(nullptr); // Clear ground

    // Case 5: Both tile and an item selected (should still be true)
    tile.setSelected(true);
    item1Ptr->setSelected(true);
    QVERIFY(tile.hasSelectedElements());
    // Reset for sanity
    tile.setSelected(false);
    item1Ptr->setSelected(false);
    QVERIFY(!tile.hasSelectedElements());

    // Note: Creature and SpawnData selection influences on hasSelectedElements are not tested here
    // as their isSelected() methods are not part of this immediate refactor's scope.
    // The Tile::hasSelectedElements implementation will call them, and they'd default to false
    // if the methods don't exist or the objects are null.
}


QTEST_MAIN(TestTile)
#include "TestTile.moc"
