#ifndef RME_DISK_NODE_FILE_WRITE_HANDLE_H
#define RME_DISK_NODE_FILE_WRITE_HANDLE_H

#include "core/io/NodeFileWriteHandle.h"
#include <QFile>
#include <QDataStream>
#include <QString>

namespace RME {
namespace core {
namespace io {

/**
 * @brief Implements NodeFileWriteHandle for writing to a disk file.
 *
 * This class uses QFile and QDataStream to perform the actual file I/O operations,
 * providing a concrete implementation for serializing OTBM data to disk.
 * It handles opening the file for writing (truncating if it exists), writing data,
 * and closing the file. The base class NodeFileWriteHandle manages the node
 * structure serialization logic.
 */
class DiskNodeFileWriteHandle : public NodeFileWriteHandle {
public:
    /**
     * @brief Constructs a DiskNodeFileWriteHandle.
     *
     * Attempts to open the specified file for writing. If the file exists, it is
     * truncated. If the file cannot be opened, an error state is set internally.
     * @param filePath The path to the OTBM file to be written.
     */
    explicit DiskNodeFileWriteHandle(const QString& filePath);

    /**
     * @brief Destroys the DiskNodeFileWriteHandle.
     *
     * Ensures that any buffered data is flushed and the file is closed if it was opened.
     */
    ~DiskNodeFileWriteHandle() override;

    // Delete copy constructor and assignment operator to prevent copying
    DiskNodeFileWriteHandle(const DiskNodeFileWriteHandle&) = delete;
    DiskNodeFileWriteHandle& operator=(const DiskNodeFileWriteHandle&) = delete;

    /**
     * @brief Clears the handle's state, if applicable.
     *
     * For a disk file opened in truncate mode, this typically has no significant effect
     * beyond ensuring the error state of the handle itself is reset if it was in one
     * (though a new handle should be used for a fresh operation if a persistent error occurred).
     * The underlying file is already prepared (cleared) by QIODevice::Truncate on open.
     */
    void clear() override;

    /**
     * @brief Flushes any buffered data from the QDataStream to the disk file.
     * @return True if flushing was successful or not needed, false on error.
     */
    bool flush();

protected:
    /**
     * @brief Writes a sequence of bytes to the output stream, applying OTBM escaping.
     * @param data Pointer to the byte array.
     * @param length Number of bytes to write.
     */
    void writeEscapedBytesInternal(const char* data, qsizetype length) override;

    /**
     * @brief Writes a sequence of raw bytes to the output stream (no escaping).
     * @param data Pointer to the byte array.
     * @param length Number of bytes to write.
     */
    void writeRawBytesInternal(const char* data, qsizetype length) override;

private:
    QFile m_file;         ///< The QFile object representing the disk file.
    QDataStream m_stream; ///< The QDataStream used for writing to the file.
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_DISK_NODE_FILE_WRITE_HANDLE_H
