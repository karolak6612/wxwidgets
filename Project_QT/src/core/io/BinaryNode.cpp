#include "core/io/BinaryNode.h"
#include "core/io/NodeFileReadHandle.h" // For m_fileHandle interaction and node fetching
#include "core/io/otbm_constants.h"   // For error constants if needed, though not directly used here

#include <cstring>   // For memcpy
#include <stdexcept> // For potential exceptions (though not used in this basic version)
#include <QString>   // For getString(QString&)
#include <QtEndian>  // For qFromLittleEndian
#include <QDebug>    // For warnings or errors if necessary

namespace RME {
namespace core {
namespace io {

BinaryNode::BinaryNode(NodeFileReadHandle* handle, BinaryNode* parentNode) :
    m_type(0),
    m_readOffset(0),
    m_fileHandle(handle),
    m_parent(parentNode),
    m_child(nullptr) // Child is lazily fetched via getChild()
{
    if (!m_fileHandle) {
        // This should ideally not happen if NodeFileReadHandle manages creation.
        // Consider throwing an exception or setting an error state.
        qWarning("BinaryNode created with null file handle.");
    }
}

BinaryNode::~BinaryNode() {
    // As per the plan, NodeFileReadHandle is responsible for the lifecycle of nodes it creates.
    // If m_child was fetched, it's a raw pointer to a node owned by NodeFileReadHandle's pool.
    // BinaryNode itself does not delete m_child directly here.
    // The original wxWidgets BinaryNode called file->freeNode(child).
    // In the Qt6 version, if NodeFileReadHandle uses unique_ptrs in its pool, this is handled by the pool.
    // If manual recycling is implemented in NodeFileReadHandle, then:
    // if (m_child && m_fileHandle) {
    //     m_fileHandle->recycleNode(m_child); // Or a more complex recursive recycle if children can have children
    // }
    // For now, assuming NodeFileReadHandle manages child lifecycle through its pool mechanism.
    m_child = nullptr; // Avoid dangling pointer just in case.
}

void BinaryNode::setProperties(const QByteArray& propsData) {
    m_properties = propsData;
    resetReadOffset();
}

void BinaryNode::resetReadOffset() {
    m_readOffset = 0;
}

bool BinaryNode::hasMoreProperties() const {
    return m_readOffset < m_properties.size();
}

// Internal template helper for reading little-endian multi-byte types
template <typename T>
bool BinaryNode::readType(T& value) {
    if (m_readOffset + static_cast<qsizetype>(sizeof(T)) > m_properties.size()) {
        // Not enough data for this type
        return false;
    }
    // QByteArray::constData() returns const char*. Need to reinterpret_cast to const uchar* for qFromLittleEndian.
    value = qFromLittleEndian<T>(reinterpret_cast<const uchar*>(m_properties.constData() + m_readOffset));
    m_readOffset += sizeof(T);
    return true;
}

bool BinaryNode::getU8(uint8_t& value) {
    if (m_readOffset + static_cast<qsizetype>(sizeof(uint8_t)) > m_properties.size()) {
        return false;
    }
    // QByteArray::at() returns char, so cast to uint8_t.
    value = static_cast<uint8_t>(m_properties.at(m_readOffset));
    m_readOffset += sizeof(uint8_t);
    return true;
}

bool BinaryNode::getU16(uint16_t& value) {
    return readType(value);
}

bool BinaryNode::getU32(uint32_t& value) {
    return readType(value);
}

bool BinaryNode::getU64(uint64_t& value) {
    return readType(value);
}

bool BinaryNode::skipBytes(size_t bytesToSkip) {
    if (m_readOffset + static_cast<qsizetype>(bytesToSkip) > m_properties.size()) {
        m_readOffset = m_properties.size(); // Move to end if skipping too much
        return false;
    }
    m_readOffset += bytesToSkip;
    return true;
}

bool BinaryNode::getBytes(uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return false; // Nothing to do or invalid buffer
    if (m_readOffset + static_cast<qsizetype>(length) > m_properties.size()) {
        return false; // Not enough data
    }
    memcpy(buffer, m_properties.constData() + m_readOffset, length);
    m_readOffset += length;
    return true;
}

// Overload for std::vector<uint8_t>
bool BinaryNode::getBytes(std::vector<uint8_t>& buffer, size_t length) {
    if (length == 0) {
        buffer.clear();
        return true;
    }
    if (m_readOffset + static_cast<qsizetype>(length) > m_properties.size()) {
        return false; // Not enough data
    }
    buffer.resize(length);
    memcpy(buffer.data(), m_properties.constData() + m_readOffset, length);
    m_readOffset += length;
    return true;
}

bool BinaryNode::getString(std::string& value) {
    quint16 len;
    qsizetype originalOffset = m_readOffset;
    if (!getU16(len)) { // getU16 handles endianness and advances m_readOffset
        return false;
    }
    if (m_readOffset + static_cast<qsizetype>(len) > m_properties.size()) {
        m_readOffset = originalOffset; // Restore offset if string data is incomplete
        return false;
    }
    value.assign(m_properties.constData() + m_readOffset, static_cast<size_t>(len));
    m_readOffset += len;
    return true;
}

bool BinaryNode::getString(QString& value) {
    quint16 len;
    qsizetype originalOffset = m_readOffset;
    if (!getU16(len)) { // getU16 handles endianness and advances m_readOffset
        return false;
    }
    if (m_readOffset + static_cast<qsizetype>(len) > m_properties.size()) {
        m_readOffset = originalOffset; // Restore offset if string data is incomplete
        return false;
    }
    // QString::fromUtf8 is suitable for OTBM strings which are typically UTF-8 or compatible.
    value = QString::fromUtf8(m_properties.constData() + m_readOffset, static_cast<int>(len));
    m_readOffset += len;
    return true;
}

BinaryNode* BinaryNode::getChild() {
    if (!m_fileHandle) {
        qWarning("BinaryNode::getChild() called with null file handle.");
        return nullptr;
    }
    // If a child has already been fetched, return it.
    // To get subsequent children (siblings of the first child), one should call getNextChild() on this parent node,
    // or advance() on the child node itself.
    if (m_child) {
        return m_child;
    }

    // Ask the file handle to parse the next node, which should be our first child.
    // The NodeFileReadHandle::readNextNode method is responsible for advancing its internal stream pointer
    // and creating/returning the child node.
    m_child = m_fileHandle->readNextNode(this /* parentNode */, nullptr /* previousSiblingNode */);
    return m_child;
}

BinaryNode* BinaryNode::getNextChild() {
    if (!m_fileHandle) {
        qWarning("BinaryNode::getNextChild() called with null file handle.");
        return nullptr;
    }
    if (!m_child) {
        // No current child, so getChild() would fetch the first one.
        // This method is for getting the *next* sibling of the *current* m_child.
        // If there's no m_child, it implies getChild() hasn't been called or returned null.
        // So, effectively, we try to get the first child again.
        return getChild();
    }

    // We have a current child (m_child). We need to ask the file handle to get its next sibling.
    // The file handle will manage recycling/updating the old m_child if necessary.
    m_child = m_fileHandle->readNextNode(this /* parentNode */, m_child /* previousSiblingNode */);
    return m_child;
}

BinaryNode* BinaryNode::advance() {
    if (!m_parent) {
        // Root node cannot advance to a sibling via its (non-existent) parent.
        return nullptr;
    }
    if (!m_fileHandle) {
        qWarning("BinaryNode::advance() called with null file handle on a non-root node.");
        return nullptr;
    }

    // Ask our parent to get the next sibling relative to us.
    // This is tricky because m_parent->m_child might not be `this` if multiple children were processed.
    // The NodeFileReadHandle::readNextNode(parent, previousSibling) is the key.
    // When parent->getNextChild() is called, if 'this' was the 'm_child' of parent,
    // then parent->m_child will be updated to our next sibling.
    // So, we ask our parent to update ITS m_child to our next sibling.
    return m_parent->getNextChild(); // This will effectively make m_parent->m_child point to our sibling.
}

// Explicit template instantiation if needed, though usually not required for member templates in .cpp
// template bool BinaryNode::readType<uint16_t>(uint16_t&);
// template bool BinaryNode::readType<uint32_t>(uint32_t&);
// template bool BinaryNode::readType<uint64_t>(uint64_t&);

} // namespace io
} // namespace core
} // namespace RME
