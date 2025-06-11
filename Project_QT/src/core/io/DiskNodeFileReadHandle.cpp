#include "core/io/DiskNodeFileReadHandle.h"
// #include "core/io/otbm_constants.h" // Base class NodeFileReadHandle includes this if needed for its parsing logic

#include <QDebug> // For potential logging

namespace RME {
namespace core {
namespace io {

/**
 * @brief Constructs a DiskNodeFileReadHandle.
 * @param filePath The path to the OTBM file to be read.
 */
DiskNodeFileReadHandle::DiskNodeFileReadHandle(const QString& filePath) :
    NodeFileReadHandle(), m_file(filePath) {
    if (!m_file.open(QIODevice::ReadOnly)) {
        m_error = RME_OTBM_IO_ERROR_FILE_OPEN; // Use defined error code
        qWarning() << "DiskNodeFileReadHandle: Failed to open file:" << filePath << "Error:" << m_file.errorString();
        return;
    }
    m_stream.setDevice(&m_file);
    m_stream.setByteOrder(QDataStream::LittleEndian); // OTBM is typically little-endian

    // The base class constructor NodeFileReadHandle() or an explicit init method
    // in NodeFileReadHandle would typically call getRootNode() or similar,
    // which then uses ensureBytesAvailable and readByteUnsafe.
    // No explicit priming is needed here beyond setting up the stream.
}

/**
 * @brief Destroys the DiskNodeFileReadHandle.
 * Closes the file if it is open.
 */
DiskNodeFileReadHandle::~DiskNodeFileReadHandle() {
    if (m_file.isOpen()) {
        m_file.close();
    }
}

/**
 * @brief Gets the current read position in the file.
 * @return Current position in bytes, or 0 on error/closed file.
 */
size_t DiskNodeFileReadHandle::tell() const {
    if (!m_file.isOpen() || m_error != RME_OTBM_IO_NO_ERROR) {
        // It might be better to throw an exception or return a specific error value like static_cast<size_t>(-1)
        // if the design allows for it, rather than returning 0 which could be a valid position.
        // However, consistency with base class or typical stream behavior is also important.
        return 0;
    }
    return static_cast<size_t>(m_file.pos());
}

/**
 * @brief Checks if the end of the file has been reached.
 * @return True if EOF is reached or file is not open/error, false otherwise.
 */
bool DiskNodeFileReadHandle::isEof() const {
    if (!m_file.isOpen() || m_error != RME_OTBM_IO_NO_ERROR) {
        return true; // If there's an error or not open, treat as EOF
    }
    return m_stream.atEnd();
}

/**
 * @brief Ensures that the specified number of bytes are available to be read.
 * Sets an error flag if not enough bytes are available or if already in an error state.
 * @param bytes Number of bytes to check for availability.
 * @return True if bytes are available, false otherwise.
 */
bool DiskNodeFileReadHandle::ensureBytesAvailable(size_t bytes) {
    if (m_error != RME_OTBM_IO_NO_ERROR) {
        return false; // Already in an error state
    }
    if (!m_file.isOpen()) {
        m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN; // Should not happen if constructor succeeded
        return false;
    }

    // Check if trying to read 0 bytes, which is always "available" (no-op)
    if (bytes == 0) {
        return true;
    }

    // QDataStream's atEnd() is reliable. If it's true, no more data can be read.
    if (m_stream.atEnd()) {
        m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF; // Trying to read past EOF
        return false;
    }

    // For QDataStream, a more robust check for available bytes for a specific read
    // often involves attempting the read and checking QDataStream::status().
    // However, `bytesAvailable()` on the device can give some indication,
    // but it might not account for QDataStream's internal buffer.
    // A simple check: if (current_pos + requested_bytes > file_size) -> error
    // This check helps prevent QDataStream from trying to read past the physical end of file
    // if its internal buffer logic isn't perfectly aligned with our needs here.
    if ((static_cast<qint64>(m_file.pos()) + static_cast<qint64>(bytes)) > m_file.size()) {
        m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        return false;
    }

    return true; // Assume QDataStream will handle it, or readByteUnsafe will catch issues.
}

/**
 * @brief Reads a single byte from the stream.
 * Sets an error flag if the read fails.
 * @return The byte read. Returns 0 if an error occurred.
 */
uint8_t DiskNodeFileReadHandle::readByteUnsafe() {
    if (m_error != RME_OTBM_IO_NO_ERROR) { // Should not be called if already in error state
        return 0;
    }
     if (!m_file.isOpen() || m_stream.atEnd()) { // Double check, though ensureBytesAvailable should prevent this
        m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        return 0;
    }

    uint8_t byte;
    int bytesRead = m_stream.readRawData(reinterpret_cast<char*>(&byte), 1);

    if (bytesRead != 1) {
        // An error occurred during read (e.g., actual EOF reached, or other I/O error)
        m_error = RME_OTBM_IO_ERROR_READ_FAILED; // Or more specific based on m_stream.status()
        if (m_stream.status() == QDataStream::ReadPastEnd) {
             m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        }
        // qWarning() << "DiskNodeFileReadHandle: Failed to read byte. Stream status:" << m_stream.status();
        return 0; // Return dummy value
    }
    return byte;
}

} // namespace io
} // namespace core
} // namespace RME
