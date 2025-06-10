#ifndef RME_NODE_FILE_WRITE_HANDLE_H
#define RME_NODE_FILE_WRITE_HANDLE_H

#include <vector>
#include <string>
#include <cstdint>

// otbm_constants.h should be included by .cpp or users if needed for node types
// For addNode(uint8_t nodeType)

namespace RME {
namespace core {
namespace io {

class NodeFileWriteHandle {
public:
    NodeFileWriteHandle(size_t initialBufferSize = 8192); // 8KB initial buffer
    virtual ~NodeFileWriteHandle() = default;

    NodeFileWriteHandle(const NodeFileWriteHandle&) = delete;
    NodeFileWriteHandle& operator=(const NodeFileWriteHandle&) = delete;

    // Node structure
    bool addNode(uint8_t nodeType); // Writes NODE_START then nodeType
    bool endNode();                 // Writes NODE_END

    // Property writing methods
    bool addU8(uint8_t value);
    bool addByte(uint8_t value) { return addU8(value); } // Alias
    bool addU16(uint16_t value); // Assumes little-endian host, writes as such
    bool addU32(uint32_t value);
    bool addU64(uint64_t value);
    bool addString(const std::string& value);    // Length (U16) prefixed
    bool addString(const char* value);           // Length (U16) prefixed
    // bool addLongString(const std::string& value); // Length (U32) prefixed (if needed)
    bool addBytes(const uint8_t* data, size_t length); // Writes raw bytes, handles escaping

    bool isOk() const { return m_error == 0; }
    int getError() const { return m_error; }

    virtual void clear() = 0; // Clears internal buffer and state

protected:
    // Internal method to write bytes, handling OTBM escaping.
    // This is the core method all addX methods will use for actual byte writing.
    virtual void writeEscapedBytes(const uint8_t* data, size_t length) = 0;

    // Helper to write simple types without needing external escaping logic before calling.
    template <typename T>
    void addRawType(const T& value) {
        writeEscapedBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
    }

    int m_error; // 0 for no error
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_NODE_FILE_WRITE_HANDLE_H
