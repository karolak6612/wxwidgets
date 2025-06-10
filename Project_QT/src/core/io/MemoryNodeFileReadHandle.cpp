#include "core/io/MemoryNodeFileReadHandle.h"

namespace RME {
namespace core {
namespace io {

MemoryNodeFileReadHandle::MemoryNodeFileReadHandle(const uint8_t* data, size_t size, size_t initialPoolSize)
    : NodeFileReadHandle(initialPoolSize), m_data(data), m_size(size), m_currentPosition(0) {
    if (!m_data && m_size > 0) {
        // Invalid arguments, data is null but size is positive
        m_error = 2; // Or a specific error code for bad arguments
        m_size = 0; // Prevent reading
    }
}

void MemoryNodeFileReadHandle::assign(const uint8_t* data, size_t size) {
    m_data = data;
    m_size = size;
    m_currentPosition = 0;
    m_error = 0;
    // m_rootNode should be reset, but getRootNode() handles this.
    // However, if we are re-assigning, any old root node from previous data is invalid.
    // This needs careful handling of m_rootNode and the node pool if we want to reuse the handle object.
    // For simplicity, let's assume assign is like a constructor for now.
    // The ~NodeFileReadHandle will clear the pool if this instance is destroyed.
    // If this instance is reused, m_rootNode needs to be explicitly reset.
    if (m_rootNode) {
        // This is tricky. If nodes from pool were used for old m_rootNode,
        // they should be recycled. But unique_ptrs in pool manage them.
        // Simplest: clear the pool and reset root.
        // This is handled by NodeFileReadHandle destructor if this object is destroyed and new one made.
        // If assign is meant for true reuse of the handle object:
        // delete m_rootNode; // If not using pool or if root is special
        // m_nodePool.clear(); // Deletes all nodes
        // while(!m_recycledNodes.empty()) m_recycledNodes.pop();
    }
     m_rootNode = nullptr; // Reset root node
     m_lastByteWasStart = false;

}


size_t MemoryNodeFileReadHandle::tell() const {
    return m_currentPosition;
}

bool MemoryNodeFileReadHandle::isEof() const {
    return m_currentPosition >= m_size;
}

bool MemoryNodeFileReadHandle::ensureBytesAvailable(size_t bytes) {
    if (m_error) return false;
    if (m_currentPosition + bytes > m_size) {
        // Don't set m_error for normal EOF, only if unexpected.
        // isEof() will be true.
        return false;
    }
    return true;
}

uint8_t MemoryNodeFileReadHandle::readByteUnsafe() {
    // Assumes ensureBytesAvailable(1) was true.
    return m_data[m_currentPosition++];
}

} // namespace io
} // namespace core
} // namespace RME
