#include "core/network/MapProtocolCodec.h"
#include "core/network/NetworkMessage.h"
// Add other essential includes for cursor methods if any, like Position.h
#include "core/Position.h" // For RME::core::Position in LiveCursor
#include <QString>         // For LiveCursor::userName

#include "core/io/MemoryNodeFileWriteHandle.h"
#include "core/io/MemoryNodeFileReadHandle.h" // Added this include
#include "core/io/otbm_constants.h"
#include "core/Item.h"
#include "core/Tile.h" // Added this include
#include "core/map/MapVersionInfo.h" // Should be included by Map.h, but good to be explicit if used directly
#include "core/map/QTreeNode.h" // Added this include
#include "core/map/Floor.h"     // Added this include
#include "core/Map.h"           // For RME::core::map::MAP_LAYERS and Map::getOrCreateTile
// IItemTypeProvider is used by deserializeItem_from_Node, ensure it's available
// It's forward declared in MapProtocolCodec.h, but full def might be needed for Item::create or Item methods
// For now, assuming Item::create doesn't need full IItemTypeProvider def.

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

static std::unique_ptr<RME::core::Item> deserializeItem_from_Node(RME::core::io::BinaryNode* itemNode,
                                                                  const RME::core::map::MapVersionInfo& version,
                                                                  RME::core::IItemTypeProvider* itemProvider) {
    if (!itemNode || !itemProvider) return nullptr;

    uint8_t itemNodeType;
    if (!itemNode->getByte(itemNodeType)) {
        // Could not read item node type byte
        return nullptr;
    }
    if (itemNodeType != io::OTBM_ITEM) {
        // Expected OTBM_ITEM node type
        return nullptr;
    }

    uint16_t itemId;
    if (!itemNode->getU16(itemId)) {
        // Could not read item ID
        return nullptr;
    }

    std::unique_ptr<RME::core::Item> item = RME::core::Item::create(itemId);
    if (!item) {
        // Item::create failed
        return nullptr;
    }

    // Placeholder: Item needs a method to deserialize its own attributes from itemNode's properties
    // Example: if (!item->unserializeAttributesFromOtbmNode(version, itemNode, itemProvider)) { return nullptr; }
    // This method would loop itemNode->getByte(attribute_id) for specific item attributes.
    // For now, only ID is read.

    // Placeholder: If item is a container, deserialize its children
    // io::BinaryNode* childContentNode = itemNode->getChild();
    // while(childContentNode) {
    //    std::unique_ptr<Item> childItem = deserializeItem_from_Node(childContentNode, version, itemProvider); // Recursive call
    //    if (childItem && item->isContainer()) { // Assuming Item::isContainer
    //        item->getContainer()->addItem(std::move(childItem)); // Assuming Item::getContainer and Container::addItem
    //    } else { /* error handling */ break; }
    //    childContentNode = itemNode->getNextChild();
    // }

    return item;
}

static bool deserializeTileContent_from_Node(RME::core::Tile* tile, RME::core::io::BinaryNode* tileNode,
                                             const RME::core::map::MapVersionInfo& version,
                                             RME::core::IItemTypeProvider* itemProvider,
                                             RME::core::Map& map) {
    if (!tile || !tileNode || !itemProvider) return false;

    uint8_t nodeType;
    if (!tileNode->getByte(nodeType)) return false;

    tile->clear(); // Assuming Tile::clear() exists to remove old content

    if (nodeType == io::OTBM_HOUSETILE) {
        uint32_t houseId;
        if (!tileNode->getU32(houseId)) return false;
        tile->setHouseID(houseId);
        // Placeholder: Link to actual House object if map.getHouses().findHouse(houseId) exists
        // Example: RME::core::House* house = map.getHouses().getHouseById(houseId);
        // if (house) tile->setHouseObject(house); // Assuming Tile::setHouseObject
    } else if (nodeType != io::OTBM_TILE) {
        return false; // Not a valid tile node type
    }

    // Read tile attributes
    uint8_t attribute;
    while (tileNode->getByte(attribute)) {
        switch (attribute) {
            case io::OTBM_ATTR_TILE_FLAGS: {
                uint32_t flags_val;
                if (!tileNode->getU32(flags_val)) return false;
                tile->setMapFlags(RME::core::TileMapFlags(flags_val)); // Assumes constructor from int

                // Placeholder: Zone ID deserialization
                // if (tile->getMapFlags().testFlag(RME::core::TileMapFlag::TILESTATE_ZONE_BRUSH)) { // Assuming flag exists
                //     uint16_t zoneId = 0;
                //     do {
                //         if (!tileNode->getU16(zoneId)) return false;
                //         if (zoneId != 0) {
                //             // tile->addZoneId(zoneId); // Assuming addZoneId exists
                //         }
                //     } while (zoneId != 0);
                // }
                break;
            }
            case io::OTBM_ATTR_ITEM: {
                // This attribute marks a ground item.
                // In OTBM, this typically means the item's ID (and potentially more for complex items)
                // is part of the tileNode's properties, NOT a child node.
                // However, our current serializeItem_to_Writer creates a full child node for ground items too.
                // For consistency with that, we'd expect a child node.
                // If OTBM_ATTR_ITEM means compact data:
                // std::unique_ptr<Item> groundItem = deserializeItem_from_Node_compact(tileNode, version, itemProvider);
                // For now, assume OTBM_ATTR_ITEM is a flag, and the ground item is the first child.
                // This means this attribute is just informational if ground is always a child.
                break;
            }
            default:
                // Unknown attribute. A robust parser must skip the attribute's data.
                // This is complex if attributes have variable length and no length prefix.
                // For now, encountering an unknown attribute is an error.
                // qWarning("deserializeTileContent_from_Node: Unknown tile attribute %u", attribute);
                return false;
        }
    }

    // Read child item nodes (ground item if present, then top items)
    io::BinaryNode* childItemNode = tileNode->getChild();
    while (childItemNode) {
        // Each child should be an OTBM_ITEM node.
        std::unique_ptr<Item> item = deserializeItem_from_Node(childItemNode, version, itemProvider);
        if (item) {
            tile->addItem(std::move(item)); // addItem should handle ground vs top placement
        } else {
            // qWarning("deserializeTileContent_from_Node: Failed to deserialize item from child node.");
            return false;
        }
        childItemNode = tileNode->getNextChild();
    }
    return true;
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
    if (!tile) return false;

    io::MemoryNodeFileWriteHandle writer; // Temporary writer for this tile's node structure

    // Determine node type based on whether it's a house tile
    uint8_t nodeType = (tile->getHouseID() != 0) ? io::OTBM_HOUSETILE : io::OTBM_TILE;
    if (!writer.addNode(nodeType)) return false;

    // Write HouseID if it's a house tile
    if (tile->getHouseID() != 0) {
        if (!writer.addU32(tile->getHouseID())) return false;
    }

    // Write TileFlags attribute if any flags are set
    RME::core::TileMapFlags tileFlags = tile->getMapFlags(); // Assume getMapFlags() exists
    if (tileFlags != RME::core::TileMapFlag::NO_FLAGS) {
        if (!writer.addByte(io::OTBM_ATTR_TILE_FLAGS)) return false;
        if (!writer.addU32(static_cast<uint32_t>(tileFlags.toInt()))) return false; // Assumes toInt() or direct cast

        // Placeholder: Zone ID serialization
        // if (tileFlags.testFlag(RME::core::TileMapFlag::TILESTATE_ZONE_BRUSH)) { // Assuming TILESTATE_ZONE_BRUSH is in TileMapFlag
        //     const auto& zoneIds = tile->getZoneIds(); // Assuming getZoneIds returns std::vector<uint16_t> or similar
        //     for (uint16_t zoneId : zoneIds) {
        //         if (!writer.addU16(zoneId)) return false;
        //     }
        //     if (!writer.addU16(0)) return false; // Terminator for zone IDs
        // }
    }

    // Serialize ground item (if any)
    const Item* ground = tile->getGround();
    if (ground) {
        // OTBM_ATTR_ITEM attribute indicates a ground item.
        // The item itself is then serialized as a child node using serializeItem_to_Writer.
        if (!writer.addByte(io::OTBM_ATTR_ITEM)) return false;
        if (!serializeItem_to_Writer(ground, writer, version)) return false;
    }

    // Serialize top items (each as a child node)
    for (const auto& itemPtr : tile->getItems()) { // Assuming getItems() returns QList<unique_ptr<Item>>
        if (itemPtr) {
            if (!serializeItem_to_Writer(itemPtr.get(), writer, version)) return false;
        }
    }

    if (!writer.endNode()) return false; // End the main tile node (OTBM_TILE or OTBM_HOUSETILE)

    if (!writer.isOk()) return false;
    msg.addBytes(writer.getData(), writer.getSize()); // Add the serialized tile node to the NetworkMessage
    return true;
}

bool MapProtocolCodec::deserializeTileContent(RME::core::Tile* tile, NetworkMessage& msg,
                                           const RME::core::map::MapVersionInfo& version,
                                           RME::core::IItemTypeProvider* itemProvider,
                                           RME::core::Map& map) {
    if (!tile || !itemProvider) return false;

    size_t initialReadPos = msg.getReadPosition();
    // Create a reader for the segment of the message that represents this one tile node.
    io::MemoryNodeFileReadHandle reader(msg.getData() + initialReadPos, msg.getBytesReadable());
    io::BinaryNode* tileNode = reader.getRootNode(); // Reads the initial NODE_START for the tile node

    if (!tileNode) {
        // Could not read the tile's root node from the message segment
        return false;
    }

    // Call the helper function to do the actual deserialization from the BinaryNode
    bool success = deserializeTileContent_from_Node(tile, tileNode, version, itemProvider, map);

    if (success) {
        // If successful, advance the main message's read position by how much the reader consumed.
        msg.setReadPosition(initialReadPos + reader.tell());
    }
    // If !success, msg read position is not advanced.
    return success;
}

std::unique_ptr<RME::core::Item> MapProtocolCodec::deserializeItem(NetworkMessage& msg,
                                                            const RME::core::map::MapVersionInfo& version,
                                                            RME::core::IItemTypeProvider* itemProvider) {
    if (!itemProvider) return nullptr; // Guard against null itemProvider

    size_t initialReadPos = msg.getReadPosition();
    // Create a reader for the segment of the message that represents this one item node.
    io::MemoryNodeFileReadHandle reader(msg.getData() + initialReadPos, msg.getBytesReadable());
    io::BinaryNode* itemOtbmNode = reader.getRootNode(); // Reads the initial NODE_START for the item node

    if (!itemOtbmNode) {
        // Could not read the item's root node from the message segment
        return nullptr;
    }

    // The itemOtbmNode *is* the OTBM_ITEM node. Pass it to the helper.
    std::unique_ptr<Item> item = deserializeItem_from_Node(itemOtbmNode, version, itemProvider);

    if (item) {
        // If successful, advance the main message's read position by how much the reader consumed.
        msg.setReadPosition(initialReadPos + reader.tell());
    }
    // If item is nullptr, an error occurred, message read position is not advanced,
    // allowing the caller to inspect the message state or attempt recovery.
    return item;
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

bool MapProtocolCodec::serializeMapSector(const RME::core::map::QTreeNode* qtreeNode, NetworkMessage& msg, const RME::core::map::MapVersionInfo& version) {
    if (!qtreeNode) return false;

    // This MemoryNodeFileWriteHandle will build the content for the entire sector
    // which consists of:
    // U16 floorSendMask
    // For each floor in mask:
    //   U16 tileBits
    //   If tileBits > 0:
    //     U16 blobLength
    //     U8[blobLength] blobData (which is a mini OTBM document with a dummy root and tile children)
    io::MemoryNodeFileWriteHandle sectorDataWriter;

    uint16_t floorSendMask = 0;
    bool hasFloorsToSend = false;
    for (uint8_t z = 0; z < RME::core::map::MAP_LAYERS; ++z) { // MAP_LAYERS needs to be accessible
        const Floor* floor = qtreeNode->getFloor(z); // Assuming QTreeNode::getFloor(z)
        if (floor) { // If floor exists, consider it for sending (original checked floorMask from parameters)
                     // For sending everything in the node, we check if floor is non-null.
            floorSendMask |= (1 << z);
            hasFloorsToSend = true;
        }
    }
    if (!sectorDataWriter.addU16(floorSendMask)) return false;

    if (hasFloorsToSend) {
        for (uint8_t z = 0; z < RME::core::map::MAP_LAYERS; ++z) {
            if (floorSendMask & (1 << z)) {
                const Floor* floor = qtreeNode->getFloor(z);
                if (!floor) return false; // Should not happen if mask was set correctly

                // For each floor, we first write its tileBits (U16)
                // Then, if there are tiles, we write blobLength (U16) and blobData (U8[])
                // This all goes into sectorDataWriter.

                uint16_t tileBits = 0;
                int validTilesOnFloor = 0; // To check if blob needs to be written

                for (uint8_t y_offset = 0; y_offset < 4; ++y_offset) {
                    for (uint8_t x_offset = 0; x_offset < 4; ++x_offset) {
                        const Tile* tile = floor->getTile(x_offset, y_offset); // Assuming Floor::getTile(local_x, local_y)
                        if (tile && !tile->isEmpty()) { // Assuming Tile::isEmpty()
                            tileBits |= (1 << ((y_offset * 4) + x_offset)); // y*W+x
                            validTilesOnFloor++;
                        }
                    }
                }
                if (!sectorDataWriter.addU16(tileBits)) return false;

                if (validTilesOnFloor > 0) {
                    io::MemoryNodeFileWriteHandle tilesBlobWriter; // Temporary writer for this floor's tiles OTBM blob
                    if(!tilesBlobWriter.addNode(0)) return false; // Dummy root node for the list of tiles on this floor

                    for (uint8_t y_offset = 0; y_offset < 4; ++y_offset) {
                        for (uint8_t x_offset = 0; x_offset < 4; ++x_offset) {
                            if (tileBits & (1 << ((y_offset * 4) + x_offset))) {
                                const Tile* tile = floor->getTile(x_offset, y_offset);
                                if (!tile) return false; // Should exist if bit is set
                                // serializeTileData appends its output (a self-contained tile node)
                                // to the tilesBlobWriter's internal buffer.
                                if (!serializeTileData(tile, tilesBlobWriter, version)) return false;
                            }
                        }
                    }
                    if (!tilesBlobWriter.endNode()) return false; // End dummy root node
                    if (!tilesBlobWriter.isOk()) return false;

                    // Now write the length of this blob, then the blob itself, to sectorDataWriter
                    if (!sectorDataWriter.addU16(static_cast<uint16_t>(tilesBlobWriter.getSize()))) return false;
                    if (!sectorDataWriter.addBytes(tilesBlobWriter.getData(), tilesBlobWriter.getSize())) return false;
                }
            }
        }
    }

    if (!sectorDataWriter.isOk()) return false;
    // Add the entire sector data (floorMask + per-floor data) to the main NetworkMessage
    msg.addBytes(sectorDataWriter.getData(), sectorDataWriter.getSize());
    return true;
}

bool MapProtocolCodec::deserializeMapSector(RME::core::map::QTreeNode* qtreeNode, NetworkMessage& msg,
                                         RME::core::Map& map,
                                         const RME::core::map::MapVersionInfo& version,
                                         RME::core::IItemTypeProvider* itemProvider) {
    if (!qtreeNode || !itemProvider) return false;

    uint16_t floorMask;
    if (!msg.getU16(floorMask)) return false;

    if (floorMask == 0) return true; // No floors in this sector update, valid empty sector.

    for (uint8_t z = 0; z < RME::core::map::MAP_LAYERS; ++z) {
        if (floorMask & (1 << z)) {
            Floor* floor = qtreeNode->getFloor(z);
            if (!floor) {
                 floor = qtreeNode->createFloor(z); // Assuming QTreeNode::createFloor exists
                 if(!floor) {
                    // Failed to create floor, attempt to skip this floor's data in the message
                    uint16_t tileBitsToSkip; if(!msg.getU16(tileBitsToSkip)) return false; // Read tileBits
                    if(tileBitsToSkip != 0) { // If there were tiles, there's a data blob
                        uint16_t dataLenToSkip; if(!msg.getU16(dataLenToSkip)) return false; // Read blob length
                        if(msg.getBytesReadable() < dataLenToSkip) return false; // Not enough data to skip
                        msg.setReadPosition(msg.getReadPosition() + dataLenToSkip); // Skip blob data
                    }
                    continue; // Move to next floor in mask
                 }
            }
            floor->clear(); // Assuming Floor::clear() exists to remove old tiles

            uint16_t tileBits;
            if (!msg.getU16(tileBits)) return false;

            if (tileBits == 0) { // Floor sent, but all its 16 tiles are empty
                // Floor already cleared, nothing more to do for this floor.
                continue;
            }

            uint16_t floorTilesBlobLen;
            if (!msg.getU16(floorTilesBlobLen)) return false; // Length of the OTBM blob for this floor's tiles
            if (msg.getBytesReadable() < floorTilesBlobLen) return false; // Not enough data in message

            size_t blobStartReadPosInMainMsg = msg.getReadPosition();

            // Create a MemoryNodeFileReadHandle for the blob of tile data for this specific floor
            io::MemoryNodeFileReadHandle tilesBlobReader(msg.getData() + blobStartReadPosInMainMsg, floorTilesBlobLen);

            io::BinaryNode* dummyFloorRootNode = tilesBlobReader.getRootNode(); // Expects dummy root node (type 0)
            if (!dummyFloorRootNode) { /* Error reading dummy root */ return false; }

            io::BinaryNode* currentTileDataNode = dummyFloorRootNode->getChild(); // First actual tile node

            for (uint8_t y_offset = 0; y_offset < 4; ++y_offset) {
                for (uint8_t x_offset = 0; x_offset < 4; ++x_offset) {
                    // QTreeNode::getPosition() should give the top-left (min x, min y) of the node.
                    Position tilePos(qtreeNode->getPosition().x + x_offset, qtreeNode->getPosition().y + y_offset, z);

                    if (tileBits & (1 << ((y_offset * 4) + x_offset))) { // y*W+x
                        if (!currentTileDataNode) return false; // Not enough tile nodes in the blob

                        Tile* tile = map.getOrCreateTile(tilePos); // Assumes Map::getOrCreateTile clears/prepares it
                        if (!tile) return false;

                        // Use the helper to deserialize content from the current BinaryNode
                        if (!deserializeTileContent_from_Node(tile, currentTileDataNode, version, itemProvider, map)) {
                            // qWarning("deserializeMapSector: Failed to deserialize content for tile at (%d,%d,%d)", tilePos.x, tilePos.y, tilePos.z);
                            return false;
                        }
                        currentTileDataNode = dummyFloorRootNode->getNextChild(); // Move to next tile node in blob
                    } else {
                         // Tile is not in tileBits, ensure it's empty or created empty
                         map.getOrCreateTile(tilePos); // Should ensure it exists and is empty
                    }
                }
            }
            if (currentTileDataNode) { /* Error: Not all tile nodes from blob were consumed */ return false; }

            // Advance the main message's read position by the length of the blob,
            // only after successfully processing it.
            msg.setReadPosition(blobStartReadPosInMainMsg + floorTilesBlobLen);
        }
    }
    return true;
}


} // namespace network
} // namespace core
} // namespace RME
