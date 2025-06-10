#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QSettings>
#include <memory>

#include "core/settings/AppSettings.h"

using namespace RME;

class TestAppSettings : public QObject
{
    Q_OBJECT

private:
    const QString testOrgName = "RME_TestOrg_AppSettings";
    const QString testAppName = "RME_TestApp_AppSettingsFile";

    std::unique_ptr<AppSettings> createAppSettingsForTest() {
        return std::make_unique<AppSettings>(
            QSettings::IniFormat,
            QSettings::UserScope,
            testOrgName,
            testAppName
        );
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testDefaultValues_Subset1();
    void testDefaultValues_Subset2();
    void testSetAndGet_Bool();
    void testSetAndGet_String();
    void testSetAndGet_Float();
    void testSetAndGet_Int();
    void testPersistence_Extended();
    void testGetValue_WithCustomDefault();
};

void TestAppSettings::initTestCase() {
    if (QCoreApplication::organizationName().isEmpty()) {
        QCoreApplication::setOrganizationName("RMEditor_TestSuite_Org");
    }
    if (QCoreApplication::applicationName().isEmpty()) {
        QCoreApplication::setApplicationName("RME-Qt_TestSuite_App");
    }
}

void TestAppSettings::cleanupTestCase() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, testOrgName, testAppName);
    settings.clear();
}

void TestAppSettings::init() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, testOrgName, testAppName);
    settings.clear();
}

void TestAppSettings::cleanup() { }


void TestAppSettings::testDefaultValues_Subset1() {
    auto as = createAppSettingsForTest();

    // Original subset
    QCOMPARE(as->isTransparentFloorsEnabled(), false);
    QCOMPARE(as->isShowGridEnabled(), false);
    QCOMPARE(as->getDataDirectory(), QString(""));
    QCOMPARE(as->getScrollSpeed(), 3.5f);
    QCOMPARE(as->getUndoSize(), 40);
    QCOMPARE(as->isTextureManagementEnabled(), true);
    QCOMPARE(as->getPaletteColCount(), 8);
    QCOMPARE(as->getLiveHost(), QString("localhost"));
    QCOMPARE(as->getLivePort(), 12356);

    // --- Added settings for broader coverage ---
    // Version Group
    QCOMPARE(as->isUseCustomDataDirectory(), false);
    QCOMPARE(as->getExtensionsDirectory(), QString(""));
    // Graphics Group
    QCOMPARE(as->getTextureCleanPulse(), 15);
    QCOMPARE(as->getScreenshotFormat(), QString("png"));
    // View Group
    QCOMPARE(as->isShowCreaturesEnabled(), true); // Default 1
    QCOMPARE(as->isHighlightItemsEnabled(), false);
}

void TestAppSettings::testDefaultValues_Subset2() {
    auto as = createAppSettingsForTest();
    // Editor Group
    QCOMPARE(as->getZoomSpeed(), 1.4f);
    QCOMPARE(as->isBorderizePasteEnabled(), true); // Default 1
    QCOMPARE(as->getRecentFiles(), QString("")); // Default was complex string, now empty
    // UI Group
    QCOMPARE(as->useLargeTerrainToolbar(), true); // Default 1
    QCOMPARE(as->getPaletteItemStyle(), QString("listbox"));
    // Window Group
    QCOMPARE(as->getWindowHeight(), 500);
    QCOMPARE(as->isWindowMaximized(), false); // Default 0
    // Network Group
    QCOMPARE(as->getLivePassword(), QString(""));
    // Interface Group (Dark Mode)
    QCOMPARE(as->isDarkModeEnabled(), false); // Default 0
    QCOMPARE(as->getDarkModeRed(), 45);
    // HouseCreation Group
    QCOMPARE(as->getMaxHouseTiles(), 5000);
    // LOD Group
    QCOMPARE(as->getTooltipMaxZoom(), 10);
    // PaletteGrid Group
    QCOMPARE(as->getGridChunkSize(), 3000);
    // Misc/Root Level
    QCOMPARE(as->isGoToWebsiteOnBootEnabled(), false); // Default 0
}


void TestAppSettings::testSetAndGet_Bool() {
    auto as = createAppSettingsForTest();

    as->setTransparentFloorsEnabled(true);
    QCOMPARE(as->isTransparentFloorsEnabled(), true);
    as->setShowGridEnabled(true);
    QCOMPARE(as->isShowGridEnabled(), true);
    as->setTextureManagementEnabled(false);
    QCOMPARE(as->isTextureManagementEnabled(), false);

    // Added
    as->setUseCustomDataDirectory(true);
    QCOMPARE(as->isUseCustomDataDirectory(), true);
    as->setShowCreaturesEnabled(false);
    QCOMPARE(as->isShowCreaturesEnabled(), false);
    as->setBorderizePasteEnabled(false);
    QCOMPARE(as->isBorderizePasteEnabled(), false);
    as->setUseLargeTerrainToolbar(false);
    QCOMPARE(as->useLargeTerrainToolbar(), false);
    as->setWindowMaximized(true);
    QCOMPARE(as->isWindowMaximized(), true);
    as->setDarkModeEnabled(true);
    QCOMPARE(as->isDarkModeEnabled(), true);
    as->setGoToWebsiteOnBootEnabled(true);
    QCOMPARE(as->isGoToWebsiteOnBootEnabled(), true);
}

void TestAppSettings::testSetAndGet_String() {
    auto as = createAppSettingsForTest();
    as->setDataDirectory("/test/data");
    QCOMPARE(as->getDataDirectory(), QString("/test/data"));
    as->setLiveHost("192.168.0.1");
    QCOMPARE(as->getLiveHost(), QString("192.168.0.1"));

    // Added
    as->setExtensionsDirectory("/test/ext");
    QCOMPARE(as->getExtensionsDirectory(), QString("/test/ext"));
    as->setScreenshotFormat("jpg");
    QCOMPARE(as->getScreenshotFormat(), QString("jpg"));
    as->setRecentFiles("map1.rme|map2.rme");
    QCOMPARE(as->getRecentFiles(), QString("map1.rme|map2.rme"));
    as->setPaletteItemStyle("icons");
    QCOMPARE(as->getPaletteItemStyle(), QString("icons"));
    as->setLivePassword("secret");
    QCOMPARE(as->getLivePassword(), QString("secret"));
}

void TestAppSettings::testSetAndGet_Float() {
    auto as = createAppSettingsForTest();
    as->setScrollSpeed(5.0f);
    QCOMPARE(as->getScrollSpeed(), 5.0f);

    // Added
    as->setZoomSpeed(2.0f);
    QCOMPARE(as->getZoomSpeed(), 2.0f);
}

void TestAppSettings::testSetAndGet_Int() {
    auto as = createAppSettingsForTest();
    as->setUndoSize(100);
    QCOMPARE(as->getUndoSize(), 100);
    as->setPaletteColCount(12);
    QCOMPARE(as->getPaletteColCount(), 12);
    as->setLivePort(8888);
    QCOMPARE(as->getLivePort(), 8888);

    // Added
    as->setTextureCleanPulse(30);
    QCOMPARE(as->getTextureCleanPulse(), 30);
    as->setWindowHeight(800);
    QCOMPARE(as->getWindowHeight(), 800);
    as->setDarkModeRed(50);
    QCOMPARE(as->getDarkModeRed(), 50);
    as->setMaxHouseTiles(9000);
    QCOMPARE(as->getMaxHouseTiles(), 9000);
    as->setTooltipMaxZoom(5);
    QCOMPARE(as->getTooltipMaxZoom(), 5);
    as->setGridChunkSize(100);
    QCOMPARE(as->getGridChunkSize(), 100);
}

void TestAppSettings::testPersistence_Extended() {
    // Original subset
    QString testHost = "persistent.example.com";
    int testPort = 9999;
    bool testGrid = true;
    // Added settings for extended test
    QString testScreenshotDir = "/screenshots/game";
    int testWindowWidth = 1024;
    float testNewZoomSpeed = 2.5f;

    {
        auto as1 = createAppSettingsForTest();
        as1->setLiveHost(testHost);
        as1->setLivePort(testPort);
        as1->setShowGridEnabled(testGrid);
        // New
        as1->setScreenshotDirectory(testScreenshotDir);
        as1->setWindowWidth(testWindowWidth);
        as1->setZoomSpeed(testNewZoomSpeed);
        as1->setDarkModeEnabled(true); // A bool
    }

    {
        auto as2 = createAppSettingsForTest();
        QCOMPARE(as2->getLiveHost(), testHost);
        QCOMPARE(as2->getLivePort(), testPort);
        QCOMPARE(as2->isShowGridEnabled(), testGrid);
        // New
        QCOMPARE(as2->getScreenshotDirectory(), testScreenshotDir);
        QCOMPARE(as2->getWindowWidth(), testWindowWidth);
        QCOMPARE(as2->getZoomSpeed(), testNewZoomSpeed);
        QCOMPARE(as2->isDarkModeEnabled(), true);

        // Check a default value for a setting not touched in this test
        QCOMPARE(as2->getScrollSpeed(), 3.5f);
    }
}

void TestAppSettings::testGetValue_WithCustomDefault() {
    auto as = createAppSettingsForTest();
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY).toString(), QString(""));
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY, "/custom/default/path").toString(), QString("/custom/default/path"));
    as->setValue(Config::DATA_DIRECTORY, "/actual/value");
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY, "/custom/default/path_ignored").toString(), QString("/actual/value"));

    // Test with an int key and custom int default
    QCOMPARE(as->getValue(Config::TEXTURE_CLEAN_PULSE).toInt(), 15); // Map default
    QCOMPARE(as->getValue(Config::TEXTURE_CLEAN_PULSE, 999).toInt(), 999); // Custom default
}


QTEST_MAIN(TestAppSettings)
#include "TestAppSettings.moc"
