#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include "core/assets/AssetManager.h"
#include "core/assets/MaterialManager.h"
#include "core/assets/MaterialData.h"
#include "core/assets/ItemDatabase.h" // For AssetManager constructor
#include "core/IItemTypeProvider.h"   // For MockItemTypeProvider

// Minimal MockItemTypeProvider for AssetManager initialization
class MaterialTest_MockItemProvider : public RME::core::IItemTypeProvider {
public:
    // Implement pure virtual methods with minimal logic
    QString getName(uint16_t id) const override { return QString("MockItem%1").arg(id); }
    QString getDescription(uint16_t id) const override { return "Mock Description"; }
    double getWeight(uint16_t id, uint16_t subtype) const override { return 1.0; }
    bool isBlocking(uint16_t id) const override { return false; }
    bool isProjectileBlocking(uint16_t id) const override { return false; }
    bool isPathBlocking(uint16_t id) const override { return false; }
    bool isWalkable(uint16_t id) const override { return true; }
    bool isStackable(uint16_t id) const override { return false; }
    bool isGround(uint16_t id) const override { return (id == 1); } // Assume ID 1 is ground for tests
    bool isAlwaysOnTop(uint16_t id) const override { return false; }
    bool isReadable(uint16_t id) const override { return false; }
    bool isWriteable(uint16_t id) const override { return false; }
    bool isFluidContainer(uint16_t id) const override { return false; }
    bool isSplash(uint16_t id) const override { return false; }
    bool isMoveable(uint16_t id) const override { return true; }
    bool hasHeight(uint16_t id) const override { return false; }
    bool isContainer(uint16_t id) const override { return false; }
    bool isTeleport(uint16_t id) const override { return false; }
    bool isDoor(uint16_t id) const override { return false; }
    bool isPodium(uint16_t id) const override { return false; }
    bool isDepot(uint16_t id) const override { return false; }
    RME::core::assets::AssetManager& getAssetManager() override { Q_FAIL("getAssetManager on mock should not be needed here."); return *static_cast<RME::core::assets::AssetManager*>(nullptr); }
    // Dummy sprite methods
    int getSpriteX(uint16_t,uint16_t,int) const override { return 0; }
    int getSpriteY(uint16_t,uint16_t,int) const override { return 0; }
    int getSpriteWidth(uint16_t,uint16_t) const override { return 32; }
    int getSpriteHeight(uint16_t,uint16_t) const override { return 32; }
    int getSpriteRealWidth(uint16_t,uint16_t) const override { return 32; }
    int getSpriteRealHeight(uint16_t,uint16_t) const override { return 32; }
    int getSpriteOffsetX(uint16_t,uint16_t) const override { return 0; }
    int getSpriteOffsetY(uint16_t,uint16_t) const override { return 0; }
    int getAnimationFrames(uint16_t,uint16_t) const override { return 1; }
    const RME::core::SpriteSheet* getSpriteSheet(uint16_t,uint16_t) const override { return nullptr; }
    bool usesAlternativeSpriteSheet(uint16_t,uint16_t) const override { return false; }
};

class TestMaterialSystem : public QObject
{
    Q_OBJECT

public:
    TestMaterialSystem();
    ~TestMaterialSystem() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testLoadSimpleGroundMaterial();
    void testLoadWallMaterial();
    void testLoadDoodadMaterial_SingleItemAlternate();
    void testLoadDoodadMaterial_CompositeAlternate();
    void testLoadMaterial_WithIncludes();
    void testLoadMaterial_CircularInclude();
    void testGetMaterial_ExistingAndNonExisting();
    void testAssetManagerIntegration();
    void testLoadError_FileNotFound();
    void testLoadError_MalformedXML();

private:
    bool writeTempXmlFile(const QString& fileName, const QString& content);

    QTemporaryDir m_tempDir;
    RME::core::assets::AssetManager* m_assetManager; // Owns MaterialManager
    MaterialTest_MockItemProvider* m_mockItemProvider_ptr; // Owned by ItemDatabase in AssetManager
    // MaterialManager is accessed via m_assetManager->getMaterialManager()
};

TestMaterialSystem::TestMaterialSystem() : m_assetManager(nullptr), m_mockItemProvider_ptr(nullptr) {}
TestMaterialSystem::~TestMaterialSystem() {}

bool TestMaterialSystem::writeTempXmlFile(const QString& fileName, const QString& content) {
    QFile file(m_tempDir.filePath(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open temporary file for writing:" << fileName;
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();
    return true;
}

void TestMaterialSystem::initTestCase() {
    m_mockItemProvider_ptr = new MaterialTest_MockItemProvider();
    auto itemDb = new RME::core::assets::ItemDatabase(m_mockItemProvider_ptr);
    // For these tests, we don't need CreatureDatabase or SpriteManager in AssetManager
    m_assetManager = new RME::core::assets::AssetManager(itemDb, nullptr, nullptr);
    QVERIFY(m_tempDir.isValid());
}

void TestMaterialSystem::cleanupTestCase() {
    delete m_assetManager; // This will delete itemDb, which deletes m_mockItemProvider_ptr
    m_assetManager = nullptr;
    m_mockItemProvider_ptr = nullptr;
}

void TestMaterialSystem::init() {
    // Clear materials before each test by re-triggering load with a (usually) non-existent main file
    // or specific test file. AssetManager's MaterialManager is reused.
    m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "dummy_main_to_clear.xml", *m_assetManager);
}

void TestMaterialSystem::cleanup() {}

void TestMaterialSystem::testLoadSimpleGroundMaterial() {
    const QString xmlContent =
        "<materials>"
        "  <brush name=\"test_grass\" type=\"ground\" server_lookid=\"101\" z-order=\"10\">"
        "    <item id=\"101\" chance=\"80\"/>"
        "    <item id=\"102\" chance=\"20\"/>"
        "    <friend name=\"dirt\"/>"
        "    <optional id=\"555\"/>"
        "  </brush>"
        "</materials>";
    QVERIFY(writeTempXmlFile("ground_test.xml", xmlContent));

    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "ground_test.xml", *m_assetManager));

    const RME::core::assets::MaterialData* material = m_assetManager->getMaterialData("test_grass");
    QVERIFY(material != nullptr);
    if (!material) return;

    QCOMPARE(material->id, QString("test_grass"));
    QCOMPARE(material->typeAttribute, QString("ground"));
    QCOMPARE(material->serverLookId, static_cast<uint16_t>(101));
    QCOMPARE(material->zOrder, 10);

    const auto* specifics = std::get_if<RME::core::assets::MaterialGroundSpecifics>(&material->specificData);
    QVERIFY(specifics != nullptr);
    if (!specifics) return;

    QCOMPARE(specifics->items.size(), 2);
    QCOMPARE(specifics->items[0].itemId, static_cast<uint16_t>(101));
    QCOMPARE(specifics->items[0].chance, 80);
    QCOMPARE(specifics->friends.size(), 1);
    QVERIFY(specifics->friends.contains("dirt"));
    QCOMPARE(specifics->optionals.size(), 1);
    QCOMPARE(specifics->optionals[0], static_cast<uint16_t>(555));
}

void TestMaterialSystem::testLoadWallMaterial() {
    const QString xmlContent =
        "<materials>"
        "  <brush name=\"stone_wall\" type=\"wall\" server_lookid=\"1049\">"
        "    <wall type=\"horizontal\">"
        "      <item id=\"1050\" chance=\"100\"/>"
        "      <door id=\"1210\" type=\"normal\" open=\"false\"/>"
        "    </wall>"
        "    <wall type=\"vertical\">"
        "      <item id=\"1049\"/>"
        "    </wall>"
        "  </brush>"
        "</materials>";
    QVERIFY(writeTempXmlFile("wall_test.xml", xmlContent));
    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "wall_test.xml", *m_assetManager));

    const RME::core::assets::MaterialData* material = m_assetManager->getMaterialData("stone_wall");
    QVERIFY(material != nullptr);
    if (!material) return;
    QCOMPARE(material->typeAttribute, QString("wall"));

    const auto* specifics = std::get_if<RME::core::assets::MaterialWallSpecifics>(&material->specificData);
    QVERIFY(specifics != nullptr);
    if (!specifics) return;

    QCOMPARE(specifics->parts.size(), 2);
    // Basic check, more detailed checks for item IDs, door properties etc. would be good
    QCOMPARE(specifics->parts[0].orientationType, QString("horizontal"));
    QCOMPARE(specifics->parts[0].items.size(), 1);
    QCOMPARE(specifics->parts[0].doors.size(), 1);
    QCOMPARE(specifics->parts[1].orientationType, QString("vertical"));
}

void TestMaterialSystem::testLoadDoodadMaterial_SingleItemAlternate() {
    const QString xmlContent =
        "<materials>"
        "  <brush name=\"simple_torch\" type=\"doodad\" server_lookid=\"2059\" draggable=\"true\">"
        "    <alternate chance=\"60\"> <item id=\"2059\"/> </alternate>"
        "    <alternate chance=\"40\"> <item id=\"2061\"/> </alternate>"
        "  </brush>"
        "</materials>";
    QVERIFY(writeTempXmlFile("doodad_simple_alt.xml", xmlContent));
    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "doodad_simple_alt.xml", *m_assetManager));

    const RME::core::assets::MaterialData* material = m_assetManager->getMaterialData("simple_torch");
    QVERIFY(material);
    if(!material) return;
    QCOMPARE(material->typeAttribute, QString("doodad"));
    QVERIFY(material->isDraggable);
    const auto* specifics = std::get_if<RME::core::assets::MaterialDoodadSpecifics>(&material->specificData);
    QVERIFY(specifics);
    if(!specifics) return;
    QCOMPARE(specifics->alternates.size(), 2);
    QCOMPARE(specifics->alternates[0].singleItemIds.size(), 1);
    QCOMPARE(specifics->alternates[0].singleItemIds[0], static_cast<uint16_t>(2059));
    QCOMPARE(specifics->alternates[0].chance, 60);
}

void TestMaterialSystem::testLoadDoodadMaterial_CompositeAlternate() {
    const QString xmlContent =
    "<materials>"
    "  <brush name=\"big_ship\" type=\"doodad\" server_lookid=\"2113\" draggable=\"false\" on_blocking=\"true\">"
    "    <alternate>"
    "      <composite chance=\"100\">"
    "        <tile x=\"0\" y=\"0\" z=\"0\"> <item id=\"4942\"/> <item id=\"405\"/> </tile>"
    "        <tile x=\"-5\" y=\"0\" z=\"0\"> <item id=\"4942\"/> </tile>"
    "      </composite>"
    "    </alternate>"
    "  </brush>"
    "</materials>";
    QVERIFY(writeTempXmlFile("doodad_composite.xml", xmlContent));
    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "doodad_composite.xml", *m_assetManager));

    const RME::core::assets::MaterialData* material = m_assetManager->getMaterialData("big_ship");
    QVERIFY(material);
    if(!material) return;
    const auto* specifics = std::get_if<RME::core::assets::MaterialDoodadSpecifics>(&material->specificData);
    QVERIFY(specifics);
    if(!specifics) return;
    QCOMPARE(specifics->alternates.size(), 1);
    QVERIFY(specifics->alternates[0].singleItemIds.isEmpty());
    QCOMPARE(specifics->alternates[0].compositeTiles.size(), 2);
    QCOMPARE(specifics->alternates[0].compositeTiles[0].itemIds.size(), 2);
}

void TestMaterialSystem::testLoadMaterial_WithIncludes() {
    const QString subXmlContent =
        "<materials>"
        "  <brush name=\"included_ground\" type=\"ground\" server_lookid=\"102\">"
        "    <item id=\"102\"/>"
        "  </brush>"
        "</materials>";
    QVERIFY(writeTempXmlFile("sub_materials.xml", subXmlContent));

    const QString mainXmlContent =
        "<materials>"
        "  <include file=\"sub_materials.xml\"/>"
        "  <brush name=\"main_wall\" type=\"wall\" server_lookid=\"1050\">"
        "    <wall type=\"horizontal\"> <item id=\"1050\"/> </wall>"
        "  </brush>"
        "</materials>";
    QVERIFY(writeTempXmlFile("main_inc_materials.xml", mainXmlContent));

    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "main_inc_materials.xml", *m_assetManager));
    QCOMPARE(m_assetManager->getMaterialManager().getAllMaterials().size(), 2);
    QVERIFY(m_assetManager->getMaterialData("included_ground") != nullptr);
    QVERIFY(m_assetManager->getMaterialData("main_wall") != nullptr);
}

void TestMaterialSystem::testLoadMaterial_CircularInclude() {
    const QString fileA_content = "<materials><include file=\"fileB.xml\"/><brush name=\"brushA\" type=\"ground\"/></materials>";
    const QString fileB_content = "<materials><include file=\"fileA.xml\"/><brush name=\"brushB\" type=\"ground\"/></materials>";
    QVERIFY(writeTempXmlFile("fileA.xml", fileA_content));
    QVERIFY(writeTempXmlFile("fileB.xml", fileB_content));

    // Should load without infinite loop, and load brushes from both (or one, depending on parse order)
    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "fileA.xml", *m_assetManager));
    // Depending on how m_parsedFiles is used, either 1 or 2 brushes will be loaded.
    // The key is that it doesn't hang or crash.
    QVERIFY(m_assetManager->getMaterialData("brushA") != nullptr || m_assetManager->getMaterialData("brushB") != nullptr);
    int loadedCount = m_assetManager->getMaterialManager().getAllMaterials().size();
    QVERIFY(loadedCount >= 1 && loadedCount <= 2);
}

void TestMaterialSystem::testGetMaterial_ExistingAndNonExisting() {
    const QString xmlContent = "<materials><brush name=\"my_brush\" type=\"ground\"/></materials>";
    QVERIFY(writeTempXmlFile("single_brush.xml", xmlContent));
    QVERIFY(m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "single_brush.xml", *m_assetManager));

    QVERIFY(m_assetManager->getMaterialData("my_brush") != nullptr);
    QVERIFY(m_assetManager->getMaterialData("non_existent_brush") == nullptr);
}

void TestMaterialSystem::testAssetManagerIntegration() {
    // This test relies on loadAllAssets in AssetManager calling materialManager.loadMaterials
    // We need to ensure AssetManager's loadAllAssets points to our temp dir for materials.xml
    const QString xmlContent = "<materials><brush name=\"integrated_brush\" type=\"doodad\"/></materials>";
    // Create a structure AssetManager might expect, e.g. XML/760/materials.xml
    QDir(m_tempDir.path()).mkpath("XML/760");
    QVERIFY(writeTempXmlFile("XML/760/materials.xml", xmlContent));

    // Create a new AssetManager to make it use our temp path for its loading sequence
    MaterialTest_MockItemProvider* tempProvider = new MaterialTest_MockItemProvider();
    RME::core::assets::ItemDatabase* tempItemDb = new RME::core::assets::ItemDatabase(tempProvider);
    RME::core::assets::AssetManager testAssetManager(tempItemDb, nullptr, nullptr);

    // AssetManager::loadAllAssets needs a client version string that would lead to XML/760 path.
    // This is tricky without mocking ClientVersionManager or knowing its exact logic.
    // For now, we'll assume the MaterialManager inside this testAssetManager is empty,
    // then call its loadMaterials directly as AssetManager would do.
    // A more integrated test would mock ClientProfile to return specific paths.

    // Directly test the material loading part of AssetManager's responsibility
    QVERIFY(testAssetManager.getMaterialManager().loadMaterialsFromDirectory(m_tempDir.filePath("XML/760"), "materials.xml", testAssetManager));
    QVERIFY(testAssetManager.getMaterialData("integrated_brush") != nullptr);
    QCOMPARE(testAssetManager.getMaterialData("integrated_brush")->typeAttribute, QString("doodad"));
}

void TestMaterialSystem::testLoadError_FileNotFound() {
    bool result = m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "non_existent_main.xml", *m_assetManager);
    QVERIFY(!result);
    QVERIFY(!m_assetManager->getMaterialManager().getLastError().isEmpty());
    QVERIFY(m_assetManager->getMaterialManager().getLastError().contains("Could not open XML file"));
}

void TestMaterialSystem::testLoadError_MalformedXML() {
    const QString malformedXml = "<materials><brush name=\"test\" type=\"ground\">NO_END_TAG";
    QVERIFY(writeTempXmlFile("malformed.xml", malformedXml));
    bool result = m_assetManager->getMaterialManager().loadMaterialsFromDirectory(m_tempDir.path(), "malformed.xml", *m_assetManager);
    QVERIFY(!result);
    QVERIFY(!m_assetManager->getMaterialManager().getLastError().isEmpty());
    QVERIFY(m_assetManager->getMaterialManager().getLastError().contains("XML parsing error"));
}

// QTEST_MAIN(TestMaterialSystem) // Will be run by a main test runner
#include "TestMaterialSystem.moc" // Must be last line for MOC to work
