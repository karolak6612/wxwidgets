#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy> // Not strictly needed for these tests, but good to have for I/O if events were involved

#include "core/io/OtbmMapIO.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Position.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h" // For ItemType
#include "core/settings/AppSettings.h"
#include "core/IItemTypeProvider.h" // For MockItemTypeProvider

// A simple mock item type provider for testing purposes
// In a real scenario, this would be in a shared test utility.
class MockItemTypeProvider : public RME::core::IItemTypeProvider {
public:
    MockItemTypeProvider() {
        // Add a few dummy item types for testing save/load of items
        // ID 1 is typically ground, ID 2 can be a generic item
        // ItemType(uint16_t id, const QString& name, ItemGroup group = ItemGroup::NONE);
        // Make sure these IDs are consistent with what tests might use (e.g. ID 1 for ground)
        m_itemTypes[1] = std::make_unique<RME::core::assets::ItemType>(1, "Test Ground Item");
        m_itemTypes[1]->isGround = true;
        m_itemTypes[2] = std::make_unique<RME::core::assets::ItemType>(2, "Test Regular Item");
        m_itemTypes[3] = std::make_unique<RME::core::assets::ItemType>(3, "Another Test Item");
        m_itemTypes[3]->isStackable = true; // For testing count/subtype
    }

    QString getName(uint16_t id) const override {
        auto it = m_itemTypes.find(id);
        return it != m_itemTypes.end() ? it->second->name : "Unknown Mock Item";
    }
    QString getDescription(uint16_t id) const override { Q_UNUSED(id); return "Mock Description"; }
    double getWeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1.0; }
    bool isBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isBlocking : false;}
    bool isProjectileBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isProjectileBlocking : false; }
    bool isPathBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isPathBlocking : false; }
    bool isWalkable(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isWalkable : true; } // Default true for non-existent
    bool isStackable(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isStackable : false; }
    bool isGround(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isGround : false; }
    // Implement other pure virtual methods from IItemTypeProvider with minimal/default logic
    bool isAlwaysOnTop(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isReadable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isWriteable(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isFluidContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isSplash(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isMoveable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool hasHeight(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isContainer(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isTeleport(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDoor(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isPodium(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isDepot(uint16_t id) const override { Q_UNUSED(id); return false; }
    int getSpriteX(uint16_t id, uint16_t subtype, int frame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(frame); return 0; }
    int getSpriteY(uint16_t id, uint16_t subtype, int frame) const override { Q_UNUSED(id); Q_UNUSED(subtype); Q_UNUSED(frame); return 0; }
    int getSpriteWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealWidth(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteRealHeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 32; }
    int getSpriteOffsetX(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getSpriteOffsetY(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 0; }
    int getAnimationFrames(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1;}
    const RME::core::SpriteSheet* getSpriteSheet(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return nullptr;}
    bool usesAlternativeSpriteSheet(uint16_t id, uint16_t subtype) const override {Q_UNUSED(id); Q_UNUSED(subtype); return false;}

    // Helper for tests to get an ItemType pointer
    const RME::core::assets::ItemType* getItemData(uint16_t id) const {
        auto it = m_itemTypes.find(id);
        if (it != m_itemTypes.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    // Required by AssetManager (if used directly)
    AssetManager& getAssetManager() override { Q_ASSERT(false); return *static_cast<AssetManager*>(nullptr); } // Should not be called by OtbmMapIO directly

private:
    std::map<uint16_t, std::unique_ptr<RME::core::assets::ItemType>> m_itemTypes;
};


class TestOtbmMapIO : public QObject
{
    Q_OBJECT

public:
    TestOtbmMapIO();
    ~TestOtbmMapIO() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSaveAndLoad_EmptyMap();
    void testSaveAndLoad_MapWithOneTileAndItem();
    void testLoad_NonExistentFile();
    void testGetSupportedFileExtensions();
    void testGetFormatName();

private:
    MockItemTypeProvider* m_mockItemProvider; // Owned by AssetManager
    RME::core::assets::AssetManager* m_assetManager;
    RME::core::settings::AppSettings* m_appSettings;
    RME::core::io::OtbmMapIO* m_mapIO;
    RME::core::Map* m_map; // The map used for saving
    QTemporaryDir m_tempDir;
};

TestOtbmMapIO::TestOtbmMapIO() :
    m_mockItemProvider(nullptr), m_assetManager(nullptr), m_appSettings(nullptr),
    m_mapIO(nullptr), m_map(nullptr)
{
}

TestOtbmMapIO::~TestOtbmMapIO() {
    // cleanupTestCase will handle m_assetManager, m_appSettings
    // cleanup will handle m_mapIO, m_map
}

void TestOtbmMapIO::initTestCase() {
    m_appSettings = new RME::core::settings::AppSettings();
    // Initialize settings that might be used by OtbmMapIO, e.g., SkipUnknownItems
    m_appSettings->setValue("SkipUnknownItems", true);

    // AssetManager needs an ItemDatabase, which uses IItemTypeProvider.
    // The MockItemTypeProvider will serve as the source of item data.
    m_mockItemProvider = new MockItemTypeProvider(); // Will be owned by ItemDatabase

    // Create ItemDatabase and load mock items into it.
    // AssetManager constructor takes ItemDatabase*, CreatureDatabase*, SpriteManager*
    // For these tests, we primarily need a functional ItemDatabase.
    auto itemDb = new RME::core::assets::ItemDatabase(m_mockItemProvider); // ItemDatabase takes ownership of provider
    // AssetManager will own itemDb.
    m_assetManager = new RME::core::assets::AssetManager(itemDb, nullptr, nullptr); // Pass null for creature/sprite managers for now
}

void TestOtbmMapIO::cleanupTestCase() {
    delete m_assetManager; // Deletes itemDb, which deletes m_mockItemProvider
    m_assetManager = nullptr;
    m_mockItemProvider = nullptr;

    delete m_appSettings;
    m_appSettings = nullptr;
}

void TestOtbmMapIO::init() {
    QVERIFY(m_tempDir.isValid());
    // Pass the item provider from AssetManager to Map.
    // The map needs the IItemTypeProvider to correctly handle items (e.g. isGround).
    m_map = new RME::core::Map(&(m_assetManager->getItemDatabase()));
    m_mapIO = new RME::core::io::OtbmMapIO();
}

void TestOtbmMapIO::cleanup() {
    delete m_mapIO;
    m_mapIO = nullptr;
    delete m_map;
    m_map = nullptr;
    // m_tempDir auto-cleans up its directory
}

void TestOtbmMapIO::testSaveAndLoad_EmptyMap() {
    m_map->setWidth(50);
    m_map->setHeight(50);
    m_map->setDepth(1); // Standard single floor
    m_map->setDescription("Empty Test Map");

    QString filePath = m_tempDir.filePath("empty.otbm");

    bool saveResult = m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings);
    QVERIFY2(saveResult, qPrintable("Save failed: " + m_mapIO->getLastError()));

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    bool loadResult = m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings);
    QVERIFY2(loadResult, qPrintable("Load failed: " + m_mapIO->getLastError()));

    QCOMPARE(targetMap.getDescription(), m_map->getDescription());
    QCOMPARE(targetMap.getWidth(), m_map->getWidth());
    QCOMPARE(targetMap.getHeight(), m_map->getHeight());
    QCOMPARE(targetMap.getDepth(), m_map->getDepth()); // Assuming depth is saved/loaded, might not be explicit in OTBM root attrs

    // An "empty" map in OTBM might still have tile area nodes but no tile nodes.
    // Or it might have no tile areas if truly empty and saved efficiently.
    // For now, just check no items.
    QVERIFY(targetMap.getAllTiles().isEmpty());
}

void TestOtbmMapIO::testSaveAndLoad_MapWithOneTileAndItem() {
    m_map->setWidth(100);
    m_map->setHeight(100);
    m_map->setDepth(8);
    m_map->setDescription("Map With One Tile");

    RME::core::Position testPos(10, 10, 7);
    RME::core::Tile* tile = m_map->getOrCreateTile(testPos);
    QVERIFY(tile != nullptr);
    tile->addMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE);

    // Get item type data (ID 2: "Test Regular Item") from MockItemTypeProvider via AssetManager
    const RME::core::assets::ItemType* itemType = m_assetManager->getItemDatabase().getItemData(2);
    QVERIFY(itemType != nullptr);

    std::unique_ptr<RME::core::Item> item = RME::core::Item::create(itemType->id, &(m_assetManager->getItemDatabase()));
    QVERIFY(item != nullptr);
    item->setSubtype(5); // Example: count or charge
    item->setActionID(12345);

    RME::core::Item* rawItemPtr = item.get(); // for verification later if needed
    tile->addItem(std::move(item));

    QString filePath = m_tempDir.filePath("one_tile_item.otbm");
    bool saveResult = m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings);
    QVERIFY2(saveResult, qPrintable("Save failed: " + m_mapIO->getLastError()));

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    bool loadResult = m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings);
    QVERIFY2(loadResult, qPrintable("Load failed: " + m_mapIO->getLastError()));

    QCOMPARE(targetMap.getDescription(), m_map->getDescription());
    // Dimensions might be derived from loaded content, not necessarily same as original empty map size
    // QCOMPARE(targetMap.getWidth(), m_map->getWidth());
    // QCOMPARE(targetMap.getHeight(), m_map->getHeight());

    const RME::core::Tile* loadedTile = targetMap.getTile(testPos);
    QVERIFY(loadedTile != nullptr);
    QVERIFY(loadedTile->hasMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE));

    QCOMPARE(loadedTile->getItems().size(), 1);
    const RME::core::Item* loadedItem = loadedTile->getItems().first().get();
    QVERIFY(loadedItem != nullptr);
    QCOMPARE(loadedItem->getID(), rawItemPtr->getID());
    QCOMPARE(loadedItem->getSubtype(), rawItemPtr->getSubtype());
    QCOMPARE(loadedItem->getActionID(), rawItemPtr->getActionID());
}

void TestOtbmMapIO::testLoad_NonExistentFile() {
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QString nonExistentFilePath = m_tempDir.filePath("does_not_exist.otbm");
    QVERIFY(!m_mapIO->loadMap(nonExistentFilePath, targetMap, *m_assetManager, *m_appSettings));
    QVERIFY(!m_mapIO->getLastError().isEmpty());
}

void TestOtbmMapIO::testGetSupportedFileExtensions() {
    QStringList extensions = m_mapIO->getSupportedFileExtensions();
    QCOMPARE(extensions.size(), 1);
    QCOMPARE(extensions.first(), QString("*.otbm"));
}

void TestOtbmMapIO::testGetFormatName() {
    QCOMPARE(m_mapIO->getFormatName(), QString("Open Tibia Binary Map"));
}

// QTEST_MAIN(TestOtbmMapIO) // Will be run by rme_core_io_tests
#include "TestOtbmMapIO.moc" // Must be last line for MOC to work
