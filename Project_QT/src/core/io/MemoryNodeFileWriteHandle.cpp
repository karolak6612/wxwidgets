#include "core/io/MemoryNodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For error codes and special byte values
#include <QDebug>                   // For qWarning, if necessary

// Note: The header currently uses std::vector<uint8_t> m_buffer.
// The methods to override are writeEscapedBytesInternal and writeRawBytesInternal.

namespace RME {
namespace core {
namespace io {

MemoryNodeFileWriteHandle::MemoryNodeFileWriteHandle(size_t initialBufferSize)
    : NodeFileWriteHandle() // Call base constructor
{
    m_buffer.reserve(initialBufferSize);
    // Unlike DiskNodeFileWriteHandle, MemoryNodeFileWriteHandle does not write
    // a 4-byte file identifier. It's assumed the raw node stream is desired.
    // If an identifier is needed, it should be prepended to the buffer manually by the user.
}

// getBuffer(), getData(), getSize() are already provided by the header.

void MemoryNodeFileWriteHandle::clear() {
    m_buffer.clear();
    m_error = RME_OTBM_IO_NO_ERROR; // Reset error state in the base class
    // Base class's m_attributeBuffer is handled by its addNode/endNode logic.
}

// Protected method from NodeFileWriteHandle to be implemented
void MemoryNodeFileWriteHandle::writeEscapedBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    if (!data && length > 0) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX; // Or a more specific bad_argument error
        qWarning("MemoryNodeFileWriteHandle::writeEscapedBytesInternal: Null data pointer with non-zero length.");
        return;
    }

    m_buffer.reserve(m_buffer.size() + length + length / 4); // Pre-allocate, crude guess for escapes

    for (qsizetype i = 0; i < length; ++i) {
        uint8_t byte = static_cast<uint8_t>(data[i]); // Work with uint8_t for comparison with constants
        bool needsEscape = (byte == NODE_START || byte == NODE_END || byte == ESCAPE_CHAR);

        if (needsEscape) {
            m_buffer.push_back(static_cast<uint8_t>(ESCAPE_CHAR));
        }
        m_buffer.push_back(byte);
    }
    // std::vector operations (push_back) can throw std::bad_alloc on failure.
    // For simplicity, not explicitly catching it here; it would propagate.
    // If m_error needs to be set for allocation failure, try-catch would be needed.
}

// Protected method from NodeFileWriteHandle to be implemented
void MemoryNodeFileWriteHandle::writeRawBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    if (!data && length > 0) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX;
        qWarning("MemoryNodeFileWriteHandle::writeRawBytesInternal: Null data pointer with non-zero length.");
        return;
    }
    if (length == 0) return;

    // Append raw data to the vector
    const uint8_t* u_data = reinterpret_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), u_data, u_data + length);
    // Similar to above, std::vector::insert can throw std::bad_alloc.
}

// This method was in the header but is not part of the NodeFileWriteHandle interface to override.
// It seems like a duplicate or an older version of writeEscapedBytesInternal.
// Keeping it commented out or removing it from the header would be best.
/*
void MemoryNodeFileWriteHandle::writeEscapedBytes(const uint8_t* data, size_t length) {
    if (m_error || !data) return;

    m_buffer.reserve(m_buffer.size() + length + length / 4);

    for (size_t i = 0; i < length; ++i) {
        const uint8_t byte = data[i];
        if (byte == NODE_START || byte == NODE_END || byte == ESCAPE_CHAR) {
            m_buffer.push_back(ESCAPE_CHAR);
        }
        m_buffer.push_back(byte);
    }
}
*/

} // namespace io
} // namespace core
} // namespace RME
