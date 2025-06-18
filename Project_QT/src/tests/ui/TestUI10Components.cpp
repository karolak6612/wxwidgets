#include <QtTest/QtTest>
#include <QApplication>
#include <QWidget>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

#include "ui/palettes/RawItemsPaletteTab.h"
#include "ui/palettes/TerrainBrushPaletteTab.h"

class TestUI10Components : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testRawItemsPaletteTabCreation();
    void testTerrainBrushPaletteTabCreation();
    void testRawItemsPaletteTabUI();
    void testTerrainBrushPaletteTabUI();
    void testRawItemsFiltering();
    void testTerrainBrushFiltering();
    void testRawItemsSearch();
    void testTerrainBrushSearch();

private:
    QWidget* m_testWidget = nullptr;
};

void TestUI10Components::initTestCase()
{
    m_testWidget = new QWidget();
}

void TestUI10Components::cleanupTestCase()
{
    delete m_testWidget;
}

void TestUI10Components::testRawItemsPaletteTabCreation()
{
    RME::ui::palettes::RawItemsPaletteTab* palette = 
        new RME::ui::palettes::RawItemsPaletteTab(m_testWidget);
    
    QVERIFY(palette != nullptr);
    QVERIFY(palette->parent() == m_testWidget);
    QCOMPARE(palette->getSelectedItemId(), quint16(0));
    
    delete palette;
}

void TestUI10Components::testTerrainBrushPaletteTabCreation()
{
    RME::ui::palettes::TerrainBrushPaletteTab* palette = 
        new RME::ui::palettes::TerrainBrushPaletteTab(m_testWidget);
    
    QVERIFY(palette != nullptr);
    QVERIFY(palette->parent() == m_testWidget);
    QVERIFY(palette->getSelectedBrushName().isEmpty());
    
    delete palette;
}

void TestUI10Components::testRawItemsPaletteTabUI()
{
    RME::ui::palettes::RawItemsPaletteTab* palette = 
        new RME::ui::palettes::RawItemsPaletteTab(m_testWidget);
    
    // Find UI components by object name
    QComboBox* tilesetCombo = palette->findChild<QComboBox*>("tilesetCombo");
    QLineEdit* searchEdit = palette->findChild<QLineEdit*>("searchEdit");
    QPushButton* clearButton = palette->findChild<QPushButton*>("clearSearchButton");
    QListWidget* itemList = palette->findChild<QListWidget*>("itemList");
    QLabel* countLabel = palette->findChild<QLabel*>("itemCountLabel");
    QLabel* selectedLabel = palette->findChild<QLabel*>("selectedItemLabel");
    QLabel* detailsLabel = palette->findChild<QLabel*>("itemDetailsLabel");
    
    QVERIFY(tilesetCombo != nullptr);
    QVERIFY(searchEdit != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(itemList != nullptr);
    QVERIFY(countLabel != nullptr);
    QVERIFY(selectedLabel != nullptr);
    QVERIFY(detailsLabel != nullptr);
    
    // Test initial states
    QVERIFY(tilesetCombo->count() >= 1); // At least "(All Tilesets)"
    QCOMPARE(searchEdit->text(), QString());
    QCOMPARE(itemList->selectionMode(), QAbstractItemView::SingleSelection);
    QVERIFY(countLabel->text().contains("Items:"));
    
    delete palette;
}

void TestUI10Components::testTerrainBrushPaletteTabUI()
{
    RME::ui::palettes::TerrainBrushPaletteTab* palette = 
        new RME::ui::palettes::TerrainBrushPaletteTab(m_testWidget);
    
    // Find UI components by object name
    QComboBox* typeCombo = palette->findChild<QComboBox*>("brushTypeCombo");
    QLineEdit* searchEdit = palette->findChild<QLineEdit*>("searchEdit");
    QPushButton* clearButton = palette->findChild<QPushButton*>("clearSearchButton");
    QListWidget* brushList = palette->findChild<QListWidget*>("brushList");
    QLabel* countLabel = palette->findChild<QLabel*>("brushCountLabel");
    QLabel* selectedLabel = palette->findChild<QLabel*>("selectedBrushLabel");
    QLabel* detailsLabel = palette->findChild<QLabel*>("brushDetailsLabel");
    
    QVERIFY(typeCombo != nullptr);
    QVERIFY(searchEdit != nullptr);
    QVERIFY(clearButton != nullptr);
    QVERIFY(brushList != nullptr);
    QVERIFY(countLabel != nullptr);
    QVERIFY(selectedLabel != nullptr);
    QVERIFY(detailsLabel != nullptr);
    
    // Test initial states
    QVERIFY(typeCombo->count() >= 1); // At least "(All Types)"
    QCOMPARE(searchEdit->text(), QString());
    QCOMPARE(brushList->selectionMode(), QAbstractItemView::SingleSelection);
    QVERIFY(countLabel->text().contains("Brushes:"));
    
    delete palette;
}

void TestUI10Components::testRawItemsFiltering()
{
    RME::ui::palettes::RawItemsPaletteTab* palette = 
        new RME::ui::palettes::RawItemsPaletteTab(m_testWidget);
    
    QComboBox* tilesetCombo = palette->findChild<QComboBox*>("tilesetCombo");
    QVERIFY(tilesetCombo != nullptr);
    
    // Test tileset filtering
    if (tilesetCombo->count() > 1) {
        // Select different tileset
        tilesetCombo->setCurrentIndex(1);
        QString selectedTileset = palette->getSelectedTileset();
        QVERIFY(!selectedTileset.isEmpty());
    }
    
    delete palette;
}

void TestUI10Components::testTerrainBrushFiltering()
{
    RME::ui::palettes::TerrainBrushPaletteTab* palette = 
        new RME::ui::palettes::TerrainBrushPaletteTab(m_testWidget);
    
    QComboBox* typeCombo = palette->findChild<QComboBox*>("brushTypeCombo");
    QVERIFY(typeCombo != nullptr);
    
    // Test brush type filtering
    if (typeCombo->count() > 1) {
        // Select different brush type
        typeCombo->setCurrentIndex(1);
        // Should not crash
    }
    
    delete palette;
}

void TestUI10Components::testRawItemsSearch()
{
    RME::ui::palettes::RawItemsPaletteTab* palette = 
        new RME::ui::palettes::RawItemsPaletteTab(m_testWidget);
    
    QLineEdit* searchEdit = palette->findChild<QLineEdit*>("searchEdit");
    QPushButton* clearButton = palette->findChild<QPushButton*>("clearSearchButton");
    
    QVERIFY(searchEdit != nullptr);
    QVERIFY(clearButton != nullptr);
    
    // Test search functionality
    searchEdit->setText("test");
    QCOMPARE(searchEdit->text(), QString("test"));
    
    // Test clear button
    clearButton->click();
    QCOMPARE(searchEdit->text(), QString());
    
    delete palette;
}

void TestUI10Components::testTerrainBrushSearch()
{
    RME::ui::palettes::TerrainBrushPaletteTab* palette = 
        new RME::ui::palettes::TerrainBrushPaletteTab(m_testWidget);
    
    QLineEdit* searchEdit = palette->findChild<QLineEdit*>("searchEdit");
    QPushButton* clearButton = palette->findChild<QPushButton*>("clearSearchButton");
    
    QVERIFY(searchEdit != nullptr);
    QVERIFY(clearButton != nullptr);
    
    // Test search functionality
    searchEdit->setText("grass");
    QCOMPARE(searchEdit->text(), QString("grass"));
    
    // Test clear button
    clearButton->click();
    QCOMPARE(searchEdit->text(), QString());
    
    delete palette;
}

QTEST_MAIN(TestUI10Components)
#include "TestUI10Components.moc"