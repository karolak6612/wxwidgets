#include "core/network/MapProtocolCodec.h"
#include "core/network/NetworkMessage.h"
// Add other essential includes for cursor methods if any, like Position.h
#include "core/Position.h" // For RME::core::Position in LiveCursor
#include <QString>         // For LiveCursor::userName

#include "core/io/MemoryNodeFileWriteHandle.h"
#include "core/io/otbm_constants.h"
#include "core/Item.h"
#include "core/map/MapVersionInfo.h" // Should be included by Map.h, but good to be explicit if used directly

namespace RME {
namespace core {
namespace network {

// MapProtocolCodec constructor is deleted in header (static class)

// --- Static Helper Implementations ---

static bool serializeItem_to_Writer(const RME::core::Item* item, RME::core::io::NodeFileWriteHandle& writer,
                                   const RME::core::map::MapVersionInfo& version) {
    if (!item) return false;

    if (!writer.addNode(io::OTBM_ITEM)) return false;
    if (!writer.addU16(item->getID())) return false;

    // Placeholder: Item needs a method to serialize its own attributes
    // Example: if (!item->serializeAttributesToOtbmNode(version, writer)) return false;
    // This method would write attributes like count, charge, text, etc.

    // Placeholder: If item is a container, serialize its children
    // if (item->isContainer()) {
    //    for (const Item* childItem : item->getContainerItems()) {
    //        if (!serializeItem_to_Writer(childItem, writer, version)) return false;
    //    }
    // }

    if (!writer.endNode()) return false;
    return writer.isOk();
}

bool MapProtocolCodec::serializeCursor(const LiveCursor& cursor, NetworkMessage& msg) {
    if (!msg.addString(cursor.userName)) return false;
    if (!msg.addU8(cursor.color.r)) return false;
    if (!msg.addU8(cursor.color.g)) return false;
    if (!msg.addU8(cursor.color.b)) return false;
    if (!msg.addU8(cursor.color.a)) return false;
    if (!msg.addPosition(cursor.position)) return false;
    return true;
}

bool MapProtocolCodec::deserializeCursor(NetworkMessage& msg, LiveCursor& outCursor) {
    if (!msg.getString(outCursor.userName)) return false;
    if (!msg.getU8(outCursor.color.r)) return false;
    if (!msg.getU8(outCursor.color.g)) return false;
    if (!msg.getU8(outCursor.color.b)) return false;
    if (!msg.getU8(outCursor.color.a)) return false;
    if (!msg.getPosition(outCursor.position)) return false;
    return true;
}

// Other public methods will be stubbed/implemented in subsequent steps
bool MapProtocolCodec::serializeTileData(const RME::core::Tile* tile, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version) {
    return false;
}

bool MapProtocolCodec::deserializeTileContent(RME::core::Tile* tile, NetworkMessage& msg,
                                           const RME::core::map::MapVersionInfo& version,
                                           RME::core::IItemTypeProvider* itemProvider,
                                           RME::core::Map& map) {
    return false;
}

std::unique_ptr<RME::core::Item> MapProtocolCodec::deserializeItem(NetworkMessage& msg,
                                                            const RME::core::map::MapVersionInfo& version,
                                                            RME::core::IItemTypeProvider* itemProvider) {
    return nullptr;
}

bool MapProtocolCodec::serializeItem(const RME::core::Item* item, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version) {
    if (!item) return false;
    io::MemoryNodeFileWriteHandle writer; // Temporary writer for this item's node structure
    if (!serializeItem_to_Writer(item, writer, version)) {
        return false;
    }

    if (!writer.isOk()) return false;
    msg.addBytes(writer.getData(), writer.getSize());
    return true;
}

bool MapProtocolCodec::serializeMapSector(const RME::core::map::QTreeNode* node, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version) {
    return false;
}

bool MapProtocolCodec::deserializeMapSector(RME::core::map::QTreeNode* node, NetworkMessage& msg,
                                         RME::core::Map& map,
                                         const RME::core::map::MapVersionInfo& version,
                                         RME::core::IItemTypeProvider* itemProvider) {
    return false;
}


} // namespace network
} // namespace core
} // namespace RME
