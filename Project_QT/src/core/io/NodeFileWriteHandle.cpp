#include "core/io/NodeFileWriteHandle.h"
#include "core/io/otbm_constants.h"
#include <cstring>   // For strlen
#include <QtEndian>  // For qToLittleEndian
#include <QDataStream> // To help with appending to QByteArray attributeBuffer
#include <QBuffer>     // To use QDataStream on QByteArray
#include <QtZlib/qtzlib.h> // For qCompress

namespace RME {
namespace core {
namespace io {

NodeFileWriteHandle::NodeFileWriteHandle() :
    m_error(RME_OTBM_IO_NO_ERROR),
    m_currentNodeCompressProps(false),
    m_nodeLevel(0)
{
}

// --- Node Structure ---
bool NodeFileWriteHandle::addNode(uint8_t nodeType, bool compressProperties) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    char node_start_char = static_cast<char>(NODE_START);
    writeEscapedBytesInternal(&node_start_char, 1); // NODE_START is escaped
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    writeRawByteUnsafe(nodeType); // Node type is raw
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    // OTBM v2+ has a flags byte after node type
    uint8_t flags = OTBM_FLAG_NONE;
    if (compressProperties) {
        flags |= OTBM_FLAG_COMPRESSION;
    }
    writeRawByteUnsafe(flags); // Flags are raw
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    m_attributeBuffer.clear();
    m_currentNodeCompressProps = compressProperties;
    m_nodeLevel++;
    return true;
}

bool NodeFileWriteHandle::addNodeData(const QByteArray& data) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (m_nodeLevel == 0) { // Cannot add node data outside a node
        m_error = RME_OTBM_IO_ERROR_SYNTAX; return false;
    }
    writeRawDataUnsafe(data); // Node data is raw
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::endNode() {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (m_nodeLevel == 0) { // Mismatched endNode
        m_error = RME_OTBM_IO_ERROR_SYNTAX; return false;
    }

    if (m_currentNodeCompressProps) {
        QByteArray compressedAttributes = qCompress(m_attributeBuffer, -1); // Default compression level
        writeU32RawUnsafe(static_cast<quint32>(compressedAttributes.size())); // Compressed length
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;
        writeU32RawUnsafe(static_cast<quint32>(m_attributeBuffer.size()));   // Decompressed length
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;
        writeRawDataUnsafe(compressedAttributes); // Compressed data is raw
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    } else {
        // Uncompressed: U16 length, then escaped attribute data
        quint16 propsLength = static_cast<quint16>(m_attributeBuffer.size());
        writeU16RawUnsafe(propsLength); // Length is raw
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;
        writeEscapedBytesInternal(m_attributeBuffer.constData(), propsLength); // Attribute data is escaped
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    }
    m_attributeBuffer.clear();

    char node_end_char = static_cast<char>(NODE_END);
    writeEscapedBytesInternal(&node_end_char, 1); // NODE_END is escaped
    m_nodeLevel--;
    return m_error == RME_OTBM_IO_NO_ERROR;
}

// --- Attribute Writing Methods (appends to internal m_attributeBuffer) ---
template <typename T>
void NodeFileWriteHandle::appendToAttributeBuffer(const T& value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    T littleEndianValue = qToLittleEndian(value);
    m_attributeBuffer.append(reinterpret_cast<const char*>(&littleEndianValue), sizeof(T));
}

bool NodeFileWriteHandle::addU8(uint8_t value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    m_attributeBuffer.append(static_cast<char>(value));
    return true;
}

bool NodeFileWriteHandle::addU16(quint16 value) {
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addU32(quint32 value) {
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addU64(quint64 value) {
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addString(const std::string& value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (value.length() > 0xFFFF) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX; // String too long
        return false;
    }
    addU16(static_cast<quint16>(value.length()));
    m_attributeBuffer.append(value.data(), static_cast<int>(value.length()));
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addString(const QString& value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    QByteArray utf8 = value.toUtf8();
    if (utf8.length() > 0xFFFF) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX; // String too long
        return false;
    }
    addU16(static_cast<quint16>(utf8.length()));
    m_attributeBuffer.append(utf8);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addBytes(const uint8_t* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    m_attributeBuffer.append(reinterpret_cast<const char*>(data), static_cast<int>(length));
    return m_error == RME_OTBM_IO_NO_ERROR;
}


// --- Internal Helpers ---
void NodeFileWriteHandle::writeRawByteUnsafe(uint8_t byte) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    writeRawBytesInternal(reinterpret_cast<const char*>(&byte), 1);
}

void NodeFileWriteHandle::writeRawDataUnsafe(const QByteArray& data) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    writeRawBytesInternal(data.constData(), data.size());
}

void NodeFileWriteHandle::writeU16RawUnsafe(quint16 value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    quint16 le_val = qToLittleEndian(value);
    writeRawBytesInternal(reinterpret_cast<const char*>(&le_val), sizeof(le_val));
}

void NodeFileWriteHandle::writeU32RawUnsafe(quint32 value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    quint32 le_val = qToLittleEndian(value);
    writeRawBytesInternal(reinterpret_cast<const char*>(&le_val), sizeof(le_val));
}

} // namespace io
} // namespace core
} // namespace RME
