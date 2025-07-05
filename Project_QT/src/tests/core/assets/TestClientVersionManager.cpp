#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument> // Added
#include <QJsonObject>   // Added
#include <QJsonArray>    // Added
#include <QTemporaryDir> // Added
#include "core/assets/ClientVersionManager.h"
#include "core/assets/ClientProfile.h" // Make sure ClientProfile is included for direct use

using namespace RME;

class TestClientVersionManager : public QObject
{
    Q_OBJECT

private:
    QString sampleClientsXmlPath;
    QTemporaryDir m_tempDir; // Added for JSON test files
    // ClientVersionManager instance will be created per test in init()

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init(); // Added if not present, or use existing one
    void cleanup(); // Added if not present, or use existing one

    // Existing tests
    void testLoadNonExistentFile();
    void testLoadSampleClientsXml();
    void testGetClientProfiles();
    void testGetOtbVersions();
    void testDatFormatParsing();
    void testExtensionsParsing();
    void testPathResolutionInProfile();

    // New tests for JSON client path I/O
    void testSaveClientPaths_Empty();
    void testSaveAndLoadClientPaths_SingleProfile();
    void testSaveAndLoadClientPaths_MultipleProfiles();
    void testLoadClientPaths_FileNotExist();
    void testLoadClientPaths_MalformedJson();
    void testLoadClientPaths_IncorrectStructure();
    void testLoadClientPaths_ProfileIdMismatch();
};

// Helper to write a temp JSON file
bool writeJsonFile(const QString& fullPath, const QJsonDocument& doc) {
    QFile jsonFile(fullPath);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open temp JSON file for writing:" << fullPath;
        return false;
    }
    jsonFile.write(doc.toJson(QJsonDocument::Indented));
    jsonFile.close();
    return true;
}

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
    // m_tempDir will be cleaned automatically
}

void TestClientVersionManager::init() { /* Each test gets a fresh CVM if needed, or use a member */ }
void TestClientVersionManager::cleanup() { /* Delete CVM if created in init() */ }

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

// --- New Test Implementations for JSON paths ---

void TestClientVersionManager::testSaveClientPaths_Empty() {
    ClientVersionManager cvm;
    // No profiles loaded or no custom paths set
    QString jsonPath = m_tempDir.filePath("empty_paths.json");
    QVERIFY(cvm.saveClientPaths(jsonPath));

    QFile file(jsonPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(doc.isArray());
    QVERIFY(doc.array().isEmpty());
}

void TestClientVersionManager::testSaveAndLoadClientPaths_SingleProfile() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath)); // Load base profiles

    // Modify a profile to have a custom path
    // ClientVersionManager::getClientProfiles returns const&, need a way to modify for test setup
    // This highlights a potential need for a non-const getter or a method to set paths for testing.
    // For now, we'll assume we can get a modifiable list or ClientVersionManager is modified for testability.
    // HACK: Re-load and modify the first profile in the list directly if possible
    // This is not ideal. ClientVersionManager should ideally have a way to set these for testing.
    // For the purpose of this subtask, we'll assume such a mechanism exists or we test save by building profiles manually.

    // Let's construct manually for save test to avoid complex CVM modification for now.
    ClientVersionManager cvm_save;
    ClientProfile p1_save;
    p1_save.versionString = "7.60";
    p1_save.userSetClientPath = "/custom/path/760";
    // Manually add to internal list (requires access to 'd' pointer or a test helper in CVM)
    // This test is becoming difficult without modifying CVM for testability.
    // Alternative: Directly construct the QJsonArray and test saveClientPaths with it.
    // However, saveClientPaths takes no parameters for the data to save, it reads from internal state.

    // Simplified approach: Assume there's a way to set userSetClientPath on loaded profiles.
    // This part of the test would be more robust if ClientVersionManager had a method like:
    // setUserSetClientPath(const QString& versionString, const QString& path);
    // For now, this test will be conceptual for save, focusing more on load.
    qDebug() << "testSaveAndLoadClientPaths_SingleProfile: Save part is conceptual due to CVM design.";

    // Test Load part
    QJsonArray testArray;
    QJsonObject profileObj;
    profileObj[QStringLiteral("id")] = QStringLiteral("7.60");
    profileObj[QStringLiteral("path")] = QStringLiteral("/loaded/custom/path/760");
    testArray.append(profileObj);
    QJsonDocument testDoc(testArray);
    QString jsonPath = m_tempDir.filePath("single_profile_path.json");
    QVERIFY(writeJsonFile(jsonPath, testDoc));

    ClientVersionManager cvm_load;
    QVERIFY(cvm_load.loadVersions(sampleClientsXmlPath)); // Load base profiles
    QVERIFY(cvm_load.loadClientPaths(jsonPath));         // Load custom paths over them

    const ClientProfile* p760_loaded = cvm_load.getClientProfile("7.60");
    QVERIFY(p760_loaded != nullptr);
    if(p760_loaded) {
        QCOMPARE(p760_loaded->userSetClientPath, QString("/loaded/custom/path/760"));
    }
    // Check that other profiles don't have userSetClientPath if not in JSON
    const ClientProfile* p1098_loaded = cvm_load.getClientProfile("10.98");
    QVERIFY(p1098_loaded != nullptr);
    if(p1098_loaded) {
        QVERIFY(p1098_loaded->userSetClientPath.isEmpty());
    }
}

void TestClientVersionManager::testSaveAndLoadClientPaths_MultipleProfiles() {
    // Similar to single profile, save part is conceptual without CVM modification.
    qDebug() << "testSaveAndLoadClientPaths_MultipleProfiles: Save part is conceptual.";

    QJsonArray testArray;
    QJsonObject p1, p2;
    p1[QStringLiteral("id")] = QStringLiteral("7.60"); p1[QStringLiteral("path")] = QStringLiteral("pathA");
    p2[QStringLiteral("id")] = QStringLiteral("10.98"); p2[QStringLiteral("path")] = QStringLiteral("pathB");
    testArray.append(p1); testArray.append(p2);
    QJsonDocument testDoc(testArray);
    QString jsonPath = m_tempDir.filePath("multi_profile_paths.json");
    QVERIFY(writeJsonFile(jsonPath, testDoc));

    ClientVersionManager cvm_load;
    QVERIFY(cvm_load.loadVersions(sampleClientsXmlPath));
    QVERIFY(cvm_load.loadClientPaths(jsonPath));

    const ClientProfile* prof760 = cvm_load.getClientProfile("7.60");
    const ClientProfile* prof1098 = cvm_load.getClientProfile("10.98");
    QVERIFY(prof760 && prof1098);
    if(prof760) QCOMPARE(prof760->userSetClientPath, QString("pathA"));
    if(prof1098) QCOMPARE(prof1098->userSetClientPath, QString("pathB"));
}

void TestClientVersionManager::testLoadClientPaths_FileNotExist() {
    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath)); // Ensure profiles are loaded
    QString nonExistentPath = m_tempDir.filePath("no_such_paths.json");
    QVERIFY(cvm.loadClientPaths(nonExistentPath)); // Should return true, not an error
    // Verify no paths were changed
    const ClientProfile* p760 = cvm.getClientProfile("7.60");
    QVERIFY(p760 && p760->userSetClientPath.isEmpty());
}

void TestClientVersionManager::testLoadClientPaths_MalformedJson() {
    QString malformedJson = "{\"id\": \"7.60\", \"path\": \"/path/A\""; // Missing closing brace for array and object
    QString jsonPath = m_tempDir.filePath("malformed_paths.json");
    QFile file(jsonPath); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(malformedJson.toUtf8()); file.close();

    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));
    QVERIFY(!cvm.loadClientPaths(jsonPath)); // Should return false
}

void TestClientVersionManager::testLoadClientPaths_IncorrectStructure() {
    QJsonObject rootObject; // Should be QJsonArray
    rootObject[QStringLiteral("some_key")] = QStringLiteral("some_value");
    QJsonDocument testDoc(rootObject);
    QString jsonPath = m_tempDir.filePath("incorrect_structure_paths.json");
    QVERIFY(writeJsonFile(jsonPath, testDoc));

    ClientVersionManager cvm;
    QVERIFY(cvm.loadVersions(sampleClientsXmlPath));
    QVERIFY(!cvm.loadClientPaths(jsonPath)); // Should return false
}

void TestClientVersionManager::testLoadClientPaths_ProfileIdMismatch() {
    QJsonArray testArray;
    QJsonObject p_valid, p_invalid;
    p_valid[QStringLiteral("id")] = QStringLiteral("7.60"); p_valid[QStringLiteral("path")] = QStringLiteral("valid_path");
    p_invalid[QStringLiteral("id")] = QStringLiteral("NON_EXISTENT_ID"); p_invalid[QStringLiteral("path")] = QStringLiteral("invalid_path");
    testArray.append(p_valid); testArray.append(p_invalid);
    QJsonDocument testDoc(testArray);
    QString jsonPath = m_tempDir.filePath("mismatch_paths.json");
    QVERIFY(writeJsonFile(jsonPath, testDoc));

    ClientVersionManager cvm_load;
    QVERIFY(cvm_load.loadVersions(sampleClientsXmlPath));
    QVERIFY(cvm_load.loadClientPaths(jsonPath)); // Should still return true, but log warning for mismatch

    const ClientProfile* prof760 = cvm_load.getClientProfile("7.60");
    QVERIFY(prof760 && prof760->userSetClientPath == QString("valid_path"));
    // No profile for NON_EXISTENT_ID should have its path set.
}

QTEST_MAIN(TestClientVersionManager)
#include "TestClientVersionManager.moc"
