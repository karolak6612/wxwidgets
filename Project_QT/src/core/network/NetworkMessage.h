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

#ifndef RME_NETWORK_MESSAGE_H
#define RME_NETWORK_MESSAGE_H

#include <vector>
#include <string>
#include <cstdint>
#include "core/Position.h" // Assuming RME::core::Position

// Forward declare QString for addString/getString if using it directly
class QString;

namespace RME {
namespace core {
namespace network {

class NetworkMessage {
public:
    NetworkMessage(size_t initialCapacity = 128); // Constructor, optionally reserve capacity

    // Buffer management
    void clear();
    void resetRead(); // Resets only the read position
    const uint8_t* getData() const;
    uint8_t* getWriteBuffer(size_t requiredSize); // Gets raw buffer for writing 'requiredSize' bytes
    void didWrite(size_t bytesWritten); // Call after writing to buffer obtained from getWriteBuffer

    size_t getSize() const; // Current size of the payload
    size_t getReadPosition() const;
    void setReadPosition(size_t pos);
    size_t getBytesReadable() const;

    // Adding data (write operations) - these append to the buffer
    void addU8(uint8_t value);
    void addU16(uint16_t value); // Assumes little-endian or network byte order handled by user
    void addU32(uint32_t value); // Assumes little-endian or network byte order handled by user
    void addU64(uint64_t value); // Assumes little-endian or network byte order handled by user
    void addString(const std::string& value);
    void addString(const QString& value); // For convenience if QString is used
    void addPosition(const RME::core::Position& value);
    void addBytes(const uint8_t* bytes, size_t length);

    // Getting data (read operations) - these read from m_readPos and advance it
    bool getU8(uint8_t& value);
    bool getU16(uint16_t& value); // Assumes little-endian or network byte order handled by user
    bool getU32(uint32_t& value); // Assumes little-endian or network byte order handled by user
    bool getU64(uint64_t& value); // Assumes little-endian or network byte order handled by user
    bool getString(std::string& value);
    bool getString(QString& value); // For convenience
    bool getPosition(RME::core::Position& value);
    bool getBytes(uint8_t* buffer, size_t length); // Reads 'length' bytes into provided buffer
    bool peekU8(uint8_t& value) const; // Reads without advancing read position

    // Methods to directly prepare a message with a header for its own length
    // (optional, can be done externally too)
    // void finalizeMessageWithLengthHeader(); // Prepends total message length

private:
    template <typename T>
    void writeRaw(const T& value);

    template <typename T>
    bool readRaw(T& value);

    std::vector<uint8_t> m_buffer;
    size_t m_readPos;
    // Write position is implicitly m_buffer.size()
};

} // namespace network
} // namespace core
} // namespace RME

#endif // RME_NETWORK_MESSAGE_H
