#include "core/io/NodeFileReadHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END, ESCAPE_CHAR, flags, errors
#include <QByteArray>
#include <QtZlib/qtzlib.h> // For qUncompress
#include <QtEndian>    // For qFromLittleEndian
#include <QDebug>      // For warnings

namespace RME {
namespace core {
namespace io {

NodeFileReadHandle::NodeFileReadHandle(size_t initialPoolSize) :
    m_error(RME_OTBM_IO_NO_ERROR),
    m_rootNode(nullptr),
    m_lastByteWasStart(false) // Initial state: not expecting a node type immediately
{
    m_nodePool.reserve(initialPoolSize);
}

NodeFileReadHandle::~NodeFileReadHandle() {
    // m_nodePool uses unique_ptr, so nodes it owns are automatically deleted.
    // m_recycledNodes only contains non-owning pointers, ensure it's empty or nodes are managed.
    // m_rootNode is also managed by the pool if allocated through createNode.
    m_rootNode = nullptr; // Avoid dangling if it was from the pool.
    m_recycledNodes = std::stack<BinaryNode*>(); // Clear stack of raw pointers
    m_nodePool.clear(); // Clear and delete all owned BinaryNode objects
}

BinaryNode* NodeFileReadHandle::createNode(BinaryNode* parentNode) {
    // Simplified pooling: always create new, add to pool.
    // Advanced pooling with m_recycledNodes can be added later if performance dictates.
    m_nodePool.emplace_back(std::make_unique<BinaryNode>(this, parentNode));
    return m_nodePool.back().get();
}

void NodeFileReadHandle::recycleNode(BinaryNode* node) {
    // With unique_ptr in m_nodePool, explicit recycling for memory reuse is complex.
    // The unique_ptr destructor will eventually delete the node when it's removed from m_nodePool
    // (e.g., when NodeFileReadHandle is destroyed or if we implement explicit removal).
    // For now, this method can be a no-op or log if needed.
    // If m_recycledNodes stack were used for actual reuse, logic would go here.
    Q_UNUSED(node);
}

BinaryNode* NodeFileReadHandle::getRootNode() {
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;
    if (m_rootNode) return m_rootNode;

    if (!ensureBytesAvailable(1)) {
        m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        return nullptr;
    }

    uint8_t firstByte = readByteUnsafe();
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

    if (firstByte != NODE_START) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX; // Expected NODE_START for root
        qWarning() << "NodeFileReadHandle::getRootNode: Expected NODE_START, got" << Qt::hex << firstByte;
        return nullptr;
    }
    m_lastByteWasStart = true; // Consumed a NODE_START, expect node type next

    // The root node itself in OTBM doesn't have a type read from the stream here;
    // its content (the actual map data, etc.) starts as its children.
    // We create a placeholder root BinaryNode.
    m_rootNode = createNode(nullptr); // Root has no parent BinaryNode
    // m_rootNode->setType(OTBM_NODE_ROOT); // Type is implicitly root, or set by first child's type if format implies.
                                       // For OTBM, the first real node (MAP_DATA) will be its child.

    // The call to readNextNode will parse the first actual child of this conceptual root.
    return m_rootNode;
}

// Private helper to read a U16 from stream (assumes bytes are available)
uint16_t NodeFileReadHandle::readU16Unsafe() {
    uint8_t bytes[2];
    bytes[0] = readByteUnsafe();
    if (m_error != RME_OTBM_IO_NO_ERROR) return 0;
    bytes[1] = readByteUnsafe();
    if (m_error != RME_OTBM_IO_NO_ERROR) return 0;
    return qFromLittleEndian<quint16>(bytes);
}

// Private helper to read a U32 from stream (assumes bytes are available)
uint32_t NodeFileReadHandle::readU32Unsafe() {
    uint8_t bytes[4];
    for (int i = 0; i < 4; ++i) {
        bytes[i] = readByteUnsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return 0;
    }
    return qFromLittleEndian<quint32>(bytes);
}

// Reads an escaped stream of bytes until a NODE_START or NODE_END is encountered.
// The terminating marker is consumed, and m_lastByteWasStart is set accordingly.
bool NodeFileReadHandle::readEscapedStream(QByteArray& buffer) {
    buffer.clear();
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;

    forever {
        if (!ensureBytesAvailable(1)) {
            m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
            return false;
        }
        uint8_t byte = readByteUnsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return false;

        if (byte == NODE_START) {
            m_lastByteWasStart = true;
            return true;
        }
        if (byte == NODE_END) {
            m_lastByteWasStart = false;
            return true;
        }
        if (byte == ESCAPE_CHAR) {
            if (!ensureBytesAvailable(1)) {
                m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
                return false;
            }
            byte = readByteUnsafe(); // Read the escaped byte
            if (m_error != RME_OTBM_IO_NO_ERROR) return false;
            // Append the actual byte, not the escape char itself
        }
        buffer.append(static_cast<char>(byte));
    }
    // Should not be reached due to `forever` and return conditions.
    return false;
}

BinaryNode* NodeFileReadHandle::readNextNodeInternal(BinaryNode* parentNode) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

    uint8_t nodeMarker;
    if (m_lastByteWasStart) {
        // If last byte was NODE_START, we are at the beginning of a new node's content (type).
        // No marker to read here; proceed to read type.
    } else {
        // Expecting a marker (NODE_START for a new node, or NODE_END for end of children)
        if (!ensureBytesAvailable(1)) {
            if (isEof()) return nullptr; // Clean EOF, no more nodes
            m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
            return nullptr;
        }
        nodeMarker = readByteUnsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

        if (nodeMarker == NODE_END) {
            m_lastByteWasStart = false; // Consumed NODE_END
            return nullptr; // End of nodes for this parent
        }
        if (nodeMarker != NODE_START) {
            m_error = RME_OTBM_IO_ERROR_SYNTAX;
            qWarning() << "NodeFileReadHandle::readNextNodeInternal: Expected NODE_START or NODE_END, got" << Qt::hex << nodeMarker;
            return nullptr;
        }
        m_lastByteWasStart = true; // Consumed NODE_START, expect type next
    }

    // At this point, a NODE_START was effectively just processed (either in this call or prior).
    // Now, read the node's actual content: Type, Flags, Properties.

    // 1. Read Node Type (U8)
    if (!ensureBytesAvailable(1)) { m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF; return nullptr; }
    uint8_t nodeType = readByteUnsafe();
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

    BinaryNode* newNode = createNode(parentNode);
    newNode->setType(nodeType);

    // 2. Read Node Flags (U8) - This is specific to OTBM v2+ like formats.
    // The original wxOTBM code might have this integrated differently or for specific node types.
    // For a generic NodeFileReadHandle, this might be optional or part of properties.
    // Assuming OTBM v2+ structure for now based on rework proposal.
    if (!ensureBytesAvailable(1)) { m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF; return nullptr; }
    uint8_t nodeFlags = readByteUnsafe(); // Assuming flags always present after type
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

    // 3. Read Properties
    QByteArray propsData;
    if (nodeFlags & OTBM_FLAG_COMPRESSION) {
        if (!ensureBytesAvailable(8)) { // CompressedLen (U32) + DecompressedLen (U32)
            m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
            return nullptr;
        }
        uint32_t compressedLength = readU32Unsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;
        uint32_t decompressedLength = readU32Unsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

        if (compressedLength == 0 && decompressedLength == 0) {
            // Valid case: empty compressed properties
            propsData.clear();
        } else if (compressedLength > 0 && decompressedLength > 0) {
            if (!ensureBytesAvailable(compressedLength)) {
                m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
                return nullptr;
            }
            QByteArray compressedBuffer(compressedLength, Qt::Uninitialized);
            for (uint32_t i = 0; i < compressedLength; ++i) {
                compressedBuffer[i] = static_cast<char>(readByteUnsafe());
                if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;
            }
            propsData = qUncompress(compressedBuffer);
            if (propsData.isEmpty() && compressedLength > 0) { // qUncompress returns empty on error
                m_error = RME_OTBM_IO_ERROR_DECOMPRESSION;
                qWarning("NodeFileReadHandle: qUncompress failed.");
                return nullptr;
            }
            if (static_cast<uint32_t>(propsData.size()) != decompressedLength) {
                m_error = RME_OTBM_IO_ERROR_DECOMPRESSION; // Decompressed size mismatch
                qWarning() << "NodeFileReadHandle: Decompressed size mismatch. Expected:" << decompressedLength << "Got:" << propsData.size();
                return nullptr;
            }
        } else {
             m_error = RME_OTBM_IO_ERROR_SYNTAX; // Invalid combination of lengths
             qWarning() << "NodeFileReadHandle: Invalid compressed/decompressed length combination.";
             return nullptr;
        }
        // After compressed properties, the next byte is a marker (NODE_START or NODE_END)
        // This marker needs to be read to set m_lastByteWasStart correctly for the next iteration.
        if (!ensureBytesAvailable(1)) {
            m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
            return nullptr;
        }
        uint8_t postCompressionMarker = readByteUnsafe();
        if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;
        if (postCompressionMarker == NODE_START) m_lastByteWasStart = true;
        else if (postCompressionMarker == NODE_END) m_lastByteWasStart = false;
        else { m_error = RME_OTBM_IO_ERROR_SYNTAX; return nullptr; }

    } else {
        // Not compressed: properties are an escaped stream terminated by NODE_START or NODE_END.
        // readEscapedStream will consume the terminator and set m_lastByteWasStart.
        if (!readEscapedStream(propsData)) {
            // m_error is set by readEscapedStream on failure
            return nullptr;
        }
    }
    newNode->setProperties(propsData);

    // Node is fully read (type, flags, properties). The m_lastByteWasStart is now set based on
    // the terminator of the properties (if uncompressed) or the explicit marker after compressed data.
    // This state will be used by the next call to readNextNodeInternal or by parent's getChild loop.
    return newNode;
}


BinaryNode* NodeFileReadHandle::readNextNode(BinaryNode* parentNode, BinaryNode* previousSiblingNode) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return nullptr;

    // This method is the public interface called by BinaryNode's navigation methods.
    // It uses m_lastByteWasStart state correctly.
    // If previousSiblingNode is nullptr, we are trying to get the *first child* of parentNode.
    // If previousSiblingNode is not nullptr, we are trying to get the *next sibling* of previousSiblingNode.

    if (previousSiblingNode) {
        // Getting a sibling. The stream should be positioned after the previous sibling's NODE_END.
        // So, m_lastByteWasStart should be false if previous sibling ended correctly.
        // If m_lastByteWasStart is true, it implies the previous sibling's properties were
        // terminated by a NODE_START (meaning it expected a child, which is unusual for a sibling scan).
        // Or, the previous sibling was an empty node whose NODE_END was immediately followed by a new NODE_START.
        if (m_lastByteWasStart) {
            // This is a bit of a recovery/assumption. If a NODE_START was the last thing seen,
            // it means we are already at the beginning of the next node's content (type).
            // So, readNextNodeInternal can proceed directly to reading type.
        }
    } else {
        // Getting the first child of parentNode.
        // If parentNode is a new node whose properties were just read, m_lastByteWasStart
        // reflects whether its properties were terminated by NODE_START (has children) or NODE_END (no children).
        if (!m_lastByteWasStart) {
            // Parent node's properties ended with NODE_END, or there was an issue.
            // No children to read.
            return nullptr;
        }
    }
    return readNextNodeInternal(parentNode);
}

} // namespace io
} // namespace core
} // namespace RME
