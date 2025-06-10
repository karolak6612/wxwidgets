#ifndef RME_MEMORY_NODE_FILE_WRITE_HANDLE_H
#define RME_MEMORY_NODE_FILE_WRITE_HANDLE_H

#include "core/io/NodeFileWriteHandle.h"

namespace RME {
namespace core {
namespace io {

class MemoryNodeFileWriteHandle : public NodeFileWriteHandle {
public:
    MemoryNodeFileWriteHandle(size_t initialBufferSize = 8192);
    ~MemoryNodeFileWriteHandle() override = default;

    // Get the written data
    const std::vector<uint8_t>&getBuffer() const { return m_buffer; }
    const uint8_t* getData() const { return m_buffer.data(); }
    size_t getSize() const { return m_buffer.size(); }

    void clear() override;

protected:
    void writeEscapedBytes(const uint8_t* data, size_t length) override;

private:
    std::vector<uint8_t> m_buffer;
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_MEMORY_NODE_FILE_WRITE_HANDLE_H
