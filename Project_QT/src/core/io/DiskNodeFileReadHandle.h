#ifndef RME_DISK_NODE_FILE_READ_HANDLE_H
#define RME_DISK_NODE_FILE_READ_HANDLE_H

#include "core/io/NodeFileReadHandle.h"
#include <QFile>
#include <QDataStream>
#include <QString>

namespace RME {
namespace core {
namespace io {

/**
 * @brief Implements NodeFileReadHandle for reading from a disk file.
 *
 * This class uses QFile and QDataStream to perform the actual file I/O operations,
 * providing a concrete implementation for accessing OTBM data stored on disk.
 * It handles opening, reading, and closing the file, while the base class
 * NodeFileReadHandle manages the node parsing logic.
 */
class DiskNodeFileReadHandle : public NodeFileReadHandle {
public:
    /**
     * @brief Constructs a DiskNodeFileReadHandle.
     *
     * Attempts to open the specified file for reading. If the file cannot be
     * opened, an error state is set internally.
     * @param filePath The path to the OTBM file to be read.
     */
    explicit DiskNodeFileReadHandle(const QString& filePath);

    /**
     * @brief Destroys the DiskNodeFileReadHandle.
     *
     * Ensures that the file is closed if it was opened.
     */
    ~DiskNodeFileReadHandle() override;

    // Delete copy constructor and assignment operator to prevent copying
    DiskNodeFileReadHandle(const DiskNodeFileReadHandle&) = delete;
    DiskNodeFileReadHandle& operator=(const DiskNodeFileReadHandle&) = delete;

    // Implementation of virtual methods from NodeFileReadHandle
    /**
     * @brief Gets the current read position in the file.
     * @return The current position in bytes from the beginning of the file.
     *         Returns 0 if the file is not open or an error has occurred.
     */
    size_t tell() const override;

    /**
     * @brief Checks if the end of the file has been reached.
     * @return True if EOF is reached or if the file is not open, false otherwise.
     */
    bool isEof() const override;

protected:
    /**
     * @brief Ensures that the specified number of bytes are available to be read from the stream.
     *
     * This method checks if reading the requested number of bytes would go past
     * the end of the file. If not enough bytes are available, an error state is set.
     * @param bytes The number of bytes to ensure are available. Defaults to 1.
     * @return True if the requested bytes are available, false otherwise.
     */
    bool ensureBytesAvailable(size_t bytes = 1) override;

    /**
     * @brief Reads a single byte from the file stream without checking for EOF or errors.
     *
     * This method is called by the base class after `ensureBytesAvailable` has
     * confirmed that data can be read. If a read error occurs (e.g. unexpected EOF),
     * an error state is set.
     * @return The byte read from the stream. Returns 0 if an error occurs.
     */
    uint8_t readByteUnsafe() override;

private:
    QFile m_file;         ///< The QFile object representing the disk file.
    QDataStream m_stream; ///< The QDataStream used for reading from the file.
    // qint64 m_fileSize; // Could be used to proactively check EOF if m_file.atEnd() or m_stream.atEnd() is not sufficient.
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_DISK_NODE_FILE_READ_HANDLE_H
