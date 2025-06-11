#include <QtTest/QtTest>
#include "core/items/DepotItem.h"
#include "tests/core/MockItemTypeProvider.h"

class TestDepotItem : public QObject {
    Q_OBJECT
private:
    RME::MockItemTypeProvider mockProvider;
    const uint16_t DEPOT_ID = 3003;
private slots:
    void init() {
        MockItemData depotData;
        depotData.isDepot = true;
        mockProvider.setMockData(DEPOT_ID, depotData);
    }
    void construction() {
        RME::DepotItem depot(DEPOT_ID, &mockProvider);
        QCOMPARE(depot.getID(), DEPOT_ID);
        QCOMPARE(depot.getDepotId(), static_cast<uint8_t>(0)); // Default
    }
    void setAndGetDepotId() {
        RME::DepotItem depot(DEPOT_ID, &mockProvider);
        depot.setDepotId(15);
        QCOMPARE(depot.getDepotId(), static_cast<uint8_t>(15));
    }
    void deepCopy() {
        RME::DepotItem original(DEPOT_ID, &mockProvider);
        original.setDepotId(3);
        original.setAttribute("desc", "My Depot");

        std::unique_ptr<RME::Item> copyBase = original.deepCopy();
        RME::DepotItem* copy = dynamic_cast<RME::DepotItem*>(copyBase.get());
        QVERIFY(copy != nullptr);
        QCOMPARE(copy->getID(), original.getID());
        QCOMPARE(copy->getAttribute("desc").toString(), QString("My Depot"));
        QCOMPARE(copy->getDepotId(), original.getDepotId());
    }
};
// QTEST_APPLESS_MAIN(TestDepotItem)
// #include "TestDepotItem.moc"
