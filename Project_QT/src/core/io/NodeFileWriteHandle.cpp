#include "core/io/NodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END
#include <cstring> // For strlen in addString(const char*)

namespace RME {
namespace core {
namespace io {

NodeFileWriteHandle::NodeFileWriteHandle(size_t initialBufferSize) : m_error(0) {
    // initialBufferSize is for concrete classes like MemoryNodeFileWriteHandle
    // Base class doesn't manage a buffer directly.
}

// Node structure
bool NodeFileWriteHandle::addNode(uint8_t nodeType) {
    if (m_error) return false;
    uint8_t startByte = NODE_START;
    // Write NODE_START and then the nodeType. These typically don't need escaping themselves
    // as they are protocol markers, but writeEscapedBytes should be used for property data.
    // However, the original writeBytes in wxwidgets/filehandle.h *did* escape everything.
    // Let's assume protocol markers are written raw, properties are escaped.
    // Re-checking original NodeFileWriteHandle::addNode - it just wrote to cache.
    // The writeBytes method it used *did* escape. So, markers also go through escaping.
    writeEscapedBytes(&startByte, 1);
    if (m_error) return false;
    return addU8(nodeType); // nodeType is a property, so use addU8 which uses escaping
}

bool NodeFileWriteHandle::endNode() {
    if (m_error) return false;
    uint8_t endByte = NODE_END;
    writeEscapedBytes(&endByte, 1); // Markers also go through escaping
    return m_error == 0;
}

// Property writing methods
bool NodeFileWriteHandle::addU8(uint8_t value) {
    if (m_error) return false;
    addRawType(value);
    return m_error == 0;
}

bool NodeFileWriteHandle::addU16(uint16_t value) {
    if (m_error) return false;
    // Assuming little-endian host, writing as such.
    // If specific network byte order (big-endian) is required, convert 'value' before this call.
    addRawType(value);
    return m_error == 0;
}

bool NodeFileWriteHandle::addU32(uint32_t value) {
    if (m_error) return false;
    addRawType(value);
    return m_error == 0;
}

bool NodeFileWriteHandle::addU64(uint64_t value) {
    if (m_error) return false;
    addRawType(value);
    return m_error == 0;
}

bool NodeFileWriteHandle::addString(const std::string& value) {
    if (m_error) return false;
    if (value.length() > 0xFFFF) {
        m_error = 1; // String too long for uint16_t prefix
        return false;
    }
    addU16(static_cast<uint16_t>(value.length()));
    if (m_error) return false; // Check if addU16 failed
    writeEscapedBytes(reinterpret_cast<const uint8_t*>(value.data()), value.length());
    return m_error == 0;
}

bool NodeFileWriteHandle::addString(const char* value) {
    if (m_error) return false;
    size_t len = std::strlen(value);
    if (len > 0xFFFF) {
        m_error = 1; // String too long
        return false;
    }
    addU16(static_cast<uint16_t>(len));
    if (m_error) return false;
    writeEscapedBytes(reinterpret_cast<const uint8_t*>(value), len);
    return m_error == 0;
}

bool NodeFileWriteHandle::addBytes(const uint8_t* data, size_t length) {
    if (m_error) return false;
    writeEscapedBytes(data, length);
    return m_error == 0;
}

// addRawType is a template in the header.
// writeEscapedBytes is pure virtual.

} // namespace io
} // namespace core
} // namespace RME
