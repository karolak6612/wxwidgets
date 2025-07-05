#include <QtTest/QtTest>
#include <QString>
#include <memory>

#include "core/creatures/Creature.h"
#include "core/creatures/Outfit.h"
#include "core/assets/CreatureData.h" // For RME::core::assets::CreatureData
#include "core/Position.h"

// Mock CreatureData for testing purposes
struct MockCreatureData : public RME::core::assets::CreatureData {
    MockCreatureData(uint16_t id, const QString& n, bool npc = false, bool passable = true) {
        this->id = id; // Assuming CreatureData has an 'id' field or similar identifier
        this->name = n;
        this->isNpc = npc;
        this->isPassable = passable;
        // Setup a default outfit
        this->defaultOutfit.lookType = 128; // Example default
        this->defaultOutfit.lookHead = 78;
        this->defaultOutfit.lookBody = 95;
        this->defaultOutfit.lookLegs = 114;
        this->defaultOutfit.lookFeet = 115;
        this->defaultOutfit.lookAddons = 1;
    }
};

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
    void testOutfitModification();
    void testDeepCopy();
    void testFlagManagement();
    void testDelegatedGetters();
    void testPositionManagement();
    void testCopyAndMoveSemantics();

private:
    MockCreatureData* m_mockType1;       // e.g., a monster type
    MockCreatureData* m_mockType2_npc;   // e.g., an NPC type
    RME::core::creatures::Creature* m_creature; // Creature under test
};

TestCreature::TestCreature() : m_mockType1(nullptr), m_mockType2_npc(nullptr), m_creature(nullptr) {}
TestCreature::~TestCreature() {
    // initTestCase and cleanupTestCase handle m_mockType1 and m_mockType2_npc
    // init and cleanup handle m_creature
}

void TestCreature::initTestCase() {
    m_mockType1 = new MockCreatureData(1, "Dragon", false, false); // name, isNpc, isPassable
    m_mockType1->defaultOutfit.lookType = 100;
    m_mockType1->defaultOutfit.lookAddons = 0;

    m_mockType2_npc = new MockCreatureData(2, "Guard", true, true);
    m_mockType2_npc->defaultOutfit.lookType = 130; // Human male
    m_mockType2_npc->defaultOutfit.lookAddons = 3; // Addons 1 & 2
}

void TestCreature::cleanupTestCase() {
    delete m_mockType1;
    m_mockType1 = nullptr;
    delete m_mockType2_npc;
    m_mockType2_npc = nullptr;
}

void TestCreature::init() {
    // Default creature for most tests
    RME::core::Position startPos(10, 20, 7);
    m_creature = new RME::core::creatures::Creature(m_mockType1, startPos);
}

void TestCreature::cleanup() {
    delete m_creature;
    m_creature = nullptr;
}

void TestCreature::testConstruction() {
    QVERIFY(m_creature != nullptr);
    QCOMPARE(m_creature->getType(), m_mockType1);
    QCOMPARE(m_creature->getPosition(), RME::core::Position(10, 20, 7));
    QCOMPARE(m_creature->getOutfit(), m_mockType1->defaultOutfit); // Check if default outfit is copied

    // Check initial flags based on CreatureData
    QVERIFY(m_creature->hasFlag(RME::core::creatures::CreatureFlag::UNPASSABLE)); // From m_mockType1->isPassable = false
    QVERIFY(!m_creature->hasFlag(RME::core::creatures::CreatureFlag::NPC));      // From m_mockType1->isNpc = false

    RME::core::Position npcPos(5,5,5);
    RME::core::creatures::Creature npcCreature(m_mockType2_npc, npcPos);
    QCOMPARE(npcCreature.getOutfit(), m_mockType2_npc->defaultOutfit);
    QVERIFY(npcCreature.hasFlag(RME::core::creatures::CreatureFlag::NPC));
    QVERIFY(!npcCreature.hasFlag(RME::core::creatures::CreatureFlag::UNPASSABLE)); // From m_mockType2_npc->isPassable = true
}

void TestCreature::testOutfitModification() {
    RME::core::creatures::Outfit newOutfit(133, 1,2,3,4, 1, 1288, 0);
    m_creature->setOutfit(newOutfit);
    QCOMPARE(m_creature->getOutfit(), newOutfit);

    m_creature->setLookType(140);
    QCOMPARE(m_creature->getOutfit().lookType, static_cast<uint16_t>(140));

    m_creature->setLookAddons(3);
    QCOMPARE(m_creature->getOutfit().lookAddons, static_cast<uint8_t>(3));
    QVERIFY(m_creature->getOutfit().hasAddon(1));
    QVERIFY(m_creature->getOutfit().hasAddon(2));

    m_creature->setAddonFlag(1, false); // Remove addon 1
    QVERIFY(!m_creature->getOutfit().hasAddon(1));
    QVERIFY(m_creature->getOutfit().hasAddon(2));
    QCOMPARE(m_creature->getOutfit().lookAddons, static_cast<uint8_t>(2));
}

void TestCreature::testDeepCopy() {
    m_creature->setLookType(155);
    m_creature->addFlag(RME::core::creatures::CreatureFlag::SUMMON);
    m_creature->setPosition({1,2,3});

    std::unique_ptr<RME::core::creatures::Creature> copiedCreature = m_creature->deepCopy();
    QVERIFY(copiedCreature != nullptr);
    QVERIFY(copiedCreature.get() != m_creature); // Ensure it's a different object

    QCOMPARE(copiedCreature->getType(), m_creature->getType()); // Pointer to type should be same
    QCOMPARE(copiedCreature->getPosition(), m_creature->getPosition());
    QCOMPARE(copiedCreature->getOutfit(), m_creature->getOutfit());
    QCOMPARE(copiedCreature->getFlags(), m_creature->getFlags());
    QVERIFY(copiedCreature->hasFlag(RME::core::creatures::CreatureFlag::SUMMON));
    QVERIFY(copiedCreature->hasFlag(RME::core::creatures::CreatureFlag::UNPASSABLE));
}

void TestCreature::testFlagManagement() {
    m_creature->setFlags(RME::core::creatures::CreatureFlag::NONE);
    QCOMPARE(m_creature->getFlags(), RME::core::creatures::CreatureFlag::NONE);

    m_creature->addFlag(RME::core::creatures::CreatureFlag::SUMMON);
    QVERIFY(m_creature->hasFlag(RME::core::creatures::CreatureFlag::SUMMON));
    QVERIFY((m_creature->getFlags() & RME::core::creatures::CreatureFlag::SUMMON) != RME::core::creatures::CreatureFlag::NONE);

    m_creature->addFlag(RME::core::creatures::CreatureFlag::PERSISTENT);
    QVERIFY(m_creature->hasFlag(RME::core::creatures::CreatureFlag::SUMMON));
    QVERIFY(m_creature->hasFlag(RME::core::creatures::CreatureFlag::PERSISTENT));

    m_creature->removeFlag(RME::core::creatures::CreatureFlag::SUMMON);
    QVERIFY(!m_creature->hasFlag(RME::core::creatures::CreatureFlag::SUMMON));
    QVERIFY(m_creature->hasFlag(RME::core::creatures::CreatureFlag::PERSISTENT));
}

void TestCreature::testDelegatedGetters() {
    QCOMPARE(m_creature->getName(), m_mockType1->name);

    RME::core::Position npcPos(1,1,1);
    RME::core::creatures::Creature npcCreature(m_mockType2_npc, npcPos);
    QCOMPARE(npcCreature.getName(), m_mockType2_npc->name);
    QVERIFY(npcCreature.isNpc());
}

void TestCreature::testPositionManagement() {
    RME::core::Position newPos(100, 200, 3);
    m_creature->setPosition(newPos);
    QCOMPARE(m_creature->getPosition(), newPos);
}

void TestCreature::testCopyAndMoveSemantics(){
    RME::core::creatures::Creature original(m_mockType1, {1,1,1});
    original.setLookType(188);
    original.addFlag(RME::core::creatures::CreatureFlag::SUMMON);

    // Test Copy Constructor
    RME::core::creatures::Creature copied(original);
    QCOMPARE(copied.getType(), original.getType());
    QCOMPARE(copied.getPosition(), original.getPosition());
    QCOMPARE(copied.getOutfit(), original.getOutfit());
    QCOMPARE(copied.getFlags(), original.getFlags());

    // Test Copy Assignment
    RME::core::creatures::Creature assigned(m_mockType2_npc, {2,2,2});
    assigned = original;
    QCOMPARE(assigned.getType(), original.getType());
    QCOMPARE(assigned.getPosition(), original.getPosition());
    QCOMPARE(assigned.getOutfit(), original.getOutfit());
    QCOMPARE(assigned.getFlags(), original.getFlags());

    // Test Move Constructor
    RME::core::creatures::Outfit originalOutfit = original.getOutfit(); // Save for comparison
    RME::core::Position originalPosition = original.getPosition();
    RME::core::creatures::CreatureFlag originalFlags = original.getFlags();

    RME::core::creatures::Creature moved(std::move(original));
    QCOMPARE(moved.getType(), m_mockType1);
    QCOMPARE(moved.getPosition(), originalPosition);
    QCOMPARE(moved.getOutfit(), originalOutfit);
    QCOMPARE(moved.getFlags(), originalFlags);
    // `original` is now in a valid but unspecified state. Its members might be cleared or reset.

    // Test Move Assignment
    // Re-initialize 'original' to a known state for move assignment test
    original = RME::core::creatures::Creature(m_mockType1, {3,3,3});
    original.setLookType(199);
    original.addFlag(RME::core::creatures::CreatureFlag::PERSISTENT);
    RME::core::Outfit outfitForMoveAssign = original.getOutfit();
    RME::core::Position posForMoveAssign = original.getPosition();
    RME::core::creatures::CreatureFlag flagsForMoveAssign = original.getFlags();

    RME::core::creatures::Creature move_assigned_to(m_mockType2_npc, {4,4,4});
    move_assigned_to = std::move(original);
    QCOMPARE(move_assigned_to.getType(), m_mockType1);
    QCOMPARE(move_assigned_to.getPosition(), posForMoveAssign);
    QCOMPARE(move_assigned_to.getOutfit(), outfitForMoveAssign);
    QCOMPARE(move_assigned_to.getFlags(), flagsForMoveAssign);
}


// QTEST_MAIN(TestCreature) // Will be run by a main test runner
#include "TestCreature.moc" // Must be last line for MOC to work
