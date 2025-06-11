#ifndef RME_NODE_FILE_WRITE_HANDLE_H
#define RME_NODE_FILE_WRITE_HANDLE_H

#include <string>
#include <cstdint>
#include <QByteArray> // For m_attributeBuffer
#include <QtGlobal>   // For quint16, quint32

// Forward include otbm_constants.h for NODE_START etc.
#include "core/io/otbm_constants.h"


namespace RME {
namespace core {
namespace io {

/**
 * @brief Abstract base class for writing OTBM node structures.
 *
 * This class defines the interface and high-level logic for serializing
 * an OTBM tree. It handles node delimiters (NODE_START, NODE_END), attribute
 * buffering, optional attribute compression, and delegates low-level byte
 * writing (escaped or raw) to derived classes.
 */
class NodeFileWriteHandle {
public:
    NodeFileWriteHandle();
    virtual ~NodeFileWriteHandle() = default;

    NodeFileWriteHandle(const NodeFileWriteHandle&) = delete;
    NodeFileWriteHandle& operator=(const NodeFileWriteHandle&) = delete;

    // --- Node Structure ---
    /**
     * @brief Starts a new node in the OTBM stream.
     * Writes NODE_START, the nodeType, and nodeFlags.
     * Prepares to buffer attributes for this node.
     * @param nodeType The type of the node (e.g., OTBM_NODE_TILE).
     * @param compressProperties If true, attributes for this node will be ZLib compressed.
     * @return True on success, false on error.
     */
    bool addNode(uint8_t nodeType, bool compressProperties = false);

    /**
     * @brief Writes raw (unescaped, uncompressed) data for the current node.
     * This is used for OTBM node data like Item ID, Tile coordinates, etc.,
     * which comes after the node type/flags but before attributes.
     * @param data The QByteArray containing the raw data to write.
     * @return True on success, false on error.
     */
    bool addNodeData(const QByteArray& data);

    /**
     * @brief Finalizes the current node.
     * Writes buffered attributes (optionally compressed), and then NODE_END.
     * @return True on success, false on error.
     */
    bool endNode();

    // --- Attribute Writing Methods (appends to internal m_attributeBuffer) ---
    bool addU8(uint8_t value);
    bool addByte(uint8_t value) { return addU8(value); } // Alias
    bool addU16(quint16 value); // Writes in little-endian.
    bool addU32(quint32 value); // Writes in little-endian.
    bool addU64(quint64 value); // Writes in little-endian.
    bool addString(const std::string& value);    // Length (U16) prefixed, then UTF-8 bytes.
    bool addString(const QString& value);        // Length (U16) prefixed, then UTF-8 bytes.
    bool addBytes(const uint8_t* data, qsizetype length); // Appends raw bytes to attribute buffer.

    bool isOk() const { return m_error == RME_OTBM_IO_NO_ERROR; }
    int getError() const { return m_error; }

    virtual void clear() = 0; // Clears internal buffer and state. Also resets error.

protected:
    // --- Low-level I/O (to be implemented by derived classes) ---
    /**
     * @brief Writes a sequence of bytes to the output stream, applying OTBM escaping.
     * Used for NODE_START, NODE_END, and uncompressed attribute streams.
     * @param data Pointer to the byte array.
     * @param length Number of bytes to write.
     */
    virtual void writeEscapedBytesInternal(const char* data, qsizetype length) = 0;

    /**
     * @brief Writes a sequence of raw bytes to the output stream (no escaping).
     * Used for node type, flags, lengths, compressed data, and node data.
     * @param data Pointer to the byte array.
     * @param length Number of bytes to write.
     */
    virtual void writeRawBytesInternal(const char* data, qsizetype length) = 0;


    // --- Internal Helpers ---
    void writeRawByteUnsafe(uint8_t byte);
    void writeRawDataUnsafe(const QByteArray& data);
    void writeU16RawUnsafe(quint16 value);
    void writeU32RawUnsafe(quint32 value);

    template <typename T>
    void appendToAttributeBuffer(const T& value); // For numeric types, ensures little-endian.

    int m_error;                      ///< Current error state. RME_OTBM_IO_NO_ERROR if OK.
    QByteArray m_attributeBuffer;     ///< Buffer for accumulating attributes of the current node.
    bool m_currentNodeCompressProps;  ///< Flag if current node's attributes should be compressed.
    int m_nodeLevel;                  ///< Current nesting level of nodes.
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_NODE_FILE_WRITE_HANDLE_H
