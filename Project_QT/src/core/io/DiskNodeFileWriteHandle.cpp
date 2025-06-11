#include "core/io/DiskNodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For NODE_START, NODE_END, ESCAPE_CHAR

#include <QDebug> // For potential logging

namespace RME {
namespace core {
namespace io {

/**
 * @brief Constructs a DiskNodeFileWriteHandle.
 * @param filePath The path to the OTBM file to be written.
 */
DiskNodeFileWriteHandle::DiskNodeFileWriteHandle(const QString& filePath) :
    NodeFileWriteHandle(), m_file(filePath) {
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_error = RME_OTBM_IO_ERROR_FILE_OPEN_WRITE; // Use defined error code
        qWarning() << "DiskNodeFileWriteHandle: Failed to open file for writing:" << filePath << "Error:" << m_file.errorString();
        return;
    }
    m_stream.setDevice(&m_file);
    m_stream.setByteOrder(QDataStream::LittleEndian); // OTBM is typically little-endian
}

/**
 * @brief Destroys the DiskNodeFileWriteHandle.
 * Flushes data and closes the file if it is open.
 */
DiskNodeFileWriteHandle::~DiskNodeFileWriteHandle() {
    if (m_file.isOpen()) {
        // QDataStream doesn't have its own flush. Flushing the device is correct.
        if (m_error == RME_OTBM_IO_NO_ERROR) { // Only attempt flush if no prior critical error prevented writes
             m_file.flush(); // Best effort flush
        }
        m_file.close();
    }
}

/**
 * @brief Clears the handle's state.
 * For disk files opened with Truncate, this mainly resets error state.
 */
void DiskNodeFileWriteHandle::clear() {
    // If NodeFileWriteHandle::clear() was intended to reset the buffer (like m_buffer in MemoryNodeFileWriteHandle),
    // this class doesn't have such an explicit buffer; QDataStream and QFile handle buffering.
    // If an error occurred, the handle is likely in an unusable state for further writes
    // without closing and reopening the file.
    // Resetting m_error here might be risky if the stream/file is still in a bad state.
    // A robust clear might involve re-opening the file with Truncate, but that's complex for a clear().
    // For now, if an error has occurred, this clear does not attempt to fix the stream.
    // It primarily fulfills the interface, and base class clear() might reset its own state.
    if (m_error != RME_OTBM_IO_NO_ERROR) {
        // Log or indicate that an error had occurred.
        // qWarning() << "DiskNodeFileWriteHandle: clear() called on a handle with an existing error.";
        return; // Don't try to operate on a bad stream.
    }
    if (!m_file.isOpen()) {
         m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN;
         return;
    }
    // The file was opened with QIODevice::Truncate, so it's "clear" from the start.
    // If we need to allow writing to the same file object after a "clear" mid-operation,
    // we might need to m_file.seek(0) and m_file.resize(0).
    // However, the base class `NodeFileWriteHandle::clear()` primarily clears its own buffer.
    // This implementation will just ensure the error state is consistent.
    // No specific action needed for QDataStream on a Truncated file beyond error checks.
}

/**
 * @brief Flushes buffered data to the disk file.
 * @return True if successful or no error, false on error or if file not open.
 */
bool DiskNodeFileWriteHandle::flush() {
    if (m_error != RME_OTBM_IO_NO_ERROR || !m_file.isOpen()) {
        return false;
    }
    if (!m_file.flush()) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED; // Or a specific flush error
        // qWarning() << "DiskNodeFileWriteHandle: Failed to flush file:" << m_file.errorString();
        return false;
    }
    return true;
}

/**
 * @brief Writes a sequence of bytes to the file stream, escaping special OTBM characters.
 * @param data Pointer to the byte array.
 * @param length Number of bytes to write.
 */
void DiskNodeFileWriteHandle::writeEscapedBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR || !m_file.isOpen()) {
        return;
    }

    for (qsizetype i = 0; i < length; ++i) {
        char byte = data[i];
        // Note: OTBM_ESCAPE_CHAR is uint8_t, direct comparison with char might be problematic
        // if char is signed and its value is > 127. Best to cast byte to uint8_t for comparison.
        uint8_t ubyte = static_cast<uint8_t>(byte);
        bool needsEscape = (ubyte == OTBM_NODE_START || ubyte == OTBM_NODE_END || ubyte == OTBM_ESCAPE_CHAR);

        if (needsEscape) {
            char escapeChar = static_cast<char>(OTBM_ESCAPE_CHAR);
            if (m_stream.writeRawData(&escapeChar, 1) != 1) {
                m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
                return;
            }
        }
        if (m_stream.writeRawData(&byte, 1) != 1) {
            m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
            return;
        }
    }
    if (m_stream.status() != QDataStream::Ok) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
    }
}

/**
 * @brief Writes a sequence of raw bytes to the output stream (no escaping).
 * @param data Pointer to the byte array.
 * @param length Number of bytes to write.
 */
void DiskNodeFileWriteHandle::writeRawBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR || !m_file.isOpen()) {
        return;
    }
    if (m_stream.writeRawData(data, static_cast<int>(length)) != static_cast<int>(length)) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
    }
    // QDataStream status check might be redundant if writeRawData returns correct length or -1 on error.
    // However, it's safer.
    if (m_stream.status() != QDataStream::Ok && m_error == RME_OTBM_IO_NO_ERROR) {
         m_error = RME_OTBM_IO_ERROR_WRITE_FAILED; // Catch if writeRawData didn't set error but stream is bad
    }
}

} // namespace io
} // namespace core
} // namespace RME
