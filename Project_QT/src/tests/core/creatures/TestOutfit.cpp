#include <QtTest/QtTest>
#include "core/creatures/Outfit.h"

class TestOutfit : public QObject
{
    Q_OBJECT

public:
    TestOutfit();
    ~TestOutfit() override;

private slots:
    void testDefaultConstructor();
    void testFullConstructor();
    void testCopyConstructor();
    void testAssignmentOperator();
    void testComparisonOperators();
    void testAddonMethods();

};

TestOutfit::TestOutfit() {}
TestOutfit::~TestOutfit() {}

void TestOutfit::testDefaultConstructor() {
    RME::core::creatures::Outfit outfit;
    QCOMPARE(outfit.lookType, static_cast<uint16_t>(0));
    QCOMPARE(outfit.lookItem, static_cast<uint16_t>(0));
    QCOMPARE(outfit.lookMount, static_cast<uint16_t>(0));
    QCOMPARE(outfit.lookHead, static_cast<uint8_t>(0));
    QCOMPARE(outfit.lookBody, static_cast<uint8_t>(0));
    QCOMPARE(outfit.lookLegs, static_cast<uint8_t>(0));
    QCOMPARE(outfit.lookFeet, static_cast<uint8_t>(0));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(0));
}

void TestOutfit::testFullConstructor() {
    RME::core::creatures::Outfit outfit(
        130,    // lookType
        77,     // head
        88,     // body
        99,     // legs
        101,    // feet
        2,      // addons (addon 2)
        1234,   // mount
        5678    // item
    );

    QCOMPARE(outfit.lookType, static_cast<uint16_t>(130));
    QCOMPARE(outfit.lookItem, static_cast<uint16_t>(5678));
    QCOMPARE(outfit.lookMount, static_cast<uint16_t>(1234));
    QCOMPARE(outfit.lookHead, static_cast<uint8_t>(77));
    QCOMPARE(outfit.lookBody, static_cast<uint8_t>(88));
    QCOMPARE(outfit.lookLegs, static_cast<uint8_t>(99));
    QCOMPARE(outfit.lookFeet, static_cast<uint8_t>(101));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(2));
}

void TestOutfit::testCopyConstructor() {
    RME::core::creatures::Outfit original(
        130, 77, 88, 99, 101, 2, 1234, 5678
    );
    RME::core::creatures::Outfit copy = original; // Invokes copy constructor

    QCOMPARE(copy.lookType, original.lookType);
    QCOMPARE(copy.lookItem, original.lookItem);
    QCOMPARE(copy.lookMount, original.lookMount);
    QCOMPARE(copy.lookHead, original.lookHead);
    QCOMPARE(copy.lookBody, original.lookBody);
    QCOMPARE(copy.lookLegs, original.lookLegs);
    QCOMPARE(copy.lookFeet, original.lookFeet);
    QCOMPARE(copy.lookAddons, original.lookAddons);
    QVERIFY(copy == original);
}

void TestOutfit::testAssignmentOperator() {
    RME::core::creatures::Outfit original(
        130, 77, 88, 99, 101, 2, 1234, 5678
    );
    RME::core::creatures::Outfit assigned_outfit;
    assigned_outfit = original; // Invokes copy assignment operator

    QCOMPARE(assigned_outfit.lookType, original.lookType);
    QCOMPARE(assigned_outfit.lookItem, original.lookItem);
    QCOMPARE(assigned_outfit.lookMount, original.lookMount);
    QCOMPARE(assigned_outfit.lookHead, original.lookHead);
    QCOMPARE(assigned_outfit.lookBody, original.lookBody);
    QCOMPARE(assigned_outfit.lookLegs, original.lookLegs);
    QCOMPARE(assigned_outfit.lookFeet, original.lookFeet);
    QCOMPARE(assigned_outfit.lookAddons, original.lookAddons);
    QVERIFY(assigned_outfit == original);
}

void TestOutfit::testComparisonOperators() {
    RME::core::creatures::Outfit outfit1(130, 77, 88, 99, 101, 1, 123, 456);
    RME::core::creatures::Outfit outfit2(130, 77, 88, 99, 101, 1, 123, 456);
    RME::core::creatures::Outfit outfit3(131, 77, 88, 99, 101, 1, 123, 456); // Different lookType
    RME::core::creatures::Outfit outfit4(130, 78, 88, 99, 101, 1, 123, 456); // Different head
    RME::core::creatures::Outfit outfit5(130, 77, 88, 99, 101, 2, 123, 456); // Different addons

    QVERIFY(outfit1 == outfit2);
    QVERIFY(!(outfit1 != outfit2));

    QVERIFY(outfit1 != outfit3);
    QVERIFY(!(outfit1 == outfit3));

    QVERIFY(outfit1 != outfit4);
    QVERIFY(outfit1 != outfit5);
}

void TestOutfit::testAddonMethods() {
    RME::core::creatures::Outfit outfit;

    QVERIFY(!outfit.hasAddon(1)); // Addon bit 0 (value 1)
    QVERIFY(!outfit.hasAddon(2)); // Addon bit 1 (value 2)
    QVERIFY(!outfit.hasAddon(4)); // Addon bit 2 (value 4)

    outfit.setAddon(1, true);
    QVERIFY(outfit.hasAddon(1));
    QVERIFY(!outfit.hasAddon(2));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(1));

    outfit.setAddon(2, true);
    QVERIFY(outfit.hasAddon(1));
    QVERIFY(outfit.hasAddon(2));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(3)); // 1 | 2

    // Test the specific combined addon check from Outfit.h example
    // if (addonBit == 3) return (lookAddons & 0x03) == 0x03; // Both Addons 1 and 2
    // The hasAddon method should really take a single bit flag, not a combination.
    // Let's adjust the interpretation of hasAddon as per the Outfit.h setAddon:
    // hasAddon(addonBit) checks if *that specific bit* is set.

    outfit.setAddon(4, true); // Addon bit 2 (value 4)
    QVERIFY(outfit.hasAddon(4));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(7)); // 1 | 2 | 4

    outfit.setAddon(2, false);
    QVERIFY(outfit.hasAddon(1));
    QVERIFY(!outfit.hasAddon(2));
    QVERIFY(outfit.hasAddon(4));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(5)); // 1 | 4

    outfit.setAddon(1, false);
    outfit.setAddon(4, false);
    QVERIFY(!outfit.hasAddon(1));
    QVERIFY(!outfit.hasAddon(4));
    QCOMPARE(outfit.lookAddons, static_cast<uint8_t>(0));
}

// QTEST_MAIN(TestOutfit) // Will be run by a main test runner
#include "TestOutfit.moc" // Must be last line for MOC to work
