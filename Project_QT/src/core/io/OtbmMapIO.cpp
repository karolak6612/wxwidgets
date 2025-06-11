#include "core/io/OtbmMapIO.h"
#include "core/io/DiskNodeFileReadHandle.h"
#include "core/io/DiskNodeFileWriteHandle.h"
#include "core/io/BinaryNode.h"
#include "core/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/AssetManager.h" // For ItemType, ItemDatabase access
#include "core/settings/AppSettings.h"
#include "core/Position.h"

#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QtZlib/qtzlib.h> // For qUncompress, qCompress
#include <QDebug>          // For logging errors

namespace RME {
namespace core {
namespace io {

/**
 * @brief Default constructor.
 */
OtbmMapIO::OtbmMapIO() {
}

/**
 * @brief Destructor.
 */
OtbmMapIO::~OtbmMapIO() {
}

/**
 * @brief Gets supported file extensions.
 * @return List containing "*.otbm".
 */
QStringList OtbmMapIO::getSupportedFileExtensions() const {
    return {"*.otbm"};
}

/**
 * @brief Gets the format name.
 * @return "Open Tibia Binary Map".
 */
QString OtbmMapIO::getFormatName() const {
    return "Open Tibia Binary Map";
}

/**
 * @brief Decompresses data using qUncompress.
 * @param compressedData Data to decompress.
 * @return Decompressed data, or empty on error.
 */
QByteArray OtbmMapIO::decompressNodeData(const std::vector<uint8_t>& compressedData) {
    if (compressedData.empty()) {
        m_lastError = "Compressed data is empty.";
        return QByteArray();
    }
    // QByteArray::fromRawData doesn't copy, be careful with lifetime if vector is temporary.
    // It's safer to construct QByteArray that owns its data.
    QByteArray compressedByteArray(reinterpret_cast<const char*>(compressedData.data()), static_cast<int>(compressedData.size()));
    QByteArray decompressedData = qUncompress(compressedByteArray);
    if (decompressedData.isEmpty() && !compressedByteArray.isEmpty()) { // qUncompress returns empty on error
        m_lastError = "Failed to decompress data. Data might be corrupt or not zlib compressed.";
        // qWarning() << m_lastError;
    }
    return decompressedData;
}

/**
 * @brief Compresses data using qCompress.
 * @param uncompressedData Data to compress.
 * @return Compressed data, or empty on error.
 */
std::vector<uint8_t> OtbmMapIO::compressNodeData(const QByteArray& uncompressedData) {
    if (uncompressedData.isEmpty()) {
        // Compressing empty data is valid, results in a small compressed block.
        // Or, we can choose to not compress it. For OTBM, often properties are not written if empty.
    }
    QByteArray compressedData = qCompress(uncompressedData, 9); // Use high compression
    if (compressedData.isEmpty() && !uncompressedData.isEmpty()) {
        m_lastError = "Failed to compress data.";
        // qWarning() << m_lastError;
        return {};
    }
    return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(compressedData.constData()),
                                reinterpret_cast<const uint8_t*>(compressedData.constData()) + compressedData.size());
}


// --- Loading Implementation ---
bool OtbmMapIO::loadMap(const QString& filePath, Map& map, AssetManager& assetManager, AppSettings& settings) {
    m_lastError.clear();
    DiskNodeFileReadHandle readHandle(filePath);

    if (readHandle.isInErrorState()) {
        m_lastError = "Failed to open map file: " + filePath + ". Error code: " + QString::number(readHandle.getLastError());
        qWarning() << "OtbmMapIO::loadMap: " << m_lastError;
        return false;
    }

    // First four bytes are 0, then OTBM version (uint32)
    // Then width, height, item major, item minor (all uint32)
    // This initial header part is NOT a node.
    // The NodeFileReadHandle::getRootNode() expects to start reading nodes.
    // OTBM format:
    //   UINT32 0x00000000 (identifier, or could be version in some interpretations)
    //   Root BinaryNode (type 0x00)
    //     - OTBM_NODE_MAP_DATA (type 0x01)
    //       - Attributes (map version, description, etc.)
    //       - Children (tile areas, towns, waypoints)

    // The NodeFileReadHandle is designed to read the node structure.
    // The "OTBM" identifier and version are typically part of the root node's properties or first children.
    // Let's assume NodeFileReadHandle::getRootNode correctly positions itself at the start of node stream.

    uint32_t otbmVersion = readHandle.read<uint32_t>(); // Read first 4 bytes (should be 0 or version)
    if (readHandle.isInErrorState() || (otbmVersion != 0 && otbmVersion > 3)) { // OTBM v1, v2, v3 known. 0 might be initial marker
        // Some OTBM versions might start with 0x00000000 then the root node.
        // Others might have the version directly.
        // If readHandle.read<uint32_t>() is for the version *before* the root node starts,
        // we might need to adjust NodeFileReadHandle or read it here.
        // For now, assuming getRootNode handles versioning internally or it's an attribute.
        // The provided NodeFileReadHandle doesn't seem to have a method for this initial version.
        // This might need to be an attribute of the root node itself or the first map_data node.
        // Let's proceed assuming getRootNode works as intended.
        // If the first 4 bytes are not part of the node stream, NodeFileReadHandle needs adjustment or
        // we read them here and ensure the handle is then positioned for getRootNode.
        // The current NodeFileReadHandle implies it starts reading nodes directly.
    }


    std::unique_ptr<BinaryNode> root(readHandle.getRootNode());
    if (!root || readHandle.isInErrorState()) {
        m_lastError = "Failed to read root node from OTBM file. Error: " + QString::number(readHandle.getLastError());
        qWarning() << "OtbmMapIO::loadMap: " << m_lastError;
        return false;
    }

    if (root->getType() != OTBM_NODE_ROOT) {
        m_lastError = "Invalid OTBM file: Root node type is not OTBM_NODE_ROOT (0x00). Type was: " + QString::number(root->getType());
        qWarning() << "OtbmMapIO::loadMap: " << m_lastError;
        return false;
    }

    // The actual map data is usually the first child of the root node.
    BinaryNode* mapDataNode = nullptr;
    if (!root->getChildren().empty()) { // This getChildren() is conceptual from previous attempts.
                                      // BinaryNode uses getChild() and advance()
        mapDataNode = root->getChild();
    }


    if (!mapDataNode || mapDataNode->getType() != OTBM_NODE_MAP_DATA) {
        m_lastError = "Invalid OTBM file: First child of root is not OTBM_NODE_MAP_DATA (0x01).";
        if(mapDataNode) m_lastError += " Type was: " + QString::number(mapDataNode->getType());
        qWarning() << "OtbmMapIO::loadMap: " << m_lastError;
        return false;
    }

    if (!parseMapDataNode(mapDataNode, map, settings)) { // Pass assetManager here
        // m_lastError would be set by parseMapDataNode
        return false;
    }

    // TODO: Parse other top-level nodes like OTBM_NODE_TOWNS, OTBM_NODE_WAYPOINTS if they are children of root.
    // For now, focus on map data.

    return true;
}

// Corrected signature to include AssetManager
bool OtbmMapIO::parseMapDataNode(BinaryNode* mapDataNode, Map& map, /* AssetManager& assetManager,*/ AppSettings& settings) {
    // The public loadMap has assetManager, so it should be passed from there.
    // Corrected call: if (!parseMapDataNode(mapDataNode, map, assetManager, settings))
    // And this function signature should be:
    bool OtbmMapIO::parseMapDataNode(BinaryNode* mapDataNode, Map& map, AssetManager& assetManager, AppSettings& settings) {

    // --- Attribute parsing logic ---
    mapDataNode->resetReadOffset(); // Crucial before reading properties
    while(mapDataNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!mapDataNode->getU8(attribute)) { m_lastError = "Failed to read map data attribute type"; return false; }

        switch(attribute) {
            case OTBM_ATTR_DESCRIPTION: {
                QString description;
                if (!mapDataNode->getString(description)) { m_lastError = "Failed to read map description string"; return false; }
                map.setDescription(description);
                break;
            }
            case OTBM_ATTR_EXT_HOUSE_FILE: {
                QString houseFile;
                if (!mapDataNode->getString(houseFile)) { m_lastError = "Failed to read house file string"; return false; }
                // map.setHouseFile(houseFile); // Store if Map object supports it
                break;
            }
            case OTBM_ATTR_EXT_SPAWN_FILE: {
                QString spawnFile;
                if (!mapDataNode->getString(spawnFile)) { m_lastError = "Failed to read spawn file string"; return false; }
                // map.setSpawnFile(spawnFile); // Store if Map object supports it
                break;
            }
            // OTBM_ATTR_MAP_WIDTH, OTBM_ATTR_MAP_HEIGHT are not standard attributes for OTBM_NODE_MAP_DATA.
            // Dimensions are implicit from tile areas.
            default:
                m_lastError = "Unknown attribute for MAP_DATA node: " + QString::number(attribute);
                qWarning() << m_lastError;
                // This requires BinaryNode to have a robust way to skip unknown attributes.
                // For now, fail.
                return false;
        }
    }

    // --- Child node parsing ---
    BinaryNode* childNode = mapDataNode->getChild();
    while(childNode) {
        switch (childNode->getType()) {
            case OTBM_NODE_TILE_AREA: // Assuming type constant OTBM_NODE_TILE_AREA (or _V2)
                 // Pass AssetManager correctly
                if (!parseTileAreaNode(childNode, map, assetManager, settings)) return false;
                break;
            case OTBM_NODE_TOWNS:
                // TODO: Implement parseTownsNode(childNode, map);
                qWarning() << "Parsing OTBM_NODE_TOWNS not yet implemented.";
                break;
            case OTBM_NODE_WAYPOINTS:
                // TODO: Implement parseWaypointsListNode(childNode, map);
                qWarning() << "Parsing OTBM_NODE_WAYPOINTS (global) not yet implemented.";
                break;
            default:
                qWarning() << "OtbmMapIO: Unknown child node type " << childNode->getType() << " in MAP_DATA.";
                // Skip unknown child nodes if possible, or error.
                break;
        }
        childNode = childNode->advance();
    }
    return true;
}

// Added settings to signature
bool OtbmMapIO::parseTileAreaNode(BinaryNode* tileAreaNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    const QByteArray& nodeData = tileAreaNode->getNodeData(); // Use new getter
    if (nodeData.size() < 5) { // X(U16), Y(U16), Z(U8)
        m_lastError = "TileArea node data is too short for coordinates."; return false;
    }
    // Use QDataStream for safe parsing from QByteArray
    QDataStream stream(nodeData);
    stream.setByteOrder(QDataStream::LittleEndian);
    quint16 area_base_x, area_base_y;
    quint8 area_base_z;
    stream >> area_base_x >> area_base_y >> area_base_z;
    if (stream.status() != QDataStream::Ok) {
        m_lastError = "Failed to stream TileArea coordinates from node data."; return false;
    }
    Position areaBasePos(area_base_x, area_base_y, area_base_z);

    map.ensureDimensions(area_base_x + 255, area_base_y + 255, area_base_z);

    BinaryNode* tileNode = tileAreaNode->getChild();
    while(tileNode) {
        // Assuming specific V2 types for now from otbm_constants.h
        if (tileNode->getType() == OTBM_NODE_TILE || tileNode->getType() == OTBM_NODE_HOUSETILE) {
            if (!parseTileNode(tileNode, map, assetManager, areaBasePos, settings)) return false; // Pass settings
        } else {
            qWarning() << "OtbmMapIO: Unknown child node type " << tileNode->getType() << " in TILE_AREA.";
        }
        tileNode = tileNode->advance();
    }
    return true;
}

// Added settings to signature
bool OtbmMapIO::parseTileNode(BinaryNode* tileNode, Map& map, AssetManager& assetManager, const Position& areaBasePos, AppSettings& settings) {
    const QByteArray& nodeData = tileNode->getNodeData();
    if (nodeData.size() < 2) { // RelX(U8), RelY(U8)
        m_lastError = "Tile node data is too short for relative coordinates."; return false;
    }
    Position tilePos = areaBasePos;
    tilePos.x += static_cast<uint8_t>(nodeData[0]);
    tilePos.y += static_cast<uint8_t>(nodeData[1]);

    Tile* currentTile = map.getOrCreateTile(tilePos);
    if (!currentTile) {
        m_lastError = "Failed to get or create tile at " + tilePos.toString(); return false;
    }

    tileNode->resetReadOffset();
    while(tileNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!tileNode->getU8(attribute)) { m_lastError = "Failed to read tile attribute type"; return false; }
        switch(attribute) {
            case OTBM_ATTR_TILE_FLAGS: {
                quint32 flags; // Use quint32 for Qt types
                if (!tileNode->getU32(flags)) { m_lastError = "Failed to read tile flags"; return false; }
                if (flags & OTBM_TILEFLAG_PROTECTIONZONE) currentTile->addMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE);
                if (flags & OTBM_TILEFLAG_NOPVPZONE)       currentTile->addMapFlag(RME::core::TileMapFlag::NO_PVP_ZONE);
                if (flags & OTBM_TILEFLAG_NOLOGOUT)       currentTile->addMapFlag(RME::core::TileMapFlag::NO_LOGOUT_ZONE);
                if (flags & OTBM_TILEFLAG_PVPZONE)         currentTile->addMapFlag(RME::core::TileMapFlag::PVP_ZONE);
                // ... map other OTBM_TILEFLAGS to RME::core::TileMapFlag
                break;
            }
            case OTBM_ATTR_HOUSETILE_HOUSEID:
                if (tileNode->getType() == OTBM_NODE_HOUSETILE) {
                    quint32 houseId;
                    if (!tileNode->getU32(houseId)) { m_lastError = "Failed to read house ID"; return false; }
                    currentTile->setHouseID(houseId);
                } else { // Should not happen if OTBM is well-formed
                    m_lastError = "OTBM_ATTR_HOUSETILE_HOUSEID found on non-HOUSETILE node."; return false;
                }
                break;
            default:
                m_lastError = "Unknown attribute " + QString::number(attribute) + " for TILE/HOUSETILE node at " + tilePos.toString();
                // This requires a mechanism in BinaryNode to skip attributes based on type or an explicit length field
                // which is not part of simple OTBM attributes (U8 type, then data).
                // For now, this will likely fail if unknown attributes are encountered.
                qWarning() << m_lastError;
                return false; // Strict parsing
        }
    }

    BinaryNode* childDataNode = tileNode->getChild();
    while(childDataNode) {
        switch(childDataNode->getType()) {
            case OTBM_NODE_ITEM: // Assuming OTBM_NODE_ITEM_V2 if using versioned types
                if (!parseItemNode(childDataNode, currentTile, assetManager, settings)) return false;
                break;
            case OTBM_NODE_CREATURE:
                // if (!parseCreatureNode(childDataNode, currentTile, assetManager)) return false;
                qWarning() << "Parsing OTBM_NODE_CREATURE not yet implemented.";
                break;
            default:
                qWarning() << "OtbmMapIO: Unknown child node type " << childDataNode->getType() << " in TILE data for " << tilePos.toString();
                break;
        }
        childDataNode = childDataNode->advance();
    }
    currentTile->update();
    return true;
}

// Added settings to parseItemNode signature to match the call from parseTileNode
bool OtbmMapIO::parseItemNode(BinaryNode* itemNode, Tile* tile, AssetManager& assetManager, AppSettings& settings) {
    const QByteArray& nodeData = itemNode->getNodeData();
    if (nodeData.size() < 2) { // ItemID (U16)
        m_lastError = "Item node data is too short for ItemID."; return false;
    }
    quint16 itemId = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(nodeData.constData()));

    const RME::core::assets::ItemType* itemType = assetManager.getItemDatabase().getItemData(itemId);
    if (!itemType) {
        m_lastError = "Item ID " + QString::number(itemId) + " not found in ItemDatabase. Pos: " + tile->getPosition().toString();
        qWarning() << m_lastError;
        return settings.value("SkipUnknownItems", true).toBool(); // Option to skip or fail
    }

    std::unique_ptr<Item> newItem = Item::create(itemId, &assetManager.getItemDatabase());
    if (!newItem) {
        m_lastError = "Failed to create item instance for ID: " + QString::number(itemId); return false;
    }

    itemNode->resetReadOffset();
    while(itemNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!itemNode->getU8(attribute)) { m_lastError = "Failed to read item attribute type"; return false; }
        switch(attribute) {
            case OTBM_ATTR_COUNT: // Also charges for some items
            case OTBM_ATTR_CHARGES: {
                uint8_t count_charges; // Default to U8
                if (!itemNode->getU8(count_charges)) { m_lastError = "Failed to read item count/charges"; return false; }
                newItem->setSubtype(count_charges);
                // Note: Some OTBM versions or specific items might use U16 for charges.
                // This would require checking itemType properties or a more complex attribute reading.
                break;
            }
            case OTBM_ATTR_ACTION_ID: {
                quint16 actionId;
                if (!itemNode->getU16(actionId)) { m_lastError = "Failed to read item ActionID"; return false; }
                newItem->setActionID(actionId);
                break;
            }
            case OTBM_ATTR_UNIQUE_ID: {
                quint16 uniqueId;
                if (!itemNode->getU16(uniqueId)) { m_lastError = "Failed to read item UniqueID"; return false; }
                newItem->setUniqueID(uniqueId);
                break;
            }
            case OTBM_ATTR_TEXT: {
                QString text;
                if (!itemNode->getString(text)) { m_lastError = "Failed to read item text"; return false; }
                newItem->setText(text);
                break;
            }
             case OTBM_ATTR_WRITTENBY: {
                QString writtenBy;
                if (!itemNode->getString(writtenBy)) { m_lastError = "Failed to read item writtenBy"; return false; }
                // newItem->setWrittenBy(writtenBy); // If Item class supports
                break;
            }
            case OTBM_ATTR_WRITTENDATE: {
                quint32 date;
                if (!itemNode->getU32(date)) { m_lastError = "Failed to read item writtenDate"; return false; }
                // newItem->setWrittenDate(date); // If Item class supports
                break;
            }
            case OTBM_ATTR_DESCRIPTION: { // Item description
                QString itemDesc;
                if (!itemNode->getString(itemDesc)) { m_lastError = "Failed to read item description attribute"; return false; }
                // newItem->setAttribute("description", itemDesc); // Generic attribute setting
                break;
            }
            // TODO: Implement other item attributes as needed (DepotID, Teleport Dest, etc.)
            default:
                m_lastError = "Unknown attribute " + QString::number(attribute) + " for ITEM node (ID: " + QString::number(itemId) + ")";
                qWarning() << m_lastError;
                // This is where robust skipping of unknown attributes is critical.
                // The current BinaryNode::getXYZ methods consume based on known type.
                // If an attribute type implies a length prefix (like strings do), it can be skipped.
                // If it's fixed size (U8, U16, U32, U64), it can be read and discarded.
                // Without a generic "skip attribute" or type-length-value for all, this is hard.
                return false; // Strict parsing
        }
    }

    tile->addItem(std::move(newItem));
    return true;
}


// --- Saving Implementation (Refined) ---
bool OtbmMapIO::saveMap(const QString& filePath, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    m_lastError.clear();
    DiskNodeFileWriteHandle writeHandle(filePath);
    if (writeHandle.isInErrorState()) {
        m_lastError = "SaveMap: Failed to open file for writing: " + filePath + ". Error: " + QString::number(writeHandle.getLastError());
        qWarning() << m_lastError;
        return false;
    }

    // OTBM Header: U32 version (0), then Root Node
    // NodeFileWriteHandle::addU32 now appends to attribute buffer. This is wrong for header.
    // We need a raw write for the initial version.
    writeHandle.writeU32RawUnsafe(0); // Use the new raw helper
    if (!writeHandle.isOk()) {
         m_lastError = "SaveMap: Failed to write OTBM version header. Error: " + QString::number(writeHandle.getError());
         return false;
    }

    // Decide on compression for root node's properties (if any, usually none for root itself)
    bool compressRootProps = false;
    if (!writeHandle.addNode(OTBM_NODE_ROOT, compressRootProps)) {
        m_lastError = "SaveMap: Failed to write root node start. Error: " + QString::number(writeHandle.getError());
        return false;
    }

    if (!serializeMapDataNode(writeHandle, map, settings)) { /* m_lastError set by callee */ return false; }

    // TODO: Serialize OTBM_NODE_TOWNS, OTBM_NODE_WAYPOINTS (global lists)

    if (!writeHandle.endNode()) {
        m_lastError = "SaveMap: Failed to write root node end. Error: " + QString::number(writeHandle.getError());
        return false;
    }
    if (!writeHandle.flush()) {
        m_lastError = "SaveMap: Failed to flush data to disk. Error: " + QString::number(writeHandle.getError());
        return false;
    }
    if (writeHandle.isInErrorState()) {
         m_lastError = "SaveMap: An error occurred. Error: " + QString::number(writeHandle.getError());
        return false;
    }
    return true;
}

bool OtbmMapIO::serializeMapDataNode(NodeFileWriteHandle& writer, const Map& map, AppSettings& settings) {
    // For OTBM_NODE_MAP_DATA, decide if its attributes should be compressed. Usually not.
    bool compressMapDataProps = settings.value("CompressMapDataProperties", false).toBool();
    if (!writer.addNode(OTBM_NODE_MAP_DATA, compressMapDataProps)) return false;

    // Map Attributes
    if (!map.getDescription().isEmpty()) {
        if (!writer.addU8(OTBM_ATTR_DESCRIPTION) || !writer.addString(map.getDescription())) {
            m_lastError = "Failed to write map description."; return false;
        }
    }
    // TODO: Add OTBM_ATTR_VERSION_MAJOR etc. from settings

    // Map dimensions are not attributes of MAP_DATA in standard OTBM.
    // They are implicit from the union of all TILE_AREA extents.

    // Iterate map by 256x256 areas
    for (uint16_t y_base = 0; y_base < map.getHeight(); y_base += 256) {
        for (uint16_t x_base = 0; x_base < map.getWidth(); x_base += 256) {
            bool areaHasTiles = false; // Check if this area has any tiles to save
            for (uint8_t z = 0; z < Map::MAX_Z; ++z) {
                for (uint16_t y_off = 0; y_off < 256 && y_base + y_off < map.getHeight(); ++y_off) {
                    for (uint16_t x_off = 0; x_off < 256 && x_base + x_off < map.getWidth(); ++x_off) {
                        if (map.getTile(Position(x_base + x_off, y_base + y_off, z))) {
                            areaHasTiles = true; break;
                        }
                    } if (areaHasTiles) break;
                } if (areaHasTiles) break;
            }

            if (areaHasTiles) {
                uint16_t currentAreaWidth = std::min((uint16_t)256, (uint16_t)(map.getWidth() - x_base));
                uint16_t currentAreaHeight = std::min((uint16_t)256, (uint16_t)(map.getHeight() - y_base));
                if (!serializeTileAreaNode(writer, map, Position(x_base, y_base, 0), /* This z is placeholder */
                                           currentAreaWidth, currentAreaHeight,
                                           const_cast<AssetManager&>(map.getItemTypeProvider()->getAssetManager()))) { // HACK: const_cast
                    return false;
                }
            }
        }
    }

    // TODO: Serialize OTBM_NODE_TOWNS, OTBM_NODE_WAYPOINTS (global lists)

    if (!writer.endNode()) { m_lastError = "Failed to end MAP_DATA node."; return false; }
    return true;
}

bool OtbmMapIO::serializeTileAreaNode(NodeFileWriteHandle& writer, const Map& map,
                                   const Position& areaBasePosXY, // Z is iterated inside
                                   uint16_t areaWidth, uint16_t areaHeight,
                                   AssetManager& assetManager) {
    // For each floor (z) in this area that has tiles
    for (uint8_t z = 0; z < Map::MAX_Z; ++z) {
        bool floorHasTiles = false;
        for (uint16_t y_offset = 0; y_offset < areaHeight; ++y_offset) {
            for (uint16_t x_offset = 0; x_offset < areaWidth; ++x_offset) {
                if (map.getTile(Position(areaBasePosXY.x + x_offset, areaBasePosXY.y + y_offset, z))) {
                    floorHasTiles = true; break;
                }
            } if (floorHasTiles) break;
        }
        if (!floorHasTiles) continue; // Skip this floor if empty for this area

        // OTBM_NODE_TILE_AREA node per floor segment
        // Decide on compression for TileArea attributes (usually none for TileArea itself)
        if (!writer.addNode(OTBM_NODE_TILE_AREA, false)) return false;

        // Write area base coordinates (X, Y, Z) as node data
        QByteArray areaCoordsData;
        QDataStream ds(&areaCoordsData, QIODevice::WriteOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << static_cast<quint16>(areaBasePosXY.x) << static_cast<quint16>(areaBasePosXY.y) << static_cast<quint8>(z);
        if (!writer.addNodeData(areaCoordsData)) { m_lastError = "Failed to write tile area coords."; return false; }

        for (uint16_t y_offset = 0; y_offset < areaHeight; ++y_offset) {
            for (uint16_t x_offset = 0; x_offset < areaWidth; ++x_offset) {
                Position currentPos(areaBasePosXY.x + x_offset, areaBasePosXY.y + y_offset, z);
                const Tile* tile = map.getTile(currentPos);
                if (tile && (tile->getItemCount() > 0 || tile->getMapFlags() != RME::core::TileMapFlag::NO_FLAGS || tile->getHouseID() > 0)) {
                    if (!serializeTileNode(writer, tile, assetManager)) {
                         m_lastError = "Failed to serialize tile at " + currentPos.toString(); return false;
                    }
                }
            }
        }
        if (!writer.endNode()) { m_lastError = "Failed to end TILE_AREA node for z=" + QString::number(z); return false; }
    }
    return true;
}

bool OtbmMapIO::serializeTileNode(NodeFileWriteHandle& writer, const Tile* tile, AssetManager& assetManager) {
    uint8_t nodeType = tile->getHouseID() > 0 ? OTBM_NODE_HOUSETILE : OTBM_NODE_TILE;
    // Decide compression for tile attributes (usually not compressed)
    if (!writer.addNode(nodeType, false)) return false;

    // Write tile relative coordinates (X offset from area, Y offset from area) as Node Data
    QByteArray tileCoordsData;
    tileCoordsData.append(static_cast<char>(tile->getPosition().x % 256));
    tileCoordsData.append(static_cast<char>(tile->getPosition().y % 256));
    if (!writer.addNodeData(tileCoordsData)) { m_lastError = "Failed to write tile relative coords."; return false; }

    // Write Tile Attributes
    if (tile->getMapFlags() != RME::core::TileMapFlag::NO_FLAGS) {
        if(!writer.addU8(OTBM_ATTR_TILE_FLAGS) || !writer.addU32(static_cast<quint32>(tile->getMapFlags()))) {
             m_lastError = "Failed to write tile flags."; return false;
        }
    }
    if (nodeType == OTBM_NODE_HOUSETILE) { // House ID is only for housetiles
        if(!writer.addU8(OTBM_ATTR_HOUSETILE_HOUSEID) || !writer.addU32(tile->getHouseID())) {
            m_lastError = "Failed to write house ID."; return false;
        }
    }

    // Write Items
    if (tile->getGround()) {
        if (!serializeItemNode(writer, tile->getGround(), assetManager)) return false;
    }
    for (const auto& itemPtr : tile->getItems()) {
        if (!serializeItemNode(writer, itemPtr.get(), assetManager)) return false;
    }

    // TODO: Serialize Creatures, Spawns on this tile

    if (!writer.endNode()) { m_lastError = "Failed to end TILE/HOUSETILE node."; return false; }
    return true;
}

bool OtbmMapIO::serializeItemNode(NodeFileWriteHandle& writer, const Item* item, AssetManager& assetManager) {
    if (!item) return true;
    // Decide compression for item attributes (can be compressed if many/large text attributes)
    bool compressItemProps = false; // Default to false, could be based on settings or item complexity
    if (!writer.addNode(OTBM_NODE_ITEM, compressItemProps)) return false;

    // Write Item ID as Node Data (U16)
    QByteArray itemIdData;
    QDataStream ds(&itemIdData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << static_cast<quint16>(item->getID());
    if (!writer.addNodeData(itemIdData)) { m_lastError = "Failed to write item ID node data."; return false; }

    // Write Item Attributes
    if (item->getSubtype() != 0) {
        // This needs to be more specific based on item type (stackable vs charges etc.)
        // For simplicity, using OTBM_ATTR_COUNT. A real impl. might use OTBM_ATTR_CHARGES.
        if(!writer.addU8(OTBM_ATTR_COUNT)) return false; // Or OTBM_ATTR_CHARGES
        const RME::core::assets::ItemType* itemType = assetManager.getItemDatabase().getItemData(item->getID());
        if (itemType && itemType->isStackable) { // Assuming ItemType has such flags
            if(!writer.addU8(static_cast<uint8_t>(item->getSubtype()))) return false;
        } else { // For charges or other subtypes, often U16
            if(!writer.addU16(item->getSubtype())) return false;
        }
    }
    if (item->getActionID() != 0) {
        if(!writer.addU8(OTBM_ATTR_ACTION_ID) || !writer.addU16(item->getActionID())) return false;
    }
    if (item->getUniqueID() != 0) { // Assuming 0 is default/invalid UID
        if(!writer.addU8(OTBM_ATTR_UNIQUE_ID) || !writer.addU16(item->getUniqueID())) return false;
    }
    if (!item->getText().isEmpty()) {
        if(!writer.addU8(OTBM_ATTR_TEXT) || !writer.addString(item->getText())) return false;
    }
    // TODO: Other attributes: WrittenBy, WrittenDate, Description (for some items), DepotID, Teleport Dest, etc.

    if (!writer.endNode()) { m_lastError = "Failed to end ITEM node."; return false; }
    return true;
}

} // namespace io
} // namespace core
} // namespace RME
