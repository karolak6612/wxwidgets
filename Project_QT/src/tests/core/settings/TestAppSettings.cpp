#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QSettings> // Explicitly include for clearing in init()
#include <memory> // For std::unique_ptr

#include "core/settings/AppSettings.h" // Class to test

using namespace RME;

class TestAppSettings : public QObject
{
    Q_OBJECT

private:
    // Using specific org/app names for QSettings ensures tests don't interfere
    // with user settings or other tests, and makes cleanup predictable.
    const QString testOrgName = "RME_TestOrg_AppSettings";
    const QString testAppName = "RME_TestApp_AppSettingsFile";

    std::unique_ptr<AppSettings> createAppSettingsForTest() {
        // This ensures that each test creating an AppSettings instance
        // uses the same, isolated configuration.
        return std::make_unique<AppSettings>(
            QSettings::IniFormat,
            QSettings::UserScope,
            testOrgName,
            testAppName
        );
    }

private slots:
    void initTestCase();    // Called once before all tests
    void cleanupTestCase(); // Called once after all tests
    void init();            // Called before each test function
    void cleanup();         // Called after each test function

    void testDefaultValues();
    void testSetAndGet_Bool();
    void testSetAndGet_String();
    void testSetAndGet_Float();
    void testSetAndGet_Int();
    void testPersistence();
    void testGetValue_WithCustomDefault(); // Test the defaultValue parameter of getValue
};

void TestAppSettings::initTestCase() {
    // Set application/organization name for QSettings if AppSettings constructor
    // relies on default QSettings constructor in some paths.
    // The AppSettings constructor provided attempts to use these if its own args are empty.
    // For these tests, we provide explicit org/app names to AppSettings constructor,
    // but setting these can prevent warnings or issues if other code instantiates QSettings().
    if (QCoreApplication::organizationName().isEmpty()) {
        QCoreApplication::setOrganizationName("RMEditor_TestSuite_Org");
    }
    if (QCoreApplication::applicationName().isEmpty()) {
        QCoreApplication::setApplicationName("RME-Qt_TestSuite_App");
    }
}

void TestAppSettings::cleanupTestCase() {
    // Clean up the settings file created by the test suite, if any.
    // This ensures no residue is left on the system.
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, testOrgName, testAppName);
    settings.clear();
    // qInfo() << "Cleaned up test settings from:" << settings.fileName();
}

void TestAppSettings::init() {
    // Ensure a clean state for each test by clearing settings
    // associated with the specific test organization/application names.
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, testOrgName, testAppName);
    settings.clear();
    // For debugging, print the path of the settings file being used by tests:
    // if(testCounter == 0) qInfo() << "Test settings file for this run:" << settings.fileName();
    // testCounter++;
}
// static int testCounter = 0; // if using the qInfo above

void TestAppSettings::cleanup() { }


void TestAppSettings::testDefaultValues() {
    auto as = createAppSettingsForTest();

    QCOMPARE(as->isTransparentFloorsEnabled(), false);
    QCOMPARE(as->isShowGridEnabled(), false);
    QCOMPARE(as->getDataDirectory(), QString(""));
    QCOMPARE(as->getScrollSpeed(), 3.5f);
    QCOMPARE(as->getUndoSize(), 40);
    QCOMPARE(as->isTextureManagementEnabled(), true);
    QCOMPARE(as->getPaletteColCount(), 8);
    QCOMPARE(as->getLiveHost(), QString("localhost"));
    QCOMPARE(as->getLivePort(), 12356);
}

void TestAppSettings::testSetAndGet_Bool() {
    auto as = createAppSettingsForTest();

    as->setTransparentFloorsEnabled(true);
    QCOMPARE(as->isTransparentFloorsEnabled(), true);
    as->setTransparentFloorsEnabled(false);
    QCOMPARE(as->isTransparentFloorsEnabled(), false);

    as->setShowGridEnabled(true);
    QCOMPARE(as->isShowGridEnabled(), true);

    as->setTextureManagementEnabled(false);
    QCOMPARE(as->isTextureManagementEnabled(), false);
}

void TestAppSettings::testSetAndGet_String() {
    auto as = createAppSettingsForTest();
    QString testDir = "/test/data/dir";
    as->setDataDirectory(testDir);
    QCOMPARE(as->getDataDirectory(), testDir);

    QString testHost = "192.168.1.100";
    as->setLiveHost(testHost);
    QCOMPARE(as->getLiveHost(), testHost);
}

void TestAppSettings::testSetAndGet_Float() {
    auto as = createAppSettingsForTest();
    float testSpeed = 5.0f;
    as->setScrollSpeed(testSpeed);
    QCOMPARE(as->getScrollSpeed(), testSpeed);
}

void TestAppSettings::testSetAndGet_Int() {
    auto as = createAppSettingsForTest();
    int testUndo = 100;
    as->setUndoSize(testUndo);
    QCOMPARE(as->getUndoSize(), testUndo);

    int testCols = 12;
    as->setPaletteColCount(testCols);
    QCOMPARE(as->getPaletteColCount(), testCols);

    int testPort = 8888;
    as->setLivePort(testPort);
    QCOMPARE(as->getLivePort(), testPort);
}

void TestAppSettings::testPersistence() {
    QString testHost = "persistent.example.com";
    int testPort = 9999;
    bool testGrid = true;

    {
        auto as1 = createAppSettingsForTest();
        as1->setLiveHost(testHost);
        as1->setLivePort(testPort);
        as1->setShowGridEnabled(testGrid);
        // AppSettings destructor calls QSettings::sync()
    }

    {
        auto as2 = createAppSettingsForTest(); // New instance, should load from same file
        QCOMPARE(as2->getLiveHost(), testHost);
        QCOMPARE(as2->getLivePort(), testPort);
        QCOMPARE(as2->isShowGridEnabled(), testGrid);

        QCOMPARE(as2->getScrollSpeed(), 3.5f); // Check a default value not touched in this test
    }
}

void TestAppSettings::testGetValue_WithCustomDefault() {
    auto as = createAppSettingsForTest();

    // Key DATA_DIRECTORY default is ""
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY).toString(), QString(""));
    // Provide a custom default to getValue
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY, "/custom/default/path").toString(), QString("/custom/default/path"));

    // Set a value, then custom default should be ignored
    as->setValue(Config::DATA_DIRECTORY, "/actual/value");
    QCOMPARE(as->getValue(Config::DATA_DIRECTORY, "/custom/default/path_ignored").toString(), QString("/actual/value"));
}


QTEST_MAIN(TestAppSettings)
#include "TestAppSettings.moc"
