#include <QtTest/QtTest>
#include <QDebug> // For QVERIFY2 messages

#include "core/creatures/Creature.h"
#include "core/assets/CreatureData.h" // Assuming this defines CreatureData
#include "core/assets/Outfit.h"       // Assuming this defines Outfit
#include "core/Position.h"

// Using namespace for convenience in test file
using namespace RME::core;
using namespace RME::core::assets;
using namespace RME::core::creatures;

class TestCreature : public QObject
{
    Q_OBJECT

public:
    TestCreature();
    ~TestCreature() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstruction();
    void testSettersAndGetters();
    void testFlagManagement();
    void testDeepCopy();
    void testDelegationGetters();

private:
    // Test fixture members
    // These need to be pointers if we want to new/delete them in init/cleanup,
    // or ensure they have default constructors and can be assigned.
    // For complex types like CreatureData that might not have trivial default constructors,
    // pointers or std::optional/unique_ptr are better for fixtures.
    // However, the prompt shows them as direct members. This implies they have default constructors
    // or will be constructed in the TestCreature constructor / initList.
    // Let's make them direct members and initialize in init().
    CreatureData m_testCreatureData1;
    CreatureData m_testCreatureData2;
};

// Constructor for the test class
TestCreature::TestCreature() {
    // If m_testCreatureData1/2 needed complex construction not suitable for init(),
    // it could be done here or in initTestCase if they are static.
    // For now, init() will handle their setup.
}

TestCreature::~TestCreature() {
}

void TestCreature::initTestCase() {
    // Global setup for all tests in this class, if any.
    // For example, loading some global asset definition if CreatureData relied on it.
}

void TestCreature::cleanupTestCase() {
    // Global cleanup.
}

void TestCreature::init() {
    // Per-test setup. Initialize CreatureData instances.
    // This assumes CreatureData can be default constructed then members set,
    // or has appropriate setters/constructor.

    // Setup for m_testCreatureData1
    m_testCreatureData1.name = "Goblin";
    m_testCreatureData1.defaultOutfit.lookType = 100;
    m_testCreatureData1.defaultOutfit.lookHead = 1;
    m_testCreatureData1.defaultOutfit.lookBody = 2;
    m_testCreatureData1.defaultOutfit.lookLegs = 3;
    m_testCreatureData1.defaultOutfit.lookFeet = 4;
    m_testCreatureData1.defaultOutfit.lookAddons = 0;
    m_testCreatureData1.defaultOutfit.lookMount = 0;
    // Assuming CreatureData has a field for flags that can be converted/used
    // For now, let's say CreatureData itself doesn't store CreatureFlags directly
    // but has a property that isNpc() in Creature::isNpc() will use.
    m_testCreatureData1.isNpc = false; // Example: Goblin is not an NPC by default type

    // Setup for m_testCreatureData2
    m_testCreatureData2.name = "Sheep";
    m_testCreatureData2.defaultOutfit.lookType = 101;
    m_testCreatureData2.defaultOutfit.lookHead = 5;
    m_testCreatureData2.defaultOutfit.lookBody = 5;
    m_testCreatureData2.defaultOutfit.lookLegs = 5;
    m_testCreatureData2.defaultOutfit.lookFeet = 5;
    m_testCreatureData2.defaultOutfit.lookAddons = 1;
    m_testCreatureData2.defaultOutfit.lookMount = 0;
    m_testCreatureData2.isNpc = true; // Example: Sheep is an NPC by default type
}

void TestCreature::cleanup() {
    // Per-test cleanup.
}

void TestCreature::testConstruction() {
    Position pos(10, 20, 7);
    Creature creature(&m_testCreatureData1, pos);

    QCOMPARE(creature.getType(), &m_testCreatureData1);
    QCOMPARE(creature.getPosition(), pos);
    QCOMPARE(creature.getOutfit(), m_testCreatureData1.defaultOutfit); // Assumes Outfit has operator==
    QCOMPARE(creature.getInstanceName(), m_testCreatureData1.name);

    // Initial flags might be default (NONE) or copied from CreatureData if such a field existed.
    // The Creature constructor initializes m_flags to NONE unless CreatureData provides initial flags.
    // For this test, let's assume it initializes to NONE then copies type-specific behavioral flags.
    // This depends on Creature constructor and CreatureData fields.
    // If CreatureData had `defaultCreatureFlags`, we'd compare to that.
    // For now, assume Creature constructor sets some defaults or copies from type.
    // The current Creature constructor just sets m_flags to NONE.
    QCOMPARE(creature.getFlags(), CreatureFlags(CreatureFlagValue::NONE));
}

void TestCreature::testSettersAndGetters() {
    Position initialPos(10, 10, 7);
    Creature creature(&m_testCreatureData1, initialPos);

    // Test Position
    Position newPos(20, 30, 7);
    creature.setPosition(newPos);
    QCOMPARE(creature.getPosition(), newPos);

    // Test Instance Name
    QString newInstanceName = "Gobbly";
    creature.setInstanceName(newInstanceName);
    QCOMPARE(creature.getInstanceName(), newInstanceName);
    QCOMPARE(creature.getStaticName(), m_testCreatureData1.name); // Static name should remain

    // Test Full Outfit
    Outfit newOutfit;
    newOutfit.lookType = 200;
    newOutfit.lookHead = 10; newOutfit.lookBody = 11; newOutfit.lookLegs = 12; newOutfit.lookFeet = 13;
    newOutfit.lookAddons = 2; newOutfit.lookMount = 201;
    creature.setOutfit(newOutfit);
    QCOMPARE(creature.getOutfit(), newOutfit); // Assumes Outfit has operator==

    // Test Individual Outfit Parts
    creature.setLookType(250);
    QCOMPARE(creature.getOutfit().lookType, static_cast<quint16>(250));
    creature.setLookHead(15);
    QCOMPARE(creature.getOutfit().lookHead, static_cast<quint8>(15));
    creature.setLookBody(16);
    QCOMPARE(creature.getOutfit().lookBody, static_cast<quint8>(16));
    creature.setLookLegs(17);
    QCOMPARE(creature.getOutfit().lookLegs, static_cast<quint8>(17));
    creature.setLookFeet(18);
    QCOMPARE(creature.getOutfit().lookFeet, static_cast<quint8>(18));
    creature.setLookAddons(3);
    QCOMPARE(creature.getOutfit().lookAddons, static_cast<quint8>(3));
    creature.setLookMount(251);
    QCOMPARE(creature.getOutfit().lookMount, static_cast<quint16>(251));
}

void TestCreature::testFlagManagement() {
    Creature creature(&m_testCreatureData1, {0,0,0});

    QCOMPARE(creature.getFlags(), CreatureFlags(CreatureFlagValue::NONE)); // Initial state

    // Set multiple flags
    CreatureFlags flagsToSet = CreatureFlagValue::CAN_SUMMON | CreatureFlagValue::IS_HOSTILE;
    creature.setFlags(flagsToSet);
    QCOMPARE(creature.getFlags(), flagsToSet);
    QVERIFY(creature.hasFlag(CreatureFlagValue::CAN_SUMMON));
    QVERIFY(creature.hasFlag(CreatureFlagValue::IS_HOSTILE));
    QVERIFY(!creature.hasFlag(CreatureFlagValue::PUSHABLE));

    // Add a flag
    creature.addFlag(CreatureFlagValue::PUSHABLE);
    QVERIFY(creature.hasFlag(CreatureFlagValue::PUSHABLE));
    QVERIFY(creature.hasFlag(CreatureFlagValue::CAN_SUMMON)); // Should still be there

    // Remove a flag
    creature.removeFlag(CreatureFlagValue::CAN_SUMMON);
    QVERIFY(!creature.hasFlag(CreatureFlagValue::CAN_SUMMON));
    QVERIFY(creature.hasFlag(CreatureFlagValue::IS_HOSTILE)); // Should still be there
    QVERIFY(creature.hasFlag(CreatureFlagValue::PUSHABLE));   // Should still be there

    // Set back to NONE
    creature.setFlags(CreatureFlagValue::NONE);
    QCOMPARE(creature.getFlags(), CreatureFlags(CreatureFlagValue::NONE));
}

void TestCreature::testDeepCopy() {
    Position originalPos(5,5,5);
    Creature original(&m_testCreatureData1, originalPos);
    original.setInstanceName("Original Goblin");
    original.addFlag(CreatureFlagValue::LIGHT_EMITTING);
    Outfit modifiedOutfit = original.getOutfit();
    modifiedOutfit.lookBody = 50;
    original.setOutfit(modifiedOutfit);

    std::unique_ptr<Creature> copy = original.deepCopy();

    QVERIFY(copy != nullptr);
    QVERIFY(copy.get() != &original);

    QCOMPARE(copy->getType(), original.getType());
    QCOMPARE(copy->getPosition(), original.getPosition());
    QCOMPARE(copy->getOutfit(), original.getOutfit()); // Assumes Outfit has operator==
    QCOMPARE(copy->getFlags(), original.getFlags());
    QCOMPARE(copy->getInstanceName(), original.getInstanceName());

    // Modify copy and check original is not affected
    copy->setInstanceName("Copied Goblin");
    Outfit copyOutfit = copy->getOutfit();
    copyOutfit.lookBody = 60;
    copy->setOutfit(copyOutfit);
    copy->addFlag(CreatureFlagValue::CAN_SUMMON);

    QCOMPARE(original.getInstanceName(), QString("Original Goblin"));
    QCOMPARE(original.getOutfit().lookBody, static_cast<quint8>(50));
    QVERIFY(original.hasFlag(CreatureFlagValue::LIGHT_EMITTING));
    QVERIFY(!original.hasFlag(CreatureFlagValue::CAN_SUMMON));
}

void TestCreature::testDelegationGetters() {
    Creature creature1(&m_testCreatureData1, {0,0,0}); // Goblin, isNpc = false in init()
    Creature creature2(&m_testCreatureData2, {1,1,1}); // Sheep, isNpc = true in init()

    QCOMPARE(creature1.getStaticName(), m_testCreatureData1.name); // "Goblin"
    QCOMPARE(creature1.isNpc(), false); // Based on m_testCreatureData1.isNpc set in init()

    QCOMPARE(creature2.getStaticName(), m_testCreatureData2.name); // "Sheep"
    QCOMPARE(creature2.isNpc(), true);  // Based on m_testCreatureData2.isNpc set in init()
}


// QTEST_MAIN(TestCreature) // Will be run by rme_core_creatures_tests
#include "TestCreature.moc" // Must be last line
