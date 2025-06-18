#include "ItemDatabase.h"
#include <QFile>
#include <QDataStream>
#include <QXmlStreamReader>
#include <QDebug>

namespace RME {

// OTB File Format Constants
const quint8 OTB_NODE_MARKER = 0xFF;                    // Marks the start of a node
const quint8 OTB_ROOT_NODE_TYPE = 0x00;                 // Root node type identifier
const quint8 OTB_ITEM_GROUP_NODE_TYPE_DEPRECATED = 0x01; // Deprecated item group node type
const quint8 OTB_NODE_END_MARKER = 0xFE;                // RME specific end marker
const quint8 OTB_ROOT_ATTR_VERSION = 0x01;              // Root attribute: version information

// OTB Item Attribute Constants
const quint8 OTB_ATTR_SERVERID = 0x10;                  // Server ID attribute
const quint8 OTB_ATTR_CLIENTID = 0x11;                  // Client ID attribute  
const quint8 OTB_ATTR_NAME = 0x12;                      // Item name attribute
const quint8 OTB_ATTR_DESCR = 0x13;                     // Item description attribute
const quint8 OTB_ATTR_SPEED = 0x14;                     // Speed attribute
const quint8 OTB_ATTR_SLOT = 0x15;                      // Slot attribute
const quint8 OTB_ATTR_MAXITEMS = 0x16;                  // Max items attribute
const quint8 OTB_ATTR_WEIGHT = 0x17;                    // Weight attribute
const quint8 OTB_ATTR_WEAPON = 0x18;                    // Weapon attribute
const quint8 OTB_ATTR_AMU = 0x19;                       // Amulet attribute
const quint8 OTB_ATTR_ARMOR = 0x1A;                     // Armor attribute

// Size limits for safety
const quint32 MAX_OTB_ATTR_SIZE = 16384;                // Maximum attribute data size
const quint32 MAX_OTB_ROOT_PROPS_SIZE = 1024 * 1024;    // Maximum root properties size


// Forward declaration for helper
void analyzeItemType(ItemData& itemData);


struct ItemDatabase::ItemDatabaseData {
    QMap<quint16, ItemData> items;
    quint32 otbMajorVersion = 0;
    quint32 otbMinorVersion = 0;
    quint32 otbBuildNumber = 0;
    QString otbDescription;
};

ItemDatabase::ItemDatabase() : d(new ItemDatabaseData()) {
    invalidItemData.name = "Unknown Item Type";
    invalidItemData.serverID = 0;
    invalidItemData.clientID = 0;
}

ItemDatabase::~ItemDatabase() = default;

const ItemData* ItemDatabase::getItemData(quint16 serverID) const {
    auto it = d->items.constFind(serverID);
    return (it != d->items.constEnd()) ? &it.value() : &invalidItemData;
}

const ItemData& ItemDatabase::getDefaultItemData() const {
    return invalidItemData;
}

int ItemDatabase::getItemCount() const {
    return d->items.size();
}

const QMap<quint16, ItemData>& ItemDatabase::getAllItems() const {
    return d->items;
}

bool ItemDatabase::loadFromOTB(const QString& filePath) {
    if (filePath.isEmpty()) {
        qWarning() << "ItemDatabase::loadFromOTB: Empty file path provided";
        return false;
    }
    
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "ItemDatabase::loadFromOTB: File does not exist:" << filePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ItemDatabase::loadFromOTB: Could not open OTB file:" << filePath << "Error:" << file.errorString();
        return false;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    d->items.clear(); // OTB load is authoritative for base items

    quint8 marker, rootNodeType;
    stream >> marker >> rootNodeType;

    if (marker != OTB_NODE_MARKER || rootNodeType != OTB_ROOT_NODE_TYPE) {
         qWarning() << "ItemDatabase: Invalid OTB root node marker or type. Marker:" << Qt::hex << marker << "Type:" << Qt::hex << rootNodeType;
         return false;
    }

    quint32 rootPropsLen;
    stream >> rootPropsLen;
    if (stream.status() != QDataStream::Ok) { 
        qWarning() << "ItemDatabase::loadFromOTB: Failed to read root properties length"; 
        return false; 
    }
    
    if (rootPropsLen > MAX_OTB_ROOT_PROPS_SIZE) {
        qWarning() << "ItemDatabase::loadFromOTB: Root properties size too large:" << rootPropsLen << "bytes (max:" << MAX_OTB_ROOT_PROPS_SIZE << ")";
        return false;
    }

    quint64 rootPropsEndPos = stream.device()->pos() + rootPropsLen;
    while(stream.device()->pos() < rootPropsEndPos) {
        quint8 attr;
        quint16 dataLen;
        stream >> attr >> dataLen;
        if (stream.status() != QDataStream::Ok) { qWarning("Failed to read root attr/len"); return false; }

        quint64 currentAttrEndPos = stream.device()->pos() + dataLen;
        if (currentAttrEndPos > rootPropsEndPos) {
             qWarning("OTB root attribute data length exceeds root properties boundary."); return false;
        }

        if (attr == OTB_ROOT_ATTR_VERSION) {
            if (dataLen < 12) { qWarning("OTB root version attr data too short"); return false; }
            stream >> d->otbMajorVersion >> d->otbMinorVersion >> d->otbBuildNumber;
            quint16 descLen; stream >> descLen;
            if (dataLen != 12 + descLen) { qWarning("OTB root desc len mismatch"); return false; }
            QByteArray descBytes(descLen, Qt::Uninitialized);
            if (stream.readRawData(descBytes.data(), descLen) != descLen) { qWarning("Failed to read OTB root desc"); return false; }
            d->otbDescription = QString::fromLatin1(descBytes);
        } else {
            stream.device()->seek(stream.device()->pos() + dataLen); // Skip unknown
        }
        if (stream.device()->pos() != currentAttrEndPos) { // Ensure we consumed exactly dataLen
             qWarning() << "Error in OTB root attr parsing, current pos mismatch for attr" << attr;
             stream.device()->seek(currentAttrEndPos); // Attempt to correct
        }
    }
    if (stream.device()->pos() != rootPropsEndPos) {
        qWarning("Error after OTB root props, position mismatch.");
        return false; // Should be exactly at the end
    }

    qInfo() << "OTB Ver:" << d->otbMajorVersion << "." << d->otbMinorVersion << "." << d->otbBuildNumber << "Desc:" << d->otbDescription;

    // Main loop for item groups
    while (!stream.atEnd()) {
        quint8 nodeMarker;
        stream >> nodeMarker;
        if (stream.status() != QDataStream::Ok && stream.atEnd()) break; // Clean EOF
        if (stream.status() != QDataStream::Ok) { qWarning("Stream error before reading node marker"); return false; }

        if (nodeMarker == OTB_NODE_END_MARKER) { // RME specific?
            quint8 dummy; stream >> dummy; // It seems RME OTB files might have an extra 00 after FF FE
            qInfo() << "ItemDatabase: Reached OTB_NODE_END_MARKER (0xFF 0xFE).";
            break;
        }
        if (nodeMarker != OTB_NODE_MARKER) {
            qWarning() << "ItemDatabase: Expected OTB node marker (0xFF), got" << Qt::hex << nodeMarker << "at pos" << (stream.device()->pos()-1);
            return false;
        }

        quint8 groupTypeByte;
        stream >> groupTypeByte;
        if (stream.status() != QDataStream::Ok) { qWarning("Failed to read group type byte"); return false; }

        ItemGroup currentGroup = static_cast<ItemGroup>(groupTypeByte);
        if (currentGroup >= ItemGroup::LAST && currentGroup != 0) { // Group 0 might be valid for some OTB defs (e.g. NONE)
             qWarning() << "ItemDatabase: Unknown item group in OTB:" << groupTypeByte;
             return false;
        }

        quint32 groupAttrLength; // RME specific: seems to always write 4 bytes of 0 for group attr len
        stream >> groupAttrLength;
        if (groupAttrLength != 0) {
            qWarning() << "ItemDatabase: OTB Group attributes not supported, but length is" << groupAttrLength;
            stream.device()->seek(stream.device()->pos() + groupAttrLength); // Skip them
        }

        // Inner loop for items in this group
        while(!stream.atEnd()){
            qint64 preIdPos = stream.device()->pos();
            quint8 peekByte;
            stream.device()->peek(reinterpret_cast<char*>(&peekByte), 1); // Peek
            if (stream.status() != QDataStream::Ok && stream.atEnd()) break; // EOF

            if (peekByte == OTB_NODE_MARKER) { // Next node (group or end) starts
                break;
            }

            quint16 serverID;
            stream >> serverID;
            if (stream.status() != QDataStream::Ok) {
                 if (stream.atEnd()) break;
                 qWarning() << "ItemDatabase: Error reading serverID in group" << static_cast<int>(currentGroup); return false;
            }

            if (!parseOtbItem(stream, serverID, currentGroup)) {
                qWarning() << "ItemDatabase: Failed to parse OTB item with ID:" << serverID;
                return false;
            }
        }
    }
    qInfo() << "ItemDatabase: Successfully loaded" << d->items.size() << "items from OTB:" << filePath;
    return true;
}

bool ItemDatabase::parseOtbItem(QDataStream& stream, quint16 serverID, ItemGroup group) {
    ItemData itemData;
    itemData.serverID = serverID;
    itemData.clientID = serverID; // Default
    itemData.group = group;

    quint32 rawFlags;
    stream >> rawFlags;
    if (stream.status() != QDataStream::Ok) return false;
    itemData.flags = ItemFlags(rawFlags);

    if (!parseOtbAttributes(stream, itemData)) return false;

    analyzeItemType(itemData); // Set specific ItemType
    d->items.insert(serverID, itemData);
    return true;
}

bool ItemDatabase::parseOtbAttributes(QDataStream& stream, ItemData& itemData) {
    quint8 attr;
    quint16 dataLen;
    QByteArray dataBuffer;

    while(true) {
        stream >> attr;
        if (stream.status() != QDataStream::Ok) { qWarning("Failed to read OTB attr type"); return false; }
        if (attr == 0) break; // End of attributes

        stream >> dataLen;
        if (stream.status() != QDataStream::Ok) { qWarning("Failed to read OTB attr dataLen"); return false; }

        if (dataLen > MAX_OTB_ATTR_SIZE) { 
            qWarning() << "ItemDatabase::parseOtbAttributes: OTB attribute data too large:" << dataLen << "bytes (max:" << MAX_OTB_ATTR_SIZE << ")"; 
            return false; 
        }
        dataBuffer.resize(dataLen);
        if (stream.readRawData(dataBuffer.data(), dataLen) != dataLen) { qWarning("Failed to read OTB attr data"); return false; }

        QDataStream attrStream(dataBuffer);
        attrStream.setByteOrder(QDataStream::LittleEndian);

        switch (static_cast<OtbAttribute>(attr)) {
            case OtbAttribute::OTB_ATTR_ITEM: attrStream >> itemData.clientID; break;
            case OtbAttribute::OTB_ATTR_NAME: itemData.name = QString::fromLatin1(dataBuffer); break;
            case OtbAttribute::OTB_ATTR_DESCRIPTION: itemData.description = QString::fromLatin1(dataBuffer); break;
            case OtbAttribute::OTB_ATTR_PLURALNAME: itemData.pluralName = QString::fromLatin1(dataBuffer); break;
            case OtbAttribute::OTB_ATTR_ARTICLE: itemData.article = QString::fromLatin1(dataBuffer); break;
            case OtbAttribute::OTB_ATTR_TEXT: itemData.genericAttributes["text"] = QString::fromLatin1(dataBuffer); break;
            case OtbAttribute::OTB_ATTR_UNIQUE_ID: attrStream >> itemData.genericAttributes["uid"].setValue<quint16>(); break;
            case OtbAttribute::OTB_ATTR_ACTION_ID: attrStream >> itemData.genericAttributes["aid"].setValue<quint16>(); break;
            case OtbAttribute::OTB_ATTR_TELE_DEST: { // RME stores x,y,z as quint16
                quint16 x,y,z; attrStream >> x >> y >> z;
                itemData.genericAttributes["teleDestX"] = x;
                itemData.genericAttributes["teleDestY"] = y;
                itemData.genericAttributes["teleDestZ"] = z;
            } break;
            case OtbAttribute::OTB_ATTR_DEPOT_ID: attrStream >> itemData.genericAttributes["depotID"].setValue<quint16>(); break;
            case OtbAttribute::OTB_ATTR_HOUSE_DOOR_ID: itemData.genericAttributes["houseDoorID"] = dataBuffer.at(0); break; // uint8
            case OtbAttribute::OTB_ATTR_ATTACK: attrStream >> itemData.attack; break;
            case OtbAttribute::OTB_ATTR_EXTRAATTACK: attrStream >> itemData.extraAttack; break;
            case OtbAttribute::OTB_ATTR_DEFENSE: attrStream >> itemData.defense; break;
            case OtbAttribute::OTB_ATTR_EXTRADEFENSE: attrStream >> itemData.extraDefense; break;
            case OtbAttribute::OTB_ATTR_ARMOR: attrStream >> itemData.armor; break;
            case OtbAttribute::OTB_ATTR_ATTACKSPEED: attrStream >> itemData.attackSpeed; break;
            case OtbAttribute::OTB_ATTR_HITCHANCE: attrStream >> itemData.hitChance; break;
            case OtbAttribute::OTB_ATTR_SHOOTRANGE: itemData.shootRange = dataBuffer.at(0); break; // uint8
            // OTB_ATTR_MAX_ITEMS, OTB_ATTR_WEIGHT, OTB_ATTR_LIGHTLEVEL, OTB_ATTR_LIGHTCOLOR, OTB_ATTR_GROUNDSPEED etc.
            // need to be fully ported from RME's items.cpp ItemType::unserializeItem.
            // For now, store as generic if not directly mapped.
            default:
                itemData.genericAttributes.insert(QString("otb_attr_raw_%1").arg(attr), QVariant(dataBuffer));
                break;
        }
    }
    return true;
}

bool ItemDatabase::loadFromXML(const QString& filePath) {
    if (filePath.isEmpty()) {
        qWarning() << "ItemDatabase::loadFromXML: Empty file path provided";
        return false;
    }
    
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "ItemDatabase::loadFromXML: File does not exist:" << filePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ItemDatabase::loadFromXML: Could not open items.xml file:" << filePath << "Error:" << file.errorString();
        return false;
    }
    QXmlStreamReader xml(&file);
    bool inItemsTag = false;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("items")) { inItemsTag = true; continue; }
            if (!inItemsTag) continue;

            if (xml.name() == QLatin1String("item")) {
                quint16 id = xml.attributes().value("id").toUShort();
                quint16 fromid = xml.attributes().value("fromid").toUShort();
                quint16 toid = xml.attributes().value("toid").toUShort();

                if (id == 0 && fromid == 0) { xml.skipCurrentElement(); continue; }

                if (fromid > 0 && toid >= fromid) { // Item range
                    QString rangeName = xml.attributes().value("name").toString();
                    ItemData baseData; // Parse attributes into this base
                    baseData.name = rangeName;
                    while(xml.readNextStartElement()){
                        if(xml.name() == QLatin1String("attribute")) parseXmlAttribute(xml, baseData);
                        else xml.skipCurrentElement();
                    }
                    for (quint16 i = fromid; i <= toid; ++i) {
                        ItemData& itemData = d->items[i]; // Create or get
                        if(itemData.serverID == 0) { // New item from range
                            itemData = baseData; // Copy base attributes
                            itemData.serverID = i;
                            itemData.clientID = i; // Default clientID
                            itemData.name = rangeName; // Ensure name is set for range items
                        } else { // Merge with existing (OTB)
                            if (!rangeName.isEmpty()) itemData.name = rangeName; // XML name might be generic for range
                            // Selectively merge other relevant fields from baseData if needed
                            for(auto it = baseData.genericAttributes.constBegin(); it != baseData.genericAttributes.constEnd(); ++it) {
                                itemData.genericAttributes.insert(it.key(), it.value());
                            }
                        }
                        analyzeItemType(itemData);
                    }
                } else { // Single item
                    ItemData& itemData = d->items[id]; // Creates if not exists, or gets existing
                    itemData.serverID = id;
                    if (itemData.clientID == 0) itemData.clientID = id;
                    QString xmlName = xml.attributes().value("name").toString();
                    if (!xmlName.isEmpty()) itemData.name = xmlName; // XML name overrides OTB if present

                    while(xml.readNextStartElement()){
                        if(xml.name() == QLatin1String("attribute")) parseXmlAttribute(xml, itemData);
                        else xml.skipCurrentElement();
                    }
                    analyzeItemType(itemData);
                }
            }
        } else if (xml.isEndElement()) {
            if (xml.name() == QLatin1String("items")) { inItemsTag = false; break; }
        }
    }
    if (xml.hasError()) { qWarning() << "XML parsing error:" << xml.errorString(); return false; }
    qInfo() << "Processed items.xml:" << filePath << "(total items:" << d->items.size() << ")";
    return true;
}

void ItemDatabase::parseXmlAttribute(QXmlStreamReader& xml, ItemData& itemData) {
    QString key = xml.attributes().value("key").toString();
    QString valueStr = xml.readElementText();
    bool ok;

    if (key == "type") itemData.type = static_cast<ItemType>(valueStr.toUShort(&ok));
    else if (key == "group") itemData.group = static_cast<ItemGroup>(valueStr.toUShort(&ok));
    else if (key == "description") itemData.description = valueStr;
    else if (key == "article") itemData.article = valueStr;
    else if (key == "pluralname" || key == "plural") itemData.pluralName = valueStr;
    else if (key == "weight" || key == "volumeweight") itemData.weight = valueStr.toDouble(&ok) / 100.0;
    else if (key == "clientid") itemData.clientID = valueStr.toUShort(&ok);
    else if (key == "lightlevel") itemData.lightLevel = valueStr.toUShort(&ok);
    else if (key == "lightcolor") itemData.lightColor = valueStr.toUShort(&ok);
    else if (key == "attack") itemData.attack = valueStr.toUShort(&ok);
    else if (key == "defense") itemData.defense = valueStr.toUShort(&ok);
    else if (key == "armor") itemData.armor = valueStr.toUShort(&ok);
    else if (key == "shoottype") itemData.genericAttributes["shootType"] = valueStr;
    else if (key == "charges" || key == "maxcharges") itemData.maxCharges = valueStr.toUShort(&ok);
    else if (key == "decayto") itemData.decayTo = valueStr.toUShort(&ok);
    else if (key == "corpsetype") itemData.corpseType = valueStr.toUShort(&ok);
    else if (key == "fluidsource" || key == "fluidtype") itemData.genericAttributes["fluidType"] = valueStr; // fluidType is often string like "water"

    // Boolean flags
    else if (key == "blocksolid") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::BLOCK_SOLID; }
    else if (key == "blockprojectile") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::BLOCK_PROJECTILE; }
    else if (key == "blockpathfind") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::BLOCK_PATHFIND; }
    else if (key == "hasheight") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::HAS_HEIGHT; }
    else if (key == "pickupable") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::PICKUPABLE; }
    else if (key == "stackable") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::STACKABLE; }
    else if (key == "moveable") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::MOVEABLE; }
    else if (key == "alwaysontop") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::ALWAYSONTOP; }
    else if (key == "readable") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::READABLE; }
    else if (key == "rotatable") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::ROTATABLE; }
    else if (key == "hangable" || key == "canhang") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::HANGABLE; }
    else if (key == "vertical") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::VERTICAL; }
    else if (key == "horizontal") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::HORIZONTAL; }
    else if (key == "animation" || key == "hasanimation") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::ANIMATION; }
    else if (key == "allowdistread") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::ALLOWDISTREAD; }
    else if (key == "lookthrough") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::LOOKTHROUGH; }
    else if (key == "fullground" || key == "walkstack") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::WALKSTACK; }
    else if (key == "wall") { if (valueStr == "true" || valueStr == "1") itemData.flags |= ItemFlag::WALL; }

    else { // Store unmapped as generic
        int numVal = valueStr.toInt(&ok);
        if (ok) itemData.genericAttributes.insert(key, numVal);
        else {
            double doubleVal = valueStr.toDouble(&ok);
            if(ok) itemData.genericAttributes.insert(key, doubleVal);
            else itemData.genericAttributes.insert(key, valueStr);
        }
    }
}

// Basic ItemType analysis based on group and flags (needs to be more comprehensive like RME)
void analyzeItemType(ItemData& itemData) {
    if (itemData.group == ItemGroup::GROUND) itemData.type = ItemType::TYPE_NORMAL; // Ground items are often just TYPE_NORMAL
    else if (itemData.group == ItemGroup::CONTAINER) itemData.type = ItemType::TYPE_CONTAINER;
    else if (itemData.group == ItemGroup::SPLASH) itemData.type = ItemType::TYPE_SPLASH;
    else if (itemData.group == ItemGroup::FLUID) itemData.type = ItemType::TYPE_FLUID;
    else if (itemData.group == ItemGroup::DOOR) itemData.type = ItemType::TYPE_DOOR;
    else if (itemData.group == ItemGroup::MAGICFIELD) itemData.type = ItemType::TYPE_MAGICFIELD;
    else if (itemData.group == ItemGroup::TELEPORT) itemData.type = ItemType::TYPE_TELEPORT;
    else if (itemData.group == ItemGroup::PODIUM) itemData.type = ItemType::TYPE_PODIUM;
    else if (itemData.group == ItemGroup::RUNE) itemData.type = ItemType::TYPE_RUNE;
    else if (itemData.group == ItemGroup::KEY) itemData.type = ItemType::TYPE_KEY;
    // Add more specific checks, e.g., for depot, mailbox based on serverID or specific flags/attributes if any.
    // This is a simplification of RME's ItemType::analyze.
    else itemData.type = ItemType::TYPE_NORMAL; // Default
}


} // namespace RME
