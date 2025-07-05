#include <QtTest/QtTest>
#include "core/items/DoorItem.h"
#include "tests/core/MockItemTypeProvider.h"

class TestDoorItem : public QObject {
    Q_OBJECT
private:
    RME::MockItemTypeProvider mockProvider;
    const uint16_t DOOR_ID = 3002;
private slots:
    void init() {
        MockItemData doorData;
        doorData.isDoor = true;
        mockProvider.setMockData(DOOR_ID, doorData);
    }
    void construction() {
        RME::DoorItem door(DOOR_ID, &mockProvider);
        QCOMPARE(door.getID(), DOOR_ID);
        QCOMPARE(door.getDoorId(), static_cast<uint8_t>(0)); // Default
    }
    void setAndGetDoorId() {
        RME::DoorItem door(DOOR_ID, &mockProvider);
        door.setDoorId(42);
        QCOMPARE(door.getDoorId(), static_cast<uint8_t>(42));
    }
    void deepCopy() {
        RME::DoorItem original(DOOR_ID, &mockProvider);
        original.setDoorId(7);
        original.setAttribute("action_id", (qulonglong)1234); // QVariant stores uint as qulonglong

        std::unique_ptr<RME::Item> copyBase = original.deepCopy();
        RME::DoorItem* copy = dynamic_cast<RME::DoorItem*>(copyBase.get());
        QVERIFY(copy != nullptr);
        QCOMPARE(copy->getID(), original.getID());
        QCOMPARE(copy->getAttribute("action_id").toInt(), 1234);
        QCOMPARE(copy->getDoorId(), original.getDoorId());
    }
};
// QTEST_APPLESS_MAIN(TestDoorItem)
// #include "TestDoorItem.moc"
