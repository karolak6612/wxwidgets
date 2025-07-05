//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "core/network/NetworkMessage.h"
#include <cstring> // For memcpy
#include <stdexcept> // For std::runtime_error (optional for error handling)
#include <QString>   // For QString conversion

// For endianness handling - assuming little-endian for now, like most x86 systems
// If network byte order (big-endian) is required, these need to be adjusted.
// Or use Qt's qToLittleEndian/qFromLittleEndian if Qt types are used.

namespace RME {
namespace core {
namespace network {

NetworkMessage::NetworkMessage(size_t initialCapacity) : m_readPos(0), m_errorState(false) {
    m_buffer.reserve(initialCapacity);
}

void NetworkMessage::clear() {
    m_buffer.clear();
    m_readPos = 0;
    m_errorState = false;
}

void NetworkMessage::resetRead() {
    m_readPos = 0;
}

const uint8_t* NetworkMessage::getData() const {
    return m_buffer.data();
}

uint8_t* NetworkMessage::getWriteBuffer(size_t requiredSize) {
    size_t oldSize = m_buffer.size();
    m_buffer.resize(oldSize + requiredSize);
    return m_buffer.data() + oldSize;
}

void NetworkMessage::didWrite(size_t bytesWritten) {
    // This is a bit tricky. If getWriteBuffer was used, m_buffer was already resized.
    // This function is more of a conceptual "commit" of those bytes if needed,
    // but direct resize and memcpy is also fine.
    // For now, this function doesn't need to do much if m_buffer.resize was used.
    // If we want to only grow buffer on demand, this would be different.
    // Let's assume for now that getWriteBuffer + memcpy is the primary way,
    // and this function could be used to truncate if fewer bytes were written than reserved.
    // For simplicity, we'll assume it's not strictly needed if getWriteBuffer resizes correctly.
}


size_t NetworkMessage::getSize() const {
    return m_buffer.size();
}

size_t NetworkMessage::getReadPosition() const {
    return m_readPos;
}

void NetworkMessage::setReadPosition(size_t pos) {
    if (pos > m_buffer.size()) {
        m_readPos = m_buffer.size(); // Cap at end
    } else {
        m_readPos = pos;
    }
}

size_t NetworkMessage::getBytesReadable() const {
    if (m_readPos >= m_buffer.size()) {
        return 0;
    }
    return m_buffer.size() - m_readPos;
}

template <typename T>
void NetworkMessage::writeRaw(const T& value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(T));
}

template <typename T>
bool NetworkMessage::readRaw(T& value) {
    if (getBytesReadable() < sizeof(T)) {
        return false; // Not enough data
    }
    memcpy(&value, m_buffer.data() + m_readPos, sizeof(T));
    m_readPos += sizeof(T);
    return true;
}

// --- Add Methods ---
void NetworkMessage::addU8(uint8_t value) {
    writeRaw(value);
}

void NetworkMessage::addU16(uint16_t value) {
    // TODO: Consider proper endian conversion if needed (e.g. htole16 or qToLittleEndian)
    writeRaw(value);
}

void NetworkMessage::addU32(uint32_t value) {
    writeRaw(value);
}

void NetworkMessage::addU64(uint64_t value) {
    writeRaw(value);
}

void NetworkMessage::addString(const std::string& value) {
    uint16_t len = static_cast<uint16_t>(value.length());
    addU16(len); // Length prefix
    addBytes(reinterpret_cast<const uint8_t*>(value.data()), len);
}

void NetworkMessage::addString(const QString& value) {
    // Convert QString to UTF-8 for network transmission
    QByteArray utf8Bytes = value.toUtf8();
    uint16_t len = static_cast<uint16_t>(utf8Bytes.length());
    addU16(len); // Length prefix
    addBytes(reinterpret_cast<const uint8_t*>(utf8Bytes.constData()), len);
}


void NetworkMessage::addPosition(const RME::core::Position& value) {
    addU16(static_cast<uint16_t>(value.x));
    addU16(static_cast<uint16_t>(value.y));
    addU8(static_cast<uint8_t>(value.z));
}

void NetworkMessage::addBytes(const uint8_t* bytes, size_t length) {
    if (length == 0) return;
    m_buffer.insert(m_buffer.end(), bytes, bytes + length);
}

// --- Get Methods ---
bool NetworkMessage::getU8(uint8_t& value) {
    return readRaw(value);
}

bool NetworkMessage::getU16(uint16_t& value) {
    // TODO: Consider proper endian conversion if needed (e.g. le16toh or qFromLittleEndian)
    return readRaw(value);
}

bool NetworkMessage::getU32(uint32_t& value) {
    return readRaw(value);
}

bool NetworkMessage::getU64(uint64_t& value) {
    return readRaw(value);
}

bool NetworkMessage::getString(std::string& value) {
    uint16_t len;
    if (!getU16(len)) {
        return false;
    }
    if (getBytesReadable() < len) {
        // Not enough data for the string content, revert read of length
        m_readPos -= sizeof(uint16_t);
        return false;
    }
    value.assign(reinterpret_cast<const char*>(m_buffer.data() + m_readPos), len);
    m_readPos += len;
    return true;
}

bool NetworkMessage::getString(QString& value) {
    uint16_t len;
    if (!getU16(len)) {
        return false;
    }
    if (getBytesReadable() < len) {
        m_readPos -= sizeof(uint16_t);
        return false;
    }
    value = QString::fromUtf8(reinterpret_cast<const char*>(m_buffer.data() + m_readPos), len);
    m_readPos += len;
    return true;
}

bool NetworkMessage::getPosition(RME::core::Position& value) {
    uint16_t x, y;
    uint8_t z;
    size_t originalReadPos = m_readPos; // Save state for potential rollback
    if (!getU16(x) || !getU16(y) || !getU8(z)) {
        m_readPos = originalReadPos; // Rollback on partial read failure
        return false;
    }
    value.x = x;
    value.y = y;
    value.z = z;
    return true;
}

bool NetworkMessage::getBytes(uint8_t* bufferOut, size_t length) {
    if (length == 0) return true;
    if (getBytesReadable() < length) {
        return false;
    }
    memcpy(bufferOut, m_buffer.data() + m_readPos, length);
    m_readPos += length;
    return true;
}

bool NetworkMessage::peekU8(uint8_t& value) const {
    if (getBytesReadable() < sizeof(uint8_t)) {
        return false;
    }
    value = *(m_buffer.data() + m_readPos);
    return true;
}

// State checking methods
bool NetworkMessage::isEmpty() const {
    return m_buffer.empty();
}

bool NetworkMessage::isInErrorState() const {
    return m_errorState;
}

// Alias for getBytes
bool NetworkMessage::readBytes(uint8_t* buffer, size_t length) {
    return getBytes(buffer, length);
}

// Convenience methods for reading without output parameters
uint8_t NetworkMessage::readU8() {
    uint8_t value = 0;
    if (!getU8(value)) {
        m_errorState = true;
    }
    return value;
}

uint16_t NetworkMessage::readU16() {
    uint16_t value = 0;
    if (!getU16(value)) {
        m_errorState = true;
    }
    return value;
}

uint32_t NetworkMessage::readU32() {
    uint32_t value = 0;
    if (!getU32(value)) {
        m_errorState = true;
    }
    return value;
}

uint64_t NetworkMessage::readU64() {
    uint64_t value = 0;
    if (!getU64(value)) {
        m_errorState = true;
    }
    return value;
}

std::string NetworkMessage::readString() {
    std::string value;
    if (!getString(value)) {
        m_errorState = true;
    }
    return value;
}

RME::core::Position NetworkMessage::readPosition() {
    RME::core::Position value;
    if (!getPosition(value)) {
        m_errorState = true;
    }
    return value;
}

} // namespace network
} // namespace core
} // namespace RME
