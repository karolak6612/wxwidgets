#include "core/assets/MaterialManager.h"
#include "core/assets/AssetManager.h" // For context, e.g. item validation (though not deeply used in this impl yet)
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QXmlStreamReader>
#include <QDebug>

namespace RME {
namespace core {
namespace assets {

MaterialManager::MaterialManager() {}
MaterialManager::~MaterialManager() {}

const MaterialData* MaterialManager::getMaterial(const QString& id) const {
    return m_materialsById.value(id, nullptr); // QMap::value with default nullptr if not found
}

const QMap<QString, MaterialData>& MaterialManager::getAllMaterials() const {
    return m_materialsById;
}

const BorderSetData* MaterialManager::getBorderSet(const QString& setId) const {
    // Return value directly, or nullptr if not found (QMap::value behavior)
    // For pointer values, if key not found, it returns a default-constructed value (nullptr for pointers).
    // However, if the value type itself could be non-pointer and we stored BorderSetData directly,
    // value() would return a copy. To return a pointer to an object in the map, use find().
    // Since BorderSetData is a struct and likely stored by value in m_borderSetsById:
    // return m_borderSetsById.value(setId, nullptr); // This is wrong if BorderSetData is stored by value.
    // Correct approach if BorderSetData is stored by value:
    if (m_borderSetsById.contains(setId)) {
        return &m_borderSetsById.at(setId); // Return address of the object in map.
                                          // at() throws if not found, so contains() check is good.
    }
    return nullptr;
    // If m_borderSetsById stores pointers (e.g. QMap<QString, BorderSetData*>), then:
    // return m_borderSetsById.value(setId, nullptr); // This would be correct.
    // Assuming BorderSetData is stored by value as is typical for such structs in Qt containers.
}

bool MaterialManager::loadMaterialsFromDirectory(const QString& baseDir, const QString& mainXmlFile, AssetManager& assetManager) {
    m_materialsById.clear();
    m_parsedFiles.clear();
    m_lastError.clear();

    QDir directory(baseDir);
    QString fullPathToMainXml = directory.filePath(mainXmlFile);

    if (!parseXmlFile(fullPathToMainXml, assetManager, baseDir)) {
        // m_lastError should be set by parseXmlFile on critical failure
        if (m_lastError.isEmpty()) { // Ensure an error is set if parseXmlFile returns false but doesn't set one
            m_lastError = QString("Failed to parse main material file: %1").arg(fullPathToMainXml);
        }
        return false;
    }
    // Even if some non-critical errors occurred (warnings), consider loading successful if it didn't return false.
    return true;
}

bool MaterialManager::parseXmlFile(const QString& filePath, AssetManager& assetManager, const QString& currentDir) {
    if (m_parsedFiles.contains(filePath)) {
        return true; // Already parsed, skip to prevent circular includes or redundant work
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("MaterialManager: Could not open XML file: %1").arg(filePath);
        qWarning() << m_lastError;
        return false;
    }
    m_parsedFiles.insert(filePath);
    qDebug() << "MaterialManager: Parsing XML file:" << filePath;

    QXmlStreamReader xml(&file);
    QString fileProcessingDir = QFileInfo(filePath).absolutePath(); // Directory of the current file for relative includes

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QString elementName = xml.name().toString();
            if (elementName.compare(QLatin1String("materials"), Qt::CaseInsensitive) == 0) {
                // This is the root, just continue parsing its children
            } else if (elementName.compare(QLatin1String("include"), Qt::CaseInsensitive) == 0) {
                QString includePath = xml.attributes().value(QLatin1String("file")).toString();
                if (!includePath.isEmpty()) {
                    QDir currentFileDir(fileProcessingDir);
                    QString fullIncludePath = currentFileDir.filePath(includePath);
                    if (!parseXmlFile(fullIncludePath, assetManager, fileProcessingDir)) {
                        // If an include fails critically, propagate the failure
                        // m_lastError should be set by the recursive call
                        // qWarning() << "MaterialManager: Failed to parse included file:" << fullIncludePath;
                        // return false; // Decide if one failed include fails all. For now, let's be lenient and just warn.
                        qWarning() << "MaterialManager: Failed to parse or error in included file:" << fullIncludePath << "Error:" << m_lastError;
                        m_lastError.clear(); // Clear error for next file if we are lenient
                    }
                }
            } else if (elementName.compare(QLatin1String("brush"), Qt::CaseInsensitive) == 0) {
                if (!parseBrushElement(xml, assetManager, fileProcessingDir)) {
                    // Error parsing a brush, m_lastError should be set.
                    // Continue parsing other brushes for now, but log it.
                    qWarning() << "MaterialManager: Error parsing a <brush> element in" << filePath << ":" << m_lastError;
                    m_lastError.clear(); // Clear for next brush
                }
            } else if (elementName.compare(QLatin1String("border"), Qt::CaseInsensitive) == 0) { // New condition
                // This is for <border id="SET_ID"> elements directly under <materials> (e.g., in borders.xml)
                if (!parseBordersFileEntry(xml)) {
                    qWarning() << "MaterialManager: Error parsing a <border SET_ID> element in" << filePath << ":" << m_lastError;
                    m_lastError.clear(); // Clear for next element
                }
            } else if (elementName.compare(QLatin1String("tileset"), Qt::CaseInsensitive) == 0) {
                // Tilesets group materials for UI, not define new ones here.
                // Skip the content of this element for now by reading until its EndElement.
                QString tilesetName = xml.attributes().value(QLatin1String("name")).toString();
                qDebug() << "MaterialManager: Skipping <tileset name=\"" << tilesetName << "\"> element in" << filePath;
                while(!xml.atEnd() && !(xml.isEndElement() && xml.name().toString().compare(QLatin1String("tileset"), Qt::CaseInsensitive) == 0)) {
                    xml.readNext();
                }
            }
            // Else, ignore unknown top-level elements within <materials> or included files
        }
    }

    if (xml.hasError() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        // PrematureEndOfDocumentError might happen if an include failed and we continued.
        m_lastError = QString("MaterialManager: XML parsing error in %1: %2 at line %3, column %4")
                          .arg(filePath).arg(xml.errorString()).arg(xml.lineNumber()).arg(xml.columnNumber());
        qWarning() << m_lastError;
        // file.close() is handled by QFile destructor
        return false; // Critical parsing error for this file
    }
    return true;
}

bool MaterialManager::parseBrushElement(QXmlStreamReader& xml, AssetManager& assetManager, const QString& currentDir) {
    Q_ASSERT(xml.isStartElement() && xml.name().toString().compare(QLatin1String("brush"), Qt::CaseInsensitive) == 0);

    QString brushName = xml.attributes().value(QLatin1String("name")).toString();
    QString brushType = xml.attributes().value(QLatin1String("type")).toString().toLower();

    if (brushName.isEmpty()) {
        m_lastError = "Found <brush> with no name attribute.";
        return false;
    }
    if (m_materialsById.contains(brushName)) {
        qWarning() << "MaterialManager: Duplicate brush name/id '" << brushName << "'. Overwriting existing definition.";
        // Or return false / skip, depending on desired behavior.
    }

    MaterialData material(brushName, brushType);
    material.serverLookId = xml.attributes().value(QLatin1String("server_lookid")).toUShort();
    material.lookId = xml.attributes().value(QLatin1String("lookid")).toUShort();
    if (xml.attributes().hasAttribute(QLatin1String("z-order"))) {
        material.zOrder = xml.attributes().value(QLatin1String("z-order")).toInt();
    }
    // Doodad/general flags
    if (xml.attributes().hasAttribute(QLatin1String("draggable"))) material.isDraggable = xml.attributes().value(QLatin1String("draggable")).toString() == "true";
    if (xml.attributes().hasAttribute(QLatin1String("on_blocking"))) material.isOnBlocking = xml.attributes().value(QLatin1String("on_blocking")).toString() == "true";
    material.brushThickness = xml.attributes().value(QLatin1String("thickness")).toString();
    if (xml.attributes().hasAttribute(QLatin1String("one_size"))) material.isOneSize = xml.attributes().value(QLatin1String("one_size")).toString() == "true";
    if (xml.attributes().hasAttribute(QLatin1String("redo_borders"))) material.isRedoBorders = xml.attributes().value(QLatin1String("redo_borders")).toString() == "true";
    if (xml.attributes().hasAttribute(QLatin1String("on_duplicate"))) material.isOnDuplicate = xml.attributes().value(QLatin1String("on_duplicate")).toString() == "true";

    // Parse children based on type
    if (brushType == QLatin1String("ground")) {
        MaterialGroundSpecifics specifics;
        material.specificData.emplace<MaterialGroundSpecifics>(specifics);
    } else if (brushType == QLatin1String("wall")) {
        MaterialWallSpecifics specifics;
        material.specificData.emplace<MaterialWallSpecifics>(specifics);
    } else if (brushType == QLatin1String("doodad")) {
        MaterialDoodadSpecifics specifics;
        // Initialize attributes already read from <brush> tag itself
        specifics.draggable = material.isDraggable;
        specifics.onBlocking = material.isOnBlocking;
        specifics.thickness = material.brushThickness;
        specifics.oneSize = material.isOneSize;
        specifics.redoBorders = material.isRedoBorders;
        specifics.onDuplicate = material.isOnDuplicate;
        material.specificData.emplace<MaterialDoodadSpecifics>(specifics);
    } else if (brushType == QLatin1String("carpet")) {
        MaterialCarpetSpecifics specifics;
        if (xml.attributes().hasAttribute(QLatin1String("on_blocking"))) specifics.onBlocking = xml.attributes().value(QLatin1String("on_blocking")).toString() == "true";
        material.specificData.emplace<MaterialCarpetSpecifics>(specifics);
    } else if (brushType == QLatin1String("table")) {
        MaterialTableSpecifics specifics;
        if (xml.attributes().hasAttribute(QLatin1String("on_blocking"))) specifics.onBlocking = xml.attributes().value(QLatin1String("on_blocking")).toString() == "true";
        material.specificData.emplace<MaterialTableSpecifics>(specifics);
    } else {
        // Unknown brush type, store as monostate or a generic type
        qWarning() << "MaterialManager: Unknown brush type '" << brushType << "' for brush '" << brushName << "'.";
        material.specificData.emplace<std::monostate>();
    }

    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("brush"), Qt::CaseInsensitive) == 0) {
            break; // End of this brush element
        }
        if (xml.isStartElement()) {
            if (brushType == QLatin1String("ground")) {
                if (xml.name().toString() == QLatin1String("item")) parseBrushItems(xml, material);
                else if (xml.name().toString() == QLatin1String("border")) parseBrushBorders(xml, material);
                else if (xml.name().toString() == QLatin1String("friend")) parseBrushFriends(xml, material);
                else if (xml.name().toString() == QLatin1String("optional")) parseBrushOptionals(xml, material);
                else xml.skipCurrentElement();
            } else if (brushType == QLatin1String("wall")) {
                 if (xml.name().toString() == QLatin1String("wall")) parseBrushWallParts(xml, material);
                 else xml.skipCurrentElement();
            } else if (brushType == QLatin1String("doodad")) {
                if (xml.name().toString() == QLatin1String("alternate") || xml.name().toString() == QLatin1String("item") || xml.name().toString() == QLatin1String("composite")) {
                    // Doodads can have <item> or <composite> directly, or inside <alternate>
                    // parseBrushAlternates handles all these cases by checking element name.
                    parseBrushAlternates(xml, material);
                } else xml.skipCurrentElement();
            } else if (brushType == QLatin1String("carpet") || brushType == QLatin1String("table")) {
                if (xml.name().toString() == QLatin1String("carpet") || xml.name().toString() == QLatin1String("table")) {
                     parseBrushCarpetParts(xml, material); // Re-use for tables
                } else xml.skipCurrentElement();
            } else {
                xml.skipCurrentElement(); // Skip children of unknown brush types
            }
        }
    }
    m_materialsById.insert(brushName, material);
    return true;
}

// --- Specific Parsers for <brush> children ---

void MaterialManager::parseBrushItems(QXmlStreamReader& xml, MaterialData& materialData) {
    // Expects to be called when xml.isStartElement() and xml.name() == "item"
    // This is for ground brush <item id="..." chance="..."/>
    MaterialGroundSpecifics* specifics = std::get_if<MaterialGroundSpecifics>(&materialData.specificData);
    if (!specifics) return; // Should not happen if type was set correctly

    MaterialItemEntry entry;
    entry.itemId = xml.attributes().value(QLatin1String("id")).toUShort();
    if (xml.attributes().hasAttribute(QLatin1String("chance"))) {
        entry.chance = xml.attributes().value(QLatin1String("chance")).toInt();
    }
    specifics->items.append(entry);
    xml.skipCurrentElement(); // Consume the rest of this <item> element
}

void MaterialManager::parseBrushBorders(QXmlStreamReader& xml, MaterialData& materialData) {
    MaterialGroundSpecifics* specifics = std::get_if<MaterialGroundSpecifics>(&materialData.specificData);
    if (!specifics) return;

    MaterialBorderRule rule;
    rule.align = xml.attributes().value(QLatin1String("align")).toString();
    rule.toBrushName = xml.attributes().value(QLatin1String("to")).toString("none"); // Default to "none"
    rule.ruleTargetId = xml.attributes().value(QLatin1String("id")).toString(); // Changed from borderItemId
    if (xml.attributes().hasAttribute(QLatin1String("super"))) rule.isSuper = xml.attributes().value(QLatin1String("super")).toString() == "true";
    if (xml.attributes().hasAttribute(QLatin1String("ground_equivalent"))) rule.groundEquivalent = xml.attributes().value(QLatin1String("ground_equivalent")).toUShort();

    // Loop to process child elements of the current <border> tag
    while (!xml.atEnd()) {
        xml.readNext(); // Move to the next token

        if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("border"), Qt::CaseInsensitive) == 0) {
            break; // End of the current <border> element
        }

        if (xml.isStartElement()) {
            if (xml.name().toString().compare(QLatin1String("specific"), Qt::CaseInsensitive) == 0) {
                SpecificRuleCase currentCase;
                if (xml.attributes().hasAttribute(QLatin1String("keep_border"))) {
                    currentCase.keepBaseBorder = (xml.attributes().value(QLatin1String("keep_border")).toString() == "true");
                }

                // Loop for children of <specific> (i.e., <conditions> and <actions>)
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("specific"), Qt::CaseInsensitive) == 0) {
                        break; // End of current <specific> element
                    }

                    if (xml.isStartElement()) {
                        if (xml.name().toString().compare(QLatin1String("conditions"), Qt::CaseInsensitive) == 0) {
                            // Loop for children of <conditions>
                            while (!xml.atEnd()) {
                                xml.readNext();
                                if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("conditions"), Qt::CaseInsensitive) == 0) {
                                    break; // End of <conditions>
                                }
                                if (xml.isStartElement()) {
                                    if (xml.name().toString().compare(QLatin1String("match_border"), Qt::CaseInsensitive) == 0) {
                                        SpecificCondition cond;
                                        cond.type = SpecificConditionType::MATCH_BORDER;
                                        cond.targetId = xml.attributes().value(QLatin1String("id")).toString();
                                        cond.edge = xml.attributes().value(QLatin1String("edge")).toString();
                                        currentCase.conditions.append(cond);
                                        xml.skipCurrentElement(); // Consume <match_border ... />
                                    } else if (xml.name().toString().compare(QLatin1String("match_ground"), Qt::CaseInsensitive) == 0) {
                                        SpecificCondition cond;
                                        cond.type = SpecificConditionType::MATCH_GROUND;
                                        cond.targetId = xml.attributes().value(QLatin1String("id")).toString();
                                        // No 'edge' for match_ground typically
                                        currentCase.conditions.append(cond);
                                        xml.skipCurrentElement(); // Consume <match_ground ... />
                                    } else {
                                        qWarning() << "MaterialManager::parseBrushBorders: Unknown tag within <conditions>:" << xml.name().toString();
                                        xml.skipCurrentElement();
                                    }
                                }
                            } // End of <conditions> loop
                        } else if (xml.name().toString().compare(QLatin1String("actions"), Qt::CaseInsensitive) == 0) {
                            // Loop for children of <actions>
                            while (!xml.atEnd()) {
                                xml.readNext();
                                if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("actions"), Qt::CaseInsensitive) == 0) {
                                    break; // End of <actions>
                                }
                                if (xml.isStartElement()) {
                                    if (xml.name().toString().compare(QLatin1String("replace_border"), Qt::CaseInsensitive) == 0) {
                                        SpecificAction act;
                                        act.type = SpecificActionType::REPLACE_BORDER;
                                        act.targetId = xml.attributes().value(QLatin1String("id")).toString(); // Original item on edge
                                        act.edge = xml.attributes().value(QLatin1String("edge")).toString();
                                        act.withItemId = xml.attributes().value(QLatin1String("with")).toUShort();
                                        currentCase.actions.append(act);
                                        xml.skipCurrentElement(); // Consume <replace_border ... />
                                    } else if (xml.name().toString().compare(QLatin1String("add_item"), Qt::CaseInsensitive) == 0) {
                                        SpecificAction act;
                                        act.type = SpecificActionType::ADD_ITEM;
                                        act.affectedItemId = xml.attributes().value(QLatin1String("id")).toUShort(); // Item to add
                                        currentCase.actions.append(act);
                                        xml.skipCurrentElement(); // Consume <add_item ... />
                                    } else {
                                        qWarning() << "MaterialManager::parseBrushBorders: Unknown tag within <actions>:" << xml.name().toString();
                                        xml.skipCurrentElement();
                                    }
                                }
                            } // End of <actions> loop
                        } else {
                             qWarning() << "MaterialManager::parseBrushBorders: Unknown tag within <specific>:" << xml.name().toString();
                             xml.skipCurrentElement();
                        }
                    }
                } // End of <specific> loop
                rule.specificRuleCases.append(currentCase);
            } else {
                if (!xml.name().toString().isEmpty()) {
                    qWarning() << "MaterialManager::parseBrushBorders: Unknown tag '" << xml.name().toString() << "' directly under <border>. Skipping.";
                }
                // xml.skipCurrentElement(); // Don't skip if it's not a StartElement, or if it's an EndElement we need to process
                                        // This was original logic, let's ensure we only skip StartElements we don't handle
                if(xml.isStartElement()) xml.skipCurrentElement();
            }
        }
    } // End of <border> children loop

    specifics->borders.append(rule);
    // Original xml.skipCurrentElement() is removed as the loop handles consuming the element's content.
}

void MaterialManager::parseBrushFriends(QXmlStreamReader& xml, MaterialData& materialData) {
    MaterialGroundSpecifics* specifics = std::get_if<MaterialGroundSpecifics>(&materialData.specificData);
    if (!specifics) return;
    specifics->friends.insert(xml.attributes().value(QLatin1String("name")).toString());
    xml.skipCurrentElement();
}

void MaterialManager::parseBrushOptionals(QXmlStreamReader& xml, MaterialData& materialData) {
    MaterialGroundSpecifics* specifics = std::get_if<MaterialGroundSpecifics>(&materialData.specificData);
    if (!specifics) return;
    specifics->optionals.append(xml.attributes().value(QLatin1String("id")).toUShort());
    xml.skipCurrentElement();
}

void MaterialManager::parseBrushWallParts(QXmlStreamReader& xml, MaterialData& materialData) {
    // Expects xml to be at the start of a <wall type="orientation"> element
    MaterialWallSpecifics* specifics = std::get_if<MaterialWallSpecifics>(&materialData.specificData);
    if (!specifics) return;

    MaterialWallPart part;
    part.orientationType = xml.attributes().value(QLatin1String("type")).toString();

    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name() == QLatin1String("wall")) break;
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("item")) {
                MaterialItemEntry entry;
                entry.itemId = xml.attributes().value(QLatin1String("id")).toUShort();
                if (xml.attributes().hasAttribute(QLatin1String("chance"))) {
                     entry.chance = xml.attributes().value(QLatin1String("chance")).toInt();
                } else {
                    entry.chance = 100; // Default for wall items
                }
                part.items.append(entry);
                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("door")) {
                MaterialDoorDefinition door;
                door.id = xml.attributes().value(QLatin1String("id")).toUShort();
                door.doorType = xml.attributes().value(QLatin1String("type")).toString();
                if (xml.attributes().hasAttribute(QLatin1String("open"))) door.isOpen = xml.attributes().value(QLatin1String("open")).toString() == "true";
                if (xml.attributes().hasAttribute(QLatin1String("locked"))) door.isLocked = xml.attributes().value(QLatin1String("locked")).toString() == "true";
                part.doors.append(door);
                xml.skipCurrentElement();
            } else {
                xml.skipCurrentElement(); // Skip unknown tags within <wall ...>
            }
        }
    }
    specifics->parts.append(part);
}

void MaterialManager::parseBrushAlternates(QXmlStreamReader& xml, MaterialData& materialData) {
    // Expects xml to be at start of <alternate>, <item> or <composite> (for doodads without explicit <alternate>)
    MaterialDoodadSpecifics* specifics = std::get_if<MaterialDoodadSpecifics>(&materialData.specificData);
    if (!specifics) return;

    MaterialAlternate alt;
    if (xml.name().toString() == QLatin1String("alternate")) {
        if (xml.attributes().hasAttribute(QLatin1String("chance"))) {
            alt.chance = xml.attributes().value(QLatin1String("chance")).toInt();
        }
        // Now read children of <alternate> which can be <item> or <composite>
        while(!xml.atEnd()) {
            xml.readNext();
            if (xml.isEndElement() && xml.name().toString() == QLatin1String("alternate")) break;
            if (xml.isStartElement()) {
                if (xml.name().toString() == QLatin1String("item")) {
                    alt.singleItemIds.append(xml.attributes().value(QLatin1String("id")).toUShort());
                    xml.skipCurrentElement();
                } else if (xml.name().toString() == QLatin1String("composite")) {
                    // This composite is part of the current alternate
                    parseCompositeTile(xml, alt.compositeTiles); // Pass QList by ref
                } else {
                    xml.skipCurrentElement();
                }
            }
        }
    } else if (xml.name().toString() == QLatin1String("item")) {
        // Doodad brush with direct <item> children (implicitly one alternate)
        alt.singleItemIds.append(xml.attributes().value(QLatin1String("id")).toUShort());
        xml.skipCurrentElement();
    } else if (xml.name().toString() == QLatin1String("composite")) {
        // Doodad brush with direct <composite> child (implicitly one alternate)
        parseCompositeTile(xml, alt.compositeTiles);
    }
    specifics->alternates.append(alt);
}

void MaterialManager::parseCompositeTile(QXmlStreamReader& xml, QList<MaterialCompositeTile>& compositesList) {
    // Expects xml to be at start of <composite>
    MaterialAlternate tempAlternate; // Use a temporary alternate to gather tiles for *this* composite
                                     // Correction: parseCompositeTile is called when already inside an Alternate context
                                     // or a composite is directly under brush. It should populate a list of MaterialCompositeTile.
                                     // The logic needs to be for one <composite>...</composite> block.
    // This function should really populate ONE MaterialAlternate's compositeTiles or a temporary list for one composite block.
    // Let's assume this is called to parse one <composite> and add its tiles to the list passed.

    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name().toString() == QLatin1String("composite")) break;
        if (xml.isStartElement() && xml.name().toString() == QLatin1String("tile")) {
            MaterialCompositeTile compTile;
            compTile.x = xml.attributes().value(QLatin1String("x")).toInt();
            compTile.y = xml.attributes().value(QLatin1String("y")).toInt();
            if (xml.attributes().hasAttribute(QLatin1String("z"))) { // Z is optional
                 compTile.z = xml.attributes().value(QLatin1String("z")).toInt();
            }
            // Parse nested <item> tags for this tile
            while(!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement() && xml.name().toString() == QLatin1String("tile")) break;
                if (xml.isStartElement() && xml.name().toString() == QLatin1String("item")) {
                    compTile.itemIds.append(xml.attributes().value(QLatin1String("id")).toUShort());
                    xml.skipCurrentElement(); // Consume <item ... />
                }
            }
            compositesList.append(compTile);
        }
    }
}

void MaterialManager::parseBrushCarpetParts(QXmlStreamReader& xml, MaterialData& materialData) {
    // Expects xml to be at start of <carpet type="align"> or <table type="align">
    MaterialOrientedPart part;
    part.align = xml.attributes().value(QLatin1String("align")).toString();

    QString parentElementName = xml.name().toString(); // "carpet" or "table"

    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name() == parentElementName) break;
        if (xml.isStartElement() && xml.name() == QLatin1String("item")) {
            MaterialItemEntry entry;
            entry.itemId = xml.attributes().value(QLatin1String("id")).toUShort();
            if (xml.attributes().hasAttribute(QLatin1String("chance"))) {
                 entry.chance = xml.attributes().value(QLatin1String("chance")).toInt();
            } else {
                entry.chance = 100; // Default for carpet/table items
            }
            part.items.append(entry);
            xml.skipCurrentElement();
        }
    }

    if (materialData.isCarpet()) {
        MaterialCarpetSpecifics* specifics = std::get_if<MaterialCarpetSpecifics>(&materialData.specificData);
        if (specifics) specifics->parts.append(part);
    } else if (materialData.isTable()) {
        MaterialTableSpecifics* specifics = std::get_if<MaterialTableSpecifics>(&materialData.specificData);
        if (specifics) specifics->parts.append(part);
    }
}

bool MaterialManager::parseBordersFileEntry(QXmlStreamReader& xml) {
    Q_ASSERT(xml.isStartElement() && xml.name().toString().compare(QLatin1String("border"), Qt::CaseInsensitive) == 0);

    QString borderSetId = xml.attributes().value(QLatin1String("id")).toString();
    if (borderSetId.isEmpty()) {
        m_lastError = "Found <border> entry (for a border set) with no 'id' attribute.";
        qWarning() << "MaterialManager:" << m_lastError;
        xml.skipCurrentElement(); // Important to consume the element
        return false; // Or true if we want to be lenient and just skip this one
    }

    if (m_borderSetsById.contains(borderSetId)) {
        qWarning() << "MaterialManager: Duplicate border set ID '" << borderSetId << "'. Overwriting previous definition.";
        // Potentially clear existing entries if overwriting, or merge, or skip.
        // For now, simple overwrite by QMap::insert.
    }

    BorderSetData currentBorderSet(borderSetId); // Use constructor that sets ID

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name().toString().compare(QLatin1String("border"), Qt::CaseInsensitive) == 0) {
            break; // End of this <border id="SET_ID"> element
        }
        if (xml.isStartElement()) {
            if (xml.name().toString().compare(QLatin1String("borderitem"), Qt::CaseInsensitive) == 0) {
                QString edge = xml.attributes().value(QLatin1String("edge")).toString();
                bool ok = false;
                uint16_t itemId = xml.attributes().value(QLatin1String("item")).toUShort(&ok);

                if (!edge.isEmpty() && ok) {
                    currentBorderSet.edgeItems.insert(edge, itemId);
                } else {
                    qWarning() << "MaterialManager: Invalid <borderitem> in set '" << borderSetId
                               << "'. Missing 'edge' or invalid 'item' ID. Attributes:" << xml.attributes();
                }
                xml.skipCurrentElement(); // Consume the rest of <borderitem>
            } else {
                qWarning() << "MaterialManager: Unknown tag '" << xml.name().toString()
                           << "' inside <border id=\"" << borderSetId << "\">. Skipping.";
                xml.skipCurrentElement();
            }
        }
    }
    m_borderSetsById.insert(borderSetId, currentBorderSet);
    qDebug() << "MaterialManager: Parsed border set ID '" << borderSetId << "' with" << currentBorderSet.edgeItems.count() << "edge items.";
    return true;
}

} // namespace assets
} // namespace core
} // namespace RME
