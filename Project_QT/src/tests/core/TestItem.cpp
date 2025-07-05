#include <QtTest/QtTest>
#include "core/Item.h" // Class to test
#include "MockItemTypeProvider.h" // Mock provider
#include "core/items/ContainerItem.h"
#include "core/items/TeleportItem.h"
#include "core/items/DoorItem.h"
#include "core/items/DepotItem.h"
#include "core/items/PodiumItem.h"

using namespace RME;

class TestItem : public QObject
{
    Q_OBJECT

private:
    MockItemTypeProvider mockProvider;
    // Define some item IDs for testing
    const uint16_t SWORD_ID = 100;
    const uint16_t SHIELD_ID = 101;
    const uint16_t GOLD_COIN_ID = 102; // Assume stackable
    const uint16_t APPLE_ID = 103;     // Generic item

private slots:
    void initTestCase(); // Called once before all tests

    void testItemCreation();
    void testSubtype();
    void testAttributes();
    void testConvenienceAttributes();
    void testDeepCopy();
    void testPropertyDelegation();
    void testWeightCalculation();
    void itemCreateFactory();
};

void TestItem::initTestCase() {
    MockItemData swordData;
    swordData.name = "Test Sword";
    swordData.isBlocking = false; // Swords usually don't block terrain
    swordData.weight = 15.0;
    mockProvider.setMockData(SWORD_ID, swordData);

    MockItemData shieldData;
    shieldData.name = "Test Shield";
    shieldData.isBlocking = false; // Handled by creature usually
    shieldData.weight = 20.0;
    mockProvider.setMockData(SHIELD_ID, shieldData);

    MockItemData goldData;
    goldData.name = "Gold Coin";
    goldData.isStackable = true;
    goldData.weight = 0.1;
    mockProvider.setMockData(GOLD_COIN_ID, goldData);

    MockItemData appleData;
    appleData.name = "Apple";
    appleData.weight = 1.0;
    mockProvider.setMockData(APPLE_ID, appleData);
}

void TestItem::testItemCreation()
{
    auto item = Item::create(SWORD_ID, &mockProvider, 1);
    QVERIFY(item != nullptr);
    QCOMPARE(item->getID(), SWORD_ID);
    QCOMPARE(item->getSubtype(), 1); // Default subtype for non-stackable or single count
    QVERIFY(item->getTypeProvider() == &mockProvider);
}

void TestItem::testSubtype()
{
    auto gold = Item::create(GOLD_COIN_ID, &mockProvider, 50); // 50 gold coins
    QCOMPARE(gold->getSubtype(), 50);
    QVERIFY(gold->hasSubtype()); // Stackable with count > 1

    gold->setSubtype(100);
    QCOMPARE(gold->getSubtype(), 100);

    auto apple = Item::create(APPLE_ID, &mockProvider, 1); // subtype for non-stackable might be charges, fluid etc.
    QVERIFY(apple->isStackable() == false);
    QVERIFY(apple->hasSubtype() == false); // Assuming subtype 1 for non-stackable means no special subtype meaning here

    apple->setSubtype(5); // e.g. 5 charges
    QVERIFY(apple->hasSubtype());
}

void TestItem::testAttributes()
{
    auto item = Item::create(APPLE_ID, &mockProvider);
    QVERIFY(!item->hasAttribute("color"));
    item->setAttribute("color", "red");
    QVERIFY(item->hasAttribute("color"));
    QCOMPARE(item->getAttribute("color").toString(), QString("red"));

    item->setAttribute("freshness", 0.9);
    QCOMPARE(item->getAttribute("freshness").toDouble(), 0.9);

    item->clearAttribute("color");
    QVERIFY(!item->hasAttribute("color"));
    QVERIFY(item->getAttribute("color").isNull());
}

void TestItem::testConvenienceAttributes()
{
    auto item = Item::create(APPLE_ID, &mockProvider);
    item->setUniqueID(12345);
    QCOMPARE(item->getUniqueID(), 12345);

    item->setActionID(54321);
    QCOMPARE(item->getActionID(), 54321);

    item->setText("A juicy red apple.");
    QCOMPARE(item->getText(), QString("A juicy red apple."));
}

void TestItem::testDeepCopy()
{
    auto original = Item::create(SWORD_ID, &mockProvider, 1);
    original->setAttribute("enhancement", "sharpness +5");
    original->setUniqueID(777);

    auto copy = original->deepCopy();
    QVERIFY(copy != nullptr);
    QVERIFY(copy.get() != original.get()); // Different objects

    QCOMPARE(copy->getID(), original->getID());
    QCOMPARE(copy->getSubtype(), original->getSubtype());
    QVERIFY(copy->getTypeProvider() == original->getTypeProvider());

    QCOMPARE(copy->getAttribute("enhancement").toString(), QString("sharpness +5"));
    QCOMPARE(copy->getUniqueID(), 777);

    // Ensure attributes are a separate copy
    copy->setAttribute("enhancement", "sharpness +10");
    QCOMPARE(original->getAttribute("enhancement").toString(), QString("sharpness +5"));
    QCOMPARE(copy->getAttribute("enhancement").toString(), QString("sharpness +10"));
}

void TestItem::testPropertyDelegation()
{
    auto sword = Item::create(SWORD_ID, &mockProvider);
    QCOMPARE(sword->getName(), QString("Test Sword"));
    QCOMPARE(sword->isStackable(), false);

    auto gold = Item::create(GOLD_COIN_ID, &mockProvider);
    QCOMPARE(gold->getName(), QString("Gold Coin"));
    QCOMPARE(gold->isStackable(), true);
}

void TestItem::testWeightCalculation()
{
    auto sword = Item::create(SWORD_ID, &mockProvider);
    QCOMPARE(sword->getWeight(), 15.0); // From mock data

    auto goldOne = Item::create(GOLD_COIN_ID, &mockProvider, 1); // 1 gold coin
    QCOMPARE(goldOne->getWeight(), 0.1); // Weight of 1 coin

    auto goldHundred = Item::create(GOLD_COIN_ID, &mockProvider, 100); // 100 gold coins
    QCOMPARE(goldHundred->getWeight(), 10.0); // 0.1 * 100
}

void TestItem::itemCreateFactory() {
    RME::MockItemTypeProvider provider;

    // Setup mock data for different types
    uint16_t baseId = 1000, containerId = 1001, teleportId = 1002, doorId = 1003, depotId = 1004, podiumId = 1005;
    MockItemData baseData; /* default */
    MockItemData containerData; containerData.isContainer = true;
    MockItemData teleportData; teleportData.isTeleport = true;
    MockItemData doorData; doorData.isDoor = true;
    MockItemData depotData; depotData.isDepot = true;
    MockItemData podiumData; podiumData.isPodium = true;

    provider.setMockData(baseId, baseData);
    provider.setMockData(containerId, containerData);
    provider.setMockData(teleportId, teleportData);
    provider.setMockData(doorId, doorData);
    provider.setMockData(depotId, depotData);
    provider.setMockData(podiumId, podiumData);

    // Test creation of base item
    auto itemBase = RME::Item::create(baseId, &provider);
    QVERIFY(itemBase != nullptr);
    QVERIFY(dynamic_cast<RME::Item*>(itemBase.get()) != nullptr);
    QVERIFY(dynamic_cast<RME::ContainerItem*>(itemBase.get()) == nullptr); // Should not be a container

    // Test creation of ContainerItem
    auto itemContainer = RME::Item::create(containerId, &provider);
    QVERIFY(itemContainer != nullptr);
    QVERIFY(dynamic_cast<RME::ContainerItem*>(itemContainer.get()) != nullptr);

    // Test creation of TeleportItem
    auto itemTeleport = RME::Item::create(teleportId, &provider);
    QVERIFY(itemTeleport != nullptr);
    QVERIFY(dynamic_cast<RME::TeleportItem*>(itemTeleport.get()) != nullptr);

    // Test creation of DoorItem
    auto itemDoor = RME::Item::create(doorId, &provider);
    QVERIFY(itemDoor != nullptr);
    QVERIFY(dynamic_cast<RME::DoorItem*>(itemDoor.get()) != nullptr);

    // Test creation of DepotItem
    auto itemDepot = RME::Item::create(depotId, &provider);
    QVERIFY(itemDepot != nullptr);
    QVERIFY(dynamic_cast<RME::DepotItem*>(itemDepot.get()) != nullptr);

    // Test creation of PodiumItem
    auto itemPodium = RME::Item::create(podiumId, &provider);
    QVERIFY(itemPodium != nullptr);
    QVERIFY(dynamic_cast<RME::PodiumItem*>(itemPodium.get()) != nullptr);

    // Test with null provider (should create base Item)
    auto itemNullProvider = RME::Item::create(containerId, nullptr);
    QVERIFY(itemNullProvider != nullptr);
    QVERIFY(dynamic_cast<RME::Item*>(itemNullProvider.get()) != nullptr);
    QVERIFY(dynamic_cast<RME::ContainerItem*>(itemNullProvider.get()) == nullptr);
}

// QTEST_MAIN(TestItem) // Will be handled by a central test runner
#include "TestItem.moc"
