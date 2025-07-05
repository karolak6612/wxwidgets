#include <QtTest/QtTest>
#include "core/items/ContainerItem.h"
#include "core/Item.h" // For RME::Item base
#include "tests/core/MockItemTypeProvider.h" // For RME::MockItemTypeProvider

class TestContainerItem : public QObject {
    Q_OBJECT
private:
    RME::MockItemTypeProvider mockProvider;
    const uint16_t CONTAINER_ID = 3000; // Example ID

private slots:
    void init() {
        MockItemData containerData;
        containerData.isContainer = true;
        containerData.name = "Test Backpack";
        mockProvider.setMockData(CONTAINER_ID, containerData);

        // Basic data for generic items used in tests
        mockProvider.setMockData(100, MockItemData());
        mockProvider.setMockData(101, MockItemData());
        mockProvider.setMockData(105, MockItemData());
    }

    void construction() {
        RME::ContainerItem container(CONTAINER_ID, &mockProvider);
        QCOMPARE(container.getID(), CONTAINER_ID);
        QVERIFY(container.getContents().isEmpty());
        QCOMPARE(container.getName(), QString("Test Backpack"));
    }

    void addAndGetItems() {
        RME::ContainerItem container(CONTAINER_ID, &mockProvider);
        QCOMPARE(container.getItemCount(), 0);

        auto item1 = RME::Item::create(100, &mockProvider); // Generic item
        RME::Item* item1_ptr = item1.get();
        container.addItem(std::move(item1));
        QCOMPARE(container.getItemCount(), 1);
        QCOMPARE(container.getItem(0), item1_ptr);

        auto item2 = RME::Item::create(101, &mockProvider);
        container.addItem(std::move(item2));
        QCOMPARE(container.getItemCount(), 2);
        QVERIFY(container.getItem(0) == item1_ptr); // Check order
    }

    void deepCopy() {
        RME::ContainerItem original(CONTAINER_ID, &mockProvider);
        original.addItem(RME::Item::create(100, &mockProvider, 5)); // Item with subtype
        original.setAttribute("desc", "Original Container");

        auto item1_in_orig_unique_ptr = RME::Item::create(105, &mockProvider);
        // RME::Item* item1_in_orig_ptr = item1_in_orig_unique_ptr.get(); // Not needed directly for this test logic
        original.addItem(std::move(item1_in_orig_unique_ptr));


        std::unique_ptr<RME::Item> copyBase = original.deepCopy();
        RME::ContainerItem* copy = dynamic_cast<RME::ContainerItem*>(copyBase.get());
        QVERIFY(copy != nullptr);

        QCOMPARE(copy->getID(), original.getID());
        QCOMPARE(copy->getAttribute("desc").toString(), QString("Original Container"));
        QCOMPARE(copy->getItemCount(), 2);
        QVERIFY(copy->getItem(0) != original.getItem(0)); // Should be a deep copy
        QCOMPARE(copy->getItem(0)->getID(), original.getItem(0)->getID());
        QCOMPARE(copy->getItem(0)->getSubtype(), original.getItem(0)->getSubtype());
        QVERIFY(copy->getItem(1) != original.getItem(1));
        QCOMPARE(copy->getItem(1)->getID(), original.getItem(1)->getID());
    }

    void estimateMemoryUsage() {
         RME::ContainerItem container(CONTAINER_ID, &mockProvider);
         size_t baseUsage = container.estimateMemoryUsage();
         QVERIFY(baseUsage > 0);

         container.addItem(RME::Item::create(100, &mockProvider));
         size_t usageWithItem = container.estimateMemoryUsage();
         QVERIFY(usageWithItem > baseUsage);
    }
};
// QTEST_APPLESS_MAIN(TestContainerItem) // For individual execution
// #include "TestContainerItem.moc" // For linking into a larger suite
