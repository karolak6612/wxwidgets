#include "core/io/MemoryNodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END, ESCAPE_CHAR

namespace RME {
namespace core {
namespace io {

MemoryNodeFileWriteHandle::MemoryNodeFileWriteHandle(size_t initialBufferSize)
    : NodeFileWriteHandle(initialBufferSize) {
    m_buffer.reserve(initialBufferSize);
}

void MemoryNodeFileWriteHandle::clear() {
    m_buffer.clear();
    m_error = 0;
}

void MemoryNodeFileWriteHandle::writeEscapedBytes(const uint8_t* data, size_t length) {
    if (m_error || !data) return;

    m_buffer.reserve(m_buffer.size() + length + length / 4); // Pre-allocate, crude guess for escapes

    for (size_t i = 0; i < length; ++i) {
        const uint8_t byte = data[i];
        if (byte == NODE_START || byte == NODE_END || byte == ESCAPE_CHAR) {
            m_buffer.push_back(ESCAPE_CHAR);
            if (m_buffer.capacity() == m_buffer.size()) {
                // This check is imperfect as vector might grow more.
                // std::vector handles reallocation, but frequent small reallocs can be slow.
                // The reserve above helps.
            }
        }
        m_buffer.push_back(byte);
    }
    // Unlike original that used fixed cache and renewCache, std::vector handles memory.
    // If write error were possible with vector (e.g. alloc failure), m_error should be set.
    // std::vector throws std::bad_alloc on failure.
}

} // namespace io
} // namespace core
} // namespace RME
