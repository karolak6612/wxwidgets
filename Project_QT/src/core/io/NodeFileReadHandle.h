#ifndef RME_NODE_FILE_READ_HANDLE_H
#define RME_NODE_FILE_READ_HANDLE_H

#include "core/io/BinaryNode.h" // Needs full definition
#include <vector>
#include <stack>
#include <memory> // For std::unique_ptr for pool management
#include <cstdint>

namespace RME {
namespace core {
namespace io {

// Forward declare BinaryNode to break potential circular dependency if BinaryNode included NodeFileReadHandle.h
// class BinaryNode; // Not needed if BinaryNode.h only forward declares NodeFileReadHandle

class NodeFileReadHandle {
public:
    NodeFileReadHandle(size_t initialPoolSize = 16);
    virtual ~NodeFileReadHandle();

    NodeFileReadHandle(const NodeFileReadHandle&) = delete;
    NodeFileReadHandle& operator=(const NodeFileReadHandle&) = delete;

    BinaryNode* getRootNode(); // Gets the root node of the OTBM structure

    // For BinaryNode to interact with the stream and its siblings/children
    // Attempts to read the next node from the stream as a child of 'parentNode'.
    // If 'previousSibling' is provided, it means we are looking for the next sibling of that node.
    // The handle is responsible for managing the lifecycle of the returned BinaryNode.
    BinaryNode* readNextNode(BinaryNode* parentNode, BinaryNode* previousSibling = nullptr);

    // Called by BinaryNode's destructor or when a node is no longer needed
    void recycleNode(BinaryNode* node);

    // Called by BinaryNode to load its properties from the stream
    bool readNodeProperties(BinaryNode* node, std::vector<uint8_t>& properties);

    bool isOk() const { return m_error == 0; }
    int getError() const { return m_error; } // 0 for no error

    virtual size_t tell() const = 0; // Current read position in the underlying stream/buffer
    virtual bool isEof() const = 0;  // True if no more data can be read from source

protected:
    BinaryNode* createNode(BinaryNode* parentNode); // Gets a node from the pool or creates one

    // Abstract methods for stream interaction
    virtual bool ensureBytesAvailable(size_t bytes = 1) = 0; // Ensure at least 'bytes' are in cache or readable
    virtual uint8_t readByteUnsafe() = 0; // Reads a byte, assumes ensureBytesAvailable was called

    // Error codes (simple for now, can be an enum)
    // 0: No error
    // 1: EOF unexpected
    // 2: Syntax error (e.g. bad node sequence)
    int m_error;

private:
    std::vector<std::unique_ptr<BinaryNode>> m_nodePool; // Owns all nodes
    std::stack<BinaryNode*> m_recycledNodes;             // Pool of recycled nodes

    BinaryNode* m_rootNode;
    bool m_lastByteWasStart; // State for parsing node structure
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_NODE_FILE_READ_HANDLE_H
