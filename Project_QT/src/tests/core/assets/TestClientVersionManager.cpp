#include <QtTest/QtTest>
#include <QCoreApplication> // For applicationDirPath to find test data
#include <QDir>
#include <QTextStream> // For creating temp sample file
#include "core/assets/ClientVersionManager.h" // Class to test

using namespace RME;

class TestClientVersionManager : public QObject
{
    Q_OBJECT

private:
    QString sampleClientsXmlPath; // Path to the sample_clients.xml used by tests

private slots:
    void initTestCase();    // Called once before all tests
    void cleanupTestCase(); // Called once after all tests

    void testLoadNonExistentFile();
    void testLoadSampleClientsXml();
    void testGetClientProfiles();
    void testGetOtbVersions();
    void testDatFormatParsing();
    void testExtensionsParsing();
    void testPathResolutionInProfile(); // New test for path combination
};

void TestClientVersionManager::initTestCase() {
    // This test will attempt to create a sample_clients.xml in the current working directory (CWD)
    // if one doesn't already exist. This CWD is typically where the test executable is located
    // when run by CTest or directly.
    // Step 10 (Update Test CMake) should ideally handle copying necessary test data files
    // to this location or setting the CWD appropriately.
    sampleClientsXmlPath = "sample_clients.xml";

    QFile tempFile(sampleClientsXmlPath);
    if (!tempFile.exists()) {
        if (tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&tempFile);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<clients>\n"
                << "  <otbs>\n"
                << "    <otb name=\"7.60\" id=\"760\" format_version=\"2\" desc=\"Tibia 7.60 OTB\" />\n"
                << "    <otb name=\"10.98\" id=\"1098\" format_version=\"3\" desc=\"Tibia 10.98 OTB\" />\n"
                << "  </otbs>\n"
                << "  <clients>\n"
                << "    <client version=\"7.60\" desc=\"Tibia 7.60\" otb=\"760\" dat=\"760.dat\" spr=\"760.spr\" pic=\"760.pic\">\n"
                << "      <otbm>7601,7602</otbm>\n"
                << "      <paths dat=\"data/things/760/\" spr=\"data/things/760/\" pic=\"pictures/760/\"/>\n"
                << "      <signatures dat_format=\"V_760\">\n"
                << "        <dat key=\"default\">A1B2C3D4</dat>\n"
                << "        <spr key=\"default\">E5F6G7H8</spr>\n"
                << "      </signatures>\n"
                << "      <extensions extended=\"false\" transparent=\"false\" frame_durations=\"false\" pattern_z=\"false\" />\n"
                << "    </client>\n"
                << "    <client version=\"10.98\" desc=\"Tibia 10.98\" otb=\"1098\" dat=\"1098.dat\" spr=\"1098.spr\" pic=\"1098.pic\">\n"
                << "      <otbm>10981</otbm>\n"
                << "      <signatures dat_format=\"V_1090_1094\">\n"
                << "        <dat key=\"default\">11223344</dat>\n"
                << "        <spr key=\"default\">55667788</spr>\n"
                << "      </signatures>\n"
                << "      <extensions extended=\"true\" transparent=\"true\" frame_durations=\"true\" pattern_z=\"true\" u16_looktype=\"true\" />\n"
                << "      <otfi>Tibia_10.98.otfi</otfi>\n"
                << "    </client>\n"
                << "    <client version=\"8.60\" desc=\"Tibia 8.60 (No Paths Node)\" otb=\"860\" dat=\"860.dat\" spr=\"860.spr\" pic=\"860.pic\">\n"
                << "      <signatures dat_format=\"V_860_862\" />\n" // Minimal other data for path test
                << "    </client>\n"
                << "  </clients>\n"
                << "</clients>";
            tempFile.close();
            qInfo() << "TestClientVersionManager: Created temporary sample_clients.xml in CWD:" << QDir::current().filePath(sampleClientsXmlPath);
        } else {
            qWarning() << "TestClientVersionManager: Could not create temporary sample_clients.xml. Tests might fail if file not found.";
        }
    } else {
         qInfo() << "TestClientVersionManager: Using existing sample_clients.xml found in CWD:" << QDir::current().filePath(sampleClientsXmlPath);
    }
}

void TestClientVersionManager::cleanupTestCase() {
    // To ensure a clean state for next full test suite run, remove the temp file.
    // If CMake handles copying, this isn't strictly necessary but good for local dev hygiene.
    // QFile::remove(sampleClientsXmlPath);
}

void TestClientVersionManager::testLoadNonExistentFile() {
    ClientVersionManager cvm;
    QVERIFY(!cvm.loadVersions("non_existent_clients.xml"));
}

void TestClientVersionManager::testLoadSampleClientsXml() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));
    QCOMPARE(cvm.getClientProfiles().size(), 3); // Added one more for path test
    QCOMPARE(cvm.getOtbVersions().size(), 2);
}

void TestClientVersionManager::testPathResolutionInProfile() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));

    const ClientProfile* p760 = cvm.getClientProfile("7.60");
    QVERIFY(p760 != nullptr);
    if (p760) {
        // ClientVersionManager should combine <paths dat="..."> with <client dat="...">
        // So, "data/things/760/" + "760.dat"
        QCOMPARE(p760->datPathHint, QString("data/things/760/760.dat"));
        QCOMPARE(p760->sprPathHint, QString("data/things/760/760.spr"));
        QCOMPARE(p760->picPathHint, QString("pictures/760/760.pic"));
    }

    const ClientProfile* p860 = cvm.getClientProfile("8.60");
    QVERIFY(p860 != nullptr);
    if (p860) {
        // This client has no <paths> node, so hints should be just filenames
        QCOMPARE(p860->datPathHint, QString("860.dat"));
        QCOMPARE(p860->sprPathHint, QString("860.spr"));
    }
}


void TestClientVersionManager::testGetClientProfiles() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));

    const ClientProfile* p760 = cvm.getClientProfile("7.60");
    QVERIFY(p760 != nullptr);
    if (p760) {
        QCOMPARE(p760->name, QString("Tibia 7.60"));
        QCOMPARE(p760->versionString, QString("7.60"));
        QCOMPARE(p760->clientOtbmVersionId, 760u);
        // Path hints tested in testPathResolutionInProfile
        QCOMPARE(p760->supportedOtbmVersions.size(), 2);
        QVERIFY(p760->supportedOtbmVersions.contains(7601u));
        QVERIFY(p760->supportedOtbmVersions.contains(7602u));
        QCOMPARE(p760->datSignatures.value("default"), QByteArray::fromHex("A1B2C3D4"));
    }

    const ClientProfile* p1098 = cvm.getClientProfile("10.98");
    QVERIFY(p1098 != nullptr);
    if (p1098) {
        QCOMPARE(p1098->name, QString("Tibia 10.98"));
        QCOMPARE(p1098->versionString, QString("10.98"));
        QCOMPARE(p1098->clientOtbmVersionId, 1098u);
        QCOMPARE(p1098->datSignatures.value("default"), QByteArray::fromHex("11223344"));
        QVERIFY(p1098->extendedSprites);
        QVERIFY(p1098->transparentSprites);
        QVERIFY(p1098->frameDurations);
        QVERIFY(p1098->patternZ);
        QVERIFY(p1098->looktypeU16);
        QCOMPARE(p1098->customOtfIndexPath, QString("Tibia_10.98.otfi"));
    }

    QVERIFY(cvm.getClientProfile("non.existent") == nullptr);
    QVERIFY(cvm.getDefaultClientProfile() == p760);
}

void TestClientVersionManager::testGetOtbVersions() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));

    const OtbVersion* otb760 = cvm.getOtbVersionById(760);
    QVERIFY(otb760 != nullptr);
    if (otb760) {
        QCOMPARE(otb760->name, QString("7.60"));
        QCOMPARE(otb760->formatVersion, 2u);
        QCOMPARE(otb760->clientID, 760u);
        QCOMPARE(otb760->description, QString("Tibia 7.60 OTB"));
    }

    const OtbVersion* otb1098ByName = cvm.getOtbVersionByName("10.98");
    QVERIFY(otb1098ByName != nullptr);
    if (otb1098ByName) {
        QCOMPARE(otb1098ByName->formatVersion, 3u);
    }

    QVERIFY(cvm.getOtbVersionById(9999) == nullptr);
    QVERIFY(cvm.getOtbVersionByName("non.existent") == nullptr);
}

void TestClientVersionManager::testDatFormatParsing() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));
    const ClientProfile* p760 = cvm.getClientProfile("7.60");
    QVERIFY(p760 != nullptr);
    if (p760) {
        QCOMPARE(p760->datFormat, DatFormat::V_760);
    }

    const ClientProfile* p1098 = cvm.getClientProfile("10.98");
    QVERIFY(p1098 != nullptr);
    if (p1098) {
        QCOMPARE(p1098->datFormat, DatFormat::V_1090_1094);
    }
}

void TestClientVersionManager::testExtensionsParsing() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));

    const ClientProfile* p760 = cvm.getClientProfile("7.60");
    QVERIFY(p760 != nullptr);
    if (p760) {
        QVERIFY(!p760->extendedSprites);
        QVERIFY(!p760->transparentSprites);
        QVERIFY(!p760->frameDurations);
        QVERIFY(!p760->patternZ);
        QVERIFY(!p760->looktypeU16);
    }

    const ClientProfile* p1098 = cvm.getClientProfile("10.98");
    QVERIFY(p1098 != nullptr);
    if (p1098) {
        QVERIFY(p1098->extendedSprites);
        QVERIFY(p1098->transparentSprites);
        QVERIFY(p1098->frameDurations);
        QVERIFY(p1098->patternZ);
        QVERIFY(p1098->looktypeU16);
    }
}

QTEST_MAIN(TestClientVersionManager)
#include "TestClientVersionManager.moc"
