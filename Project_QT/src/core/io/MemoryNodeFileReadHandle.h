#ifndef RME_MEMORY_NODE_FILE_READ_HANDLE_H
#define RME_MEMORY_NODE_FILE_READ_HANDLE_H

#include "core/io/NodeFileReadHandle.h"

namespace RME {
namespace core {
namespace io {

class MemoryNodeFileReadHandle : public NodeFileReadHandle {
public:
    MemoryNodeFileReadHandle(const uint8_t* data, size_t size, size_t initialPoolSize = 16);
    ~MemoryNodeFileReadHandle() override = default;

    void assign(const uint8_t* data, size_t size); // Re-initialize with new data

    size_t tell() const override;
    bool isEof() const override;

protected:
    bool ensureBytesAvailable(size_t bytes = 1) override;
    uint8_t readByteUnsafe() override;

private:
    const uint8_t* m_data;    // Non-owning pointer to the memory buffer
    size_t m_size;            // Total size of the buffer
    size_t m_currentPosition; // Current read position in m_data
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_MEMORY_NODE_FILE_READ_HANDLE_H
