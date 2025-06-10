#include "CreatureDatabase.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

namespace RME {

struct CreatureDatabase::CreatureDatabaseData {
    QMap<QString, CreatureData> creatures; // Keyed by creature name (lowercase)
};

CreatureDatabase::CreatureDatabase() : d(new CreatureDatabaseData()) {
    invalidCreatureData.name = "Unknown Creature";
    // Initialize other fields of invalidCreatureData if necessary
}
CreatureDatabase::~CreatureDatabase() = default;

const CreatureData* CreatureDatabase::getCreatureData(const QString& name) const {
    QString lowerName = name.toLower();
    auto it = d->creatures.constFind(lowerName);
    return (it != d->creatures.constEnd()) ? &it.value() : &invalidCreatureData;
}

const CreatureData& CreatureDatabase::getDefaultCreatureData() const {
    return invalidCreatureData;
}

int CreatureDatabase::getCreatureCount() const {
    return d->creatures.size();
}

QMap<QString, CreatureData> CreatureDatabase::getAllCreatures() const {
    return d->creatures; // Returns a copy
}

bool CreatureDatabase::loadFromXML(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CreatureDatabase: Could not open RME creatures.xml file:" << filePath;
        return false;
    }
    // For RME's main creatures.xml, we typically clear existing data.
    // If merging from multiple RME-format files was a feature, this would change.
    // d->creatures.clear(); // Let's assume loadFromXML is for the primary file and clears.
                         // Or, if called multiple times, it should merge.
                         // For testing, it's easier if it clears.
                         // For now, let's keep the behavior of adding/updating.

    QXmlStreamReader xml(&file);
    bool inCreaturesRootTag = false;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("creatures")) {
                inCreaturesRootTag = true;
                continue;
            }
            if (!inCreaturesRootTag && filePath.endsWith("creatures.xml")) {
                qWarning() << "CreatureDatabase: Main RME creatures.xml does not start with <creatures> tag.";
                return false;
            }

            if (xml.name() == QLatin1String("creature")) { // RME format
                QString name = xml.attributes().value("name").toString();
                if (name.isEmpty()) {
                    qWarning() << "CreatureDatabase: Found <creature> tag without a name in" << filePath;
                    xml.skipCurrentElement();
                    continue;
                }

                CreatureData& creatureData = d->creatures[name.toLower()]; // Get existing or create new
                creatureData.name = name; // Ensure correct casing from this file if it's the first time

                QString typeStr = xml.attributes().value("type").toString().toLower();
                if (typeStr == "npc") {
                    creatureData.flags |= CreatureTypeFlag::IS_NPC;
                } else {
                    creatureData.flags &= ~CreatureTypeFlag::IS_NPC; // Ensure it's not NPC if type is "monster" or other
                }
                creatureData.scriptName = xml.attributes().value("script", name + ".lua").toString(); // Default script name if not provided

                parseCreatureNode(xml, creatureData, false);
            } else if (inCreaturesRootTag) { // Skip unknown tags within <creatures>
                 xml.skipCurrentElement();
            }
        } else if (xml.isEndElement()) {
            if (xml.name() == QLatin1String("creatures")) {
                inCreaturesRootTag = false;
                // break; // If only one <creatures> tag is expected at root.
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "CreatureDatabase: XML parsing error in RME creatures.xml" << filePath << ":" << xml.errorString();
        return false;
    }
    qInfo() << "CreatureDatabase: Successfully processed RME creatures.xml:" << filePath
            << "(total creatures now:" << d->creatures.size() << ")";
    return true;
}

bool CreatureDatabase::importFromOtServerXml(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CreatureDatabase: Could not open OT server creature XML file:" << filePath;
        return false;
    }

    QXmlStreamReader xml(&file);

    if (xml.readNextStartElement()) { // Find the root <monster> or <npc> tag
        if (xml.name() == QLatin1String("monster") || xml.name() == QLatin1String("npc")) {
            QString name = xml.attributes().value("name").toString();
            if (name.isEmpty()) {
                qWarning() << "CreatureDatabase: Creature name missing in OT server file:" << filePath;
                return false;
            }

            CreatureData& creatureData = d->creatures[name.toLower()]; // Get existing or create new
            creatureData.name = name; // Ensure correct casing

            // Store attributes from the root tag (monster/npc)
            for(const QXmlStreamAttribute& attr : xml.attributes()) {
                QString attrName = attr.name().toString();
                QString attrValue = attr.value().toString();
                if (attrName == "name") continue; // Already handled
                if (attrName == "script") creatureData.scriptName = attrValue;
                else if (attrName == "healthmax" || (attrName == "max" && xml.name()=="health")) creatureData.healthMax = attrValue.toInt(); // Compatibility
                else if (attrName == "manamax" || (attrName == "max" && xml.name()=="mana")) creatureData.manaMax = attrValue.toInt();
                else creatureData.genericAttributes.insert(attrName, attrValue);
            }

            if (xml.name() == QLatin1String("npc")) {
                 creatureData.flags |= CreatureTypeFlag::IS_NPC;
            } else {
                 creatureData.flags &= ~CreatureTypeFlag::IS_NPC;
            }

            parseCreatureNode(xml, creatureData, true);

            qInfo() << "CreatureDatabase: Successfully imported/updated OT server creature:" << creatureData.name << "from" << filePath;
            return true;
        } else {
            qWarning() << "CreatureDatabase: No <monster> or <npc> root tag found in OT server file:" << filePath;
            return false;
        }
    }

    if (xml.hasError()) {
        qWarning() << "CreatureDatabase: XML parsing error in OT server file" << filePath << ":" << xml.errorString();
        return false;
    }
    return false; // Should have returned true if successful
}


void CreatureDatabase::parseCreatureNode(QXmlStreamReader& xml, CreatureData& creatureData, bool isServerFormat) {
    QString parentElementName = xml.name().toString();

    while (xml.readNextStartElement()) {
        QString elementName = xml.name().toString();

        if ((elementName == "look" && !isServerFormat) || (elementName == "look" && isServerFormat) || (elementName == "outfit" && isServerFormat)) {
            creatureData.outfit.lookType = xml.attributes().value(isServerFormat ? "looktype" : "type").toUShort();
            creatureData.outfit.lookItem = xml.attributes().value("lookitem").toUShort(); // Often only in server format
            creatureData.outfit.head = xml.attributes().value("head").toUShort();
            creatureData.outfit.body = xml.attributes().value("body").toUShort();
            creatureData.outfit.legs = xml.attributes().value("legs").toUShort();
            creatureData.outfit.feet = xml.attributes().value("feet").toUShort();
            creatureData.outfit.addons = xml.attributes().value("addons").toUShort();
            creatureData.outfit.lookMount = xml.attributes().value(isServerFormat ? "mount" : "mount").toUShort(); // "mountlooktype" in some server files
            if (isServerFormat) creatureData.genericAttributes["corpseid"] = xml.attributes().value("corpse").toUInt();
            xml.skipCurrentElement();
        } else if (elementName == "health" && isServerFormat) {
            creatureData.healthMax = xml.attributes().value("max").toInt();
            xml.skipCurrentElement();
        } else if (elementName == "mana" && isServerFormat) {
            creatureData.manaMax = xml.attributes().value("max").toInt();
            xml.skipCurrentElement();
        } else if (elementName == "attribute" && !isServerFormat) {
            QString key = xml.attributes().value("key").toString();
            QString value = xml.attributes().value("value").toString();
            bool ok;
            if (key == "health_max") creatureData.healthMax = value.toInt(&ok);
            else if (key == "mana_max") creatureData.manaMax = value.toInt(&ok);
            else if (key == "description") creatureData.genericAttributes["description"] = value; // Could be specific field
            else if (key == "speed") creatureData.genericAttributes["speed"] = value.toInt(&ok);
            else if (key == "corpseid") creatureData.genericAttributes["corpseid"] = value.toUInt(&ok);
            else creatureData.genericAttributes.insert(key, value);
            xml.skipCurrentElement();
        } else if (elementName == "flags" && isServerFormat) {
            while(xml.readNextStartElement()){
                if(xml.name() == QLatin1String("flag")){
                    for(const QXmlStreamAttribute& attr : xml.attributes()) {
                        creatureData.genericAttributes.insert("flag_" + attr.name().toString(), attr.value().toInt(&ok) == 1);
                    }
                }
                xml.skipCurrentElement();
            }
        } else if (elementName == "defenses" && isServerFormat) {
            creatureData.genericAttributes["armor"] = xml.attributes().value("armor").toInt();
            creatureData.genericAttributes["defense"] = xml.attributes().value("defense").toInt();
            // Could parse child <defense element="..." defense="..."/> or <immunity ... /> here
            xml.skipCurrentElement(); // Skips children for now
        } else if (elementName == "voices" && isServerFormat) {
            QList<QString> sentences;
            while(xml.readNextStartElement()){
                if(xml.name() == QLatin1String("voice")){
                    sentences.append(xml.attributes().value("sentence").toString());
                }
                xml.skipCurrentElement();
            }
            if(!sentences.isEmpty()) creatureData.genericAttributes["voices"] = sentences.join(" | ");
        }
        // Add more parsers for <attacks>, <loot>, <immunities> etc. for server format if needed
        else {
            xml.skipCurrentElement();
        }
    }
}

} // namespace RME
