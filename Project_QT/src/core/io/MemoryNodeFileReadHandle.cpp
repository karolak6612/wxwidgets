#include "core/io/MemoryNodeFileReadHandle.h"
#include "core/io/otbm_constants.h" // For error codes
#include <QDebug>                   // For qWarning, if necessary

namespace RME {
namespace core {
namespace io {

MemoryNodeFileReadHandle::MemoryNodeFileReadHandle(const uint8_t* data, size_t size, size_t initialPoolSize)
    : NodeFileReadHandle(initialPoolSize),
      m_data(data),
      m_size(size),
      m_currentPosition(0)
{
    if (!m_data && m_size > 0) {
        // This is an invalid state: null data pointer but positive size.
        m_error = RME_OTBM_IO_ERROR_DATA_CORRUPTED; // Or a more specific bad_argument error
        qWarning() << "MemoryNodeFileReadHandle: Constructed with null data pointer but non-zero size.";
        m_size = 0; // Prevent any read attempts
    }
    // Unlike DiskNodeFileReadHandle, MemoryNodeFileReadHandle does not typically handle
    // a 4-byte file identifier itself. It's assumed the provided memory buffer
    // (m_data) starts directly with the OTBM node stream (i.e., first NODE_START).
    // If the buffer came from a file, the identifier should have been stripped prior.
}

void MemoryNodeFileReadHandle::assign(const uint8_t* data, size_t size) {
    m_data = data;
    m_size = size;
    m_currentPosition = 0;
    m_error = RME_OTBM_IO_NO_ERROR; // Reset error state

    // Reset base class state as well, if necessary (e.g. m_rootNode, m_lastByteWasStart)
    // This is important if the handle is reused for entirely new data.
    // The base class destructor handles m_nodePool if the handle object is destroyed.
    // If simply re-assigning data to an existing handle, we need to manage existing parsed state.
    if (m_rootNode) {
        // If we are truly reusing, the old node tree is invalid.
        // The nodes are owned by m_nodePool (vector of unique_ptr).
        // Clearing the pool will delete them.
        m_nodePool.clear();
        while(!m_recycledNodes.empty()) m_recycledNodes.pop();
        m_rootNode = nullptr;
    }
    m_lastByteWasStart = false; // Reset parsing state
}

size_t MemoryNodeFileReadHandle::tell() const {
    return m_currentPosition;
}

bool MemoryNodeFileReadHandle::isEof() const {
    if (m_error != RME_OTBM_IO_NO_ERROR) {
        return true; // Treat error as EOF
    }
    return m_currentPosition >= m_size;
}

bool MemoryNodeFileReadHandle::ensureBytesAvailable(size_t bytes) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (bytes == 0) return true;

    if (m_currentPosition + bytes > m_size) {
        // Only set error if it's an *unexpected* EOF during an operation that requires these bytes.
        // A simple check for EOF is handled by isEof().
        // If an operation *requires* N bytes and they aren't there, then it's an error.
        // For now, NodeFileReadHandle's logic will set RME_OTBM_IO_ERROR_UNEXPECTED_EOF if needed.
        // This method just reports availability.
        return false;
    }
    return true;
}

uint8_t MemoryNodeFileReadHandle::readByteUnsafe() {
    // This method assumes ensureBytesAvailable(1) was called and returned true,
    // and m_error is RME_OTBM_IO_NO_ERROR.
    // No further checks here for performance, as per "Unsafe" name.
    return m_data[m_currentPosition++];
}

} // namespace io
} // namespace core
} // namespace RME
