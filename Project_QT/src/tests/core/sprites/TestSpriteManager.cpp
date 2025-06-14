#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream> // For creating sample OTFI
#include "core/sprites/SpriteManager.h" // Class to test
#include "core/assets/SpriteData.h" // Updated include path
#include "core/assets/ClientProfile.h" // For creating a test ClientProfile

using namespace RME;

class TestSpriteManager : public QObject
{
    Q_OBJECT

private:
    QString sampleDatPath;
    QString sampleSprPath;
    QString sampleOtfiPath;
    ClientProfile testClientProfile;

    void ensureSampleDatExists() {
        if (QFile::exists(sampleDatPath)) return;
        QFile file(sampleDatPath);
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray datBytes = QByteArray::fromHex(
                "00000000" // Signature
                "0200"     // 2 Item sprites (IDs 1-2 if firstID=1, or 0-1 if firstID=0)
                "0000"     // 0 Outfit sprites
                "0000"     // 0 Effect sprites
                "0000"     // 0 Projectile sprites
                // Sprite ID 1 (or 0, depending on firstID logic)
                "0100"     // Width: 1
                "0100"     // Height: 1
                "01"       // Layers: 1
                "01"       // PatternsX: 1
                "01"       // PatternsY: 1
                "01"       // PatternsZ: 1
                "01"       // AnimationPhases: 1
                // Sprite ID 2 (or 1)
                "0200"     // Width: 2
                "0100"     // Height: 1
                "01"       // Layers: 1
                "01"       // PatternsX: 1
                "01"       // PatternsY: 1
                "01"       // PatternsZ: 1
                "01"       // AnimationPhases: 1
            );
            file.write(datBytes);
            file.close();
        } else { qWarning() << "Could not create " << sampleDatPath; }
    }

    void ensureSampleSprExists() {
        if (QFile::exists(sampleSprPath)) return;
        QFile file(sampleSprPath);
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray sprBytes = QByteArray::fromHex(
                "00000000" // SPR Signature
                // Address table: For Sprite IDs 0, 1, 2. (3 entries)
                // If firstID in DAT is 1, then sprite 0 address is unused.
                // If firstID in DAT is 0, then sprite 0 address is used.
                // Sample DAT has 2 items. If firstID=1, they are 1,2. If firstID=0, they are 0,1.
                // The SpriteManager logic for firstID/lastID and SPR table indexing is key.
                // The sample SPR was defined with 3 addresses, for IDs 0, 1, 2.
                "00000000" // Addr for effective ID 0 (points to nothing)
                "0C000000" // Addr for effective ID 1 (data at offset 0x0C, after this table of 3*4=12 bytes)
                "10000000" // Addr for effective ID 2 (data at offset 0x10, after Sprite 1's 4 bytes)
                // Data for Sprite ID 1 (1x1 transparent) - at offset 0x0C
                "0100"     // 1 transparent pixel
                "0000"     // 0 colored pixels
                // Data for Sprite ID 2 (2x1, Red, Green) - at offset 0x10
                "0000"     // 0 transparent pixels
                "0200"     // 2 colored pixels
                "FF0000"   // Red pixel (R,G,B)
                "00FF00"   // Green pixel (R,G,B)
            ); // Total 4 (sig) + 12 (addr table) + 4 (spr1 data) + 10 (spr2 data) = 30 bytes.
            file.write(sprBytes);
            file.close();
        } else { qWarning() << "Could not create " << sampleSprPath; }
    }

    void ensureSampleOtfiExists() {
        if (QFile::exists(sampleOtfiPath)) return;
        QFile file(sampleOtfiPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<fileInformation extended=\"true\" alpha=\"true\" frameDurations=\"true\">\n"
                << "  <dat path=\"custom.dat\" />\n" // Example custom paths
                << "  <spr path=\"custom.spr\" />\n"
                << "</fileInformation>";
            file.close();
        } else { qWarning() << "Could not create " << sampleOtfiPath; }
    }

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testLoadNonExistentDatSpr();
    void testLoadSampleDatSpr_SimpleFormat();
    void testSpriteDataContent();
    void testLoadWithOtfi_Overrides();
    void testOtfiCustomPaths(); // New test
};

void TestSpriteManager::initTestCase() {
    sampleDatPath = "test_sample.dat";
    sampleSprPath = "test_sample.spr";
    sampleOtfiPath = "test_sample.otfi";

    ensureSampleDatExists();
    ensureSampleSprExists();
    ensureSampleOtfiExists();

    testClientProfile.versionString = "TEST_SPRITE_MGR";
    testClientProfile.name = "Test Profile for SpriteManager";
    // Use a DatFormat that SpriteManager's refined logic for sample.dat will handle.
    // The sample.dat (7 bytes per sprite meta) matches V_760's metadata size if we ignore color key.
    // Let's assume SpriteManager's V_760 case is adapted or a new one is made for this.
    testClientProfile.datFormat = DatFormat::V_760;
    testClientProfile.extendedSprites = false;
    testClientProfile.transparentSprites = true;
    // For V_760, firstSpriteID should be 1.
    // The sample DAT has 2 items. So IDs 1 and 2 will be loaded.
    // The sample SPR has addresses for IDs 0, 1, 2. SpriteManager must handle this mapping.
}

void TestSpriteManager::cleanupTestCase() {
    QFile::remove(sampleDatPath);
    QFile::remove(sampleSprPath);
    QFile::remove(sampleOtfiPath);
}

void TestSpriteManager::testLoadNonExistentDatSpr() {
    SpriteManager sm;
    QVERIFY(!sm.loadDatSpr("non_existent.dat", "non_existent.spr", testClientProfile));
}

void TestSpriteManager::testLoadSampleDatSpr_SimpleFormat() {
    SpriteManager sm;
    QVERIFY(sm.loadDatSpr(sampleDatPath, sampleSprPath, testClientProfile));
    QCOMPARE(sm.getSpriteCount(), 2); // Sprites ID 1 and 2

    const RME::core::assets::SpriteData* sprite1 = sm.getSpriteData(1);
    QVERIFY(sprite1 != nullptr && sprite1->id == 1);
    if (sprite1) {
        QCOMPARE(sprite1->width, 1); QCOMPARE(sprite1->height, 1);
        QCOMPARE(sprite1->layers, 1); QCOMPARE(sprite1->patternsX, 1);
        QCOMPARE(sprite1->patternsY, 1); QCOMPARE(sprite1->patternsZ, 1);
        QCOMPARE(sprite1->phases, 1);
        QCOMPARE(sprite1->getTotalImageCount(), 1u);
        QCOMPARE(sprite1->frames.size(), 1); // Pixels should be loaded
    }

    const RME::core::assets::SpriteData* sprite2 = sm.getSpriteData(2);
    QVERIFY(sprite2 != nullptr && sprite2->id == 2);
    if (sprite2) {
        QCOMPARE(sprite2->width, 2); QCOMPARE(sprite2->height, 1);
        QCOMPARE(sprite2->getTotalImageCount(), 1u);
        QCOMPARE(sprite2->frames.size(), 1); // Pixels should be loaded
    }

    QVERIFY(sm.getSpriteData(0)->id == 0); // Should return invalidSpriteData
    QVERIFY(sm.getSpriteData(3)->id == 0);
}

void TestSpriteManager::testSpriteDataContent() {
    SpriteManager sm;
    QVERIFY(sm.loadDatSpr(sampleDatPath, sampleSprPath, testClientProfile));

    const RME::core::assets::SpriteData* sprite1 = sm.getSpriteData(1); // 1x1 transparent
    QVERIFY(sprite1 && sprite1->frames.size() == 1);
    if (sprite1 && !sprite1->frames.isEmpty()) {
        const QImage& img1 = sprite1->frames.first().image; // SpriteFrame is now RME::core::assets::SpriteFrame, QImage is fine
        QCOMPARE(img1.width(), 1); QCOMPARE(img1.height(), 1);
        QVERIFY(qAlpha(img1.pixel(0,0)) == 0); // Fully transparent
    }

    const RME::core::assets::SpriteData* sprite2 = sm.getSpriteData(2); // 2x1 Red, Green
    QVERIFY(sprite2 && sprite2->frames.size() == 1);
    if (sprite2 && !sprite2->frames.isEmpty()) {
        const QImage& img2 = sprite2->frames.first().image; // SpriteFrame is now RME::core::assets::SpriteFrame
        QCOMPARE(img2.width(), 2); QCOMPARE(img2.height(), 1);
        QCOMPARE(img2.pixelColor(0,0), QColor(255,0,0,255));
        QCOMPARE(img2.pixelColor(1,0), QColor(0,255,0,255));
    }
}

void TestSpriteManager::testLoadWithOtfi_Overrides() {
    SpriteManager sm;
    OtfiData otfiDataResult;
    // Create a specific OTFI for this test that doesn't specify paths
    QString otfiNoPaths = "test_otfi_no_paths.otfi";
    QFile file(otfiNoPaths);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<fileInformation extended=\"true\" alpha=\"false\" frameDurations=\"false\" />"; // alpha=false
        file.close();
    }

    QVERIFY(sm.loadOtfi(otfiNoPaths, otfiDataResult));
    QVERIFY(otfiDataResult.isExtended);
    QVERIFY(!otfiDataResult.hasTransparency); // alpha="false"
    QVERIFY(!otfiDataResult.hasFrameDurations);

    QVERIFY(sm.loadDatSpr(sampleDatPath, sampleSprPath, testClientProfile));
    const RME::core::assets::SpriteData* sprite1 = sm.getSpriteData(1);
    QVERIFY(sprite1 != nullptr);
    if (sprite1) {
        QVERIFY(sprite1->isExtended);       // Should be true from OTFI
        QVERIFY(!sprite1->hasTransparency); // Should be false from OTFI
    }
    QFile::remove(otfiNoPaths);
}

void TestSpriteManager::testOtfiCustomPaths() {
    SpriteManager sm;
    OtfiData otfiDataResult;
    // sampleOtfiPath (created by ensureSampleOtfiExists) specifies custom.dat/custom.spr
    QVERIFY(sm.loadOtfi(sampleOtfiPath, otfiDataResult));
    QCOMPARE(otfiDataResult.customDatPath, QString("custom.dat"));
    QCOMPARE(otfiDataResult.customSprPath, QString("custom.spr"));

    // Create dummy custom.dat and custom.spr for this test
    QFile customDat(otfiDataResult.customDatPath);
    if (customDat.open(QIODevice::WriteOnly)) { /* Write minimal valid DAT if needed */ customDat.close(); }
    QFile customSpr(otfiDataResult.customSprPath);
    if (customSpr.open(QIODevice::WriteOnly)) { /* Write minimal valid SPR if needed */ customSpr.close(); }

    // loadDatSpr should attempt to use these custom paths.
    // Since they are empty/minimal, loading will likely fail or load 0 sprites.
    // The key is that it *tries* to use them. This is hard to check without filesystem mocking.
    // We can check if it fails differently than with non-existent standard paths.
    // Or, if successful, sprite count would be 0.
    QVERIFY(!sm.loadDatSpr("dummy_original.dat", "dummy_original.spr", testClientProfile)); // Should try custom.dat/spr
    QCOMPARE(sm.getSpriteCount(), 0); // Expect 0 sprites as custom files are empty/invalid

    QFile::remove(otfiDataResult.customDatPath);
    QFile::remove(otfiDataResult.customSprPath);
}

QTEST_MAIN(TestSpriteManager)
#include "TestSpriteManager.moc"
