#include <QtTest/QtTest>
#include "core/world/TownData.h"
#include "core/Position.h"

class TestTownData : public QObject {
    Q_OBJECT
private slots:
    void constructionAndAccessors() {
        RME::TownData town1; // Default constructor
        QCOMPARE(town1.getId(), static_cast<uint32_t>(0));
        QVERIFY(town1.getName().isEmpty());
        QCOMPARE(town1.getTemplePosition(), RME::Position(0,0,0));

        RME::Position templePos(100, 200, 7);
        RME::TownData town2(1, "Testville", templePos);
        QCOMPARE(town2.getId(), static_cast<uint32_t>(1));
        QCOMPARE(town2.getName(), QString("Testville"));
        QCOMPARE(town2.getTemplePosition(), templePos);

        // Test setters
        town1.setName("New Name");
        QCOMPARE(town1.getName(), QString("New Name"));
        RME::Position newTemplePos(50,60,7);
        town1.setTemplePosition(newTemplePos);
        QCOMPARE(town1.getTemplePosition(), newTemplePos);
    }

    void comparisonOperators() {
        RME::Position pos1(10,20,7);
        RME::Position pos2(30,40,7);

        RME::TownData townA(1, "TownA", pos1);
        RME::TownData townB(1, "TownA", pos1); // Same as A
        RME::TownData townC(2, "TownA", pos1); // Different ID
        RME::TownData townD(1, "TownD", pos1); // Different Name
        RME::TownData townE(1, "TownA", pos2); // Different Position

        QVERIFY(townA == townB);
        QVERIFY(townA != townC);
        QVERIFY(townA != townD);
        QVERIFY(townA != townE);
        QVERIFY(!(townA == townC));
    }
};
// QTEST_APPLESS_MAIN(TestTownData)
// #include "TestTownData.moc"
