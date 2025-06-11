#include <QtTest/QtTest>
#include <QByteArray>
#include <QString>
#include <vector>
#include <cstdint>

#include "core/io/BinaryNode.h"
#include "core/io/NodeFileReadHandle.h" // For mock or minimal instance
#include "core/io/MemoryNodeFileReadHandle.h" // Can be used as a concrete handle for tests

// Mock NodeFileReadHandle for basic BinaryNode navigation tests if needed,
// though most property tests don't require deep handle interaction.
class MockNodeFileReadHandle : public RME::core::io::NodeFileReadHandle {
public:
    MockNodeFileReadHandle() : RME::core::io::NodeFileReadHandle() {}
    ~MockNodeFileReadHandle() override = default;

    // Implement pure virtual methods
    size_t tell() const override { return 0; }
    bool isEof() const override { return true; }

protected:
    bool ensureBytesAvailable(size_t bytes = 1) override { Q_UNUSED(bytes); return false; }
    uint8_t readByteUnsafe() override { return 0; }
    // readNextNode is not pure virtual in the provided NodeFileReadHandle.h, but if it were, or for more control:
    // RME::core::io::BinaryNode* readNextNode(RME::core::io::BinaryNode* parentNode, RME::core::io::BinaryNode* previousSiblingNode = nullptr) override {
    //     Q_UNUSED(parentNode);
    //     Q_UNUSED(previousSiblingNode);
    //     return nullptr; // Default mock behavior
    // }
};

class TestBinaryNode : public QObject
{
    Q_OBJECT

public:
    TestBinaryNode();
    ~TestBinaryNode() override;

private slots:
    void init();
    void cleanup();

    void testConstruction();
    void testSetAndGetProperties();
    void testGetNumericTypes();
    void testGetStringTypes();
    void testGetBytes();
    void testSkipBytes();
    void testReadOffsetAndHasMore();
    void testBoundaryConditions();
    void testTypeAndNodeData();
    // Navigation methods (getChild, getNextChild, advance) are more complex to unit test in isolation
    // as they depend heavily on NodeFileReadHandle's state and stream parsing.
    // They are better tested implicitly via TestNodeFileHandle and TestOtbmMapIO.

private:
    MockNodeFileReadHandle* m_mockHandle; // For basic construction
    RME::core::io::BinaryNode* m_node;    // Node under test
};

TestBinaryNode::TestBinaryNode() : m_mockHandle(nullptr), m_node(nullptr) {}
TestBinaryNode::~TestBinaryNode() {
    // cleanup() will handle deletion
}

void TestBinaryNode::init() {
    m_mockHandle = new MockNodeFileReadHandle();
    m_node = new RME::core::io::BinaryNode(m_mockHandle, nullptr /* parent */);
}

void TestBinaryNode::cleanup() {
    delete m_node;
    m_node = nullptr;
    delete m_mockHandle;
    m_mockHandle = nullptr;
}

void TestBinaryNode::testConstruction() {
    QVERIFY(m_node != nullptr);
    QCOMPARE(m_node->getType(), static_cast<uint8_t>(0));
    QVERIFY(m_node->getPropertiesData().isEmpty());
    QVERIFY(m_node->getNodeData().isEmpty());
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0));
    QVERIFY(!m_node->hasMoreProperties());
}

void TestBinaryNode::testSetAndGetProperties() {
    QByteArray props("\x01\x02\x03\x04", 4);
    m_node->setProperties(props);
    QCOMPARE(m_node->getPropertiesData(), props);
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0)); // setProperties should reset offset
    QVERIFY(m_node->hasMoreProperties());

    m_node->setProperties(QByteArray()); // Set empty properties
    QVERIFY(m_node->getPropertiesData().isEmpty());
    QVERIFY(!m_node->hasMoreProperties());
}

void TestBinaryNode::testGetNumericTypes() {
    QByteArray props;
    QDataStream stream(&props, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    uint8_t u8_val = 0xAB;
    uint16_t u16_val = 0xABCD;
    uint32_t u32_val = 0xABCDEF01;
    uint64_t u64_val = 0x0123456789ABCDEFLL;

    stream << u8_val << u16_val << u32_val << u64_val;
    m_node->setProperties(props);

    uint8_t r_u8;
    uint16_t r_u16;
    uint32_t r_u32;
    uint64_t r_u64;

    QVERIFY(m_node->getU8(r_u8));
    QCOMPARE(r_u8, u8_val);
    QVERIFY(m_node->getU16(r_u16));
    QCOMPARE(r_u16, u16_val);
    QVERIFY(m_node->getU32(r_u32));
    QCOMPARE(r_u32, u32_val);
    QVERIFY(m_node->getU64(r_u64));
    QCOMPARE(r_u64, u64_val);

    QVERIFY(!m_node->hasMoreProperties());
}

void TestBinaryNode::testGetStringTypes() {
    QByteArray props;
    QDataStream stream(&props, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    QString qstr_val = "HelloQt";
    std::string std_str_val = "HelloStd";

    // QString
    QByteArray qstr_utf8 = qstr_val.toUtf8();
    stream << static_cast<quint16>(qstr_utf8.size());
    stream.writeRawData(qstr_utf8.constData(), qstr_utf8.size());

    // std::string
    stream << static_cast<quint16>(std_str_val.length());
    stream.writeRawData(std_str_val.data(), std_str_val.length());

m_node->setProperties(props);

    QString r_qstr;
    std::string r_std_str;

    QVERIFY(m_node->getString(r_qstr));
    QCOMPARE(r_qstr, qstr_val);

    QVERIFY(m_node->getString(r_std_str));
    QCOMPARE(r_std_str, std_str_val);

    QVERIFY(!m_node->hasMoreProperties());
}

void TestBinaryNode::testGetBytes() {
    unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    QByteArray props(reinterpret_cast<const char*>(data), sizeof(data));
    m_node->setProperties(props);

    // Test getBytes(uint8_t* buffer, size_t length)
    uint8_t buffer[4];
    QVERIFY(m_node->getBytes(buffer, 4));
    QCOMPARE(buffer[0], static_cast<uint8_t>(0xDE));
    QCOMPARE(buffer[1], static_cast<uint8_t>(0xAD));
    QCOMPARE(buffer[2], static_cast<uint8_t>(0xBE));
    QCOMPARE(buffer[3], static_cast<uint8_t>(0xEF));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(4));

    // Test getBytes(std::vector<uint8_t>& buffer, size_t length)
    std::vector<uint8_t> vec_buffer;
    QVERIFY(m_node->getBytes(vec_buffer, 2));
    QCOMPARE(vec_buffer.size(), static_cast<size_t>(2));
    QCOMPARE(vec_buffer[0], static_cast<uint8_t>(0xFE));
    QCOMPARE(vec_buffer[1], static_cast<uint8_t>(0xED));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(6));
    QVERIFY(!m_node->hasMoreProperties());
}

void TestBinaryNode::testSkipBytes() {
    QByteArray props("\x01\x02\x03\x04\x05\x06", 6);
    m_node->setProperties(props);

    QVERIFY(m_node->skipBytes(2));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(2));

    uint8_t val;
    QVERIFY(m_node->getU8(val));
    QCOMPARE(val, static_cast<uint8_t>(0x03));

    QVERIFY(m_node->skipBytes(10)); // Try to skip past end
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(props.size())); // Offset should be at end
    QVERIFY(!m_node->hasMoreProperties());
    QVERIFY(!m_node->getU8(val)); // Cannot read more
}

void TestBinaryNode::testReadOffsetAndHasMore() {
    QByteArray props("\x01\x02\x03", 3);
    m_node->setProperties(props);

    QVERIFY(m_node->hasMoreProperties());
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0));

    uint8_t val;
    m_node->getU8(val);
    QVERIFY(m_node->hasMoreProperties());
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(1));

    m_node->getU8(val);
    m_node->getU8(val);
    QVERIFY(!m_node->hasMoreProperties());
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(3));

    m_node->resetReadOffset();
    QVERIFY(m_node->hasMoreProperties());
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0));
}

void TestBinaryNode::testBoundaryConditions() {
    // Test reading exactly at the end
    QByteArray props_u16("\x01\x02", 2);
    m_node->setProperties(props_u16);
    uint16_t r_u16;
    QVERIFY(m_node->getU16(r_u16));
    QVERIFY(!m_node->hasMoreProperties());

    // Test reading past the end (numeric)
    m_node->setProperties(QByteArray("\x01", 1)); // Only 1 byte
    QVERIFY(!m_node->getU16(r_u16)); // Try to read 2 bytes
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0)); // Offset should not change on failed read

    // Test reading string past end (length ok, data not)
    QByteArray props_str_bad_data;
    QDataStream stream_sbd(&props_str_bad_data, QIODevice::WriteOnly);
    stream_sbd.setByteOrder(QDataStream::LittleEndian);
    stream_sbd << static_cast<quint16>(5); // Length 5
    props_str_bad_data.append("Hi", 2); // Data only 2 bytes
    m_node->setProperties(props_str_bad_data);
    QString r_qstr;
    QVERIFY(!m_node->getString(r_qstr));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0)); // Offset should revert to before reading length

    // Test reading string past end (length itself is partial)
    m_node->setProperties(QByteArray("\x01", 1)); // Length prefix is 2 bytes, only 1 available
    QVERIFY(!m_node->getString(r_qstr));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0));

    // Test getBytes past end
    uint8_t buffer[4];
    m_node->setProperties(QByteArray("\x01\x02", 2));
    QVERIFY(!m_node->getBytes(buffer, 4));
    QCOMPARE(m_node->getReadOffset(), static_cast<qsizetype>(0));
}

void TestBinaryNode::testTypeAndNodeData() {
    uint8_t type_val = 0xEE;
    QByteArray node_data_val("\xCA\xFE", 2);

    m_node->setType(type_val);
    QCOMPARE(m_node->getType(), type_val);

    m_node->setNodeData(node_data_val);
    QCOMPARE(m_node->getNodeData(), node_data_val);
}


// QTEST_MAIN(TestBinaryNode) // Will be run by rme_core_io_tests
#include "TestBinaryNode.moc" // Must be last line for MOC to work
