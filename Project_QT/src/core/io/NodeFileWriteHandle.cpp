#include "core/io/NodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END, ESCAPE_CHAR, flags, errors
#include <QtEndian>  // For qToLittleEndian
#include <QtZlib/qtzlib.h> // For qCompress
#include <QDebug>    // For warnings if necessary

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
    // NODE_START itself is a special marker. The original wxWidgets FileHandle::writeBytes
    // would escape it if it matched NODE_START, NODE_END, or ESCAPE_CHAR.
    // So, we use writeEscapedBytesInternal for consistency with that potential interpretation.
    writeEscapedBytesInternal(&node_start_char, 1);
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    writeRawByteUnsafe(nodeType); // Node type is raw (not escaped)
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

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
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("NodeFileWriteHandle::addNodeData: Attempted to add data outside a node.");
        return false;
    }
    // Node data (like item ID, tile coordinates) is typically written raw, not escaped.
    writeRawBytesInternal(data.constData(), data.size());
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::endNode() {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (m_nodeLevel == 0) { // Mismatched endNode
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("NodeFileWriteHandle::endNode: Mismatched endNode call.");
        return false;
    }

    if (m_currentNodeCompressProps) {
        if (m_attributeBuffer.isEmpty()) {
            writeU32RawUnsafe(0); // Compressed length
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
            writeU32RawUnsafe(0); // Decompressed length
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
        } else {
            QByteArray compressedAttributes = qCompress(m_attributeBuffer, -1); // Default Qt Zlib compression level
            if (compressedAttributes.isEmpty() && !m_attributeBuffer.isEmpty()) {
                m_error = RME_OTBM_IO_ERROR_DECOMPRESSION; // Using this for compression error too
                qWarning("NodeFileWriteHandle::endNode: qCompress failed.");
                return false;
            }
            writeU32RawUnsafe(static_cast<quint32>(compressedAttributes.size()));
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
            writeU32RawUnsafe(static_cast<quint32>(m_attributeBuffer.size()));
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
            if (!compressedAttributes.isEmpty()) {
                 writeRawBytesInternal(compressedAttributes.constData(), compressedAttributes.size());
                 if (m_error != RME_OTBM_IO_NO_ERROR) return false;
            }
        }
    } else {
        // For uncompressed properties, the OTBM format expects the raw properties data
        // to be written directly, followed by a NODE_END marker.
        // The properties themselves are escaped.
        // The length of the properties is NOT written before the data in this case for uncompressed.
        // The original wxWidgets RME code for uncompressed properties:
        //   writeByte(NODE_START); writeByte(type); write_properties(); writeByte(NODE_END);
        // And `write_properties` would call `write_attributes_otbm` which does:
        //   for each attribute: writeByte(key); write_attribute_value(value);
        //   (attribute values like strings have their own length prefixes)
        // Then, crucially, the uncompressed properties stream is terminated by NODE_END.
        // The `writeEscapedBytesInternal` function handles escaping.

        // The current structure with m_attributeBuffer accumulates all properties.
        // This buffer needs to be written escaped.
        if (!m_attributeBuffer.isEmpty()) {
            writeEscapedBytesInternal(m_attributeBuffer.constData(), m_attributeBuffer.size());
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
        }
    }
    m_attributeBuffer.clear();

    char node_end_char = static_cast<char>(NODE_END);
    // NODE_END itself is a special marker and should be escaped if it were part of general data.
    // However, as a structural token, it's written raw by some OTBM parsers/writers.
    // Given addNode writes NODE_START escaped, for symmetry and caution:
    writeEscapedBytesInternal(&node_end_char, 1);
    // If strict OTBM requires raw NODE_END, this would be writeRawByteUnsafe(NODE_END);

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
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addU32(quint32 value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addU64(quint64 value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    appendToAttributeBuffer(value);
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addString(const std::string& value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (value.length() > 0xFFFF) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("NodeFileWriteHandle::addString: std::string too long for U16 length prefix.");
        return false;
    }
    addU16(static_cast<quint16>(value.length()));
    if (m_error == RME_OTBM_IO_NO_ERROR && !value.empty()) {
        m_attributeBuffer.append(value.data(), static_cast<int>(value.length()));
    }
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addString(const QString& value) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    QByteArray utf8 = value.toUtf8();
    if (utf8.length() > 0xFFFF) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("NodeFileWriteHandle::addString: QString (UTF-8) too long for U16 length prefix.");
        return false;
    }
    addU16(static_cast<quint16>(utf8.length()));
    if (m_error == RME_OTBM_IO_NO_ERROR && !utf8.isEmpty()) {
        m_attributeBuffer.append(utf8);
    }
    return m_error == RME_OTBM_IO_NO_ERROR;
}

bool NodeFileWriteHandle::addBytes(const uint8_t* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (!data && length > 0) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("NodeFileWriteHandle::addBytes: Null data pointer with non-zero length.");
        return false;
    }
    if (length > 0) {
        m_attributeBuffer.append(reinterpret_cast<const char*>(data), static_cast<int>(length));
    }
    return m_error == RME_OTBM_IO_NO_ERROR;
}


// --- Internal Helpers for direct raw writing (used by addNode, endNode for structure) ---
void NodeFileWriteHandle::writeRawByteUnsafe(uint8_t byte) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    writeRawBytesInternal(reinterpret_cast<const char*>(&byte), 1);
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
