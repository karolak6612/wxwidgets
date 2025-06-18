#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>

#include "ui/palettes/HousePaletteTab.h"
#include "ui/palettes/WaypointPaletteTab.h"
#include "ui/dialogs/EditHouseDialogQt.h"
#include "core/houses/HouseData.h"
#include "core/world/TownData.h"
#include "core/world/TownManager.h"
#include "core/waypoints/WaypointManager.h"
#include "core/Position.h"

class TestUI07Components : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testHousePaletteTabCreation();
    void testWaypointPaletteTabCreation();
    void testEditHouseDialogCreation();
    void testHousePaletteTabUI();
    void testWaypointPaletteTabUI();

private:
    QWidget* m_testWidget = nullptr;
    RME::core::world::TownManager* m_townManager = nullptr;
    RME::core::waypoints::WaypointManager* m_waypointManager = nullptr;
};

void TestUI07Components::initTestCase()
{
    m_testWidget = new QWidget();
    m_townManager = new RME::core::world::TownManager();
    m_waypointManager = new RME::core::waypoints::WaypointManager(nullptr);
    
    // Add some test towns
    RME::core::world::TownData town1;
    town1.setId(1);
    town1.setName("Test Town 1");
    town1.setTemplePosition(RME::core::Position(100, 100, 7));
    m_townManager->addTown(town1);
    
    RME::core::world::TownData town2;
    town2.setId(2);
    town2.setName("Test Town 2");
    town2.setTemplePosition(RME::core::Position(200, 200, 7));
    m_townManager->addTown(town2);
}

void TestUI07Components::cleanupTestCase()
{
    delete m_testWidget;
    delete m_townManager;
    delete m_waypointManager;
}

void TestUI07Components::testHousePaletteTabCreation()
{
    RME::ui::palettes::HousePaletteTab* housePalette = 
        new RME::ui::palettes::HousePaletteTab(m_testWidget);
    
    QVERIFY(housePalette != nullptr);
    QVERIFY(housePalette->parent() == m_testWidget);
    
    delete housePalette;
}

void TestUI07Components::testWaypointPaletteTabCreation()
{
    RME::ui::palettes::WaypointPaletteTab* waypointPalette = 
        new RME::ui::palettes::WaypointPaletteTab(m_testWidget);
    
    QVERIFY(waypointPalette != nullptr);
    QVERIFY(waypointPalette->parent() == m_testWidget);
    
    delete waypointPalette;
}

void TestUI07Components::testEditHouseDialogCreation()
{
    RME::core::houses::HouseData houseData;
    houseData.setId(1);
    houseData.setName("Test House");
    houseData.setRent(1000);
    houseData.setTownId(1);
    houseData.setGuildhall(false);
    
    RME::ui::dialogs::EditHouseDialogQt* dialog = 
        new RME::ui::dialogs::EditHouseDialogQt(m_testWidget, &houseData, m_townManager);
    
    QVERIFY(dialog != nullptr);
    QVERIFY(dialog->parent() == m_testWidget);
    QVERIFY(dialog->isModal());
    
    delete dialog;
}

void TestUI07Components::testHousePaletteTabUI()
{
    RME::ui::palettes::HousePaletteTab* housePalette = 
        new RME::ui::palettes::HousePaletteTab(m_testWidget);
    
    housePalette->setTownManager(m_townManager);
    
    // Find UI components
    QComboBox* townCombo = housePalette->findChild<QComboBox*>();
    QListWidget* houseList = housePalette->findChild<QListWidget*>();
    QPushButton* addButton = housePalette->findChild<QPushButton*>("Add House");
    QPushButton* editButton = housePalette->findChild<QPushButton*>("Edit House");
    QPushButton* removeButton = housePalette->findChild<QPushButton*>("Remove House");
    
    QVERIFY(townCombo != nullptr);
    QVERIFY(houseList != nullptr);
    // Note: Button finding by text might not work with findChild, but we can verify the palette exists
    
    // Test that town combo is populated
    QVERIFY(townCombo->count() >= 3); // "(No Town)" + 2 test towns
    QCOMPARE(townCombo->itemText(0), QString("(No Town)"));
    
    delete housePalette;
}

void TestUI07Components::testWaypointPaletteTabUI()
{
    RME::ui::palettes::WaypointPaletteTab* waypointPalette = 
        new RME::ui::palettes::WaypointPaletteTab(m_testWidget);
    
    waypointPalette->setWaypointManager(m_waypointManager);
    
    // Find UI components
    QListWidget* waypointList = waypointPalette->findChild<QListWidget*>();
    
    QVERIFY(waypointList != nullptr);
    QVERIFY(waypointList->selectionMode() == QAbstractItemView::ExtendedSelection);
    
    delete waypointPalette;
}

QTEST_MAIN(TestUI07Components)
#include "TestUI07Components.moc"