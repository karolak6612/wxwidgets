#include "core/io/OtbmMapIO.h"
#include "core/io/DiskNodeFileReadHandle.h"
#include "core/io/DiskNodeFileWriteHandle.h"
#include "core/io/BinaryNode.h"
#include "core/io/otbm_constants.h"

#include "core/Map.h" // Full definition
#include "core/Tile.h"
#include "core/Item.h" // Full definition
#include "core/Position.h"
#include "core/assets/AssetManager.h" // Full definition
#include "core/settings/AppSettings.h" // Full definition
#include "core/navigation/WaypointData.h"
#include "core/houses/HouseData.h"      // Already in OtbmMapIO.h, but good for context
#include "core/world/TownData.h"        // Already in OtbmMapIO.h
#include "core/creatures/Creature.h"    // For RME::core::creatures::Creature
#include "core/creatures/Outfit.h"      // For RME::core::creatures::Outfit
#include "core/assets/CreatureData.h"   // For RME::core::assets::CreatureData
#include "core/assets/ItemData.h"       // For RME::core::assets::ItemType and isCreatureLookItem
#include "core/Container.h" // Required for casting and accessing items in a container


#include <QFile>         // For checking file existence etc.
#include <QDataStream>   // For writing node data for area/tile coordinates
#include <QDebug>        // For logging errors and warnings

// QtZlib for qUncompress/qCompress is handled by NodeFileRead/WriteHandle now

namespace RME {
namespace core {
namespace io {

OtbmMapIO::OtbmMapIO() {
    // Constructor
}

OtbmMapIO::~OtbmMapIO() {
    // Destructor
}

QStringList OtbmMapIO::getSupportedFileExtensions() const {
    return {"*.otbm"};
}

QString OtbmMapIO::getFormatName() const {
    return "Open Tibia Binary Map";
}

// --- Loading Implementation ---
bool OtbmMapIO::loadMap(const QString& filePath, Map& map, AssetManager& assetManager, AppSettings& settings) {
    m_lastError.clear();
    map.setChanged(false); // Reset changed status on new load attempt

    DiskNodeFileReadHandle readHandle(filePath);

    if (!readHandle.isOk()) {
        m_lastError = QString("Failed to open or read initial part of map file: %1. Error code: %2 (%3)")
                          .arg(filePath)
                          .arg(readHandle.getError())
                          .arg(readHandle.isOk() ? "No specific message" : "Handle not OK");
        qWarning() << "OtbmMapIO::loadMap:" << m_lastError;
        return false;
    }

    BinaryNode* rootOtbmNode = readHandle.getRootNode();
    if (!rootOtbmNode || !readHandle.isOk()) {
        m_lastError = QString("Failed to read OTBM root node structure. Error: %1").arg(readHandle.getError());
        qWarning() << "OtbmMapIO::loadMap:" << m_lastError;
        return false;
    }

    // The first actual child of the conceptual root should be OTBM_NODE_MAP_DATA
    BinaryNode* mapDataNode = rootOtbmNode->getChild();

    if (!mapDataNode || !readHandle.isOk()) {
        m_lastError = "OTBM: MAP_DATA node not found or error reading it.";
        qWarning() << "OtbmMapIO::loadMap:" << m_lastError;
        return false;
    }

    if (mapDataNode->getType() != OTBM_NODE_MAP_DATA) {
        m_lastError = QString("OTBM: Expected MAP_DATA node (type %1), got type %2.")
                        .arg(OTBM_NODE_MAP_DATA).arg(mapDataNode->getType());
        qWarning() << "OtbmMapIO::loadMap:" << m_lastError;
        return false;
    }

    if (!parseMapDataNode(mapDataNode, map, assetManager, settings)) { // Pass assetManager
        // m_lastError would be set by parseMapDataNode
        return false;
    }

    // Check for any remaining errors from the read handle after parsing is complete
    if (!readHandle.isOk()) {
        m_lastError = QString("Error encountered during map file parsing. Error code: %1").arg(readHandle.getError());
        qWarning() << "OtbmMapIO::loadMap:" << m_lastError;
        return false;
    }

    map.setChanged(false); // Successfully loaded, reset changed flag
    return true;
}

// --- Creature Specific Parsing/Serialization ---
bool OtbmMapIO::parseCreatureNode(BinaryNode* creatureNode, Tile* tile, AssetManager& assetManager, AppSettings& settings) {
    if (!tile) {
        m_lastError = "parseCreatureNode: Called with null tile.";
        qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
        return false;
    }

    std::string creatureName_std;
    creatureNode->resetReadOffset();

    uint8_t firstAttr;
    // Assuming OTBM_ATTR_CREATURE_NAME is mandatory and first.
    if (!creatureNode->getU8(firstAttr) || firstAttr != OTBM_ATTR_CREATURE_NAME) {
        m_lastError = "Failed to read OTBM_ATTR_CREATURE_NAME or it's not the first attribute for creature on tile " + tile->getPosition().toString();
        qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
        return false;
    }
    if (!creatureNode->getString(creatureName_std)) {
         m_lastError = "Failed to read creature name string for creature on tile " + tile->getPosition().toString();
         qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
         return false;
    }

    QString creatureName = QString::fromStdString(creatureName_std);
    const RME::core::assets::CreatureData* creatureData = assetManager.getCreatureData(creatureName);

    if (!creatureData) {
        QString msg = QString("Creature type '%1' not found in AssetManager. Tile: %2").arg(creatureName).arg(tile->getPosition().toString());
        bool skipUnknown = settings.getValue(RME::core::settings::Config::Key::SKIP_UNKNOWN_ITEMS, QVariant(true)).toBool();
        if (skipUnknown) {
            qWarning() << "OtbmMapIO::parseCreatureNode:" << msg << "Skipping creature.";
            while(creatureNode->hasMoreProperties()){
                uint8_t dummyAttr;
                if(!creatureNode->getU8(dummyAttr)) { break; }
                // This skip logic is simplified. A robust solution requires BinaryNode to handle skipping unknown attribute types.
            }
            return true;
        }
        m_lastError = msg;
        qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
        return false;
    }

    auto newCreature = std::make_unique<RME::core::creatures::Creature>(creatureData, tile->getPosition());
    if (!newCreature) {
        m_lastError = "Failed to create Creature instance for: " + creatureName;
        qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
        return false;
    }

    RME::core::creatures::Outfit currentOutfit = newCreature->getOutfit();
    bool outfitWasOverridden = false;

    while(creatureNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!creatureNode->getU8(attribute)) { m_lastError = "Failed to read creature outfit attribute type."; return false; }

        switch(attribute) {
            case OTBM_ATTR_CREATURE_OUTFIT_LOOK_TYPE:
                if (!creatureNode->getU16(currentOutfit.lookType)) { m_lastError="Failed to read looktype for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_ITEM_ID:
                if (!creatureNode->getU16(currentOutfit.lookItem)) { m_lastError="Failed to read lookitem for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_HEAD:
                if (!creatureNode->getU8(currentOutfit.lookHead)) { m_lastError="Failed to read lookhead for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_BODY:
                if (!creatureNode->getU8(currentOutfit.lookBody)) { m_lastError="Failed to read lookbody for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_LEGS:
                if (!creatureNode->getU8(currentOutfit.lookLegs)) { m_lastError="Failed to read looklegs for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_FEET:
                if (!creatureNode->getU8(currentOutfit.lookFeet)) { m_lastError="Failed to read lookfeet for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_ADDONS:
                if (!creatureNode->getU8(currentOutfit.lookAddons)) { m_lastError="Failed to read lookaddons for " + creatureName; return false; } outfitWasOverridden = true; break;
            case OTBM_ATTR_CREATURE_OUTFIT_MOUNT_ID:
                if (!creatureNode->getU16(currentOutfit.lookMount)) { m_lastError="Failed to read lookmount for " + creatureName; return false; } outfitWasOverridden = true; break;
            default:
                m_lastError = QString("Unknown attribute type %1 for CREATURE node (%2) on tile %3").arg(attribute).arg(creatureName).arg(tile->getPosition().toString());
                qWarning() << "OtbmMapIO::parseCreatureNode:" << m_lastError;
                return false;
        }
    }

    if (outfitWasOverridden) {
        newCreature->setOutfit(currentOutfit);
    }

    tile->setCreature(std::move(newCreature));
    return true;
}

bool OtbmMapIO::serializeCreatureNode(NodeFileWriteHandle& writer, const RME::core::creatures::Creature* creature, AssetManager& assetManager, AppSettings& settings) {
    if (!creature || !creature->getType()) {
        m_lastError = "serializeCreatureNode: Null creature or creature type.";
        qWarning() << m_lastError;
        return false;
    }

    if (!writer.addNode(OTBM_NODE_CREATURE, false)) {
        m_lastError = "Failed to start CREATURE node for: " + creature->getName();
        return false;
    }

    if (!writer.addU8(OTBM_ATTR_CREATURE_NAME) || !writer.addString(creature->getName())) {
         m_lastError = "Failed to write creature name for: " + creature->getName(); return false;
    }

    const RME::core::creatures::Outfit& currentOutfit = creature->getOutfit();
    const RME::core::assets::CreatureData* creatureTypeData = creature->getType();
    const RME::core::creatures::Outfit& defaultOutfit = creatureTypeData->defaultOutfit;

    if (currentOutfit.lookType != defaultOutfit.lookType) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_LOOK_TYPE) || !writer.addU16(currentOutfit.lookType)) {m_lastError="Ser: Failed looktype for " + creature->getName(); return false;}
    }

    const RME::core::assets::ItemType* itemTypeForLook = assetManager.getItemData(currentOutfit.lookType);
    if (itemTypeForLook && itemTypeForLook->isCreatureLookItem()) {
        if (currentOutfit.lookItem != 0 && currentOutfit.lookItem != defaultOutfit.lookItem) {
             if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_ITEM_ID) || !writer.addU16(currentOutfit.lookItem)) {m_lastError="Ser: Failed lookitem for " + creature->getName(); return false;}
        }
    } else {
        if (currentOutfit.lookItem != 0 && currentOutfit.lookItem != defaultOutfit.lookItem) {
             if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_ITEM_ID) || !writer.addU16(currentOutfit.lookItem)) {m_lastError="Ser: Failed lookitem for non-item type on " + creature->getName(); return false;}
        }
    }

    if (currentOutfit.lookHead != defaultOutfit.lookHead) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_HEAD) || !writer.addU8(currentOutfit.lookHead)) {m_lastError="Ser: Failed lookhead for " + creature->getName(); return false;}
    }
    if (currentOutfit.lookBody != defaultOutfit.lookBody) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_BODY) || !writer.addU8(currentOutfit.lookBody)) {m_lastError="Ser: Failed lookbody for " + creature->getName(); return false;}
    }
    if (currentOutfit.lookLegs != defaultOutfit.lookLegs) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_LEGS) || !writer.addU8(currentOutfit.lookLegs)) {m_lastError="Ser: Failed looklegs for " + creature->getName(); return false;}
    }
    if (currentOutfit.lookFeet != defaultOutfit.lookFeet) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_FEET) || !writer.addU8(currentOutfit.lookFeet)) {m_lastError="Ser: Failed lookfeet for " + creature->getName(); return false;}
    }
    if (currentOutfit.lookAddons != defaultOutfit.lookAddons) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_ADDONS) || !writer.addU8(currentOutfit.lookAddons)) {m_lastError="Ser: Failed lookaddons for " + creature->getName(); return false;}
    }
    if (currentOutfit.lookMount != defaultOutfit.lookMount) {
        if(!writer.addU8(OTBM_ATTR_CREATURE_OUTFIT_MOUNT_ID) || !writer.addU16(currentOutfit.lookMount)) {m_lastError="Ser: Failed lookmount for " + creature->getName(); return false;}
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end CREATURE node for: " + creature->getName(); return false;
    }
    return true;
}

// Corrected signature to include AssetManager
bool OtbmMapIO::parseMapDataNode(BinaryNode* mapDataNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    mapDataNode->resetReadOffset();

    RME::core::ClientVersionInfo clientVersionInfo; // Initialize struct to hold version info
    clientVersionInfo.major = 0;
    clientVersionInfo.minor = 0;
    clientVersionInfo.build = 0;

    while(mapDataNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!mapDataNode->getU8(attribute)) {
            m_lastError = "Failed to read map data attribute type.";
            qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError;
            return false;
        }

        switch(attribute) {
            case OTBM_ATTR_DESCRIPTION: {
                QString description;
                if (!mapDataNode->getString(description)) {
                    m_lastError = "Failed to read map description string.";
                    qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError;
                    return false;
                }
                map.setDescription(description);
                break;
            }
            case OTBM_ATTR_EXT_HOUSE_FILE: {
                QString houseFile;
                if (!mapDataNode->getString(houseFile)) {
                    m_lastError = "Failed to read house file string.";
                    qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError;
                    return false;
                }
                map.setHouseFile(houseFile);
                break;
            }
            case OTBM_ATTR_EXT_SPAWN_FILE: {
                QString spawnFile;
                if (!mapDataNode->getString(spawnFile)) {
                    m_lastError = "Failed to read spawn file string.";
                    qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError;
                    return false;
                }
                map.setSpawnFile(spawnFile);
                break;
            }
            // Map version attributes (Major, Minor, Build) for client features
            case OTBM_ATTR_MAP_VERSION_MAJOR: {
                uint32_t majorVer;
                if (!mapDataNode->getU32(majorVer)) { m_lastError = "Failed to read OTBM_ATTR_MAP_VERSION_MAJOR"; qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError; return false; }
                clientVersionInfo.major = majorVer;
                break;
            }
            case OTBM_ATTR_MAP_VERSION_MINOR: {
                uint32_t minorVer;
                if (!mapDataNode->getU32(minorVer)) { m_lastError = "Failed to read OTBM_ATTR_MAP_VERSION_MINOR"; qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError; return false; }
                clientVersionInfo.minor = minorVer;
                break;
            }
            case OTBM_ATTR_MAP_VERSION_BUILD: {
                 uint32_t buildVer;
                if (!mapDataNode->getU32(buildVer)) { m_lastError = "Failed to read OTBM_ATTR_MAP_VERSION_BUILD"; qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError; return false; }
                clientVersionInfo.build = buildVer;
                break;
            }
            default: {
                // This is where robust attribute skipping is needed in BinaryNode
                // For now, if an unknown attribute is encountered, it's an error.
                m_lastError = QString("Unknown attribute type %1 for MAP_DATA node.").arg(attribute);
                qWarning() << "OtbmMapIO::parseMapDataNode:" << m_lastError;
                // To skip: need to know attribute's data type or if it has a length prefix.
                // BinaryNode::skipBytes(length) would be used if length is known.
                // For now, strict parsing: unknown attribute is an error.
                return false;
            }
        }
    }
    map.setClientVersionInfo(clientVersionInfo); // Set the parsed version info on the map object

    // Parse child nodes of MAP_DATA (Tile Areas, Towns, Waypoints)
    BinaryNode* childNode = mapDataNode->getChild();
    while(childNode) {
        switch (childNode->getType()) {
            case OTBM_NODE_TILE_AREA:
                if (!parseTileAreaNode(childNode, map, assetManager, settings)) return false;
                break;
            case OTBM_NODE_TOWNS:
                if (!parseTownsContainerNode(childNode, map, assetManager, settings)) return false;
                break;
            case OTBM_NODE_HOUSES: // Added case for houses
                if (!parseHousesContainerNode(childNode, map, assetManager, settings)) return false;
                break;
            case OTBM_NODE_WAYPOINTS:
                if (!parseWaypointsContainerNode(childNode, map, assetManager, settings)) return false;
                break;
            default:
                qWarning() << "OtbmMapIO: Unknown child node type" << childNode->getType() << "in MAP_DATA.";
                // Potentially skip unknown child nodes if parser is to be lenient.
                break;
        }
        childNode = mapDataNode->getNextChild(); // Get next child of mapDataNode
    }
    return true;
}

// Added settings to signature
bool OtbmMapIO::parseTileAreaNode(BinaryNode* tileAreaNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    // Tile Area node data contains: BaseX (U16), BaseY (U16), BaseZ (U8)
    const QByteArray& nodeData = tileAreaNode->getNodeData();
    if (nodeData.size() < 5) {
        m_lastError = "TileArea node data too short for coordinates.";
        qWarning() << "OtbmMapIO::parseTileAreaNode:" << m_lastError;
        return false;
    }

    QDataStream stream(nodeData); // Use QDataStream for safe parsing from QByteArray
    stream.setByteOrder(QDataStream::LittleEndian);
    quint16 area_base_x, area_base_y;
    quint8 area_base_z;
    stream >> area_base_x >> area_base_y >> area_base_z;
    if (stream.status() != QDataStream::Ok) {
        m_lastError = "Failed to stream TileArea coordinates from node data.";
        qWarning() << "OtbmMapIO::parseTileAreaNode:" << m_lastError;
        return false;
    }
    Position areaBasePos(area_base_x, area_base_y, area_base_z);

    // Ensure map dimensions are large enough. Map class should handle this.
    // map.ensureDimensions(area_base_x + 255, area_base_y + 255, area_base_z);

    BinaryNode* tileNode = tileAreaNode->getChild();
    while(tileNode) {
        if (tileNode->getType() == OTBM_NODE_TILE || tileNode->getType() == OTBM_NODE_HOUSETILE) {
            if (!parseTileNode(tileNode, map, assetManager, areaBasePos, settings)) return false; // Pass settings
        } else {
            qWarning() << "OtbmMapIO: Unknown child node type" << tileNode->getType() << "in TILE_AREA at base" << areaBasePos.toString();
        }
        tileNode = tileAreaNode->getNextChild(); // Get next child of tileAreaNode
    }
    return true;
}

// Added settings to signature
bool OtbmMapIO::parseTileNode(BinaryNode* tileNode, Map& map, AssetManager& assetManager, const Position& areaBasePos, AppSettings& settings) {
    // Tile node data contains: RelX (U8), RelY (U8)
    const QByteArray& nodeData = tileNode->getNodeData();
    if (nodeData.size() < 2) {
        m_lastError = "Tile node data too short for relative coordinates.";
        qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
        return false;
    }
    Position tilePos = areaBasePos;
    // tilePos.rx += static_cast<uint8_t>(nodeData[0]); // Assuming Position has rx, ry for relative
    // tilePos.ry += static_cast<uint8_t>(nodeData[1]);
    // If Position only stores absolute, then:
    tilePos.setX(areaBasePos.x() + static_cast<uint8_t>(nodeData[0]));
    tilePos.setY(areaBasePos.y() + static_cast<uint8_t>(nodeData[1]));
    // map.toAbsolute(tilePos); // Convert relative to absolute if needed by Map - already absolute now

    Tile* currentTile = map.getOrCreateTile(tilePos);
    if (!currentTile) {
        m_lastError = "Failed to get or create tile at " + tilePos.toString();
        qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
        return false;
    }

    tileNode->resetReadOffset();
    while(tileNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!tileNode->getU8(attribute)) {
            m_lastError = "Failed to read tile attribute type for tile at " + tilePos.toString();
            qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
            return false;
        }
        switch(attribute) {
            case OTBM_ATTR_TILE_FLAGS: {
                quint32 flagsValue;
                if (!tileNode->getU32(flagsValue)) {
                    m_lastError = "Failed to read tile flags for tile at " + tilePos.toString();
                    qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                    return false;
                }
                // Assuming Tile::setMapFlags takes the raw uint32_t value
                currentTile->setMapFlags(flagsValue);
                break;
            }
            case OTBM_ATTR_HOUSETILE_HOUSEID: // This constant is from wx, ensure it's correct in our otbm_constants.h
                                          // Current otbm_constants.h has OTBM_ATTR_HOUSETILE_HOUSEID = 0x11
                if (tileNode->getType() == OTBM_NODE_HOUSETILE) {
                    quint32 houseId;
                    if (!tileNode->getU32(houseId)) {
                        m_lastError = "Failed to read house ID for housetile at " + tilePos.toString();
                        qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                        return false;
                    }
                    currentTile->setHouseID(houseId);
                } else {
                    m_lastError = "OTBM_ATTR_HOUSETILE_HOUSEID found on non-HOUSETILE node at " + tilePos.toString();
                    qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                    // Strict parsing: error out. Could also skip if lenient.
                    return false;
                }
                break;
            // --- Start Tile Spawn Data Parsing ---
            case OTBM_ATTR_SPAWN_RADIUS: {
                uint16_t radius_u16;
                if (!tileNode->getU16(radius_u16)) {
                    m_lastError = "Failed to read spawn radius for tile at " + tilePos.toString();
                    qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                    return false;
                }
                currentTile->setSpawnRadius(static_cast<int>(radius_u16));
                break;
            }
            case OTBM_ATTR_SPAWN_INTERVAL: {
                uint32_t interval_u32;
                if (!tileNode->getU32(interval_u32)) {
                    m_lastError = "Failed to read spawn interval for tile at " + tilePos.toString();
                    qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                    return false;
                }
                currentTile->setSpawnIntervalSeconds(static_cast<int>(interval_u32));
                break;
            }
            case OTBM_ATTR_SPAWN_CREATURE_LIST: {
                std::string creatureList_std;
                if (!tileNode->getString(creatureList_std)) {
                    m_lastError = "Failed to read spawn creature list for tile at " + tilePos.toString();
                    qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                    return false;
                }
                QStringList creatures = QString::fromStdString(creatureList_std).split(',', Qt::SkipEmptyParts);
                currentTile->setSpawnCreatureList(creatures);
                break;
            }
            // --- End Tile Spawn Data Parsing ---
            default: {
                m_lastError = QString("Unknown attribute type %1 for TILE/HOUSETILE node at %2").arg(attribute).arg(tilePos.toString());
                qWarning() << "OtbmMapIO::parseTileNode:" << m_lastError;
                return false; // Strict parsing
            }
        }
    }

    // Parse child item/creature nodes for this tile
    BinaryNode* itemOrCreatureNode = tileNode->getChild();
    while(itemOrCreatureNode) {
        switch(itemOrCreatureNode->getType()) {
            case OTBM_NODE_ITEM:
                if (!parseItemNode(itemOrCreatureNode, currentTile, assetManager, settings)) return false;
                break;
            case OTBM_NODE_CREATURE:
                if (!parseCreatureNode(itemOrCreatureNode, currentTile, assetManager, settings)) return false;
                break;
            default:
                qWarning() << "OtbmMapIO: Unknown child node type" << itemOrCreatureNode->getType() << "in TILE data for" << tilePos.toString();
                break;
        }
        itemOrCreatureNode = tileNode->getNextChild(); // Get next child of tileNode
    }
    // currentTile->update(); // If Tile has an update method after modifications
    return true;
}

// Added settings to parseItemNode signature
bool OtbmMapIO::parseItemNode(BinaryNode* itemNode, Tile* tile, AssetManager& assetManager, AppSettings& settings) {
    // Item node data contains: ItemID (U16)
    const QByteArray& nodeData = itemNode->getNodeData();
    if (nodeData.size() < 2) {
        m_lastError = "Item node data too short for ItemID on tile " + tile->getPosition().toString();
        qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
        return false;
    }
    quint16 itemId = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(nodeData.constData()));

    const RME::core::assets::ItemType* itemType = assetManager.getItemData(itemId);
    if (!itemType) {
        QString msg = QString("Item ID %1 not found in ItemDatabase. Position: %2").arg(itemId).arg(tile->getPosition().toString());
        bool skipUnknown = settings.getValue(Config::Key::SKIP_UNKNOWN_ITEMS, true).toBool(); // Example setting
        if (skipUnknown) {
            qWarning() << "OtbmMapIO::parseItemNode:" << msg << "Skipping item.";
            return true; // Skip this item successfully
        }
        m_lastError = msg;
        qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
        return false; // Fail if not skipping
    }

    std::unique_ptr<Item> newItem = Item::create(itemId, &assetManager); // Assuming Item::create takes AssetManager now
    if (!newItem) {
        m_lastError = "Failed to create item instance for ID: " + QString::number(itemId) + " on tile " + tile->getPosition().toString();
        qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
        return false;
    }

    itemNode->resetReadOffset();
    while(itemNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!itemNode->getU8(attribute)) {
            m_lastError = "Failed to read item attribute type for item ID " + QString::number(itemId) + " on tile " + tile->getPosition().toString();
            qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
            return false;
        }
        switch(attribute) {
            case OTBM_ATTR_COUNT: // Also charges for some items, use itemType to differentiate if needed
            case OTBM_ATTR_CHARGES: { // Assuming CHARGES uses same value as COUNT from constants for now
                uint8_t count_charges_u8; // Default to U8
                // This is tricky: OTBM can use U8 or U16 for count/charges based on item type or OTBM version.
                // The wxOTBM code has specific logic for this, often checking itemType.stackable or isFluidContainer.
                // For simplicity here, let's assume U8 for now. A real implementation needs more nuance.
                if (!itemNode->getU8(count_charges_u8)) {
                    m_lastError = "Failed to read item count/charges for item ID " + QString::number(itemId);
                    qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
                    return false;
                }
                newItem->setSubtype(count_charges_u8);
                break;
            }
            case OTBM_ATTR_ACTION_ID: {
                quint16 actionId;
                if (!itemNode->getU16(actionId)) { /* error */ return false; }
                newItem->setActionID(actionId);
                break;
            }
            case OTBM_ATTR_UNIQUE_ID: {
                quint16 uniqueId;
                if (!itemNode->getU16(uniqueId)) { /* error */ return false; }
                newItem->setUniqueID(uniqueId);
                break;
            }
            case OTBM_ATTR_TEXT: {
                QString text;
                if (!itemNode->getString(text)) { /* error */ return false; }
                newItem->setText(text);
                break;
            }
            // Additional item attributes are now implemented below
            case OTBM_ATTR_TELE_DEST_X: {
                uint16_t x; if (!itemNode->getU16(x)) { m_lastError = "Failed to read tele_dest_x"; return false; }
                Position dest = newItem->getTeleportDestination(); dest.setX(x); newItem->setTeleportDestination(dest); break;
            }
            case OTBM_ATTR_TELE_DEST_Y: {
                uint16_t y; if (!itemNode->getU16(y)) { m_lastError = "Failed to read tele_dest_y"; return false; }
                Position dest = newItem->getTeleportDestination(); dest.setY(y); newItem->setTeleportDestination(dest); break;
            }
            case OTBM_ATTR_TELE_DEST_Z: {
                uint8_t z; if (!itemNode->getU8(z)) { m_lastError = "Failed to read tele_dest_z"; return false; }
                Position dest = newItem->getTeleportDestination(); dest.setZ(z); newItem->setTeleportDestination(dest); break;
            }
            case OTBM_ATTR_DEPOT_ID: {
                uint16_t depotId; if (!itemNode->getU16(depotId)) { m_lastError = "Failed to read depot_id"; return false; }
                newItem->setDepotID(depotId); break;
            }
            // OTBM_ATTR_ITEM_CHARGES is often the same as OTBM_ATTR_COUNT or subtype.
            // If it's a distinct attribute for specific items, handle it. Otherwise, covered by OTBM_ATTR_CHARGES.
            // For this task, assuming OTBM_ATTR_CHARGES (already handled by setSubtype) is sufficient.
            // If a specific OTBM_ATTR_ITEM_CHARGES constant exists and is used:
            // case OTBM_ATTR_ITEM_CHARGES: {
            //     uint16_t charges; if (!itemNode->getU16(charges)) { m_lastError = "Failed to read item_charges"; return false; }
            //     newItem->setCharges(charges); break;
            // }
            case OTBM_ATTR_WRITTEN_TEXT: {
                std::string text_std; if (!itemNode->getString(text_std)) { m_lastError = "Failed to read written_text"; return false; }
                newItem->setWrittenText(QString::fromStdString(text_std)); break;
            }
            case OTBM_ATTR_WRITTEN_BY: {
                std::string author_std; if (!itemNode->getString(author_std)) { m_lastError = "Failed to read written_by"; return false; }
                newItem->setAuthor(QString::fromStdString(author_std)); break;
            }
            case OTBM_ATTR_DATE: {
                uint32_t date; if (!itemNode->getU32(date)) { m_lastError = "Failed to read date"; return false; }
                newItem->setWrittenDate(date); break;
            }
            case OTBM_ATTR_LIGHT_LEVEL: { // OTBM_LIGHT_LEVEL in some versions
                uint16_t level; if (!itemNode->getU16(level)) { m_lastError = "Failed to read light_level"; return false; }
                newItem->setLightLevel(level); break;
            }
            case OTBM_ATTR_LIGHT_COLOR: { // OTBM_LIGHT_COLOR in some versions
                uint16_t color; if (!itemNode->getU16(color)) { m_lastError = "Failed to read light_color"; return false; }
                newItem->setLightColor(color); break;
            }
            case OTBM_ATTR_DURATION: { // OTBM_DURATION in some versions
                uint32_t duration; if (!itemNode->getU32(duration)) { m_lastError = "Failed to read duration"; return false; }
                newItem->setDuration(duration); break;
            }
            case OTBM_ATTR_ARTICLE: {
                std::string article_std; if(!itemNode->getString(article_std)) { m_lastError = "Failed to read article"; return false; }
                newItem->setAttribute("article", QString::fromStdString(article_std)); break;
            }
            // OTBM_ATTR_DESCRIPTION_TEXT might be OTBM_ATTR_TEXT, or a separate one for items with specific descriptions
            // If it's the same as OTBM_ATTR_TEXT, it's handled. If separate:
            // case OTBM_ATTR_DESCRIPTION_TEXT: {
            //    std::string desc_std; if(!itemNode->getString(desc_std)) { m_lastError = "Failed to read desc_text"; return false; }
            //    newItem->setDescription(QString::fromStdString(desc_std)); break; // Assuming Item has setDescription
            // }
            default: {
                m_lastError = QString("Unknown attribute type %1 for ITEM node (ID: %2) on tile %3").arg(attribute).arg(itemId).arg(tile->getPosition().toString());
                qWarning() << "OtbmMapIO::parseItemNode:" << m_lastError;
                // Add robust attribute skipping here if BinaryNode supports it.
                // For now, returning false for strictness.
                return false;
            }
        }
            }

    if (itemType && itemType->isContainer()) {
        BinaryNode* childItemNode = itemNode->getChild();
        if (childItemNode) {
            qWarning() << "OtbmMapIO::parseItemNode: Recursive parsing of items inside container ID " << itemId << " requires a dedicated helper function and is NOT fully implemented in this pass. Child items are being skipped to prevent errors.";
            while(childItemNode) {
                // To properly skip, each child node must be fully processed or skipped.
                // BinaryNode::getNextChild moves to the next sibling. If children have children, this simple loop isn't enough.
                // This requires BinaryNode to handle tree traversal for skipping, or a recursive skip.
                // For now, this consumes direct children (siblings).
                childItemNode = itemNode->getNextChild();
            }
            }
        }
    tile->addItem(std::move(newItem));
    return true;
}


// --- Saving Implementation ---
bool OtbmMapIO::saveMap(const QString& filePath, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    m_lastError.clear();
    DiskNodeFileWriteHandle writer(filePath);

    if (!writer.isOk()) {
        m_lastError = QString("SaveMap: Failed to open file for writing: %1. Error: %2").arg(filePath).arg(writer.getError());
        qWarning() << "OtbmMapIO::saveMap:" << m_lastError;
        return false;
    }

    // DiskNodeFileWriteHandle constructor already wrote the 4-byte OTBM identifier (e.g. 0x00000000).

    // Root OTBM Node (Type 0, no explicit attributes usually, properties not compressed)
    if (!writer.addNode(OTBM_NODE_ROOT, false)) {
        m_lastError = "SaveMap: Failed to write root node start. Error: " + QString::number(writer.getError());
        qWarning() << "OtbmMapIO::saveMap:" << m_lastError;
        return false;
    }

    if (!serializeMapDataNode(writer, map, assetManager, settings)) { // Pass assetManager
        // m_lastError set by callee
        return false;
    }

    if (!writer.endNode()) { // End OTBM_NODE_ROOT
        m_lastError = "SaveMap: Failed to write root node end. Error: " + QString::number(writer.getError());
        qWarning() << "OtbmMapIO::saveMap:" << m_lastError;
        return false;
    }

    if (!writer.flush()) { // Ensure all data is written to disk
        m_lastError = "SaveMap: Failed to flush data to disk. Error: " + QString::number(writer.getError());
        qWarning() << "OtbmMapIO::saveMap:" << m_lastError;
        return false;
    }

    if (!writer.isOk()) {
        m_lastError = "SaveMap: An error occurred during file writing. Error: " + QString::number(writer.getError());
        qWarning() << "OtbmMapIO::saveMap:" << m_lastError;
        return false;
    }

    // map.setChanged(false); // Should be done by caller after successful save
    return true;
}

// Pass AssetManager to serializeMapDataNode
bool OtbmMapIO::serializeMapDataNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    // OTBM_NODE_MAP_DATA - properties usually not compressed
    bool compressMapDataProps = settings.getValue(Config::Key::COMPRESS_MAP_DATA_PROPERTIES, false).toBool(); // Example setting
    if (!writer.addNode(OTBM_NODE_MAP_DATA, compressMapDataProps)) {
        m_lastError = "Failed to start MAP_DATA node.";
        return false;
    }

    // Map Attributes
    if (!map.getDescription().isEmpty()) {
        if (!writer.addU8(OTBM_ATTR_DESCRIPTION) || !writer.addString(map.getDescription())) {
            m_lastError = "Failed to write map description."; return false;
        }
    }
    if (!map.getHouseFile().isEmpty()) {
        if (!writer.addU8(OTBM_ATTR_EXT_HOUSE_FILE) || !writer.addString(map.getHouseFile())) {
            m_lastError = "Failed to write house file attribute."; return false;
        }
    }
    if (!map.getSpawnFile().isEmpty()) {
        if (!writer.addU8(OTBM_ATTR_EXT_SPAWN_FILE) || !writer.addString(map.getSpawnFile())) {
            m_lastError = "Failed to write spawn file attribute."; return false;
        }
    }

    // Write map client version attributes if they are set
    RME::core::ClientVersionInfo versionInfo = map.getClientVersionInfo();
    if (versionInfo.major != 0 || versionInfo.minor != 0 || versionInfo.build != 0) { // Only write if not default
        if (!writer.addU8(OTBM_ATTR_MAP_VERSION_MAJOR) || !writer.addU32(versionInfo.major)) {
            m_lastError = "Failed to write OTBM_ATTR_MAP_VERSION_MAJOR."; return false;
        }
        if (!writer.addU8(OTBM_ATTR_MAP_VERSION_MINOR) || !writer.addU32(versionInfo.minor)) {
            m_lastError = "Failed to write OTBM_ATTR_MAP_VERSION_MINOR."; return false;
        }
        if (!writer.addU8(OTBM_ATTR_MAP_VERSION_BUILD) || !writer.addU32(versionInfo.build)) {
            m_lastError = "Failed to write OTBM_ATTR_MAP_VERSION_BUILD."; return false;
        }
    }

    // Iterate map by 256x256x1 areas (per floor)
    // Ensure map dimensions are positive before iterating
    if (map.getWidth() > 0 && map.getHeight() > 0 && map.getFloors() > 0) {
        for (int z = 0; z < map.getFloors(); ++z) {
            for (int y_base = 0; y_base < map.getHeight(); y_base += 256) {
                for (int x_base = 0; x_base < map.getWidth(); x_base += 256) {
                    // Check if this specific x_base, y_base, z area has any tiles to save
                    bool areaHasTiles = false;
                    for (int y_off = 0; y_off < 256 && (y_base + y_off) < map.getHeight(); ++y_off) {
                        for (int x_off = 0; x_off < 256 && (x_base + x_off) < map.getWidth(); ++x_off) {
                            if (map.getTile(Position(x_base + x_off, y_base + y_off, z))) {
                                areaHasTiles = true; break;
                            }
                        }
                        if (areaHasTiles) break;
                    }

                    if (areaHasTiles) {
                        if (!serializeTileAreaNode(writer, map, Position(x_base, y_base, z), assetManager, settings)) {
                            return false; // m_lastError set by callee
                        }
                    }
                }
            }
        }
    }


    // Serialize Waypoints if any
    if (!map.getWaypoints().isEmpty()) {
        if (!serializeWaypointsContainerNode(writer, map, assetManager, settings)) {
            return false; // m_lastError set by callee
        }
    }
    // Serialize Towns if any
    if (!map.getTowns().isEmpty()) {
        if (!serializeTownsContainerNode(writer, map, assetManager, settings)) {
            return false;
        }
    }
    // Serialize Houses if any
    if (!map.getHouses().isEmpty()) {
        if (!serializeHousesContainerNode(writer, map, assetManager, settings)) {
            return false;
        }
    }

    if (!writer.endNode()) { m_lastError = "Failed to end MAP_DATA node."; return false; }
    return true;
}

// Pass AssetManager and AppSettings
bool OtbmMapIO::serializeTileAreaNode(NodeFileWriteHandle& writer, const Map& map,
                                   const Position& areaBasePos, // This now includes Z
                                   AssetManager& assetManager, AppSettings& settings) {
    // OTBM_NODE_TILE_AREA - properties usually not compressed
    if (!writer.addNode(OTBM_NODE_TILE_AREA, false)) {
        m_lastError = "Failed to start TILE_AREA node for " + areaBasePos.toString() + ".";
        return false;
    }

    // Write area base coordinates (X, Y, Z) as node data
    QByteArray areaCoordsData;
    QDataStream ds(&areaCoordsData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << static_cast<quint16>(areaBasePos.x())
       << static_cast<quint16>(areaBasePos.y())
       << static_cast<quint8>(areaBasePos.z());
    if (!writer.addNodeData(areaCoordsData)) {
        m_lastError = "Failed to write tile area coordinates for " + areaBasePos.toString() + ".";
        return false;
    }

    // Iterate tiles within this 256x256 area for the given floor (areaBasePos.z())
    for (int y_offset = 0; y_offset < 256; ++y_offset) {
        int current_y = areaBasePos.y() + y_offset;
        if (current_y >= map.getHeight()) break;

        for (int x_offset = 0; x_offset < 256; ++x_offset) {
            int current_x = areaBasePos.x() + x_offset;
            if (current_x >= map.getWidth()) break;

            Position currentPos(current_x, current_y, areaBasePos.z());
            const Tile* tile = map.getTile(currentPos);

            // Only save non-empty tiles or tiles that have significant properties (flags, houseID)
            if (tile && (!tile->isEmpty() || tile->getMapFlags() != 0 || tile->getHouseID() != 0)) {
                if (!serializeTileNode(writer, tile, assetManager, settings)) { // Pass assetManager & settings
                    m_lastError = "Failed to serialize tile at " + currentPos.toString() + ". " + m_lastError;
                    return false;
                }
            }
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end TILE_AREA node for " + areaBasePos.toString() + ".";
        return false;
    }
    return true;
}

// Pass AssetManager and AppSettings
bool OtbmMapIO::serializeTileNode(NodeFileWriteHandle& writer, const Tile* tile, AssetManager& assetManager, AppSettings& settings) {
    uint8_t nodeType = tile->getHouseID() > 0 ? OTBM_NODE_HOUSETILE : OTBM_NODE_TILE;
    // Tile attributes usually not compressed
    if (!writer.addNode(nodeType, false)) {
        m_lastError = "Failed to start TILE/HOUSETILE node for " + tile->getPosition().toString() + ".";
        return false;
    }

    // Write tile relative coordinates (X offset from area, Y offset from area) as Node Data
    QByteArray tileCoordsData;
    tileCoordsData.append(static_cast<char>(tile->getPosition().x() % 256));
    tileCoordsData.append(static_cast<char>(tile->getPosition().y() % 256));
    if (!writer.addNodeData(tileCoordsData)) {
        m_lastError = "Failed to write tile relative coords for " + tile->getPosition().toString() + ".";
        return false;
    }

    // Write Tile Attributes
    if (tile->getMapFlags() != 0) { // Check if there are any flags to write
        if(!writer.addU8(OTBM_ATTR_TILE_FLAGS) || !writer.addU32(tile->getMapFlags())) {
             m_lastError = "Failed to write tile flags for " + tile->getPosition().toString() + "."; return false;
        }
    }
    if (nodeType == OTBM_NODE_HOUSETILE) {
        if(!writer.addU8(OTBM_ATTR_HOUSETILE_HOUSEID) || !writer.addU32(tile->getHouseID())) {
            m_lastError = "Failed to write house ID for " + tile->getPosition().toString() + "."; return false;
        }
    }

    // --- Start Tile Spawn Data Serialization ---
    if (tile->isSpawnTile()) {
        if (tile->getSpawnRadius() > 0) {
            if(!writer.addU8(OTBM_ATTR_SPAWN_RADIUS) || !writer.addU16(static_cast<uint16_t>(tile->getSpawnRadius()))) {
                 m_lastError = "Failed to write spawn radius for " + tile->getPosition().toString(); return false;
            }
        }
        if (tile->getSpawnIntervalSeconds() > 0) {
            if(!writer.addU8(OTBM_ATTR_SPAWN_INTERVAL) || !writer.addU32(static_cast<uint32_t>(tile->getSpawnIntervalSeconds()))) {
                 m_lastError = "Failed to write spawn interval for " + tile->getPosition().toString(); return false;
            }
        }

        if (!tile->getSpawnCreatureList().isEmpty()) {
            QString creatureListStr = tile->getSpawnCreatureList().join(',');
            if(!writer.addU8(OTBM_ATTR_SPAWN_CREATURE_LIST) || !writer.addString(creatureListStr)) {
                 m_lastError = "Failed to write spawn creature list for " + tile->getPosition().toString(); return false;
            }
        }
    }
    // --- End Tile Spawn Data Serialization ---

    // Write Items (Ground item first, then top items)
    if (tile->getGround()) {
        if (!serializeItemNode(writer, tile->getGround(), assetManager, settings)) { // Pass assetManager & settings
             m_lastError = "Failed to serialize ground item at " + tile->getPosition().toString() + ". " + m_lastError;
             return false;
        }
    }
    for (const auto& itemPtr : tile->getItems()) { // Assuming getItems() returns a list of unique_ptr or shared_ptr
        if (itemPtr) {
            if (!serializeItemNode(writer, itemPtr.get(), assetManager, settings)) { // Pass assetManager & settings
                m_lastError = "Failed to serialize item ID " + QString::number(itemPtr->getID()) + " at " + tile->getPosition().toString() + ". " + m_lastError;
                return false;
            }
        }
    }

    // Serialize Creature if present
    if (tile->getCreature()) {
        if (!serializeCreatureNode(writer, tile->getCreature(), assetManager, settings)) {
            // m_lastError is set by serializeCreatureNode
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end TILE/HOUSETILE node for " + tile->getPosition().toString() + ".";
        return false;
    }
    return true;
}

// Pass AssetManager and AppSettings
bool OtbmMapIO::serializeItemNode(NodeFileWriteHandle& writer, const Item* item, AssetManager& assetManager, AppSettings& settings) {
    if (!item) return true; // Should not happen if called correctly

    // Item attributes usually not compressed, unless specific items have very large text attributes.
    // For now, assume no compression for item attributes.
    bool compressItemProps = false;
    if (!writer.addNode(OTBM_NODE_ITEM, compressItemProps)) {
        m_lastError = "Failed to start ITEM node for ID " + QString::number(item->getID()) + ".";
        return false;
    }

    // Write Item ID as Node Data (U16)
    QByteArray itemIdData;
    QDataStream ds(&itemIdData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << static_cast<quint16>(item->getID());
    if (!writer.addNodeData(itemIdData)) {
        m_lastError = "Failed to write item ID node data for ID " + QString::number(item->getID()) + ".";
        return false;
    }

    // Write Item Attributes
    if (item->getSubtype() != 0) { // Subtype could be count, charges, fluid type etc.
        // This needs to be more specific based on item type (stackable vs charges etc.)
        // For simplicity, using OTBM_ATTR_COUNT. A real impl. might use OTBM_ATTR_CHARGES.
        const RME::core::assets::ItemType* itemType = assetManager.getItemData(item->getID());
        if (itemType && (itemType->isStackable() || itemType->isSplash())) { // Example check
            if(!writer.addU8(OTBM_ATTR_COUNT) || !writer.addU8(static_cast<uint8_t>(item->getSubtype())) ){
                m_lastError = "Failed to write item count for ID " + QString::number(item->getID()) + "."; return false;
            }
        } else if (itemType && itemType->isFluidContainer()) { // Example for fluids that might use full subtype for type
             if(!writer.addU8(OTBM_ATTR_COUNT) || !writer.addU8(static_cast<uint8_t>(item->getSubtype())) ){ // Should be OTBM_ATTR_CHARGES or similar for fluid type
                m_lastError = "Failed to write fluid subtype for ID " + QString::number(item->getID()) + "."; return false;
            }
        } else { // Default to charges or other generic subtype, potentially U16 if itemType indicates
             if(!writer.addU8(OTBM_ATTR_CHARGES) || !writer.addU16(item->getSubtype())) {
                m_lastError = "Failed to write item charges/subtype for ID " + QString::number(item->getID()) + "."; return false;
            }
        }
    }
    if (item->getActionID() != 0) {
        if(!writer.addU8(OTBM_ATTR_ACTION_ID) || !writer.addU16(item->getActionID())) {
            m_lastError = "Failed to write ActionID for item ID " + QString::number(item->getID()) + "."; return false;
        }
    }
    if (item->getUniqueID() != 0) {
        if(!writer.addU8(OTBM_ATTR_UNIQUE_ID) || !writer.addU16(item->getUniqueID())) {
            m_lastError = "Failed to write UniqueID for item ID " + QString::number(item->getID()) + "."; return false;
        }
    }
    if (!item->getText().isEmpty()) {
        if(!writer.addU8(OTBM_ATTR_TEXT) || !writer.addString(item->getText())) {
            m_lastError = "Failed to write text for item ID " + QString::number(item->getID()) + "."; return false;
        }
    }
    // Additional item attributes are now implemented above
    // Adding new attributes based on subtask instructions
    Position dest = item->getTeleportDestination();
    if (dest.isValid()) {
        if(!writer.addU8(OTBM_ATTR_TELE_DEST_X) || !writer.addU16(dest.x())) {m_lastError="Failed to write tele dest X"; return false;}
        if(!writer.addU8(OTBM_ATTR_TELE_DEST_Y) || !writer.addU16(dest.y())) {m_lastError="Failed to write tele dest Y"; return false;}
        if(!writer.addU8(OTBM_ATTR_TELE_DEST_Z) || !writer.addU8(dest.z())) {m_lastError="Failed to write tele dest Z"; return false;}
    }
    if (item->getDepotID() > 0) {
        if(!writer.addU8(OTBM_ATTR_DEPOT_ID) || !writer.addU16(item->getDepotID())) {m_lastError="Failed to write depot id"; return false;}
    }
    // Assuming OTBM_ATTR_ITEM_CHARGES is distinct from OTBM_ATTR_CHARGES or OTBM_ATTR_COUNT for specific items.
    // For now, we'll use OTBM_ATTR_CHARGES if itemType->isChargeableOrFluid() and it's already handled by subtype logic.
    // If OTBM_ATTR_ITEM_CHARGES is a separate field to be always written for certain items:
    // if (itemType && itemType->isChargeableOrFluid() && item->getCharges() > 0) { // Assuming Item has getCharges()
    //    if(!writer.addU8(OTBM_ATTR_ITEM_CHARGES) || !writer.addU16(item->getCharges())) {m_lastError="Failed to write item charges"; return false;}
    // }
    if (!item->getWrittenText().isEmpty()) {
        if(!writer.addU8(OTBM_ATTR_WRITTEN_TEXT) || !writer.addString(item->getWrittenText())) {m_lastError="Failed to write written text"; return false;}
    }
    if (!item->getAuthor().isEmpty()) {
        if(!writer.addU8(OTBM_ATTR_WRITTEN_BY) || !writer.addString(item->getAuthor())) {m_lastError="Failed to write written by"; return false;}
    }
    if (item->getWrittenDate() > 0) {
        if(!writer.addU8(OTBM_ATTR_DATE) || !writer.addU32(item->getWrittenDate())) {m_lastError="Failed to write date"; return false;}
    }
    if (item->getLightLevel() > 0) { // Assuming 0 is default/no light
        if(!writer.addU8(OTBM_ATTR_LIGHT_LEVEL) || !writer.addU16(item->getLightLevel())) {m_lastError="Failed to write light level"; return false;}
    }
    if (item->getLightColor() > 0) { // Assuming 0 is default/no color
        if(!writer.addU8(OTBM_ATTR_LIGHT_COLOR) || !writer.addU16(item->getLightColor())) {m_lastError="Failed to write light color"; return false;}
    }
    if (item->getDuration() > 0) {
        if(!writer.addU8(OTBM_ATTR_DURATION) || !writer.addU32(item->getDuration())) {m_lastError="Failed to write duration"; return false;}
    }
    if (item->hasAttribute("article")) {
        if(!writer.addU8(OTBM_ATTR_ARTICLE) || !writer.addString(item->getAttribute("article").toString())) {m_lastError="Failed to write article"; return false;}
    }
    if (!item->getDescription().isEmpty()) { // Assuming Item has getDescription()
        if(!writer.addU8(OTBM_ATTR_DESCRIPTION_TEXT) || !writer.addString(item->getDescription())) {m_lastError="Failed to write description text"; return false;}
    }


    // If the item is a container, serialize its children.
    const RME::core::assets::ItemType* itemTypeSerialize = assetManager.getItemData(item->getID());
    if (itemTypeSerialize && itemTypeSerialize->isContainer()) {
        const RME::core::Container* containerItem = dynamic_cast<const RME::core::Container*>(item);
        if (containerItem) {
            for (const auto& childItemPtr : containerItem->getItems()) {
                if (childItemPtr) {
                    if (!serializeItemNode(writer, childItemPtr.get(), assetManager, settings)) {
                        return false;
                    }
                }
            }
        } else {
             qWarning() << "OtbmMapIO::serializeItemNode: Item ID " << item->getID() << " is a container type but could not be cast to Container to serialize children.";
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end ITEM node for ID " + QString::number(item->getID()) + ".";
        return false;
    }
    return true;
}

    // --- Waypoint Specific Parsing/Serialization ---
bool OtbmMapIO::parseWaypointsContainerNode(BinaryNode* containerNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    BinaryNode* waypointNode = containerNode->getChild();
    while(waypointNode) {
        if (waypointNode->getType() == OTBM_NODE_WAYPOINT) {
            if (!parseWaypointNode(waypointNode, map, assetManager, settings)) {
                return false; // m_lastError set in parseWaypointNode
            }
        } else {
            qWarning() << "OtbmMapIO: Unknown child node type" << waypointNode->getType() << "in WAYPOINTS_CONTAINER.";
        }
        waypointNode = containerNode->getNextChild();
    }
    return true;
}

bool OtbmMapIO::parseWaypointNode(BinaryNode* waypointNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    QString wpName;
    Position wpPos;
    QList<QString> connectionNames; // Temporary list to store connection names

    waypointNode->resetReadOffset();
    while(waypointNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!waypointNode->getU8(attribute)) { m_lastError = "Failed to read waypoint attribute type."; return false; }

        switch(attribute) {
            case OTBM_ATTR_WAYPOINT_NAME:{
                std::string name_std;
                if (!waypointNode->getString(name_std)) { m_lastError = "Failed to read waypoint name."; return false; }
                wpName = QString::fromStdString(name_std);
                break;
            }
            case OTBM_ATTR_WAYPOINT_POSITION_X:{
                uint16_t x;
                if (!waypointNode->getU16(x)) { m_lastError = "Failed to read waypoint pos X."; return false; }
                wpPos.setX(x);
                break;
            }
            case OTBM_ATTR_WAYPOINT_POSITION_Y:{
                uint16_t y;
                if (!waypointNode->getU16(y)) { m_lastError = "Failed to read waypoint pos Y."; return false; }
                wpPos.setY(y);
                break;
            }
            case OTBM_ATTR_WAYPOINT_POSITION_Z:{
                uint8_t z;
                if (!waypointNode->getU8(z)) { m_lastError = "Failed to read waypoint pos Z."; return false; }
                wpPos.setZ(z);
                break;
            }
            case OTBM_ATTR_WAYPOINT_CONNECTION_TO: {
                std::string connectedName_std;
                if (!waypointNode->getString(connectedName_std)) {
                    m_lastError = "Failed to read waypoint connection name.";
                    return false;
                }
                connectionNames.append(QString::fromStdString(connectedName_std));
                break;
            }
            default: {
                m_lastError = QString("Unknown attribute type %1 for WAYPOINT node.").arg(attribute);
                qWarning() << "OtbmMapIO::parseWaypointNode:" << m_lastError << "Skipping attribute.";
                // Attempt to skip this attribute based on its type if BinaryNode supports it,
                // or return false if strict parsing is required.
                // For now, let's assume BinaryNode's getString/getU16 etc. consume the correct amount of data
                // and the loop continues. If not, this could lead to further parsing errors.
                // A robust skip would require knowing attribute data length.
                // Returning false for unknown attributes is safer for strict formats.
                return false;
            }
        }
    }

    if (wpName.isEmpty()) {
        m_lastError = "Waypoint loaded with empty name. Position: " + wpPos.toString();
        qWarning() << "OtbmMapIO::parseWaypointNode:" << m_lastError;
        return false; // Waypoint name is essential
    }
    if (!wpPos.isValid()) { // Assuming Position::isValid() checks if it's not default e.g. (-1,-1,-1) or (0,0,0) if 0,0,0 is invalid start
        // Or if specific coordinates are out of expected range if not covered by map->addWaypoint
        m_lastError = "Waypoint '" + wpName + "' loaded with invalid position: " + wpPos.toString();
        qWarning() << "OtbmMapIO::parseWaypointNode:" << m_lastError;
        return false;
    }

    RME::core::navigation::WaypointData newWaypoint(wpName, wpPos);
    for (const QString& connectedName : connectionNames) {
        newWaypoint.addConnection(connectedName);
    }

    if (!map.addWaypoint(std::move(newWaypoint))) {
        // map.addWaypoint might return false if a waypoint with the same name already exists
        // and the policy is not to overwrite. Or if other validation fails.
        m_lastError = "Failed to add waypoint '" + wpName + "' to map. It might already exist or be invalid.";
        qWarning() << "OtbmMapIO::parseWaypointNode:" << m_lastError;
        return false; // Indicate failure to add to map
    }
    return true;
}

bool OtbmMapIO::serializeWaypointsContainerNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    if (map.getWaypoints().isEmpty()) {
        return true; // Nothing to serialize
    }
    if (!writer.addNode(OTBM_NODE_WAYPOINTS, false)) {
        m_lastError = "Failed to start WAYPOINTS_CONTAINER node."; return false;
    }

    const auto& waypointsMap = map.getWaypoints();
    for (auto it = waypointsMap.constBegin(); it != waypointsMap.constEnd(); ++it) {
        const RME::core::navigation::WaypointData& waypoint = it.value();
        if (!serializeWaypointNode(writer, waypoint, assetManager, settings)) {
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end WAYPOINTS_CONTAINER node."; return false;
    }
    return true;
}

bool OtbmMapIO::serializeWaypointNode(NodeFileWriteHandle& writer, const RME::core::navigation::WaypointData& waypoint, AssetManager& assetManager, AppSettings& settings) {
    if (!writer.addNode(OTBM_NODE_WAYPOINT, false)) {
        m_lastError = "Failed to start WAYPOINT node for: " + waypoint.name;
        return false;
    }

    // Waypoint name and position are attributes
    if (!writer.addU8(OTBM_ATTR_WAYPOINT_NAME) || !writer.addString(waypoint.name)) {
        m_lastError = "Failed to write waypoint name for: " + waypoint.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_WAYPOINT_POSITION_X) || !writer.addU16(waypoint.position.x())) {
         m_lastError = "Failed to write waypoint pos X for: " + waypoint.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_WAYPOINT_POSITION_Y) || !writer.addU16(waypoint.position.y())) {
         m_lastError = "Failed to write waypoint pos Y for: " + waypoint.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_WAYPOINT_POSITION_Z) || !writer.addU8(static_cast<uint8_t>(waypoint.position.z()))) {
         m_lastError = "Failed to write waypoint pos Z for: " + waypoint.name; return false;
    }

    // Serialize connections
    for (const QString& connectedName : waypoint.getConnections()) {
        if (!writer.addU8(OTBM_ATTR_WAYPOINT_CONNECTION_TO) || !writer.addString(connectedName)) {
            m_lastError = "Failed to write waypoint connection to '" + connectedName + "' for waypoint '" + waypoint.name + "'.";
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end WAYPOINT node for: " + waypoint.name; return false;
    }
    return true;
}

// Decompress/Compress helpers from header - not used by core property handling anymore
// but kept if they have other uses. They work on std::vector<uint8_t>.
QByteArray OtbmMapIO::decompressNodeData(const std::vector<uint8_t>& compressedData) {
    if (compressedData.empty()) {
        m_lastError = "Compressed data is empty.";
        return QByteArray();
    }
    QByteArray compressedByteArray(reinterpret_cast<const char*>(compressedData.data()), static_cast<int>(compressedData.size()));
    QByteArray decompressedData = qUncompress(compressedByteArray);
    if (decompressedData.isEmpty() && !compressedByteArray.isEmpty()) {
        m_lastError = "Failed to decompress data with OtbmMapIO::decompressNodeData.";
    }
    return decompressedData;
}

std::vector<uint8_t> OtbmMapIO::compressNodeData(const QByteArray& uncompressedData) {
    QByteArray compressedData = qCompress(uncompressedData, -1); // Default Qt Zlib compression
    if (compressedData.isEmpty() && !uncompressedData.isEmpty()) {
        m_lastError = "Failed to compress data with OtbmMapIO::compressNodeData.";
        return {};
    }
    return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(compressedData.constData()),
                                reinterpret_cast<const uint8_t*>(compressedData.constData()) + compressedData.size());
}

// --- Town Specific Parsing/Serialization ---
bool OtbmMapIO::parseTownsContainerNode(BinaryNode* containerNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    BinaryNode* townNode = containerNode->getChild();
    while(townNode) {
        if (townNode->getType() == OTBM_NODE_TOWN) {
            if (!parseTownNode(townNode, map, assetManager, settings)) {
                return false; // m_lastError set in parseTownNode
            }
        } else {
            qWarning() << "OtbmMapIO: Unknown child node type" << townNode->getType() << "in TOWNS_CONTAINER.";
        }
        townNode = containerNode->getNextChild();
    }
    return true;
}

bool OtbmMapIO::parseTownNode(BinaryNode* townNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    uint32_t townId_u32 = 0; // Store as uint32_t from TownData
    uint16_t townId_u16 = 0; // For reading OTBM U16
    QString townName;
    Position templePos;
    bool idSet = false, nameSet = false, posSet = false;

    townNode->resetReadOffset();
    while(townNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!townNode->getU8(attribute)) { m_lastError = "Failed to read town attribute type."; return false; }

        switch(attribute) {
            case OTBM_ATTR_TOWN_ID: {
                if (!townNode->getU16(townId_u16)) { m_lastError = "Failed to read town ID (U16)."; return false; }
                townId_u32 = static_cast<uint32_t>(townId_u16);
                idSet = true;
                break;
            }
            case OTBM_ATTR_TOWN_NAME: {
                std::string name_std;
                if (!townNode->getString(name_std)) { m_lastError = "Failed to read town name."; return false; }
                townName = QString::fromStdString(name_std);
                nameSet = true;
                break;
            }
            case OTBM_ATTR_TOWN_TEMPLE_POS_X: {
                uint16_t x;
                if (!townNode->getU16(x)) { m_lastError = "Failed to read town temple pos X."; return false; }
                templePos.setX(x);
                break;
            }
            case OTBM_ATTR_TOWN_TEMPLE_POS_Y: {
                uint16_t y;
                if (!townNode->getU16(y)) { m_lastError = "Failed to read town temple pos Y."; return false; }
                templePos.setY(y);
                break;
            }
            case OTBM_ATTR_TOWN_TEMPLE_POS_Z: {
                uint8_t z;
                if (!townNode->getU8(z)) { m_lastError = "Failed to read town temple pos Z."; return false; }
                templePos.setZ(z);
                posSet = true; // Assuming Z is the last component of position
                break;
            }
            default: {
                m_lastError = QString("Unknown attribute type %1 for TOWN node.").arg(attribute);
                qWarning() << "OtbmMapIO::parseTownNode:" << m_lastError;
                return false;
            }
        }
    }

    if (!idSet || !nameSet || !posSet) {
        m_lastError = QString("Incomplete town data: ID set=%1, Name set=%2, Pos set=%3 for a town.").arg(idSet).arg(nameSet).arg(posSet);
        qWarning() << "OtbmMapIO::parseTownNode:" << m_lastError;
        return false;
    }

    if (townName.isEmpty() || townId_u32 == 0) { // Check against the u32 version
         m_lastError = "Town loaded with empty name or ID 0.";
         qWarning() << "OtbmMapIO::parseTownNode:" << m_lastError;
         return false;
    }

    RME::core::world::TownData newTown(townId_u32, townName, templePos);
    if (!map.addTown(std::move(newTown))) {
        m_lastError = "Failed to add town (ID: " + QString::number(townId_u32) + ", Name: " + townName + ") to map. It might already exist or be invalid.";
        qWarning() << "OtbmMapIO::parseTownNode:" << m_lastError;
        return false;
    }
    return true;
}

bool OtbmMapIO::serializeTownsContainerNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    if (map.getTowns().isEmpty()) {
        return true;
    }
    if (!writer.addNode(OTBM_NODE_TOWNS, false)) {
        m_lastError = "Failed to start TOWNS_CONTAINER node."; return false;
    }

    const auto& townsMap = map.getTowns(); // getTowns() returns const QMap<uint32_t, RME::core::world::TownData>&
    for (auto it = townsMap.constBegin(); it != townsMap.constEnd(); ++it) {
        if (!serializeTownNode(writer, it.value(), assetManager, settings)) {
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end TOWNS_CONTAINER node."; return false;
    }
    return true;
}

bool OtbmMapIO::serializeTownNode(NodeFileWriteHandle& writer, const RME::core::world::TownData& town, AssetManager& assetManager, AppSettings& settings) {
    if (!writer.addNode(OTBM_NODE_TOWN, false)) {
        m_lastError = "Failed to start TOWN node for: " + town.name;
        return false;
    }

    if (!writer.addU8(OTBM_ATTR_TOWN_ID) || !writer.addU16(static_cast<uint16_t>(town.id))) {
        m_lastError = "Failed to write town ID for: " + town.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_TOWN_NAME) || !writer.addString(town.name)) {
        m_lastError = "Failed to write town name for: " + town.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_TOWN_TEMPLE_POS_X) || !writer.addU16(town.templePosition.x())) {
         m_lastError = "Failed to write town temple pos X for: " + town.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_TOWN_TEMPLE_POS_Y) || !writer.addU16(town.templePosition.y())) {
         m_lastError = "Failed to write town temple pos Y for: " + town.name; return false;
    }
    if (!writer.addU8(OTBM_ATTR_TOWN_TEMPLE_POS_Z) || !writer.addU8(static_cast<uint8_t>(town.templePosition.z()))) {
         m_lastError = "Failed to write town temple pos Z for: " + town.name; return false;
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end TOWN node for: " + town.name; return false;
    }
    return true;
}


// --- House Specific Parsing/Serialization ---
bool OtbmMapIO::parseHousesContainerNode(BinaryNode* containerNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    BinaryNode* houseNode = containerNode->getChild();
    while(houseNode) {
        if (houseNode->getType() == OTBM_NODE_HOUSE) {
            if (!parseHouseNode(houseNode, map, assetManager, settings)) {
                return false; // m_lastError set in parseHouseNode
            }
        } else {
            qWarning() << "OtbmMapIO: Unknown child node type" << houseNode->getType() << "in HOUSES_CONTAINER.";
        }
        houseNode = containerNode->getNextChild();
    }
    return true;
}

bool OtbmMapIO::parseHouseNode(BinaryNode* houseNode, Map& map, AssetManager& assetManager, AppSettings& settings) {
    uint32_t houseId = 0;
    QString houseName;
    Position entryPos;
    uint32_t rent = 0;
    uint32_t guildId = 0;
    bool idSet = false, nameSet = false, entrySet = false;

    houseNode->resetReadOffset();
    while(houseNode->hasMoreProperties()) {
        uint8_t attribute;
        if (!houseNode->getU8(attribute)) { 
            m_lastError = "Failed to read house attribute type."; 
            return false; 
        }

        switch(attribute) {
            case OTBM_ATTR_HOUSE_ID: {
                if (!houseNode->getU32(houseId)) { 
                    m_lastError = "Failed to read house ID."; 
                    return false; 
                }
                idSet = true;
                break;
            }
            case OTBM_ATTR_HOUSE_NAME: {
                std::string name_std;
                if (!houseNode->getString(name_std)) { 
                    m_lastError = "Failed to read house name."; 
                    return false; 
                }
                houseName = QString::fromStdString(name_std);
                nameSet = true;
                break;
            }
            case OTBM_ATTR_HOUSE_ENTRY_X: {
                uint16_t x;
                if (!houseNode->getU16(x)) { 
                    m_lastError = "Failed to read house entry pos X."; 
                    return false; 
                }
                entryPos.setX(x);
                break;
            }
            case OTBM_ATTR_HOUSE_ENTRY_Y: {
                uint16_t y;
                if (!houseNode->getU16(y)) { 
                    m_lastError = "Failed to read house entry pos Y."; 
                    return false; 
                }
                entryPos.setY(y);
                break;
            }
            case OTBM_ATTR_HOUSE_ENTRY_Z: {
                uint8_t z;
                if (!houseNode->getU8(z)) { 
                    m_lastError = "Failed to read house entry pos Z."; 
                    return false; 
                }
                entryPos.setZ(z);
                entrySet = true;
                break;
            }
            case OTBM_ATTR_HOUSE_RENT: {
                if (!houseNode->getU32(rent)) { 
                    m_lastError = "Failed to read house rent."; 
                    return false; 
                }
                break;
            }
            case OTBM_ATTR_HOUSE_GUILD_ID: {
                if (!houseNode->getU32(guildId)) { 
                    m_lastError = "Failed to read house guild ID."; 
                    return false; 
                }
                break;
            }
            default: {
                m_lastError = QString("Unknown attribute type %1 for HOUSE node.").arg(attribute);
                qWarning() << "OtbmMapIO::parseHouseNode:" << m_lastError;
                return false;
            }
        }
    }

    if (!idSet || !nameSet || !entrySet) {
        m_lastError = QString("Incomplete house data: ID set=%1, Name set=%2, Entry set=%3").arg(idSet).arg(nameSet).arg(entrySet);
        qWarning() << "OtbmMapIO::parseHouseNode:" << m_lastError;
        return false;
    }

    if (houseName.isEmpty() || houseId == 0) {
        m_lastError = "House loaded with empty name or ID 0.";
        qWarning() << "OtbmMapIO::parseHouseNode:" << m_lastError;
        return false;
    }

    RME::core::houses::HouseData newHouse(houseId, houseName, entryPos);
    newHouse.setRent(rent);
    newHouse.setGuildId(guildId);
    
    if (!map.addHouse(std::move(newHouse))) {
        m_lastError = "Failed to add house (ID: " + QString::number(houseId) + ", Name: " + houseName + ") to map.";
        qWarning() << "OtbmMapIO::parseHouseNode:" << m_lastError;
        return false;
    }
    return true;
}

bool OtbmMapIO::serializeHousesContainerNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings) {
    if (map.getHouses().isEmpty()) {
        return true;
    }
    if (!writer.addNode(OTBM_NODE_HOUSES, false)) {
        m_lastError = "Failed to start HOUSES_CONTAINER node."; 
        return false;
    }

    const auto& housesMap = map.getHouses();
    for (auto it = housesMap.constBegin(); it != housesMap.constEnd(); ++it) {
        if (!serializeHouseNode(writer, it.value(), assetManager, settings)) {
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end HOUSES_CONTAINER node."; 
        return false;
    }
    return true;
}

bool OtbmMapIO::serializeHouseNode(NodeFileWriteHandle& writer, const RME::core::houses::HouseData& house, AssetManager& assetManager, AppSettings& settings) {
    if (!writer.addNode(OTBM_NODE_HOUSE, false)) {
        m_lastError = "Failed to start HOUSE node for: " + house.getName();
        return false;
    }

    if (!writer.addU8(OTBM_ATTR_HOUSE_ID) || !writer.addU32(house.getId())) {
        m_lastError = "Failed to write house ID for: " + house.getName(); 
        return false;
    }
    if (!writer.addU8(OTBM_ATTR_HOUSE_NAME) || !writer.addString(house.getName())) {
        m_lastError = "Failed to write house name for: " + house.getName(); 
        return false;
    }
    if (!writer.addU8(OTBM_ATTR_HOUSE_ENTRY_X) || !writer.addU16(house.getEntryPosition().x())) {
        m_lastError = "Failed to write house entry pos X for: " + house.getName(); 
        return false;
    }
    if (!writer.addU8(OTBM_ATTR_HOUSE_ENTRY_Y) || !writer.addU16(house.getEntryPosition().y())) {
        m_lastError = "Failed to write house entry pos Y for: " + house.getName(); 
        return false;
    }
    if (!writer.addU8(OTBM_ATTR_HOUSE_ENTRY_Z) || !writer.addU8(static_cast<uint8_t>(house.getEntryPosition().z()))) {
        m_lastError = "Failed to write house entry pos Z for: " + house.getName(); 
        return false;
    }
    
    if (house.getRent() > 0) {
        if (!writer.addU8(OTBM_ATTR_HOUSE_RENT) || !writer.addU32(house.getRent())) {
            m_lastError = "Failed to write house rent for: " + house.getName(); 
            return false;
        }
    }
    
    if (house.getGuildId() > 0) {
        if (!writer.addU8(OTBM_ATTR_HOUSE_GUILD_ID) || !writer.addU32(house.getGuildId())) {
            m_lastError = "Failed to write house guild ID for: " + house.getName(); 
            return false;
        }
    }

    if (!writer.endNode()) {
        m_lastError = "Failed to end HOUSE node for: " + house.getName(); 
        return false;
    }
    return true;
}

} // namespace io
} // namespace core
} // namespace RME
