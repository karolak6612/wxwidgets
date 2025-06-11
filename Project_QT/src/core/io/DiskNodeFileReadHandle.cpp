#include "core/io/DiskNodeFileReadHandle.h"
#include "core/io/otbm_constants.h" // For error codes
#include <QDebug>                   // For qWarning
#include <QtEndian>                 // For qFromLittleEndian (if reading header values)

namespace RME {
namespace core {
namespace io {

DiskNodeFileReadHandle::DiskNodeFileReadHandle(const QString& filePath)
    : NodeFileReadHandle() // Call base constructor
    , m_file(filePath)
    // m_stream is initialized after file is opened
{
    if (!m_file.open(QIODevice::ReadOnly)) {
        m_error = RME_OTBM_IO_ERROR_FILE_OPEN;
        qWarning() << "DiskNodeFileReadHandle: Failed to open file:" << filePath << "Error:" << m_file.errorString();
        return;
    }

    m_stream.setDevice(&m_file);
    m_stream.setByteOrder(QDataStream::LittleEndian); // OTBM is typically little-endian

    // Handle OTBM File Identifier (first 4 bytes)
    // Common practice: 0x00000000 or "OTBM".
    // wxDiskNodeFileReadHandle reads this and validates against acceptable_identifiers.
    // For this version, we'll read it. Strict validation can be added or be OtbmMapIO's job.
    if (m_file.size() < 4) {
        m_error = RME_OTBM_IO_ERROR_SYNTAX; // Or a more specific header error
        qWarning() << "DiskNodeFileReadHandle: File too short for OTBM identifier:" << filePath;
        m_file.close();
        return;
    }

    char identifier[4];
    qint64 bytesRead = m_stream.readRawData(identifier, 4);

    if (bytesRead != 4) {
        m_error = RME_OTBM_IO_ERROR_READ_FAILED;
        qWarning() << "DiskNodeFileReadHandle: Failed to read OTBM identifier from:" << filePath << "Stream status:" << m_stream.status();
        m_file.close();
        return;
    }

    // Basic check: could compare against known identifiers like "OTBM" or check if first byte is NODE_START
    // For now, we just consume these 4 bytes. NodeFileReadHandle::getRootNode() expects to see NODE_START next.
    // If these 4 bytes *were* the NODE_START and type etc., then NodeFileReadHandle logic would be different.
    // But typical OTBM has these as a preamble.
    // Example validation (can be expanded):
    // uint32_t magic = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(identifier));
    // if (magic != 0x00000000 && !(identifier[0]=='O' && identifier[1]=='T' && identifier[2]=='B' && identifier[3]=='M')) {
    //     qWarning() << "DiskNodeFileReadHandle: Invalid OTBM identifier.";
    //     m_error = RME_OTBM_IO_ERROR_SYNTAX;
    //     m_file.close();
    //     return;
    // }

    // If the identifier itself was NODE_START, then we need to unread or adjust.
    // However, typical OTBM has this 4-byte header *before* the first NODE_START.
    // So, after reading these 4 bytes, the stream should be positioned at the first NODE_START.
}

DiskNodeFileReadHandle::~DiskNodeFileReadHandle() {
    if (m_file.isOpen()) {
        m_file.close();
    }
}

size_t DiskNodeFileReadHandle::tell() const {
    if (!m_file.isOpen() || m_error != RME_OTBM_IO_NO_ERROR) {
        return 0; // Or some other error indicator if size_t can't be -1
    }
    // Returns position from start of file, which is what's needed.
    // The initial 4 bytes are already consumed from m_stream's perspective if constructor succeeded.
    // Base NodeFileReadHandle works on the stream *after* this header.
    return static_cast<size_t>(m_file.pos());
}

bool DiskNodeFileReadHandle::isEof() const {
    if (m_error != RME_OTBM_IO_NO_ERROR || !m_file.isOpen()) {
        return true; // Treat error or closed file as EOF
    }
    return m_stream.atEnd(); // QDataStream::atEnd is usually reliable
}

bool DiskNodeFileReadHandle::ensureBytesAvailable(size_t bytes) {
    if (m_error != RME_OTBM_IO_NO_ERROR) return false;
    if (!m_file.isOpen()) {
        m_error = RME_OTBM_IO_ERROR_FILE_NOT_OPEN;
        return false;
    }
    if (bytes == 0) return true;

    // Check against file size from current position
    if (static_cast<qint64>(m_file.pos()) + static_cast<qint64>(bytes) > m_file.size()) {
        m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        return false;
    }
    // QDataStream also has internal buffering, but this check against physical EOF is a good safeguard.
    return true;
}

uint8_t DiskNodeFileReadHandle::readByteUnsafe() {
    // This method assumes ensureBytesAvailable(1) was called and returned true.
    // And m_error is RME_OTBM_IO_NO_ERROR.
    char byte_char;
    qint64 bytesRead = m_stream.readRawData(&byte_char, 1);

    if (bytesRead != 1) {
        // This indicates an issue, e.g., actual EOF or stream error.
        m_error = RME_OTBM_IO_ERROR_READ_FAILED;
        if (m_stream.status() == QDataStream::ReadPastEnd) {
            m_error = RME_OTBM_IO_ERROR_UNEXPECTED_EOF;
        }
        // qWarning() << "DiskNodeFileReadHandle: Failed to read byte. Stream status:" << m_stream.status();
        return 0; // Return dummy value
    }
    return static_cast<uint8_t>(byte_char);
}

} // namespace io
} // namespace core
} // namespace RME
