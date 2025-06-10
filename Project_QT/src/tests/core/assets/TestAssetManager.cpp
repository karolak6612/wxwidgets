#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream> // For creating sample files

#include "core/assets/AssetManager.h" // Class to test
#include "core/assets/ClientProfile.h"
#include "core/assets/ItemData.h"
#include "core/assets/CreatureData.h"
#include "core/sprites/SpriteData.h"

using namespace RME;

class TestAssetManager : public QObject
{
    Q_OBJECT

private:
    QString testDataRootPath = "test_asset_manager_data";
    QString clientsXmlPath;
    QString itemsOtbPath;
    QString itemsXmlPath;
    QString creaturesRmeXmlPath;
    // QString sampleOtsMonsterXmlPath; // For individual monster, if needed for more detailed test
    QString datPath;
    QString sprPath;
    QString otfiPath; // Optional OTFI for one of the clients

    void createAllSampleFiles() {
        QDir rootDir(testDataRootPath);
        if (rootDir.exists()) { // Clean up from previous run if any
            rootDir.removeRecursively();
        }
        rootDir.mkpath(".");

        // --- clients.xml ---
        clientsXmlPath = rootDir.filePath("clients.xml"); // Changed from sample_clients.xml to match AssetManager default
        QFile clientsFile(clientsXmlPath);
        if (clientsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&clientsFile);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<clients>\n"
                << "  <otbs>\n"
                << "    <otb name=\"7.60\" id=\"760\" format_version=\"2\" desc=\"Tibia 7.60 OTB\" />\n"
                << "  </otbs>\n"
                << "  <clients>\n"
                << "    <client version=\"7.60\" desc=\"Tibia 7.60 Test\" otb=\"760\" dat=\"sample.dat\" spr=\"sample.spr\" pic=\"sample.pic\">\n"
                << "      <otbm>7601</otbm>\n"
                << "      <signatures dat_format=\"V_760\">\n"
                << "        <dat key=\"default\">A1B2C3D4</dat>\n"
                << "        <spr key=\"default\">E5F6G7H8</spr>\n"
                << "      </signatures>\n"
                << "      <extensions extended=\"false\" transparent=\"true\" />\n"
                // No OTFI specified for this client, AssetManager might try conventional name
                << "    </client>\n"
                << "    <client version=\"10.98\" desc=\"Tibia 10.98 Test\" otb=\"1098\" dat=\"sample1098.dat\" spr=\"sample1098.spr\" pic=\"sample1098.pic\">\n"
                << "      <otfi>specific.otfi</otfi>\n" // Specific OTFI for this client
                << "      <signatures dat_format=\"V_1090_1094\" />\n"
                << "    </client>\n"
                << "  </clients>\n"
                << "</clients>";
            clientsFile.close();
        } else { qWarning() << "Could not create " << clientsXmlPath; }

        // --- items.otb (named items_7.60.otb to match client profile) ---
        itemsOtbPath = rootDir.filePath("items_7.60.otb"); // Specific name
        QFile otbFile(itemsOtbPath);
        if (otbFile.open(QIODevice::WriteOnly)) {
            QByteArray otbBytes = QByteArray::fromHex(
                "FF001C000000010C0003000000D20400000100000001090053616D706C654F544200"
                "FF0100000000" // Group GROUND, 0 len attributes
                "6400A0000000" // Item 100, flags 0xA0 (Stackable, Pickupable)
                "0902006500"   // Attr clientID 101
                "0E0D0053616D706C652047726F756E64" // Attr name "Sample Ground"
                "00"           // End item attributes
                "FFFE00"       // RME End marker
            );
            otbFile.write(otbBytes);
            otbFile.close();
        } else { qWarning() << "Could not create " << itemsOtbPath; }

        // --- items.xml ---
        itemsXmlPath = rootDir.filePath("items.xml"); // Generic name, loaded by all
        QFile itemsFile(itemsXmlPath);
        if (itemsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&itemsFile);
             out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<items>\n"
                << "  <item id=\"100\" name=\"Sample Ground XML Override\">\n"
                << "    <attribute key=\"description\" value=\"Desc from XML for ID 100.\"/>\n"
                << "  </item>\n"
                << "  <item id=\"200\" name=\"Magic Sword from XML\">\n"
                << "    <attribute key=\"article\" value=\"a\"/>\n"
                << "    <attribute key=\"blockprojectile\" value=\"true\"/>\n"
                << "  </item>\n"
                << "</items>";
            itemsFile.close();
        } else { qWarning() << "Could not create " << itemsXmlPath; }

        // --- creatures.xml (RME format) ---
        creaturesRmeXmlPath = rootDir.filePath("creatures.xml"); // Generic name
        QFile creaturesFile(creaturesRmeXmlPath);
        if (creaturesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&creaturesFile);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<creatures>\n"
                << "  <creature name=\"Rat Test\" type=\"monster\">\n" // Unique name for test
                << "    <look type=\"21\"/>\n"
                << "  </creature>\n"
                << "</creatures>";
            creaturesFile.close();
        } else { qWarning() << "Could not create " << creaturesRmeXmlPath; }

        // --- sample.dat (for client 7.60) ---
        datPath = rootDir.filePath("sample.dat"); // Matches client 7.60 dat hint
        QFile datFile(datPath);
        if (datFile.open(QIODevice::WriteOnly)) {
             QByteArray datBytes = QByteArray::fromHex(
                "00000000" "0200" "0000" "0000" "0000"
                "010001000101010101"
                "020001000101010101"
            );
            datFile.write(datBytes);
            datFile.close();
        } else { qWarning() << "Could not create " << datPath; }

        // --- sample.spr (for client 7.60) ---
        sprPath = rootDir.filePath("sample.spr"); // Matches client 7.60 spr hint
        QFile sprFile(sprPath);
        if (sprFile.open(QIODevice::WriteOnly)) {
            QByteArray sprBytes = QByteArray::fromHex(
                "00000000" "0C000000" "10000000"
                "01000000"
                "00000200FF000000FF00"
            );
            sprFile.write(sprBytes);
            sprFile.close();
        } else { qWarning() << "Could not create " << sprPath; }

        // --- specific.otfi (for client 10.98) ---
        otfiPath = rootDir.filePath("specific.otfi"); // Matches client 10.98 otfi hint
        QFile otfiFile(otfiPath);
        if (otfiFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&otfiFile);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<fileInformation extended=\"false\" alpha=\"false\" frameDurations=\"false\">\n" // Different from default
                << "  <dat path=\"custom_1098.dat\"/>\n"
                << "  <spr path=\"custom_1098.spr\"/>\n"
                << "</fileInformation>";
            otfiFile.close();
        } else { qWarning() << "Could not create " << otfiPath; }
    }

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testLoadAllAssets_Success_760();
    void testLoadAllAssets_Success_1098_with_OTFI();
    void testLoadAllAssets_MissingClientProfile();
    void testLoadAllAssets_MissingEssential_Items();
    void testLoadAllAssets_MissingEssential_Sprites();
    void testAssetManager_As_IItemTypeProvider();
};

void TestAssetManager::initTestCase() {
    createAllSampleFiles();
}

void TestAssetManager::cleanupTestCase() {
    QDir(testDataRootPath).removeRecursively();
}

void TestAssetManager::testLoadAllAssets_Success_760() {
    AssetManager am;
    QVERIFY(am.loadAllAssets(testDataRootPath, "7.60"));

    const ClientProfile* cp = am.getCurrentClientProfile();
    QVERIFY(cp != nullptr);
    if(cp) QCOMPARE(cp->versionString, QString("7.60"));

    QVERIFY(am.getItemDatabase().getItemCount() == 2); // 100 from OTB+XML, 200 from XML
    const ItemData* item100 = am.getItemData(100);
    QVERIFY(item100 && item100->serverID == 100);
    QCOMPARE(item100->name, QString("Sample Ground XML Override"));
    QVERIFY(item100->hasFlag(ItemFlag::STACKABLE));

    QVERIFY(am.getCreatureDatabase().getCreatureCount() == 1);
    const CreatureData* rat = am.getCreatureData("Rat Test");
    QVERIFY(rat && rat->name == "Rat Test");
    QCOMPARE(rat->outfit.lookType, 21);

    QVERIFY(am.getSpriteManager().getSpriteCount() == 2); // Sprites 1 and 2
    const SpriteData* sprite1 = am.getSpriteData(1); // Assumes firstSpriteID=1 for this profile
    QVERIFY(sprite1 && sprite1->id == 1);
    QCOMPARE(sprite1->width, 1);
    QVERIFY(!sprite1->isExtended); // From client profile, as no specific.otfi for 7.60
}

void TestAssetManager::testLoadAllAssets_Success_1098_with_OTFI() {
    AssetManager am;
    // Create dummy custom DAT/SPR files for the 10.98 client, as specified in specific.otfi
    QFile customDat(QDir(testDataRootPath).filePath("custom_1098.dat"));
    if(customDat.open(QIODevice::WriteOnly)) { /* Minimal DAT */
        QByteArray datBytes = QByteArray::fromHex("000000000000000000000000"); // Sig + 0 counts
        customDat.write(datBytes); customDat.close();
    }
    QFile customSpr(QDir(testDataRootPath).filePath("custom_1098.spr"));
    if(customSpr.open(QIODevice::WriteOnly)) { /* Minimal SPR */
        QByteArray sprBytes = QByteArray::fromHex("00000000"); // Sig
        customSpr.write(sprBytes); customSpr.close();
    }

    QVERIFY(am.loadAllAssets(testDataRootPath, "10.98"));
    const ClientProfile* cp = am.getCurrentClientProfile();
    QVERIFY(cp != nullptr);
    if(cp) {
        QCOMPARE(cp->versionString, QString("10.98"));
        QCOMPARE(cp->customOtfIndexPath, QString("specific.otfi"));
    }
    // SpriteManager should have loaded using custom_1098.dat/spr due to OTFI
    // Since these are minimal/empty, sprite count might be 0
    QCOMPARE(am.getSpriteManager().getSpriteCount(), 0);
    // Check if OTFI properties were applied (e.g. if SpriteManager exposed them or if tested via sprite data)
    // This is indirectly tested if SpriteManager uses the custom files.
}


void TestAssetManager::testLoadAllAssets_MissingClientProfile() {
    AssetManager am;
    QVERIFY(!am.loadAllAssets(testDataRootPath, "0.00"));
}

void TestAssetManager::testLoadAllAssets_MissingEssential_Items() {
    AssetManager am;
    QFile::remove(itemsOtbPath);
    QFile::remove(itemsXmlPath);
    QVERIFY(!am.loadAllAssets(testDataRootPath, "7.60"));
    createAllSampleFiles(); // Recreate for other tests
}

void TestAssetManager::testLoadAllAssets_MissingEssential_Sprites() {
    AssetManager am;
    QFile::remove(datPath);
    QFile::remove(sprPath);
    QVERIFY(!am.loadAllAssets(testDataRootPath, "7.60"));
    createAllSampleFiles(); // Recreate
}

void TestAssetManager::testAssetManager_As_IItemTypeProvider() {
    AssetManager am;
    QVERIFY(am.loadAllAssets(testDataRootPath, "7.60"));

    QCOMPARE(am.getName(100), QString("Sample Ground XML Override"));
    QVERIFY(am.isStackable(100));
    QVERIFY(am.isPickupable(100));
    QVERIFY(!am.isBlocking(100));

    QCOMPARE(am.getName(200), QString("Magic Sword from XML"));
    QVERIFY(am.isProjectileBlocking(200));
    QVERIFY(!am.isStackable(200));

    QCOMPARE(am.getName(999), QString("Unknown Item"));
    QVERIFY(am.isBlocking(999));
}

QTEST_MAIN(TestAssetManager)
#include "TestAssetManager.moc"
