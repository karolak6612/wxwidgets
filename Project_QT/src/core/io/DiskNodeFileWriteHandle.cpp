#include "core/io/DiskNodeFileWriteHandle.h"
#include "core/io/otbm_constants.h" // For error codes and special byte values
#include <QDebug>                   // For qWarning
#include <QtEndian>                 // For qToLittleEndian

namespace RME {
namespace core {
namespace io {

DiskNodeFileWriteHandle::DiskNodeFileWriteHandle(const QString& filePath)
    : NodeFileWriteHandle() // Call base constructor
    , m_file(filePath)
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_error = RME_OTBM_IO_ERROR_FILE_OPEN_WRITE;
        qWarning() << "DiskNodeFileWriteHandle: Failed to open file for writing:" << filePath << "Error:" << m_file.errorString();
        return;
    }

    m_stream.setDevice(&m_file);
    m_stream.setByteOrder(QDataStream::LittleEndian);

    // Write a default 4-byte OTBM identifier (e.g., 0x00000000 or 'OTBM')
    // wxDiskNodeFileWriteHandle took an identifier. We'll use a common default.
    // A common practice is four zero bytes, or the characters 'O','T','B','M'.
    // Let's use four zero bytes as a neutral default for now.
    quint32 defaultIdentifier = 0x00000000;
    char identifierBytes[4];
    qToLittleEndian(defaultIdentifier, reinterpret_cast<uchar*>(identifierBytes));

    if (m_stream.writeRawData(identifierBytes, 4) != 4) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
        qWarning() << "DiskNodeFileWriteHandle: Failed to write OTBM identifier to:" << filePath << "Stream status:" << m_stream.status();
        m_file.close(); // Attempt to close the file on error
        return;
    }
}

DiskNodeFileWriteHandle::~DiskNodeFileWriteHandle() {
    if (m_file.isOpen()) {
        if (m_error == RME_OTBM_IO_NO_ERROR) {
            // Only attempt flush if no prior critical error likely made the stream unusable
            // QDataStream doesn't have its own flush. Flushing the device is correct.
            if (!m_file.flush()) {
                 // Set error, but proceed to close. This error might not be catchable by user easily at destructor time.
                 // qWarning() << "DiskNodeFileWriteHandle: Failed to flush file on close:" << m_file.fileName() << "Error:" << m_file.errorString();
                 // If m_error is already set, don't overwrite it with a flush error unless it's more critical.
                 if(m_error == RME_OTBM_IO_NO_ERROR) m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
            }
        }
        m_file.close();
    }
}

void DiskNodeFileWriteHandle::clear() {
    // The base class NodeFileWriteHandle::m_attributeBuffer should be cleared by its own logic if necessary.
    // For DiskNodeFileWriteHandle, if an error occurred, the stream is likely bad.
    // Re-opening with Truncate would be a full reset but is too complex for clear().
    // If the file is still open and no error, clear primarily resets our error state.
    // The file was opened with QIODevice::Truncate, so it's "clear" from the start of its life.
    if (m_file.isOpen() && m_error != RME_OTBM_IO_NO_ERROR) {
        // If an error occurred, the handle might be in an inconsistent state.
        // Resetting error without addressing the cause could be problematic.
        // However, the interface requires a clear() method.
        // qWarning() << "DiskNodeFileWriteHandle::clear() called on a handle with an existing error state.";
    }
    m_error = RME_OTBM_IO_NO_ERROR; // Reset error state. User must ensure stream is still valid.
    // The base NodeFileWriteHandle should also have its clear() called if it has state (e.g. m_attributeBuffer).
    // This will be handled if a user calls (BaseClass*)this->clear(), or if NodeFileWriteHandle::clear() is not pure virtual and called from here.
    // Since it IS pure virtual, derived class must implement. We don't call base's version directly.
    // The crucial part is m_attributeBuffer in base class, which is cleared by NodeFileWriteHandle::addNode and endNode.
}

bool DiskNodeFileWriteHandle::flush() {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (!m_file.isOpen()) {
        m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN;
        return false;
    }
    if (!m_file.flush()) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
        qWarning() << "DiskNodeFileWriteHandle: Failed to flush file:" << m_file.fileName() << "Error:" << m_file.errorString();
        return false;
    }
    return true;
}

void DiskNodeFileWriteHandle::writeEscapedBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    if (!m_file.isOpen()) { m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN; return; }

    for (qsizetype i = 0; i < length; ++i) {
        char byte = data[i];
        uint8_t ubyte = static_cast<uint8_t>(byte);
        bool needsEscape = (ubyte == NODE_START || ubyte == NODE_END || ubyte == ESCAPE_CHAR);

        if (needsEscape) {
            char escapeChar = static_cast<char>(ESCAPE_CHAR);
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
    // Check stream status after loop for robustness, though individual write failures should set m_error.
    if (m_stream.status() != QDataStream::Ok) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
    }
}

void DiskNodeFileWriteHandle::writeRawBytesInternal(const char* data, qsizetype length) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return;
    if (!m_file.isOpen()) { m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN; return; }
    if (length == 0) return;

    if (m_stream.writeRawData(data, static_cast<int>(length)) != static_cast<int>(length)) {
        m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
    }
    // QDataStream status check might be redundant if writeRawData returns correct length or -1 on error.
    // However, it's safer for conditions where writeRawData might not reflect underlying stream errors immediately.
    if (m_stream.status() != QDataStream::Ok && m_error == RME_OTBM_IO_NO_ERROR) {
         m_error = RME_OTBM_IO_ERROR_WRITE_FAILED;
    }
}

} // namespace io
} // namespace core
} // namespace RME
