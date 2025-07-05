#include <QtTest/QtTest>
#include "core/items/PodiumItem.h"
#include "core/assets/Outfit.h"
#include "tests/core/MockItemTypeProvider.h"

class TestPodiumItem : public QObject {
    Q_OBJECT
private:
    RME::MockItemTypeProvider mockProvider;
    const uint16_t PODIUM_ID = 3004;
private slots:
    void init() {
        MockItemData podiumData;
        podiumData.isPodium = true;
        mockProvider.setMockData(PODIUM_ID, podiumData);
    }
    void construction() {
        RME::PodiumItem podium(PODIUM_ID, &mockProvider);
        QCOMPARE(podium.getID(), PODIUM_ID);
        QCOMPARE(podium.getOutfit(), RME::Outfit()); // Default outfit
        QCOMPARE(podium.getDirection(), static_cast<uint8_t>(0));
        QVERIFY(podium.getShowOutfit());
        QVERIFY(podium.getShowMount());
        QVERIFY(podium.getShowPlatform());
    }
    void setAndGetProperties() {
        RME::PodiumItem podium(PODIUM_ID, &mockProvider);
        RME::Outfit newOutfit;
        newOutfit.lookType = 130; newOutfit.head = 1;
        podium.setOutfit(newOutfit);
        QCOMPARE(podium.getOutfit(), newOutfit);

        podium.setDirection(3);
        QCOMPARE(podium.getDirection(), static_cast<uint8_t>(3));

        podium.setShowOutfit(false);
        QVERIFY(!podium.getShowOutfit());
        podium.setShowMount(false);
        QVERIFY(!podium.getShowMount());
        podium.setShowPlatform(false);
        QVERIFY(!podium.getShowPlatform());
    }
    void deepCopy() {
        RME::PodiumItem original(PODIUM_ID, &mockProvider);
        RME::Outfit originalOutfit;
        originalOutfit.lookType = 128; originalOutfit.addons = 1;
        original.setOutfit(originalOutfit);
        original.setDirection(2);
        original.setShowMount(false);

        std::unique_ptr<RME::Item> copyBase = original.deepCopy();
        RME::PodiumItem* copy = dynamic_cast<RME::PodiumItem*>(copyBase.get());
        QVERIFY(copy != nullptr);
        QCOMPARE(copy->getID(), original.getID());
        QCOMPARE(copy->getOutfit(), original.getOutfit());
        QCOMPARE(copy->getDirection(), original.getDirection());
        QCOMPARE(copy->getShowOutfit(), original.getShowOutfit());
        QCOMPARE(copy->getShowMount(), original.getShowMount());
        QCOMPARE(copy->getShowPlatform(), original.getShowPlatform());
    }
};
// QTEST_APPLESS_MAIN(TestPodiumItem)
// #include "TestPodiumItem.moc"
