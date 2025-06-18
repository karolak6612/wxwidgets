#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>

#include "ui/dialogs/EditSpawnDialogQt.h"
#include "ui/widgets/SpawnSettingsWidget.h"
#include "core/Tile.h"
#include "core/Position.h"
#include "core/assets/CreatureDatabase.h"
#include "tests/core/MockItemTypeProvider.h"

class TestUI08Components : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testEditSpawnDialogCreation();
    void testSpawnSettingsWidgetCreation();
    void testEditSpawnDialogUI();
    void testSpawnSettingsWidgetUI();
    void testEditSpawnDialogDataHandling();
    void testSpawnSettingsWidgetSettings();

private:
    QWidget* m_testWidget = nullptr;
    RME::core::Tile* m_testTile = nullptr;
    RME::core::assets::CreatureDatabase* m_creatureDatabase = nullptr;
    MockItemTypeProvider* m_mockProvider = nullptr;
};

void TestUI08Components::initTestCase()
{
    m_testWidget = new QWidget();
    m_mockProvider = new MockItemTypeProvider();
    m_creatureDatabase = new RME::core::assets::CreatureDatabase();
    
    // Create a test tile with spawn data
    m_testTile = new RME::core::Tile(RME::core::Position(100, 100, 7), m_mockProvider);
    m_testTile->setSpawnRadius(5);
    m_testTile->setSpawnIntervalSeconds(60);
    m_testTile->addCreatureToSpawnList("Rat");
    m_testTile->addCreatureToSpawnList("Cave Rat");
}

void TestUI08Components::cleanupTestCase()
{
    delete m_testWidget;
    delete m_testTile;
    delete m_creatureDatabase;
    delete m_mockProvider;
}

void TestUI08Components::testEditSpawnDialogCreation()
{
    RME::ui::dialogs::EditSpawnDialogQt* dialog = 
        new RME::ui::dialogs::EditSpawnDialogQt(m_testWidget, m_testTile, m_creatureDatabase);
    
    QVERIFY(dialog != nullptr);
    QVERIFY(dialog->parent() == m_testWidget);
    QVERIFY(dialog->isModal());
    QCOMPARE(dialog->windowTitle(), QString("Edit Spawn Properties"));
    
    delete dialog;
}

void TestUI08Components::testSpawnSettingsWidgetCreation()
{
    RME::ui::widgets::SpawnSettingsWidget* widget = 
        new RME::ui::widgets::SpawnSettingsWidget(m_testWidget);
    
    QVERIFY(widget != nullptr);
    QVERIFY(widget->parent() == m_testWidget);
    QCOMPARE(widget->title(), QString("Spawn Settings"));
    
    delete widget;
}

void TestUI08Components::testEditSpawnDialogUI()
{
    RME::ui::dialogs::EditSpawnDialogQt* dialog = 
        new RME::ui::dialogs::EditSpawnDialogQt(m_testWidget, m_testTile, m_creatureDatabase);
    
    // Find UI components by object name
    QSpinBox* radiusSpinBox = dialog->findChild<QSpinBox*>("radiusSpinBox");
    QSpinBox* respawnTimeSpinBox = dialog->findChild<QSpinBox*>("respawnTimeSpinBox");
    QListWidget* creatureListWidget = dialog->findChild<QListWidget*>("creatureListWidget");
    QPushButton* addButton = dialog->findChild<QPushButton*>("addCreatureButton");
    QPushButton* removeButton = dialog->findChild<QPushButton*>("removeCreatureButton");
    
    QVERIFY(radiusSpinBox != nullptr);
    QVERIFY(respawnTimeSpinBox != nullptr);
    QVERIFY(creatureListWidget != nullptr);
    QVERIFY(addButton != nullptr);
    QVERIFY(removeButton != nullptr);
    
    // Test that data is loaded correctly
    QCOMPARE(radiusSpinBox->value(), 5);
    QCOMPARE(respawnTimeSpinBox->value(), 60);
    QCOMPARE(creatureListWidget->count(), 2);
    
    delete dialog;
}

void TestUI08Components::testSpawnSettingsWidgetUI()
{
    RME::ui::widgets::SpawnSettingsWidget* widget = 
        new RME::ui::widgets::SpawnSettingsWidget(m_testWidget);
    
    // Find UI components by object name
    QCheckBox* enableCheckBox = widget->findChild<QCheckBox*>("enableSpawnModeCheckBox");
    QSpinBox* radiusSpinBox = widget->findChild<QSpinBox*>("spawnRadiusSpinBox");
    QSpinBox* timeSpinBox = widget->findChild<QSpinBox*>("spawnTimeSpinBox");
    
    QVERIFY(enableCheckBox != nullptr);
    QVERIFY(radiusSpinBox != nullptr);
    QVERIFY(timeSpinBox != nullptr);
    
    // Test initial state
    QVERIFY(radiusSpinBox->minimum() >= 1);
    QVERIFY(radiusSpinBox->maximum() <= 50);
    QVERIFY(timeSpinBox->minimum() >= 1);
    QVERIFY(timeSpinBox->maximum() >= 3600);
    
    delete widget;
}

void TestUI08Components::testEditSpawnDialogDataHandling()
{
    RME::ui::dialogs::EditSpawnDialogQt* dialog = 
        new RME::ui::dialogs::EditSpawnDialogQt(m_testWidget, m_testTile, m_creatureDatabase);
    
    // Test data access methods
    QCOMPARE(dialog->getSpawnRadius(), 5);
    QCOMPARE(dialog->getRespawnTime(), 60);
    
    QStringList creatures = dialog->getCreatureList();
    QCOMPARE(creatures.size(), 2);
    QVERIFY(creatures.contains("Rat"));
    QVERIFY(creatures.contains("Cave Rat"));
    
    // Test validation
    QVERIFY(dialog->hasValidData());
    
    delete dialog;
}

void TestUI08Components::testSpawnSettingsWidgetSettings()
{
    RME::ui::widgets::SpawnSettingsWidget* widget = 
        new RME::ui::widgets::SpawnSettingsWidget(m_testWidget);
    
    // Test setting values
    widget->setSpawnRadius(10);
    QCOMPARE(widget->getSpawnRadius(), 10);
    
    widget->setSpawnTime(120);
    QCOMPARE(widget->getSpawnTime(), 120);
    
    widget->setSpawnModeEnabled(true);
    QVERIFY(widget->isSpawnModeEnabled());
    
    widget->setSpawnModeEnabled(false);
    QVERIFY(!widget->isSpawnModeEnabled());
    
    delete widget;
}

QTEST_MAIN(TestUI08Components)
#include "TestUI08Components.moc"