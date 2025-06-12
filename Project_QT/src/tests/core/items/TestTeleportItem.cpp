#include <QtTest/QtTest>
#include "core/items/TeleportItem.h"
#include "core/Position.h"
#include "tests/core/MockItemTypeProvider.h"

class TestTeleportItem : public QObject {
    Q_OBJECT
private:
    RME::MockItemTypeProvider mockProvider;
    const uint16_t TELEPORT_ID = 3001;
private slots:
    void init() {
        MockItemData teleData;
        teleData.isTeleport = true;
        teleData.name = "Magic Teleport";
        mockProvider.setMockData(TELEPORT_ID, teleData);
    }
    void construction() {
        RME::TeleportItem teleport(TELEPORT_ID, &mockProvider);
        QCOMPARE(teleport.getID(), TELEPORT_ID);
        QCOMPARE(teleport.getDestination(), RME::Position(0,0,0)); // Default
        QVERIFY(!teleport.hasDestination());
        QCOMPARE(teleport.getName(), QString("Magic Teleport"));
    }
    void setAndGetDestination() {
        RME::TeleportItem teleport(TELEPORT_ID, &mockProvider);
        RME::Position dest(100, 200, 7);
        teleport.setDestination(dest);
        QCOMPARE(teleport.getDestination(), dest);
        QVERIFY(teleport.hasDestination());
    }
    void deepCopy() {
        RME::TeleportItem original(TELEPORT_ID, &mockProvider);
        original.setDestination(RME::Position(123, 234, 5));
        original.setAttribute("uid", (qulonglong)500); // QVariant stores uint as qulonglong for toInt()

        std::unique_ptr<RME::Item> copyBase = original.deepCopy();
        RME::TeleportItem* copy = dynamic_cast<RME::TeleportItem*>(copyBase.get());
        QVERIFY(copy != nullptr);
        QCOMPARE(copy->getID(), original.getID());
        QCOMPARE(copy->getAttribute("uid").toInt(), 500);
        QCOMPARE(copy->getDestination(), original.getDestination());
    }
};
// QTEST_APPLESS_MAIN(TestTeleportItem)
// #include "TestTeleportItem.moc"
