#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy> // Not strictly needed for these tests, but good to have for I/O if events were involved

#include "core/io/OtbmMapIO.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h" // Needed for Item class
#include "core/Container.h" // Needed for Container class & dynamic_cast
#include "core/Position.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ItemDatabase.h" // For ItemType
#include "core/settings/AppSettings.h"
#include "core/IItemTypeProvider.h" // For MockItemTypeProvider
#include "core/map/MapElements.h"   // For WaypointData
#include "core/io/MemoryNodeFileWriteHandle.h" // For creating malformed files

// Enhanced MockItemTypeProvider
class MockItemTypeProvider : public RME::core::IItemTypeProvider {
public:
    MockItemTypeProvider() {
        addItem(1, "Test Ground Item", RME::core::ItemGroup::GROUND, true);
        addItem(2, "Test Regular Item");
        addItem(3, "Test Stackable Item", RME::core::ItemGroup::NONE, false, true);
        addItem(4, "Test Item With Text", RME::core::ItemGroup::NONE, false, false, true);
        addItem(5, "Test Depot Item", RME::core::ItemGroup::NONE, false, false, false, RME::core::ItemTypes::TYPE_DEPOT);
        addItem(6, "Test Container Item", RME::core::ItemGroup::CONTAINER, false, false, false, RME::core::ItemTypes::TYPE_CONTAINER);
        addItem(7, "Test Teleport Item", RME::core::ItemGroup::TELEPORT, false, false, false, RME::core::ItemTypes::TYPE_TELEPORT);
        addItem(8, "Test Door Item", RME::core::ItemGroup::DOOR, false, false, false, RME::core::ItemTypes::TYPE_DOOR);
    }

    void addItem(uint16_t id, const QString& name,
                 RME::core::ItemGroup group = RME::core::ItemGroup::NONE,
                 bool isGround = false, bool isStackable = false, bool isReadable = false,
                 RME::core::ItemTypes type = RME::core::ItemTypes::TYPE_NONE,
                 bool isContainer = false) {
        auto itemType = std::make_unique<RME::core::assets::ItemType>(id, name);
        itemType->group = group;
        itemType->isGround = isGround;
        itemType->isStackable = isStackable;
        itemType->isReadable = isReadable; // Example: ItemType has isReadable
        itemType->type = type;             // Example: ItemType has type for depot, door etc.
        if (group == RME::core::ItemGroup::CONTAINER) itemType->isContainer = true;
        else itemType->isContainer = isContainer;
        m_itemTypes[id] = std::move(itemType);
    }

    // --- IItemTypeProvider interface ---
    QString getName(uint16_t id) const override {
        auto it = m_itemTypes.find(id);
        return it != m_itemTypes.end() ? it->second->name : "Unknown Mock Item";
    }
    QString getDescription(uint16_t id) const override { Q_UNUSED(id); return "Mock Description"; }
    double getWeight(uint16_t id, uint16_t subtype) const override { Q_UNUSED(id); Q_UNUSED(subtype); return 1.0; }
    bool isBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isBlocking : false;}
    bool isProjectileBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isProjectileBlocking : false; }
    bool isPathBlocking(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isPathBlocking : false; }
    bool isWalkable(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isWalkable : true; }
    bool isStackable(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isStackable : false; }
    bool isGround(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isGround : false; }
    bool isAlwaysOnTop(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isReadable(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isReadable : false; }
    bool isWriteable(uint16_t id) const override { Q_UNUSED(id); return false; } // For simplicity
    bool isFluidContainer(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->group == RME::core::ItemGroup::FLUID : false; }
    bool isSplash(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->group == RME::core::ItemGroup::SPLASH : false; }
    bool isMoveable(uint16_t id) const override { Q_UNUSED(id); return true; }
    bool hasHeight(uint16_t id) const override { Q_UNUSED(id); return false; }
    bool isContainer(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->isContainer : false; }
    bool isTeleport(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->type == RME::core::ItemTypes::TYPE_TELEPORT : false; }
    bool isDoor(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->type == RME::core::ItemTypes::TYPE_DOOR : false; }
    bool isPodium(uint16_t id) const override { Q_UNUSED(id); return false; } // Add if needed
    bool isDepot(uint16_t id) const override { auto it = m_itemTypes.find(id); return it != m_itemTypes.end() ? it->second->type == RME::core::ItemTypes::TYPE_DEPOT : false; }
    // Dummy implementations for sprite methods
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
    AssetManager& getAssetManager() override { Q_FAIL("getAssetManager should not be called on MockItemTypeProvider in this context."); return *static_cast<AssetManager*>(nullptr); }

    const RME::core::assets::ItemType* getItemData(uint16_t id) const {
        auto it = m_itemTypes.find(id);
        return (it != m_itemTypes.end()) ? it->second.get() : nullptr;
    }
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
    void testSaveAndLoad_MapWithWaypointConnections();
    // New tests
    void testSaveAndLoad_TileFlags();
    void testSaveAndLoad_ItemAttributes();
    void testSaveAndLoad_ContainerItems();
    void testSaveAndLoad_MultipleTileAreas();
    void testLoad_MalformedFile_UnexpectedEOF();
    void testLoad_MalformedFile_BadNodeType();
    void testLoad_UnknownItemHandling();

private:
    MockItemTypeProvider* m_mockItemProvider_ptr; // Raw pointer, owned by ItemDatabase
    RME::core::assets::AssetManager* m_assetManager;
    RME::core::settings::AppSettings* m_appSettings;
    RME::core::io::OtbmMapIO* m_mapIO;
    RME::core::Map* m_map; // The map used for saving
    QTemporaryDir m_tempDir;
};

TestOtbmMapIO::TestOtbmMapIO() :
    m_mockItemProvider_ptr(nullptr), m_assetManager(nullptr), m_appSettings(nullptr),
    m_mapIO(nullptr), m_map(nullptr)
{}

TestOtbmMapIO::~TestOtbmMapIO() {}

void TestOtbmMapIO::initTestCase() {
    m_appSettings = new RME::core::settings::AppSettings(QSettings::IniFormat, QSettings::UserScope, "RME-Qt-Test", "OtbmMapIOTest");
    m_appSettings->setValue(RME::core::Config::Key::SKIP_UNKNOWN_ITEMS, true); // Default for tests

    m_mockItemProvider_ptr = new MockItemTypeProvider();
    auto itemDb = new RME::core::assets::ItemDatabase(m_mockItemProvider_ptr);
    m_assetManager = new RME::core::assets::AssetManager(itemDb, nullptr, nullptr);
}

void TestOtbmMapIO::cleanupTestCase() {
    delete m_assetManager; // Deletes itemDb, which deletes m_mockItemProvider_ptr
    m_assetManager = nullptr;
    m_mockItemProvider_ptr = nullptr;

    delete m_appSettings;
    m_appSettings = nullptr;
}

void TestOtbmMapIO::init() {
    QVERIFY(m_tempDir.isValid());
    m_map = new RME::core::Map(&(m_assetManager->getItemDatabase()));
    m_mapIO = new RME::core::io::OtbmMapIO();
}

void TestOtbmMapIO::cleanup() {
    delete m_mapIO;
    m_mapIO = nullptr;
    delete m_map;
    m_map = nullptr;
}

void TestOtbmMapIO::testSaveAndLoad_EmptyMap() {
    m_map->setWidth(50);
    m_map->setHeight(50);
    m_map->setDepth(1);
    m_map->setDescription("Empty Test Map");

    QString filePath = m_tempDir.filePath("empty.otbm");
    bool saveResult = m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings);
    QVERIFY2(saveResult, qPrintable("Save failed: " + m_mapIO->getLastError()));

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    bool loadResult = m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings);
    QVERIFY2(loadResult, qPrintable("Load failed: " + m_mapIO->getLastError()));

    QCOMPARE(targetMap.getDescription(), m_map->getDescription());
    // For a truly empty map, width/height might be 0 or minimal after load if not explicitly stored as map attributes
    // QCOMPARE(targetMap.getWidth(), m_map->getWidth());
    // QCOMPARE(targetMap.getHeight(), m_map->getHeight());
    QVERIFY(targetMap.getAllTiles().isEmpty());
}

void TestOtbmMapIO::testSaveAndLoad_MapWithOneTileAndItem() {
    m_map->setDescription("Map With One Tile And Item");
    RME::core::Position testPos(10, 10, 7);
    RME::core::Tile* tile = m_map->getOrCreateTile(testPos);
    QVERIFY(tile != nullptr);

    std::unique_ptr<RME::core::Item> item = RME::core::Item::create(2, &(m_assetManager->getItemDatabase())); // ID 2: Test Regular Item
    QVERIFY(item != nullptr);
    item->setSubtype(5);
    item->setActionID(12345);
    uint16_t item_id = item->getID();
    uint16_t item_subtype = item->getSubtype();
    uint16_t item_aid = item->getActionID();
    tile->addItem(std::move(item));

    QString filePath = m_tempDir.filePath("one_tile_item.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));
    const RME::core::Tile* loadedTile = targetMap.getTile(testPos);
    QVERIFY(loadedTile != nullptr);
    QCOMPARE(loadedTile->getItems().size(), 1);
    const RME::core::Item* loadedItem = loadedTile->getItems().first().get();
    QCOMPARE(loadedItem->getID(), item_id);
    QCOMPARE(loadedItem->getSubtype(), item_subtype);
    QCOMPARE(loadedItem->getActionID(), item_aid);
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

void TestOtbmMapIO::testSaveAndLoad_MapWithWaypointConnections() {
    m_map->setDescription("Map With Waypoints");
    RME::WaypointData wp1("CentralHub", {100, 100, 7});
    // WaypointData in MapElements.h does not have addConnection/isConnectedTo directly.
    // OtbmMapIO directly serializes the name and position.
    // Connections would need to be part of WaypointData or handled differently if complex.
    m_map->addWaypoint(wp1);
    RME::WaypointData wp2("NorthExit", {100, 50, 7});
    m_map->addWaypoint(wp2);

    QString filePath = m_tempDir.filePath("map_with_wp.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));
    QCOMPARE(targetMap.getWaypoints().size(), 2);
    const RME::WaypointData* loadedWp1 = targetMap.getWaypoint("CentralHub");
    QVERIFY(loadedWp1 != nullptr);
    QCOMPARE(loadedWp1->position, RME::Position(100, 100, 7));
}

// --- New Test Cases ---
void TestOtbmMapIO::testSaveAndLoad_TileFlags() {
    m_map->setDescription("Map With Tile Flags");
    RME::core::Position pos(5,5,7);
    RME::core::Tile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);
    tile->addMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE);
    tile->addMapFlag(RME::core::TileMapFlag::NO_PVP_ZONE);

    QString filePath = m_tempDir.filePath("tile_flags.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));
    const RME::core::Tile* loadedTile = targetMap.getTile(pos);
    QVERIFY(loadedTile);
    QVERIFY(loadedTile->hasMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE));
    QVERIFY(loadedTile->hasMapFlag(RME::core::TileMapFlag::NO_PVP_ZONE));
    QVERIFY(!loadedTile->hasMapFlag(RME::core::TileMapFlag::NO_LOGOUT_ZONE));
}

void TestOtbmMapIO::testSaveAndLoad_ItemAttributes() {
    m_map->setDescription("Map With Item Attributes");
    RME::core::Position pos(3,3,1);
    RME::core::Tile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);

    // Item with text (ID 4)
    auto itemText = RME::core::Item::create(4, &(m_assetManager->getItemDatabase()));
    itemText->setText("Test text attribute");
    itemText->setUniqueID(1001);
    itemText->setActionID(5001);
    tile->addItem(std::move(itemText));

    // Stackable item (ID 3)
    auto itemStack = RME::core::Item::create(3, &(m_assetManager->getItemDatabase()));
    itemStack->setSubtype(55); // Count
    tile->addItem(std::move(itemStack));

    // Depot item (ID 5)
    auto itemDepot = RME::core::Item::create(5, &(m_assetManager->getItemDatabase()));
    // Assuming DepotID might be stored as a generic attribute or via setUniqueID/ActionID if mapping exists
    // For this test, just check it saves and loads.
    tile->addItem(std::move(itemDepot));

    QString filePath = m_tempDir.filePath("item_attrs.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));
    const RME::core::Tile* loadedTile = targetMap.getTile(pos);
    QVERIFY(loadedTile);
    QCOMPARE(loadedTile->getItems().size(), 3);

    const RME::core::Item* loadedItemText = loadedTile->getItemById(4);
    QVERIFY(loadedItemText);
    QCOMPARE(loadedItemText->getText(), QString("Test text attribute"));
    QCOMPARE(loadedItemText->getUniqueID(), static_cast<uint16_t>(1001));
    QCOMPARE(loadedItemText->getActionID(), static_cast<uint16_t>(5001));

    const RME::core::Item* loadedItemStack = loadedTile->getItemById(3);
    QVERIFY(loadedItemStack);
    QCOMPARE(loadedItemStack->getSubtype(), static_cast<uint16_t>(55));

    const RME::core::Item* loadedItemDepot = loadedTile->getItemById(5);
    QVERIFY(loadedItemDepot); // Verify it loaded
}

void TestOtbmMapIO::testSaveAndLoad_ContainerItems() {
    m_map->setDescription("Map With Container Items");
    RME::core::Position pos(7,7,7);
    RME::core::Tile* tile = m_map->getOrCreateTile(pos);
    QVERIFY(tile);

    auto container = RME::core::Item::createUnique(6, &(m_assetManager->getItemDatabase())); // ID 6: Test Container Item
    QVERIFY(container);
    RME::core::Container* actualContainer = dynamic_cast<RME::core::Container*>(container.get());
    QVERIFY(actualContainer != nullptr);

    if (actualContainer) {
        actualContainer->addItem(RME::core::Item::createUnique(2, &(m_assetManager->getItemDatabase()))); // Regular item
        actualContainer->addItem(RME::core::Item::createUnique(3, &(m_assetManager->getItemDatabase()))); // Stackable item
    }
    tile->addItem(std::move(container));

    QString filePath = m_tempDir.filePath("container_items.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));
    const RME::core::Tile* loadedTile = targetMap.getTile(pos);
    QVERIFY(loadedTile);
    QCOMPARE(loadedTile->getItems().size(), 1);
    const RME::core::Item* loadedItem = loadedTile->getItems().first().get();
    QVERIFY(loadedItem && loadedItem->getID() == 6);
    const RME::core::Container* loadedContainer = dynamic_cast<const RME::core::Container*>(loadedItem);
    QVERIFY(loadedContainer);
    if (loadedContainer) {
        QCOMPARE(loadedContainer->getItems().size(), 2);
        QVERIFY(loadedContainer->hasItemOfType(2));
        QVERIFY(loadedContainer->hasItemOfType(3));
    }
}

void TestOtbmMapIO::testSaveAndLoad_MultipleTileAreas() {
    m_map->setDescription("Map With Multiple Tile Areas");
    // Create tiles that will span multiple 256x256 areas
    RME::core::Position pos1(10, 10, 0);
    RME::core::Position pos2(300, 10, 0); // Different X-area
    RME::core::Position pos3(10, 300, 0); // Different Y-area
    RME::core::Position pos4(10, 10, 1);  // Different Z-area (floor)

    m_map->getOrCreateTile(pos1)->addItem(RME::core::Item::createUnique(2, &(m_assetManager->getItemDatabase())));
    m_map->getOrCreateTile(pos2)->addItem(RME::core::Item::createUnique(2, &(m_assetManager->getItemDatabase())));
    m_map->getOrCreateTile(pos3)->addItem(RME::core::Item::createUnique(2, &(m_assetManager->getItemDatabase())));
    m_map->getOrCreateTile(pos4)->addItem(RME::core::Item::createUnique(2, &(m_assetManager->getItemDatabase())));

    QString filePath = m_tempDir.filePath("multi_area.otbm");
    QVERIFY(m_mapIO->saveMap(filePath, *m_map, *m_assetManager, *m_appSettings));
    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    QVERIFY(m_mapIO->loadMap(filePath, targetMap, *m_assetManager, *m_appSettings));

    QVERIFY(targetMap.getTile(pos1) != nullptr && targetMap.getTile(pos1)->hasItemOfType(2));
    QVERIFY(targetMap.getTile(pos2) != nullptr && targetMap.getTile(pos2)->hasItemOfType(2));
    QVERIFY(targetMap.getTile(pos3) != nullptr && targetMap.getTile(pos3)->hasItemOfType(2));
    QVERIFY(targetMap.getTile(pos4) != nullptr && targetMap.getTile(pos4)->hasItemOfType(2));
    QCOMPARE(targetMap.getAllTiles().size(), 4); // Ensure only these 4 tiles exist
}

void TestOtbmMapIO::testLoad_MalformedFile_UnexpectedEOF() {
    RME::core::io::MemoryNodeFileWriteHandle writer;
    writer.addNode(RME::core::io::OTBM_NODE_ROOT, false); // Root node
    writer.addNode(RME::core::io::OTBM_NODE_MAP_DATA, false); // Map data
    writer.addU8(RME::core::io::OTBM_ATTR_DESCRIPTION);
    writer.addString("Truncated Map");
    // writer.endNode(); // Don't end MAP_DATA
    // writer.endNode(); // Don't end ROOT

    const auto& bufferStdVec = writer.getBuffer();
    QByteArray buffer(reinterpret_cast<const char*>(bufferStdVec.data()), bufferStdVec.size());

    // Truncate it more if needed to ensure EOF during property read for example
    QByteArray truncatedBuffer = buffer.left(buffer.size() > 5 ? buffer.size() - 5 : buffer.size());

    QFile file(m_tempDir.filePath("malformed_eof.otbm"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(truncatedBuffer);
    file.close();

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    bool loadResult = m_mapIO->loadMap(file.fileName(), targetMap, *m_assetManager, *m_appSettings);
    QVERIFY(!loadResult);
    QVERIFY(m_mapIO->getLastError().contains("EOF", Qt::CaseInsensitive) ||
            m_mapIO->getLastError().contains("syntax", Qt::CaseInsensitive) ||
            m_mapIO->getLastError().contains("Failed to read", Qt::CaseInsensitive));
}

void TestOtbmMapIO::testLoad_MalformedFile_BadNodeType() {
    RME::core::io::MemoryNodeFileWriteHandle writer;
    writer.addNode(RME::core::io::OTBM_NODE_ROOT, false);
    // Write an invalid node type instead of MAP_DATA
    writer.addNode(0xEE, false); // Assuming 0xEE is not a valid top-level node type here
    writer.endNode(); // End invalid node
    writer.endNode(); // End root

    const auto& bufferStdVec = writer.getBuffer();
    QByteArray buffer(reinterpret_cast<const char*>(bufferStdVec.data()), bufferStdVec.size());

    QFile file(m_tempDir.filePath("malformed_badtype.otbm"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(buffer);
    file.close();

    RME::core::Map targetMap(&(m_assetManager->getItemDatabase()));
    bool loadResult = m_mapIO->loadMap(file.fileName(), targetMap, *m_assetManager, *m_appSettings);
    QVERIFY(!loadResult);
    QVERIFY(m_mapIO->getLastError().contains("Expected MAP_DATA", Qt::CaseInsensitive) ||
            m_mapIO->getLastError().contains("type EE", Qt::CaseInsensitive));
}

void TestOtbmMapIO::testLoad_UnknownItemHandling() {
    RME::core::io::MemoryNodeFileWriteHandle writer;
    writer.addNode(RME::core::io::OTBM_NODE_ROOT, false);
    writer.addNode(RME::core::io::OTBM_NODE_MAP_DATA, false);
    writer.addNode(RME::core::io::OTBM_NODE_TILE_AREA, false);
    QByteArray areaCoords; QDataStream dsArea(&areaCoords, QIODevice::WriteOnly); dsArea.setByteOrder(QDataStream::LittleEndian);
    dsArea << static_cast<quint16>(0) << static_cast<quint16>(0) << static_cast<quint8>(7);
    writer.addNodeData(areaCoords);
    writer.addNode(RME::core::io::OTBM_NODE_TILE, false);
    QByteArray tileCoords; tileCoords.append(char(0)); tileCoords.append(char(0));
    writer.addNodeData(tileCoords);
    writer.addNode(RME::core::io::OTBM_NODE_ITEM, false);
    QByteArray itemData; QDataStream dsItem(&itemData, QIODevice::WriteOnly); dsItem.setByteOrder(QDataStream::LittleEndian);
    dsItem << static_cast<quint16>(9999); // Unknown item ID
    writer.addNodeData(itemData);
    writer.endNode(); // Item
    writer.endNode(); // Tile
    writer.endNode(); // Tile Area
    writer.endNode(); // Map Data
    writer.endNode(); // Root

    const auto& bufferStdVec = writer.getBuffer();
    QByteArray buffer(reinterpret_cast<const char*>(bufferStdVec.data()), bufferStdVec.size());
    QFile file(m_tempDir.filePath("unknown_item.otbm"));
    QVERIFY(file.open(QIODevice::WriteOnly)); file.write(buffer); file.close();

    RME::core::Map targetMapSkip(&(m_assetManager->getItemDatabase()));
    m_appSettings->setValue(RME::core::Config::Key::SKIP_UNKNOWN_ITEMS, true);
    bool loadResultSkip = m_mapIO->loadMap(file.fileName(), targetMapSkip, *m_assetManager, *m_appSettings);
    QVERIFY2(loadResultSkip, qPrintable("Load with SkipUnknownItems=true failed: " + m_mapIO->getLastError()));
    const RME::core::Tile* tileSkip = targetMapSkip.getTile({0,0,7});
    QVERIFY(tileSkip == nullptr || tileSkip->getItems().isEmpty()); // Tile might not be created if item is skipped and tile becomes empty

    RME::core::Map targetMapNoSkip(&(m_assetManager->getItemDatabase()));
    m_appSettings->setValue(RME::core::Config::Key::SKIP_UNKNOWN_ITEMS, false);
    bool loadResultNoSkip = m_mapIO->loadMap(file.fileName(), targetMapNoSkip, *m_assetManager, *m_appSettings);
    QVERIFY2(!loadResultNoSkip, "Load with SkipUnknownItems=false succeeded unexpectedly for unknown item ID 9999.");
    QVERIFY(m_mapIO->getLastError().contains("9999")); // Error message should mention the unknown ID
}


#include "TestOtbmMapIO.moc" // Must be last line for MOC to work
