#include <QtTest/QtTest>
#include <memory>
#include <QRegularExpression> // Added for QRegularExpression

#include "editor_logic/commands/SetHouseExitCommand.h"
#include "core/map/Map.h"
#include "core/houses/HouseData.h"
#include "core/Tile.h"
#include "core/Position.h"
#include "core/Item.h"
#include "core/assets/AssetManager.h"
#include "core/sprites/SpriteManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/ItemData.h"
#include "core/assets/CreatureDatabase.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/MaterialManager.h"

// Using declarations
using RMEPosition = RME::core::Position;
using RMEMap = RME::core::Map;
using RMEHouseData = RME::core::HouseData;
using RMETile = RME::core::Tile;
using RMEItem = RME::core::Item;
namespace RME_COMMANDS = RME::editor::commands;
using RMESetHouseExitCommand = RME_COMMANDS::SetHouseExitCommand;

class TestSetHouseExitCommand : public QObject {
    Q_OBJECT

public:
    TestSetHouseExitCommand() = default;
    ~TestSetHouseExitCommand() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testRedo_SetsNewExit();
    void testUndo_RestoresOldExit();
    void testRedo_WithNonExistentHouse();
    void testUndo_WithNonExistentHouse();
    void testCommandText();
    void testSetExit_ToClear(); // New slot

private:
    std::unique_ptr<RMEMap> m_map;
    std::unique_ptr<RME::core::assets::ClientVersionManager> m_clientVersionManager;
    std::unique_ptr<RME::core::assets::ItemDatabase> m_itemDatabase;
    std::unique_ptr<RME::core::assets::CreatureDatabase> m_creatureDatabase;
    std::unique_ptr<RME::core::sprites::SpriteManager> m_spriteManager;
    std::unique_ptr<RME::core::assets::MaterialManager> m_materialManager;
    std::unique_ptr<RME::core::assets::AssetManager> m_assetManager;

    const uint32_t HOUSE_ID = 1;
    RMEPosition m_initialExitPos{5, 5, 7};
    RMEPosition m_newExitPos{10, 10, 7};
    RMEPosition m_clearedExitPos{};
    const uint16_t DUMMY_GROUND_ID = 12345;
};

void TestSetHouseExitCommand::initTestCase() {
    // Global setup if any
}
void TestSetHouseExitCommand::cleanupTestCase() {
    // Global cleanup
}

void TestSetHouseExitCommand::init() {
    m_clientVersionManager = std::make_unique<RME::core::assets::ClientVersionManager>();
    m_itemDatabase = std::make_unique<RME::core::assets::ItemDatabase>(*m_clientVersionManager);

    RME::core::assets::ItemData groundData;
    groundData.id = DUMMY_GROUND_ID;
    groundData.name = "Dummy Ground";
    groundData.isGround = true;
    groundData.isBlocking = false;
    m_itemDatabase->addItemData(groundData);

    m_creatureDatabase = std::make_unique<RME::core::assets::CreatureDatabase>();
    m_spriteManager = std::make_unique<RME::core::sprites::SpriteManager>(*m_clientVersionManager);
    m_materialManager = std::make_unique<RME::core::assets::MaterialManager>(*m_clientVersionManager);
    m_assetManager = std::make_unique<RME::core::assets::AssetManager>(
        *m_itemDatabase, *m_creatureDatabase, *m_spriteManager, *m_clientVersionManager, *m_materialManager
    );
    m_map = std::make_unique<RMEMap>(20, 20, 8, m_assetManager.get());

    RMEHouseData house(HOUSE_ID, "TestHouse");
    RMETile* initialExitTile = m_map->getOrCreateTile(m_initialExitPos);
    if (initialExitTile && !initialExitTile->getGround()) {
        initialExitTile->setGround(std::make_unique<RMEItem>(DUMMY_GROUND_ID, m_itemDatabase->getItemData(DUMMY_GROUND_ID)));
    }
    if (initialExitTile) {
        initialExitTile->setHouseId(0);
        initialExitTile->update(); // Ensure flags are updated
    }


    house.setEntryPoint(m_initialExitPos, m_map.get());
    m_map->addHouse(std::move(house));

    RMETile* newExitTile = m_map->getOrCreateTile(m_newExitPos);
    if (newExitTile && !newExitTile->getGround()) {
        newExitTile->setGround(std::make_unique<RMEItem>(DUMMY_GROUND_ID, m_itemDatabase->getItemData(DUMMY_GROUND_ID)));
    }
     if (newExitTile) {
        newExitTile->setHouseId(0);
        newExitTile->update();
     }
}

void TestSetHouseExitCommand::cleanup() {
    m_map.reset();
    m_assetManager.reset();
    m_materialManager.reset();
    m_spriteManager.reset();
    m_creatureDatabase.reset();
    m_itemDatabase.reset();
    m_clientVersionManager.reset();
}

void TestSetHouseExitCommand::testRedo_SetsNewExit() {
    RMEHouseData* house = m_map->getHouse(HOUSE_ID);
    QVERIFY(house);
    QCOMPARE(house->getEntryPoint(), m_initialExitPos);

    RMETile* oldExitTile = m_map->getTile(m_initialExitPos);
    RMETile* newExitTile = m_map->getTile(m_newExitPos);
    QVERIFY(oldExitTile && oldExitTile->isHouseExit());
    QVERIFY(newExitTile && !newExitTile->isHouseExit());

    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), HOUSE_ID, m_initialExitPos, m_newExitPos
    );
    cmd->redo();

    QCOMPARE(house->getEntryPoint(), m_newExitPos);
    QVERIFY(oldExitTile && !oldExitTile->isHouseExit());
    QVERIFY(newExitTile && newExitTile->isHouseExit());
    QVERIFY(cmd->text().contains("Set House Exit"));
}

void TestSetHouseExitCommand::testUndo_RestoresOldExit() {
    RMEHouseData* house = m_map->getHouse(HOUSE_ID);
    QVERIFY(house);

    RMETile* oldExitTile = m_map->getTile(m_initialExitPos);
    RMETile* newExitTile = m_map->getTile(m_newExitPos);
    QVERIFY(oldExitTile && newExitTile);

    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), HOUSE_ID, m_initialExitPos, m_newExitPos
    );
    cmd->redo();
    QCOMPARE(house->getEntryPoint(), m_newExitPos);

    cmd->undo();

    QCOMPARE(house->getEntryPoint(), m_initialExitPos);
    QVERIFY(oldExitTile && oldExitTile->isHouseExit());
    QVERIFY(newExitTile && !newExitTile->isHouseExit());
    QVERIFY(cmd->text().contains("Undo Set House Exit"));
}

void TestSetHouseExitCommand::testRedo_WithNonExistentHouse() {
    const uint32_t NON_EXISTENT_HOUSE_ID = 999;
    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), NON_EXISTENT_HOUSE_ID, m_initialExitPos, m_newExitPos
    );

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("House ID 999 not found in Map::getHouse for SetHouseExitCommand"));
    cmd->redo();
    QVERIFY(cmd->text().contains("not found"));

    RMEHouseData* house = m_map->getHouse(HOUSE_ID);
    QVERIFY(house);
    QCOMPARE(house->getEntryPoint(), m_initialExitPos);
}

void TestSetHouseExitCommand::testUndo_WithNonExistentHouse() {
    RMEHouseData* house = m_map->getHouse(HOUSE_ID);
    QVERIFY(house);
    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), HOUSE_ID, m_initialExitPos, m_newExitPos
    );
    cmd->redo();
    QCOMPARE(house->getEntryPoint(), m_newExitPos);

    m_map->removeHouse(HOUSE_ID);
    QVERIFY(m_map->getHouse(HOUSE_ID) == nullptr);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("House ID 1 not found in Map::getHouse for SetHouseExitCommand undo"));
    cmd->undo();
    QVERIFY(cmd->text().contains("not found"));
}

void TestSetHouseExitCommand::testCommandText() {
    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), HOUSE_ID, m_initialExitPos, m_newExitPos
    );

    cmd->redo();
    QVERIFY(cmd->text().contains(QString::number(HOUSE_ID)));
    QVERIFY(cmd->text().contains(QString::number(m_newExitPos.x)));
    QVERIFY(cmd->text().contains(QString::number(m_newExitPos.y)));
    QVERIFY(cmd->text().contains(QString::number(m_newExitPos.z)));
    QVERIFY(!cmd->text().contains("Undo"));

    cmd->undo();
    QVERIFY(cmd->text().contains("Undo"));
    QVERIFY(cmd->text().contains(QString::number(m_initialExitPos.x))); // Check for old pos in undo text
}

void TestSetHouseExitCommand::testSetExit_ToClear() {
    RMEHouseData* house = m_map->getHouse(HOUSE_ID);
    QVERIFY(house);
    QVERIFY(house->getEntryPoint() == m_initialExitPos);
    RMETile* initialExitTile = m_map->getTile(m_initialExitPos);
    QVERIFY(initialExitTile && initialExitTile->isHouseExit());

    auto cmd = std::make_unique<RMESetHouseExitCommand>(
        m_map.get(), HOUSE_ID, m_initialExitPos, m_clearedExitPos
    );
    cmd->redo();

    QCOMPARE(house->getEntryPoint(), m_clearedExitPos);
    QVERIFY(initialExitTile && !initialExitTile->isHouseExit());
}


// #include "TestSetHouseExitCommand.moc" // For AUTOMOC
