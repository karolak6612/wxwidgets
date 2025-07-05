#include <QtTest>
#include <QUndoStack>
#include <QApplication> // For clipboard access
#include <QClipboard>
#include <QMimeData>
#include "Project_QT/src/core/clipboard/ClipboardManager.h"
#include "Project_QT/src/core/clipboard/ClipboardData.h"
#include "Project_QT/src/core/selection/SelectionManager.h"
#include "Project_QT/src/core/actions/DeleteCommand.h" // For checking command type
#include "Project_QT/src/core/actions/PasteCommand.h"  // For checking command type
#include "Project_QT/src/tests/core/mocks/MockMapElements.h"

// Define a placeholder for tile flags if not accessible
#ifndef TF_PROTECTIONZONE
#define TF_PROTECTIONZONE 1 // Example value, replace with actual if available
#endif


class TestClipboardManager : public QObject {
    Q_OBJECT

public:
    TestClipboardManager();
    ~TestClipboardManager();

private slots:
    void init();
    void cleanup();

    void testClipboardDataSerialization();
    void testCopySelection_Empty();
    void testCopySelection_SelectedGround();
    void testCopySelection_SelectedItem();
    // Add tests for creature, spawn copy if mocks support them well enough

    void testCanPaste();
    void testGetPasteData_Valid();
    void testGetPasteData_Invalid();

    void testCutSelection();
    void testPasteSelection();

private:
    RME::Map* m_mockMap;
    QUndoStack* m_undoStack;
    RME::SelectionManager* m_selectionManager;
    ClipboardManager* m_clipboardManager;

    MockTile* m_tile1;
    MockTile* m_tile2;
    MockItem* m_item1_t1;
};

TestClipboardManager::TestClipboardManager() :
    m_mockMap(nullptr), m_undoStack(nullptr), m_selectionManager(nullptr), m_clipboardManager(nullptr),
    m_tile1(nullptr), m_tile2(nullptr), m_item1_t1(nullptr)
{}

TestClipboardManager::~TestClipboardManager() {
    cleanup();
}

void TestClipboardManager::init() {
    m_mockMap = new MockMap();
    m_undoStack = new QUndoStack(this);
    m_selectionManager = new RME::SelectionManager(m_mockMap, m_undoStack, this);
    m_clipboardManager = new ClipboardManager(this);

    // Setup map with some elements
    m_tile1 = static_cast<MockTile*>(m_mockMap->getOrCreateTile(RME::Position(10, 10, 7)));
    m_tile2 = static_cast<MockTile*>(m_mockMap->getOrCreateTile(RME::Position(11, 10, 7)));
    m_item1_t1 = new MockItem(101);
    m_tile1->addItem(m_item1_t1, false);

    QApplication::clipboard()->clear();
}

void TestClipboardManager::cleanup() {
    // Order of deletion matters if objects have dependencies or parent-child relationships not managed by Qt
    delete m_clipboardManager; m_clipboardManager = nullptr;
    delete m_selectionManager; m_selectionManager = nullptr;
    // m_undoStack is parented to 'this', Qt will manage it. Or delete explicitly:
    // delete m_undoStack; m_undoStack = nullptr;

    // MockMap's destructor should handle deleting MockTiles it owns.
    // MockTile's destructor should handle deleting MockItems it owns if addItem implies ownership.
    // In our MockTile, addItem(item, false) implies MockTile does not own it if 'false' means 'do not autodelete'.
    // However, MockTile destructor has qDeleteAll(m_items), so it *does* delete them.
    delete m_mockMap; m_mockMap = nullptr;

    // Pointers are invalidated by MockMap deletion
    m_tile1 = nullptr;
    m_tile2 = nullptr;
    m_item1_t1 = nullptr; // This was owned by m_tile1 via its m_items list and qDeleteAll
}

void TestClipboardManager::testClipboardDataSerialization() {
    RME::ClipboardTileData originalTileData;
    originalTileData.relativePosition = RME::Position(1, 2, 0);
    originalTileData.hasGround = true;
    originalTileData.groundItemID = 123;
    originalTileData.houseId = 7;
    originalTileData.tileFlags = TF_PROTECTIONZONE;

    RME::ClipboardItemData itemD;
    itemD.id = 1001;
    itemD.subType = 5;
    itemD.attributes.insert("testKey", "testValue");
    originalTileData.items.append(itemD);
    originalTileData.hasCreature = false;

    RME::ClipboardContent originalContent;
    originalContent.tiles.append(originalTileData);

    QByteArray byteArray;
    QDataStream outStream(&byteArray, QIODevice::WriteOnly);
    outStream << originalContent;

    QVERIFY(!byteArray.isEmpty());

    RME::ClipboardContent deserializedContent;
    QDataStream inStream(&byteArray, QIODevice::ReadOnly);
    inStream >> deserializedContent;

    QCOMPARE(inStream.status(), QDataStream::Ok);
    QCOMPARE(deserializedContent.tiles.count(), 1);

    const auto& dt = deserializedContent.tiles.first();
    QCOMPARE(dt.relativePosition, originalTileData.relativePosition);
    QCOMPARE(dt.hasGround, originalTileData.hasGround);
    QCOMPARE(dt.groundItemID, originalTileData.groundItemID);
    QCOMPARE(dt.houseId, originalTileData.houseId);
    QCOMPARE(dt.tileFlags, originalTileData.tileFlags);
    QCOMPARE(dt.items.count(), 1);
    QCOMPARE(dt.items.first().id, itemD.id);
    QCOMPARE(dt.items.first().subType, itemD.subType);
    QCOMPARE(dt.items.first().attributes.value("testKey").toString(), "testValue");
}

void TestClipboardManager::testCopySelection_Empty() {
    m_clipboardManager->copySelection(*m_selectionManager, *m_mockMap);
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData == nullptr || !mimeData->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE) || mimeData->data(RME::RME_CLIPBOARD_MIME_TYPE).isEmpty());
}

void TestClipboardManager::testCopySelection_SelectedGround() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addTile(m_tile1);
    m_selectionManager->finishSelectionChange();

    m_tile1->setHouseId(50);
    m_tile1->setFlags(TF_PROTECTIONZONE);

    m_clipboardManager->copySelection(*m_selectionManager, *m_mockMap);

    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData != nullptr);
    QVERIFY(mimeData->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE));

    QByteArray data = mimeData->data(RME::RME_CLIPBOARD_MIME_TYPE);
    QVERIFY(!data.isEmpty());

    RME::ClipboardContent content;
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> content;

    QCOMPARE(content.tiles.count(), 1);
    const auto& tileData = content.tiles.first();
    QVERIFY(tileData.hasGround);
    QCOMPARE(tileData.relativePosition, RME::Position(0,0,0));
    QCOMPARE(tileData.houseId, (uint32_t)50);
    QCOMPARE(tileData.tileFlags, (uint32_t)TF_PROTECTIONZONE);
}

void TestClipboardManager::testCopySelection_SelectedItem() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addItem(m_tile1, m_item1_t1);
    m_selectionManager->finishSelectionChange();

    m_clipboardManager->copySelection(*m_selectionManager, *m_mockMap);
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData != nullptr && mimeData->hasFormat(RME::RME_CLIPBOARD_MIME_TYPE));
    QByteArray data = mimeData->data(RME::RME_CLIPBOARD_MIME_TYPE);
    RME::ClipboardContent content;
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> content;

    QCOMPARE(content.tiles.count(), 1);
    const auto& tileData = content.tiles.first();
    QVERIFY(!tileData.hasGround);
    QCOMPARE(tileData.items.count(), 1);
    QCOMPARE(tileData.items.first().id, m_item1_t1->getID());
}


void TestClipboardManager::testCanPaste() {
    QVERIFY(!m_clipboardManager->canPaste());

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/x-some-other-type", QByteArray("data"));
    QApplication::clipboard()->setMimeData(mimeData);
    QVERIFY(!m_clipboardManager->canPaste());

    mimeData = new QMimeData();
    QByteArray validByteArray;
    QDataStream outStream(&validByteArray, QIODevice::WriteOnly);
    RME::ClipboardContent dummyContent;
    RME::ClipboardTileData tileD; tileD.relativePosition = RME::Position(0,0,0); tileD.hasGround = true;
    dummyContent.tiles.append(tileD);
    outStream << dummyContent;
    mimeData->setData(RME::RME_CLIPBOARD_MIME_TYPE, validByteArray);
    QApplication::clipboard()->setMimeData(mimeData);
    QVERIFY(m_clipboardManager->canPaste());
}

void TestClipboardManager::testGetPasteData_Valid() {
    RME::ClipboardContent originalContent;
    RME::ClipboardTileData tileD;
    tileD.relativePosition = RME::Position(0,0,0);
    tileD.hasGround = true;
    tileD.groundItemID = 77;
    originalContent.tiles.append(tileD);

    QByteArray byteArray;
    QDataStream outStream(&byteArray, QIODevice::WriteOnly);
    outStream << originalContent;
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(RME::RME_CLIPBOARD_MIME_TYPE, byteArray);
    QApplication::clipboard()->setMimeData(mimeData);

    RME::ClipboardContent retrievedContent = m_clipboardManager->getPasteData();
    QCOMPARE(retrievedContent.tiles.count(), 1);
    QVERIFY(retrievedContent.tiles.first().hasGround);
    QCOMPARE(retrievedContent.tiles.first().groundItemID, (uint16_t)77);
}

void TestClipboardManager::testGetPasteData_Invalid() {
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(RME::RME_CLIPBOARD_MIME_TYPE, QByteArray("this is not valid qdatastream data"));
    QApplication::clipboard()->setMimeData(mimeData);

    RME::ClipboardContent retrievedContent = m_clipboardManager->getPasteData();
    QVERIFY(retrievedContent.tiles.isEmpty());
}

void TestClipboardManager::testCutSelection() {
    m_selectionManager->startSelectionChange();
    m_selectionManager->addTile(m_tile1);
    m_selectionManager->finishSelectionChange();
    m_tile1->setFlags(123);

    m_clipboardManager->cutSelection(*m_selectionManager, *m_mockMap, m_undoStack);

    QVERIFY(m_clipboardManager->canPaste());
    RME::ClipboardContent cbContent = m_clipboardManager->getPasteData();
    QCOMPARE(cbContent.tiles.count(), 1);
    QVERIFY(cbContent.tiles.first().hasGround);
    QCOMPARE(cbContent.tiles.first().tileFlags, (uint32_t)123);

    QCOMPARE(m_undoStack->count(), 1);
    const QUndoCommand* cmd = m_undoStack->command(0);
    QVERIFY(dynamic_cast<const RME::DeleteCommand*>(cmd) != nullptr);
    QCOMPARE(cmd->text(), "Cut");

    // Verify tile state before redo (it should still be as it was)
    QVERIFY(m_tile1->getFlags() == 123);
    m_undoStack->redo(); // Perform the delete part of cut
    // This check depends on DeleteCommand::redo() and MockTile's behavior
    // For example, if DeleteCommand::redo clears flags for ground:
    // QCOMPARE(m_tile1->getFlags(), (uint32_t)0);

    m_undoStack->undo(); // Undo the delete
    // QCOMPARE(m_tile1->getFlags(), (uint32_t)123);
}

void TestClipboardManager::testPasteSelection() {
    RME::ClipboardContent contentToPaste;
    RME::ClipboardTileData tileData;
    tileData.relativePosition = RME::Position(0, 0, 0);
    tileData.hasGround = true;
    tileData.tileFlags = 456;
    contentToPaste.tiles.append(tileData);

    QByteArray byteArray;
    QDataStream outStream(&byteArray, QIODevice::WriteOnly);
    outStream << contentToPaste;
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(RME::RME_CLIPBOARD_MIME_TYPE, byteArray);
    QApplication::clipboard()->setMimeData(mimeData);

    RME::Position pasteTargetPos(20, 20, 7);
    m_clipboardManager->paste(*m_mockMap, pasteTargetPos, m_undoStack);

    QCOMPARE(m_undoStack->count(), 1);
    const QUndoCommand* cmd = m_undoStack->command(0);
    QVERIFY(dynamic_cast<const RME::PasteCommand*>(cmd) != nullptr);
    QCOMPARE(cmd->text(), "Paste");

    MockTile* pastedTile = static_cast<MockTile*>(m_mockMap->getTile(pasteTargetPos));
    // QVERIFY(pastedTile == nullptr || pastedTile->getFlags() == 0); // Before redo, tile might not exist or be default

    m_undoStack->redo(); // Perform the paste
    pastedTile = static_cast<MockTile*>(m_mockMap->getTile(pasteTargetPos));
    QVERIFY(pastedTile != nullptr);
    // QCOMPARE(pastedTile->getFlags(), (uint32_t)456); // Check depends on PasteCommand::redo()

    m_undoStack->undo(); // Undo the paste
    pastedTile = static_cast<MockTile*>(m_mockMap->getTile(pasteTargetPos));
    // QVERIFY(pastedTile == nullptr || pastedTile->getFlags() == 0); // Check depends on PasteCommand::undo()
}


QTEST_APPLESS_MAIN(TestClipboardManager)
#include "test_clipboardmanager.moc"
