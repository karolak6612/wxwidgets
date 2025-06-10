#include "core/io/BinaryNode.h"
#include "core/io/NodeFileReadHandle.h" // For m_fileHandle interaction
#include "core/io/otbm_constants.h"   // For NODE_START, NODE_END, ESCAPE_CHAR
#include <cstring> // For memcpy
#include <stdexcept> // For potential exceptions

namespace RME {
namespace core {
namespace io {

BinaryNode::BinaryNode(NodeFileReadHandle* handle, BinaryNode* parentNode) :
    m_readOffset(0),
    m_fileHandle(handle),
    m_parent(parentNode),
    m_child(nullptr) {
    // Properties are loaded by calling loadProperties() externally by NodeFileReadHandle
}

BinaryNode::~BinaryNode() {
    // The original used a custom memory pool via m_fileHandle->freeNode(m_child).
    // If m_fileHandle is responsible for freeing children, this destructor might be empty
    // or cooperate with the handle. For now, assume m_fileHandle manages this.
    if (m_child) {
        m_fileHandle->recycleNode(m_child); // Tell handle to recycle child
        m_child = nullptr;
    }
}

template <typename T>
bool BinaryNode::readType(T& value) {
    if (m_readOffset + sizeof(T) > m_properties.size()) {
        // Attempted to read past the end of the properties buffer
        return false;
    }
    memcpy(&value, m_properties.data() + m_readOffset, sizeof(T));
    // TODO: Handle endianness if required. Assuming little-endian for now.
    m_readOffset += sizeof(T);
    return true;
}

bool BinaryNode::getU8(uint8_t& value) {
    return readType(value);
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
    if (m_readOffset + bytesToSkip > m_properties.size()) {
        m_readOffset = m_properties.size(); // Move to end
        return false; // Skipped past end
    }
    m_readOffset += bytesToSkip;
    return true;
}

bool BinaryNode::getBytes(uint8_t* buffer, size_t length) {
    if (m_readOffset + length > m_properties.size()) {
        return false; // Not enough data
    }
    memcpy(buffer, m_properties.data() + m_readOffset, length);
    m_readOffset += length;
    return true;
}

bool BinaryNode::getBytes(std::vector<uint8_t>& buffer, size_t length) {
    if (m_readOffset + length > m_properties.size()) {
        return false; // Not enough data
    }
    buffer.resize(length);
    memcpy(buffer.data(), m_properties.data() + m_readOffset, length);
    m_readOffset += length;
    return true;
}

bool BinaryNode::getString(std::string& value) {
    uint16_t len;
    if (!getU16(len)) {
        return false;
    }
    if (m_readOffset + len > m_properties.size()) {
        // Not enough data for the string content, revert read of length
        m_readOffset -= sizeof(uint16_t);
        return false;
    }
    value.assign(reinterpret_cast<const char*>(m_properties.data() + m_readOffset), len);
    m_readOffset += len;
    return true;
}

// void BinaryNode::loadProperties() - Implementation moved to NodeFileReadHandle::readNodeProperties
// as it needs access to the file handle's internal buffer and escape logic.


BinaryNode* BinaryNode::getChild() {
    if (!m_fileHandle) return nullptr; // Should not happen

    // If a child already exists and was processed, advance it first to see if it has siblings.
    // This logic is tricky. The original 'getChild' was simpler and assumed it was called once.
    // Let's stick to: getChild gets the *first* child. getNextChild navigates siblings.
    if (m_child) { // If we already have a child, this is not asking for the first one.
        // This might indicate a logic error in usage, or getChild is being called repeatedly.
        // For now, return existing child if any.
        // To get next sibling of m_child, one would call m_child->advance() or m_fileHandle->getNextNode(m_child).
        return m_child; // Or perhaps null if it's meant to be "get *next* unread child"
    }

    // Ask the file handle to parse the next node, which should be our child.
    m_child = m_fileHandle->readNextNode(this); // Pass 'this' as parent
    return m_child;
}

BinaryNode* BinaryNode::getNextChild() {
    // This method is intended to be called on the PARENT node
    // to get the SIBLING of the current m_child.
    if (!m_fileHandle) return nullptr;

    if (!m_child) { // No current child, so get the first one.
        return getChild();
    }

    // We have a current child. Ask the file handle to advance IT to its next sibling.
    // The file handle will manage recycling the old m_child if it's done.
    m_child = m_fileHandle->readNextNode(this, m_child); // Pass current child to get its sibling
    return m_child;
}


BinaryNode* BinaryNode::advance() {
    // This advances the CURRENT node to its NEXT SIBLING.
    // This is effectively what getNextChild does for a parent.
    // The parent node should call this on its child.
    if (!m_parent) return nullptr; // Root node cannot advance this way
    return m_parent->getNextChild(); // Ask parent to get my next sibling
}


// Original BinaryNode::load() logic from wxwidgets/filehandle.cpp:
// void BinaryNode::load() {
// 	ASSERT(file);
// 	// Read until next node starts
// 	uint8_t*& cache = file->cache; // This was NodeFileReadHandle's cache
// 	size_t& cache_length = file->cache_length;
// 	size_t& local_read_index = file->local_read_index;
// 	while (true) {
// 		if (local_read_index >= cache_length) {
// 			if (!file->renewCache()) {
// 				// Failed to renew, exit
// 				file->error_code = FILE_PREMATURE_END;
// 				return;
// 			}
// 		}
// 		uint8_t op = cache[local_read_index];
// 		++local_read_index;
// 		switch (op) {
// 			case NODE_START: { file->last_was_start = true; return; }
// 			case NODE_END: { file->last_was_start = false; return; }
// 			case ESCAPE_CHAR: { /* ... escape logic ... */ break; }
// 			default: break;
// 		}
// 		data.append(1, op); // Original used std::string data
// 	}
// }
// This logic is now primarily handled within NodeFileReadHandle::readNodeProperties.

} // namespace io
} // namespace core
} // namespace RME
