#include <QtTest/QtTest>
#include "core/Item.h" // Class to test
#include "MockItemTypeProvider.h" // Mock provider

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


QTEST_MAIN(TestItem)
#include "TestItem.moc"
