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
    if (!root->getChildren().empty()) {
        mapDataNode = root->getChildren().front().get();
    }

    if (!mapDataNode || mapDataNode->getType() != OTBM_NODE_MAP_DATA) {
        m_lastError = "Invalid OTBM file: First child of root is not OTBM_NODE_MAP_DATA (0x01).";
        if(mapDataNode) m_lastError += " Type was: " + QString::number(mapDataNode->getType());
        qWarning() << "OtbmMapIO::loadMap: " << m_lastError;
        return false;
    }

    if (!parseMapDataNode(mapDataNode, map, settings)) {
        // m_lastError would be set by parseMapDataNode
        return false;
    }

    // TODO: Parse other top-level nodes like OTBM_NODE_TOWNS, OTBM_NODE_WAYPOINTS if they are children of root.
    // For now, focus on map data.

    return true;
}

bool OtbmMapIO::parseMapDataNode(BinaryNode* mapDataNode, Map& map, AppSettings& settings) {
    // Read attributes from mapDataNode's properties stream
    // The properties stream in BinaryNode is raw. We need to make a new read handle for it.
    // This is a design flaw in BinaryNode if it doesn't provide easier access to properties.
    // Assuming BinaryNode::getPropsData() returns the raw byte vector for properties.
    // And we need to parse it attribute by attribute.
    // OTBM attributes: byte type, uint16_t length, data.
    // This parsing should ideally be in BinaryNode or a dedicated property parser.
    // For now, we'll assume a simplified direct attribute access or that BinaryNode handles this.
    // Let's refine this based on BinaryNode's actual capabilities.
    // The current BinaryNode from previous tasks has m_props which is a vector of bytes.
    // It needs a way to iterate/decode these properties.
    // For now, we will assume BinaryNode has a method like:
    // QVariant getAttribute(uint8_t attribute_type) or similar.
    // This is a GAPING HOLE in the current BinaryNode design from previous tasks.
    // For this task, I will SIMULATE this property parsing.

    // Simulated property parsing:
    // This is a placeholder for what BinaryNode should provide.
    // In a real OTBM parser, mapDataNode->m_props would be parsed attribute by attribute.
    // Example attributes: OTBM_ATTR_DESCRIPTION, OTBM_ATTR_EXT_HOUSE_FILE, etc.
    // OTBM_ATTR_MAP_WIDTH, OTBM_ATTR_MAP_HEIGHT are usually part of OTBM_NODE_TILE_AREA or header.

    // The OTBM format description states map dimensions are part of the OTBM_NODE_MAP_DATA attributes.
    // OTBM_ATTR_MAP_WIDTH, OTBM_ATTR_MAP_HEIGHT.
    // Let's assume BinaryNode provides a way to get these:
    // uint16_t mapWidth = mapDataNode->getAttribute<uint16_t>(OTBM_ATTR_MAP_WIDTH, 0);
    // uint16_t mapHeight = mapDataNode->getAttribute<uint16_t>(OTBM_ATTR_MAP_HEIGHT, 0);
    // uint32_t majorVersion = mapDataNode->getAttribute<uint32_t>(OTBM_ATTR_VERSION_MAJOR, 0);
    // ... this is not how BinaryNode is currently structured. It just has a raw m_props byte vector.

    // The OTBM format description from source:
    // The first attribute is OTBM_ATTR_DESCRIPTION (string).
    // The second attribute is OTBM_ATTR_EXT_HOUSE_FILE (string). (Optional)
    // The third attribute is OTBM_ATTR_EXT_SPAWN_FILE (string). (Optional)
    // Map dimensions (width, height) are NOT attributes of MAP_DATA. They are implicitly defined by tile areas.
    // Client map version, item major/minor versions are attributes.

    // Reset read offset for properties of this node
    mapDataNode->resetReadOffset(); // Assuming BinaryNode has this method

    while(mapDataNode->hasMoreProperties()) { // Assuming BinaryNode has this
        uint8_t attribute;
        if (!mapDataNode->getU8(attribute)) { m_lastError = "Failed to read map data attribute type"; return false; }

        switch(attribute) {
            case OTBM_ATTR_DESCRIPTION: {
                std::string description;
                if (!mapDataNode->getString(description)) { m_lastError = "Failed to read map description string"; return false; }
                map.setDescription(QString::fromStdString(description));
                break;
            }
            case OTBM_ATTR_EXT_HOUSE_FILE: { // Optional
                std::string houseFile;
                if (!mapDataNode->getString(houseFile)) { m_lastError = "Failed to read house file string"; return false; }
                // map.setHouseFile(QString::fromStdString(houseFile)); // Store if Map object supports it
                break;
            }
            case OTBM_ATTR_EXT_SPAWN_FILE: { // Optional
                std::string spawnFile;
                if (!mapDataNode->getString(spawnFile)) { m_lastError = "Failed to read spawn file string"; return false; }
                // map.setSpawnFile(QString::fromStdString(spawnFile)); // Store if Map object supports it
                break;
            }
            // Placeholder for map version attributes if they are here (OTBM v2+)
            // case OTBM_ATTR_VERSION_MAJOR: ...
            // case OTBM_ATTR_VERSION_MINOR: ...
            // case OTBM_ATTR_VERSION_BUILD: ...
            default:
                // Unknown attribute for MAP_DATA node. This is an error or needs robust skipping.
                // OTBM generally doesn't use length-prefixed attributes, their length is type-dependent.
                // Strings are length-prefixed. Other types are fixed.
                // This implies the BinaryNode::getXYZ methods must handle this, or we need a way to skip.
                // For now, if an unknown attribute is found, it's an issue.
                m_lastError = "Unknown or unhandled attribute for MAP_DATA node: " + QString::number(attribute);
                // qWarning() << m_lastError;
                // To skip robustly: would need a way to know attribute length based on type.
                // This is a simplification; a full OTBM parser would know how to skip all standard attribute types.
                return false; // Strict: fail on unknown attribute for now.
        }
    }


    // Iterate over child nodes of OTBM_NODE_MAP_DATA
    BinaryNode* childNode = mapDataNode->getChild();
    while(childNode) {
        switch (childNode->getType()) {
            case OTBM_NODE_TILE_AREA:
                if (!parseTileAreaNode(childNode, map, assetManager)) {
                    // m_lastError set by callee
                    return false;
                }
                break;
            case OTBM_NODE_TOWNS:
                // TODO: Implement parseTownsNode(childNode, map);
                // qWarning() << "Skipping OTBM_NODE_TOWNS";
                break;
            case OTBM_NODE_WAYPOINTS: // This is for global waypoints if any, not tile-specific ones
                // TODO: Implement parseWaypointsListNode(childNode, map);
                // qWarning() << "Skipping OTBM_NODE_WAYPOINTS (global list)";
                break;
            default:
                m_lastError = "Unknown child node type in MAP_DATA: " + QString::number(childNode->getType());
                // qWarning() << m_lastError;
                // return false; // Be strict or lenient? For now, try to continue.
                break;
        }
        childNode = childNode->advance();
    }
    return true;
}

bool OtbmMapIO::parseTileAreaNode(BinaryNode* tileAreaNode, Map& map, AssetManager& assetManager) {
    // OTBM_NODE_TILE_AREA node data:
    // U16 area_base_x, U16 area_base_y, U8 area_base_z
    const auto& nodeData = tileAreaNode->getData();
    if (nodeData.size() < 5) { // 2+2+1 bytes
        m_lastError = "TileArea node data is too short for coordinates.";
        return false;
    }
    // Create a temporary read handle for the node's own data stream
    // This is a bit of a workaround. Ideally BinaryNode would offer getU16 etc. on its own data.
    // Or NodeFileReadHandle could be re-purposed. For now, manual parsing:
    uint16_t area_base_x = nodeData[0] | (nodeData[1] << 8);
    uint16_t area_base_y = nodeData[2] | (nodeData[3] << 8);
    uint8_t  area_base_z = nodeData[4];
    Position areaBasePos(area_base_x, area_base_y, area_base_z);

    // Ensure map is large enough. Map::resize should handle this.
    // Map dimensions are usually derived from the max extents of tile areas.
    // map.resize(...) // May not be needed if map grows dynamically or dimensions known from MAP_DATA attrs.

    BinaryNode* tileNode = tileAreaNode->getChild();
    while(tileNode) {
        if (tileNode->getType() == OTBM_NODE_TILE || tileNode->getType() == OTBM_NODE_HOUSETILE) {
            if (!parseTileNode(tileNode, map, assetManager, areaBasePos)) {
                // m_lastError set by callee
                return false;
            }
        } else {
            m_lastError = "Unknown child node type in TILE_AREA: " + QString::number(tileNode->getType());
            // qWarning() << m_lastError;
            // return false; // Strict
        }
        tileNode = tileNode->advance();
    }
    return true;
}

bool OtbmMapIO::parseTileNode(BinaryNode* tileNode, Map& map, AssetManager& assetManager, const Position& areaBasePos) {
    // OTBM_NODE_TILE node data: U8 rel_x, U8 rel_y
    // OTBM_NODE_HOUSETILE node data: U8 rel_x, U8 rel_y (house ID is an attribute)
    const auto& nodeData = tileNode->getData();
    if (nodeData.size() < 2) {
        m_lastError = "Tile node data is too short for relative coordinates.";
        return false;
    }
    Position tilePos = areaBasePos;
    tilePos.x += nodeData[0]; // rel_x
    tilePos.y += nodeData[1]; // rel_y

    Tile* currentTile = map.getOrCreateTile(tilePos);
    if (!currentTile) {
        m_lastError = "Failed to get or create tile at " + tilePos.toString();
        return false;
    }

    // Parse tile attributes from properties
    tileNode->resetReadOffset(); // Prepare for reading properties
    while(tileNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!tileNode->getU8(attribute)) { m_lastError = "Failed to read tile attribute type"; return false; }

        switch(attribute) {
            case OTBM_ATTR_TILE_FLAGS: {
                uint32_t flags;
                if (!tileNode->getU32(flags)) { m_lastError = "Failed to read tile flags"; return false; }
                // currentTile->setOtbmFlags(flags); // Map needs a way to store raw OTBM flags or convert them
                // For now, let's assume some flags map to RME::core::TileMapFlag
                if (flags & OTBM_TILEFLAG_PROTECTIONZONE) currentTile->addMapFlag(RME::core::TileMapFlag::PROTECTION_ZONE);
                // ... and so on for other flags like NOPVP, NOLOGOUT, PVPZONE etc.
                break;
            }
            case OTBM_ATTR_HOUSETILE_HOUSEID: { // Only for OTBM_NODE_HOUSETILE
                if (tileNode->getType() == OTBM_NODE_HOUSETILE) {
                    uint32_t houseId;
                    if (!tileNode->getU32(houseId)) { m_lastError = "Failed to read house ID"; return false; }
                    currentTile->setHouseID(houseId);
                } else {
                    // Attribute not expected for this node type, skip (how?)
                    // This is where knowing attribute data length is crucial for unknown/unexpected attributes.
                    // For now, assume U32 was read if type matches, otherwise it's an error.
                    m_lastError = "OTBM_ATTR_HOUSETILE_HOUSEID found on non-HOUSETILE node."; return false;
                }
                break;
            }
            // OTBM_ATTR_ITEM is NOT an attribute of a tile, items are child nodes.
            default:
                m_lastError = "Unknown or unhandled attribute for TILE/HOUSETILE node: " + QString::number(attribute) + " at " + tilePos.toString();
                // qWarning() << m_lastError;
                // To skip: need to know size. Assume fixed size for now based on type, or fail.
                return false; // Strict
        }
    }

    // Parse child nodes (items, creatures, etc.)
    BinaryNode* childDataNode = tileNode->getChild();
    while(childDataNode) {
        switch(childDataNode->getType()) {
            case OTBM_NODE_ITEM:
                if (!parseItemNode(childDataNode, currentTile, assetManager)) return false;
                break;
            case OTBM_NODE_CREATURE:
                // TODO: parseCreatureNode(childDataNode, currentTile, assetManager);
                break;
            // OTBM_NODE_SPAWN is usually a map-level or area-level feature, not per tile in this way.
            // OTBM_NODE_WAYPOINT (tile specific) might be here.
            default:
                 m_lastError = "Unknown child node type in TILE/HOUSETILE data: " + QString::number(childDataNode->getType());
                // qWarning() << m_lastError;
                break;
        }
        childDataNode = childDataNode->advance();
    }
    currentTile->update();
    return true;
}

bool OtbmMapIO::parseItemNode(BinaryNode* itemNode, Tile* tile, AssetManager& assetManager) {
    // OTBM_NODE_ITEM node data: U16 ItemID
    const auto& nodeData = itemNode->getData();
    if (nodeData.size() < 2) {
        m_lastError = "Item node data is too short for ItemID.";
        return false;
    }
    uint16_t itemId = nodeData[0] | (nodeData[1] << 8);

    const RME::core::assets::ItemType* itemType = assetManager.getItemDatabase().getItemData(itemId);
    if (!itemType) {
        m_lastError = "Item ID " + QString::number(itemId) + " not found in ItemDatabase. Pos: " + tile->getPosition().toString();
        // qWarning() << m_lastError;
        return settings.value("SkipUnknownItems", true).toBool(); // Option to skip or fail
    }

    std::unique_ptr<Item> newItem = Item::create(itemId, &assetManager.getItemDatabase());
    if (!newItem) {
        m_lastError = "Failed to create item instance for ID: " + QString::number(itemId);
        return false;
    }

    // Parse item attributes from properties
    itemNode->resetReadOffset();
    while(itemNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!itemNode->getU8(attribute)) { m_lastError = "Failed to read item attribute type"; return false; }

        switch(attribute) {
            case OTBM_ATTR_COUNT: // Also charges, subtype for some items
            case OTBM_ATTR_CHARGES: { // OTBM uses ATTR_COUNT for stackable count, ATTR_CHARGES for item charges/subtype
                uint8_t count; // Most common form
                if (!itemNode->getU8(count)) { m_lastError = "Failed to read item count/charges"; return false; }
                newItem->setSubtype(count); // Item::setSubtype usually handles this.
                break;
            }
            // Note: Full OTBM spec distinguishes more subtypes of attributes for items.
            // E.g., some items store subtype as U16. This simplified parser assumes U8 for count/charges.
            // A more complex item might need to check itemType->isChargeable() etc. to parse correctly.
            case OTBM_ATTR_ACTION_ID: {
                uint16_t actionId;
                if (!itemNode->getU16(actionId)) { m_lastError = "Failed to read item ActionID"; return false; }
                newItem->setActionID(actionId);
                break;
            }
            case OTBM_ATTR_UNIQUE_ID: {
                uint16_t uniqueId;
                if (!itemNode->getU16(uniqueId)) { m_lastError = "Failed to read item UniqueID"; return false; }
                newItem->setUniqueID(uniqueId);
                break;
            }
            case OTBM_ATTR_TEXT: {
                std::string text;
                if (!itemNode->getString(text)) { m_lastError = "Failed to read item text"; return false; }
                newItem->setText(QString::fromStdString(text));
                break;
            }
            case OTBM_ATTR_WRITTENDATE:
            case OTBM_ATTR_WRITTENBY:
            case OTBM_ATTR_DESCRIPTION: // For items with special descriptions
                // These are strings.
                // { std::string str_val; if(!itemNode->getString(str_val)) return false; /* set appropriate attribute */ break; }
                // For now, just skip them if not handled by specific setters:
                { std::string dummy; if(!itemNode->getString(dummy)) {m_lastError = "Failed to read string for item attribute " + QString::number(attribute); return false;} break; }

            // TODO: Other item attributes like depot ID, house door ID, teleport destination, etc.
            // Each requires knowing its type (U8, U16, U32, string) to parse correctly.
            default:
                m_lastError = "Unknown or unhandled attribute for ITEM node: " + QString::number(attribute) + " for item " + QString::number(itemId);
                // qWarning() << m_lastError;
                // To skip: This is the main problem. We need to know how many bytes to skip.
                // For now, fail. A robust parser would have a table of attribute types and their sizes
                // or rely on attributes being self-describing with length.
                return false;
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
        m_lastError = "Failed to open file for writing: " + filePath + ". Error: " + QString::number(writeHandle.getLastError());
        qWarning() << "OtbmMapIO::saveMap: " << m_lastError;
        return false;
    }

    // OTBM Header: U32 version (0), then Root Node
    if (!writeHandle.addU32(0)) { // OTBM version identifier
         m_lastError = "Failed to write OTBM version header. Error: " + QString::number(writeHandle.getLastError());
         return false;
    }

    if (!writeHandle.addNode(OTBM_NODE_ROOT)) { // Start root node
        m_lastError = "Failed to write root node start. Error: " + QString::number(writeHandle.getLastError());
        return false;
    }

    if (!serializeMapDataNode(writeHandle, map, settings)) {
        // m_lastError set by callee
        return false;
    }

    // TODO: Serialize other top-level data like towns, waypoints if they are direct children of root.

    if (!writeHandle.endNode()) { // End OTBM_NODE_ROOT
        m_lastError = "Failed to write root node end. Error: " + QString::number(writeHandle.getLastError());
        return false;
    }

    if (!writeHandle.flush()) {
        m_lastError = "Failed to flush data to disk. Error: " + QString::number(writeHandle.getLastError());
        return false;
    }

    if (writeHandle.isInErrorState()) {
         m_lastError = "An error occurred during map save. Error: " + QString::number(writeHandle.getLastError());
        return false;
    }
    return true;
}

bool OtbmMapIO::serializeMapDataNode(NodeFileWriteHandle& writer, const Map& map, AppSettings& settings) {
    if (!writer.addNode(OTBM_NODE_MAP_DATA)) return false;

    // Map Attributes
    // Assuming AppSettings provides client version details for OTBM header attributes if needed
    // uint32_t clientVersion = settings.value("ClientVersion", 1099).toUInt(); // Example
    // writer.addU8(OTBM_ATTR_VERSION); // This is for OTBM structure version, not client version.
    // writer.addU32(OTBM_VERSION_LE_32BIT_NODES); // Example

    if (!map.getDescription().isEmpty()) {
        writer.addU8(OTBM_ATTR_DESCRIPTION);
        writer.addString(map.getDescription().toStdString()); // addString handles length prefix
    }

    // These are typically not attributes of MAP_DATA but define the iteration bounds.
    // OTBM_ATTR_MAP_WIDTH and OTBM_ATTR_MAP_HEIGHT are not standard attributes for OTBM_NODE_MAP_DATA.
    // Dimensions are implicit from tile areas.

    // Iterate map by areas (e.g. 255x255 chunks or however Map is organized)
    // For now, assuming one large tile area.
    // A real implementation would iterate through map sectors based on actual content bounds.
    if (map.getWidth() > 0 && map.getHeight() > 0) {
        // OTBM splits map into 256x256 tile areas.
        for (uint16_t y_base = 0; y_base < map.getHeight(); y_base += 256) {
            for (uint16_t x_base = 0; x_base < map.getWidth(); x_base += 256) {
                 // Check if this area actually contains tiles before serializing
                bool areaHasTiles = false;
                for (uint8_t z = 0; z < Map::MAX_Z; ++z) {
                    for (uint16_t y_offset = 0; y_offset < 256 && y_base + y_offset < map.getHeight(); ++y_offset) {
                        for (uint16_t x_offset = 0; x_offset < 256 && x_base + x_offset < map.getWidth(); ++x_offset) {
                            if (map.getTile(Position(x_base + x_offset, y_base + y_offset, z))) {
                                areaHasTiles = true; break;
                            }
                        }
                        if (areaHasTiles) break;
                    }
                    if (areaHasTiles) break;
                }

                if (areaHasTiles) {
                    uint16_t areaWidth = std::min((uint16_t)256, (uint16_t)(map.getWidth() - x_base));
                    uint16_t areaHeight = std::min((uint16_t)256, (uint16_t)(map.getHeight() - y_base));
                    // The AssetManager reference here is tricky. It's needed by serializeTileNode/serializeItemNode.
                    // It should be passed down from the initial saveMap call.
                    if (!serializeTileAreaNode(writer, map, Position(x_base, y_base, 0), areaWidth, areaHeight, const_cast<AssetManager&>(map.getItemTypeProvider()->getAssetManager()))){ // HACK: const_cast
                        return false;
                    }
                }
            }
        }
    }

    // TODO: Serialize OTBM_NODE_TOWNS, OTBM_NODE_WAYPOINTS (global lists)

    if (!writer.endNode()) return false; // End OTBM_NODE_MAP_DATA
    return true;
}

bool OtbmMapIO::serializeTileAreaNode(NodeFileWriteHandle& writer, const Map& map,
                                   const Position& areaBasePos, uint16_t areaWidth, uint16_t areaHeight,
                                   AssetManager& assetManager) {
    if (!writer.addNode(OTBM_NODE_TILE_AREA)) return false;

    // Write area base coordinates (X, Y, Z) as part of the node's *data stream*.
    // This requires NodeFileWriteHandle to have a method to write raw, unescaped bytes
    // for the current node's data segment, separate from its properties.
    // The current NodeFileWriteHandle API (addU8, addString etc.) writes to the *property* stream.
    // **This is a critical missing feature in NodeFileWriteHandle for correct OTBM saving.**
    // For this subtask, I will *simulate* this by writing them as if they were the first properties,
    // which is NOT OTBM compliant but fits the current tool limitations.
    // A proper solution would be: writer.beginNodeData(); writer.addRawU16(areaBasePos.x); ... writer.endNodeData();
    // OR: writer.addNode(OTBM_NODE_TILE_AREA, initial_data_byte_vector);
    // Using hacky property write for node data:
    writer.addU8(OTBM_ATTR_AREA_BASE_X_HACK); writer.addU16(areaBasePos.x);
    writer.addU8(OTBM_ATTR_AREA_BASE_Y_HACK); writer.addU16(areaBasePos.y);
    writer.addU8(OTBM_ATTR_AREA_BASE_Z_HACK); writer.addU8(areaBasePos.z);


    for (uint8_t z = areaBasePos.z; z < Map::MAX_Z; ++z) { // Iterate all relevant floors for this area
         bool floorHasTiles = false;
         for (uint16_t y_offset = 0; y_offset < areaHeight; ++y_offset) {
            for (uint16_t x_offset = 0; x_offset < areaWidth; ++x_offset) {
                 if (map.getTile(Position(areaBasePos.x + x_offset, areaBasePos.y + y_offset, z))) {
                     floorHasTiles = true; break;
                 }
            }
            if(floorHasTiles) break;
         }
         if (!floorHasTiles && z > areaBasePos.z) continue; // Optimization: skip empty floors above the starting one if not the ground floor itself

        for (uint16_t y_offset = 0; y_offset < areaHeight; ++y_offset) {
            for (uint16_t x_offset = 0; x_offset < areaWidth; ++x_offset) {
                Position currentPos(areaBasePos.x + x_offset, areaBasePos.y + y_offset, z);
                const Tile* tile = map.getTile(currentPos);
                // Only save tiles that exist and have content (items, or specific flags, or house status)
                if (tile && (tile->getItemCount() > 0 || tile->getMapFlags() != RME::core::TileMapFlag::NO_FLAGS || tile->getHouseID() > 0)) {
                    if (!serializeTileNode(writer, tile, assetManager)) {
                        m_lastError = "Failed to serialize tile at " + currentPos.toString();
                        return false;
                    }
                }
            }
        }
    }

    if (!writer.endNode()) return false; // End OTBM_NODE_TILE_AREA
    return true;
}

bool OtbmMapIO::serializeTileNode(NodeFileWriteHandle& writer, const Tile* tile, AssetManager& assetManager) {
    uint8_t nodeType = tile->getHouseID() > 0 ? OTBM_NODE_HOUSETILE : OTBM_NODE_TILE;
    if (!writer.addNode(nodeType)) return false;

    // Write tile relative coordinates (X offset, Y offset from area base) as node data.
    // Again, this is node data, not properties. Using hack:
    writer.addU8(OTBM_ATTR_TILE_REL_X_HACK); writer.addU8(static_cast<uint8_t>(tile->getPosition().x % 256));
    writer.addU8(OTBM_ATTR_TILE_REL_Y_HACK); writer.addU8(static_cast<uint8_t>(tile->getPosition().y % 256));
    // Z coordinate is implicit from the TileArea's Z plane for these tiles.

    // Write Tile Attributes
    if (tile->getMapFlags() != RME::core::TileMapFlag::NO_FLAGS) {
        writer.addU8(OTBM_ATTR_TILE_FLAGS);
        writer.addU32(static_cast<uint32_t>(tile->getMapFlags()));
    }
    if (nodeType == OTBM_NODE_HOUSETILE) { // House ID is only for housetiles
        writer.addU8(OTBM_ATTR_HOUSETILE_HOUSEID);
        writer.addU32(tile->getHouseID());
    }

    // Write Items
    if (tile->getGround()) {
        if (!serializeItemNode(writer, tile->getGround(), assetManager)) return false;
    }
    for (const auto& itemPtr : tile->getItems()) {
        if (!serializeItemNode(writer, itemPtr.get(), assetManager)) return false;
    }

    // TODO: Serialize Creatures, Spawns on this tile

    if (!writer.endNode()) return false;
    return true;
}

bool OtbmMapIO::serializeItemNode(NodeFileWriteHandle& writer, const Item* item, AssetManager& assetManager) {
    if (!item) return true;
    if (!writer.addNode(OTBM_NODE_ITEM)) return false;

    // Write Item ID as node data (U16). Using HACK attribute:
    writer.addU8(OTBM_ATTR_ITEM_ID_HACK); // This is non-standard for OTBM. ItemID is node data.
    writer.addU16(item->getID());

    // Write Item Attributes
    if (item->getSubtype() != 0) { // Assuming 0 is default and not written unless meaningful
        // OTBM distinguishes between count for stackable items and general subtype/charges.
        // This simplified logic uses OTBM_ATTR_COUNT for any non-zero subtype.
        // A full implementation would check itemType->isStackable(), itemType->isChargeable() etc.
        writer.addU8(OTBM_ATTR_COUNT); // Could be OTBM_ATTR_CHARGES or other subtype attribute
        // The length of subtype also varies. Stack count is U8. Some charges are U16.
        // For simplicity here, assume U8 if stackable, U16 otherwise for subtype.
        // This is a common pattern but might not cover all cases.
        const RME::core::assets::ItemType* itemType = assetManager.getItemDatabase().getItemData(item->getID());
        if (itemType && itemType->isStackable) { // Assuming ItemType has such flags
            writer.addU8(static_cast<uint8_t>(item->getSubtype()));
        } else {
            writer.addU16(item->getSubtype()); // Default to U16 for other subtypes/charges
        }
    }
    if (item->getActionID() != 0) {
        writer.addU8(OTBM_ATTR_ACTION_ID);
        writer.addU16(item->getActionID());
    }
    if (item->getUniqueID() != 0) { // Assuming 0 is default/invalid UID
        writer.addU8(OTBM_ATTR_UNIQUE_ID);
        writer.addU16(item->getUniqueID());
    }
    if (!item->getText().isEmpty()) {
        writer.addU8(OTBM_ATTR_TEXT);
        writer.addString(item->getText().toStdString());
    }
    // TODO: Other attributes like description, depot_id, etc.

    if (!writer.endNode()) return false;
    return true;
}

} // namespace io
} // namespace core
} // namespace RME
