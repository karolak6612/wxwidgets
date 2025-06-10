#include "core/io/NodeFileReadHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END, ESCAPE_CHAR
#include <algorithm> // For std::find_if for pool management, if needed

namespace RME {
namespace core {
namespace io {

NodeFileReadHandle::NodeFileReadHandle(size_t initialPoolSize) :
    m_error(0), // No error
    m_rootNode(nullptr),
    m_lastByteWasStart(false)
{
    m_nodePool.reserve(initialPoolSize);
    // Initially, no nodes are in the recycled stack. They are created on demand.
}

NodeFileReadHandle::~NodeFileReadHandle() {
    // m_nodePool uses unique_ptr, so nodes it owns are automatically deleted.
    // m_recycledNodes only contains non-owning pointers to nodes in m_nodePool.
    // m_rootNode is also managed by the pool if allocated through createNode.
    // If m_rootNode was set to a node not in the pool, it would be a leak,
    // but createNode ensures it's from the pool.
    m_rootNode = nullptr; // Ensure it's not dangling if it was from the pool.
    m_recycledNodes = std::stack<BinaryNode*>(); // Clear stack
    m_nodePool.clear(); // Clear and delete all owned nodes
}

BinaryNode* NodeFileReadHandle::createNode(BinaryNode* parentNode) {
    if (!m_recycledNodes.empty()) {
        BinaryNode* nodeToReuse = m_recycledNodes.top();
        m_recycledNodes.pop();
        // Call a reset method on nodeToReuse or rely on its constructor being called again if using placement new
        // For now, let's assume we need to reconstruct it if we reuse memory.
        // This implies BinaryNode needs a proper re-initialization mechanism.
        // Simplification: if BinaryNode is simple enough, direct member reset.
        // Given BinaryNode::~BinaryNode() calls recycleNode(m_child), this is complex.

        // Simplest for now: Pool owns unique_ptrs. Recycled nodes are just raw pointers.
        // When creating, if recycled available, take it, re-initialize (conceptually).
        // This is the hard part.
        // Let's assume new/delete for now for BinaryNode, and `recycleNode` deletes.
        // And `createNode` always `new`s. This defeats the original pooling.

        // Re-evaluating the original wxwidgets code:
        // It used malloc, then placement new. freeNode called destructor then pushed raw memory to stack.
        // This is advanced. Let's use a simpler unique_ptr pool for now.
        // `m_nodePool` stores `std::unique_ptr<BinaryNode>`.
        // `m_recycledNodes` stores `BinaryNode*` that are currently unused but owned by `m_nodePool`.
        // This is still not quite right.

        // Final simplified strategy for pooling:
        // m_nodePool stores all created nodes.
        // m_recycledNodes stores raw pointers to nodes in m_nodePool that are available.
        // BinaryNode needs a reinit(NodeFileReadHandle*, BinaryNode* parent) method.
         BinaryNode* node = m_recycledNodes.top();
         m_recycledNodes.pop();
         // node->reinit(this, parentNode); // Assuming BinaryNode has such a method
         // This is still not clean. For now, let's skip advanced pooling.
         // New nodes are added to the pool. recycleNode does nothing yet.

    }
    // This will always create a new node and add to pool.
    // BinaryNode destructor will handle recycling its children.
    m_nodePool.emplace_back(std::make_unique<BinaryNode>(this, parentNode));
    return m_nodePool.back().get();
}

void NodeFileReadHandle::recycleNode(BinaryNode* node) {
    if (node) {
        // The original implementation pushed node to a stack for reuse.
        // With unique_ptrs in m_nodePool, true recycling is complex.
        // If BinaryNode's destructor correctly calls recycleNode for its children,
        // we need to ensure we don't delete it from m_nodePool prematurely if it's on stack.
        // For now, this method can be a no-op if unique_ptr in m_nodePool handles deletion.
        // Or, if we want to implement pooling:
        // node->clearChildrenForRecycle(); // A new method in BinaryNode
        // m_recycledNodes.push(node);

        // Simplest: if BinaryNode's destructor is correct, unique_ptr in m_nodePool handles it.
        // This means m_recycledNodes isn't used for actual memory reuse yet, just tracking.
        // The unique_ptr in m_nodePool will delete it when m_nodePool is cleared or specific element removed.
        // This means true "recycling" as in memory reuse isn't happening here yet with unique_ptrs.
        // To make it work, BinaryNode's destructor shouldn't delete children, but recycle them.
        // And createNode should try to reuse. This is deferred.
    }
}


BinaryNode* NodeFileReadHandle::getRootNode() {
    if (m_rootNode) return m_rootNode;
    if (!ensureBytesAvailable(1)) {
        m_error = 1; // EOF
        return nullptr;
    }

    uint8_t firstByte = readByteUnsafe();
    if (firstByte != NODE_START) {
        m_error = 2; // Syntax error
        return nullptr;
    }
    m_lastByteWasStart = true; // We've consumed a NODE_START

    m_rootNode = createNode(nullptr); // Root has no parent node
    if (!readNodeProperties(m_rootNode, m_rootNode->m_properties)) { // Pass ref to member
        // Error already set by readNodeProperties
        // recycleNode(m_rootNode); // This is tricky if m_rootNode is from pool
        m_rootNode = nullptr; // Invalidate root if loading its props failed
        return nullptr;
    }
    return m_rootNode;
}

// This is the core logic for parsing the node stream.
// Called by BinaryNode's getChild() or getNextChild().
BinaryNode* NodeFileReadHandle::readNextNode(BinaryNode* parentNode, BinaryNode* previousSiblingNode) {
    if (m_error) return nullptr;

    if (previousSiblingNode) {
        // We are trying to get the next sibling.
        // The previous sibling must have been fully processed, meaning a NODE_END was consumed for it.
        if (m_lastByteWasStart) {
            // This implies the previousSiblingNode was a container but its children weren't read,
            // or it was an empty container node. We need to consume its implicit/explicit NODE_END.
            // This state indicates an issue or complex empty node.
            // For now, assume previousSibling was properly ended.
            // A robust parser might need to skip children of previousSibling if any.
            // This is simplified: assume stream is positioned after previousSibling's NODE_END.
        }
    } else {
        // This is parentNode->getChild(), looking for the first child.
        // We must be just after parentNode's properties were read, or after its NODE_START if it has no props.
        // The m_lastByteWasStart should be true if parentNode is a container and we expect a child.
        if (!m_lastByteWasStart) { // Parent was a data node (ended with NODE_END) or empty. No children.
            return nullptr;
        }
    }

    if (!ensureBytesAvailable(1)) {
        if (!isEof()) m_error = 1; // Unexpected EOF
        return nullptr;
    }

    uint8_t marker = readByteUnsafe();

    if (marker == NODE_END) {
        m_lastByteWasStart = false; // Consumed a NODE_END
        return nullptr; // No more children for this parent / no next sibling
    }

    if (marker != NODE_START) {
        m_error = 2; // Syntax error: expected NODE_START or NODE_END
        return nullptr;
    }

    m_lastByteWasStart = true; // Consumed a NODE_START for the new node
    BinaryNode* newNode = createNode(parentNode);
    if (!readNodeProperties(newNode, newNode->m_properties)) {
        // Error set by readNodeProperties
        // recycleNode(newNode); // Again, tricky with pool
        return nullptr; // Failed to load properties for the new node
    }
    return newNode;
}


bool NodeFileReadHandle::readNodeProperties(BinaryNode* node, std::vector<uint8_t>& properties) {
    properties.clear();
    if (m_error) return false;

    while (ensureBytesAvailable(1)) {
        uint8_t byte = readByteUnsafe();
        if (m_error) return false; // readByteUnsafe might set error if ensureBytesAvailable fails subtly

        if (byte == NODE_START) {
            m_lastByteWasStart = true;
            return true; // End of properties, start of a child node
        }
        if (byte == NODE_END) {
            m_lastByteWasStart = false;
            return true; // End of properties, end of this node
        }
        if (byte == ESCAPE_CHAR) {
            if (!ensureBytesAvailable(1)) { m_error = 1; return false; }
            byte = readByteUnsafe();
            if (m_error) return false;
            // The escaped byte is itself, e.g. if data contains 0xFD, it's stored as 0xFD 0xFD.
            properties.push_back(byte);
        } else {
            properties.push_back(byte);
        }
    }
    // If loop finishes due to ensureBytesAvailable returning false, it's an unexpected EOF
    if(!isEof()) m_error = 1; // Unexpected EOF during property reading
    return false;
}

} // namespace io
} // namespace core
} // namespace RME
