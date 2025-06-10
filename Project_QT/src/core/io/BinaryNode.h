#ifndef RME_BINARY_NODE_H
#define RME_BINARY_NODE_H

#include <vector>
#include <string> // For getString, getLongString return types, and potentially internal use if not fully vector<uint8_t>
#include <cstdint>
#include <memory> // Required if BinaryNode manages children with unique_ptr, though original uses raw with custom pool

// Forward declaration
namespace RME {
namespace core {
namespace io {
class NodeFileReadHandle; // Forward declare the abstract base class
} // namespace io
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace io {

class BinaryNode {
public:
    // Constructor takes a pointer to its parent NodeFileReadHandle and its parent BinaryNode (if any)
    BinaryNode(NodeFileReadHandle* handle, BinaryNode* parentNode);
    ~BinaryNode();

    // Prevent copying and assignment, as it owns resources (like m_child)
    // and has complex relationships that are not trivially copyable.
    BinaryNode(const BinaryNode&) = delete;
    BinaryNode& operator=(const BinaryNode&) = delete;
    BinaryNode(BinaryNode&&) = delete; // Consider if move semantics are needed/safe
    BinaryNode& operator=(BinaryNode&&) = delete;


    bool getU8(uint8_t& value);
    bool getByte(uint8_t& value) { return getU8(value); } // Alias
    bool getU16(uint16_t& value);
    bool getU32(uint32_t& value);
    bool getU64(uint64_t& value);

    bool skipBytes(size_t bytesToSkip);

    bool getBytes(uint8_t* buffer, size_t length);
    bool getBytes(std::vector<uint8_t>& buffer, size_t length);
    bool getString(std::string& value); // Reads a string prefixed by uint16_t length
    // bool getLongString(std::string& value); // Reads a string prefixed by uint32_t length (if needed)

    BinaryNode* getChild(); // Gets the first child of this node
    BinaryNode* getNextChild(); // Advances to the next sibling of the current child (or gets first if no current child)
    BinaryNode* advance();    // Advances this node to its next sibling

    const std::vector<uint8_t>& getPropertiesData() const { return m_properties; } // Access to raw properties data

private:
    template <typename T>
    bool readType(T& value);

    std::vector<uint8_t> m_properties; // Stores the properties (data) of this node
    size_t m_readOffset;               // Current read offset within m_properties

    NodeFileReadHandle* m_fileHandle; // Non-owning pointer to the file handle that created it
    BinaryNode* m_parent;             // Non-owning pointer to the parent node
    BinaryNode* m_child;              // Owning pointer to the first child node (managed by custom pool in original)
                                      // For now, let's assume m_fileHandle handles actual allocation/deallocation
                                      // and m_child is just a pointer to a node owned by m_fileHandle's pool.

    friend class NodeFileReadHandle; // To allow NodeFileReadHandle to call loadProperties and manage child nodes
    friend class MemoryNodeFileReadHandle;
    friend class DiskNodeFileReadHandle; // If Disk version is also ported
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_BINARY_NODE_H
