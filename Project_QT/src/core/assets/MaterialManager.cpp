#include "core/assets/MaterialManager.h"
#include "core/assets/AssetManager.h"
#include <QFile>
#include <QDir>
#include <QXmlStreamReader> // Changed from tinyxml2.h
#include <QDebug>
#include <QFileInfo>

namespace RME {
namespace core {
namespace assets {

// Helper function to convert QStringRef to quint16, with default
static quint16 toUShort(const QStringRef& strRef, quint16 def = 0) {
    bool ok;
    quint16 val = strRef.toUShort(&ok);
    return ok ? val : def;
}

// Helper function to convert QStringRef to int, with default
static int toInt(const QStringRef& strRef, int def = 0) {
    bool ok;
    int val = strRef.toInt(&ok);
    return ok ? val : def;
}

// Helper function to convert QStringRef to bool
static bool toBool(const QStringRef& strRef, bool def = false) {
    if (strRef.isEmpty()) return def;
    QString lower = strRef.toString().toLower();
    if (lower == "true" || lower == "yes" || lower == "1") return true;
    if (lower == "false" || lower == "no" || lower == "0") return false;
    return def;
}


MaterialManager::MaterialManager() {
}

MaterialManager::~MaterialManager() {
}

bool MaterialManager::loadMaterialsFromDirectory(const QString& directoryPath, const QString& mainFileName, AssetManager& assetManager) {
    m_lastError.clear();
    QSet<QString> processedIncludes;
    QDir dir(directoryPath);
    QString fullPathToMainFile = dir.filePath(mainFileName);

    QFileInfo mainFileInfo(fullPathToMainFile);
    if (!mainFileInfo.exists() || !mainFileInfo.isFile()) {
        m_lastError = "Main material file not found: " + fullPathToMainFile;
        qWarning() << "MaterialManager:" << m_lastError;
        return false;
    }

    parseMaterialFile(mainFileInfo.canonicalFilePath(), mainFileInfo.canonicalPath(), assetManager, processedIncludes);

    if (!m_lastError.isEmpty()) {
        qWarning() << "MaterialManager: Errors occurred during material loading. Last error:" << m_lastError;
    }
    return m_lastError.isEmpty(); // Return true only if no errors.
}

void MaterialManager::parseMaterialFile(const QString& xmlFilePath, const QString& baseDirectoryPath, AssetManager& assetManager, QSet<QString>& processedIncludes) {
    QString canonicalFilePath = QFileInfo(xmlFilePath).canonicalFilePath();
    if (processedIncludes.contains(canonicalFilePath)) {
        return;
    }
    processedIncludes.insert(canonicalFilePath);

    QFile file(xmlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Failed to open XML file: " + xmlFilePath + ". Error: " + file.errorString();
        qWarning() << "MaterialManager:" << m_lastError;
        return;
    }

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement() || reader.name().toString() != "materials") {
        m_lastError = "Invalid material XML: Missing <materials> root element in " + xmlFilePath;
        qWarning() << "MaterialManager:" << m_lastError << "(Actual root:" << reader.name().toString() << ")";
        if (reader.hasError()) {
             m_lastError += " XML Reader Error: " + reader.errorString();
        }
        file.close();
        return;
    }

    while (reader.readNextStartElement()) { // Reads next child of <materials>
        QString elementName = reader.name().toString();
        if (elementName == "include") {
            QString includePath = reader.attributes().value("file").toString();
            if (!includePath.isEmpty()) {
                QDir baseDir(baseDirectoryPath);
                QString includedFilePath = baseDir.filePath(includePath);
                // Pass the directory of the *current* file as the base for nested includes
                parseMaterialFile(includedFilePath, QFileInfo(includedFilePath).path(), assetManager, processedIncludes);
            } else {
                m_lastError = "<include> tag missing 'file' attribute in " + xmlFilePath;
                qWarning() << "MaterialManager:" << m_lastError;
            }
            reader.skipCurrentElement(); // Important to consume the rest of the <include> element
        } else if (elementName == "brush") {
            QString brushName = reader.attributes().value("name").toString();
            if (!brushName.isEmpty()) {
                MaterialData materialData(brushName);
                parseBrushElement(reader, materialData, assetManager); // Pass reader by reference
                if (!materialData.id.isEmpty()) { // Should be same as brushName
                    m_materialsById[materialData.id] = materialData;
                } else {
                     m_lastError = "Brush found with no name (parsed as empty) in " + xmlFilePath;
                     qWarning() << "MaterialManager:" << m_lastError;
                }
            } else {
                m_lastError = "<brush> tag missing 'name' attribute in " + xmlFilePath;
                qWarning() << "MaterialManager:" << m_lastError;
                reader.skipCurrentElement(); // Skip this invalid brush
            }
        } else {
            qWarning() << "MaterialManager: Unknown element" << elementName << "in" << xmlFilePath;
            reader.skipCurrentElement();
        }
    }

    file.close();
    if (reader.hasError()) {
        m_lastError = "XML parsing error in " + xmlFilePath + ": " + reader.errorString() +
                      " at line " + QString::number(reader.lineNumber()) +
                      ", col " + QString::number(reader.columnNumber());
        qWarning() << "MaterialManager:" << m_lastError;
    }
}

void MaterialManager::parseBrushElement(QXmlStreamReader& reader, MaterialData& materialData, AssetManager& assetManager) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "brush");

    QXmlStreamAttributes attrs = reader.attributes();
    materialData.brushType = attrs.value("type").toString().toLower();
    materialData.serverLookId = toUShort(attrs.value("server_lookid"));
    materialData.lookId = toUShort(attrs.value("lookid"), materialData.serverLookId);
    materialData.zOrder = toInt(attrs.value("z-order"));
    materialData.soloOptional = toBool(attrs.value("solo_optional"));
    materialData.onBlocking = toBool(attrs.value("on_blocking"));
    materialData.redoBorders = toBool(attrs.value("redo_borders"));
    materialData.oneSize = toBool(attrs.value("one_size"));
    materialData.thickness = attrs.value("thickness").toString();

    while (reader.readNextStartElement()) {
        QString childName = reader.name().toString();
        if (childName == "item") {
            parseItemChild(reader, materialData, nullptr);
        } else if (childName == "border") {
            parseBorderChild(reader, materialData);
        } else if (childName == "friend") {
            parseFriendChild(reader, materialData);
        } else if (childName == "optional") {
            parseOptionalChild(reader, materialData);
        } else if (childName == "wall" && materialData.brushType == "wall") {
            parseWallChild(reader, materialData);
        } else if (childName == "composite" && materialData.brushType == "doodad") {
            parseCompositeChild(reader, materialData);
        } else {
            qWarning() << "MaterialManager: Unknown child" << childName << "under brush" << materialData.id;
            reader.skipCurrentElement();
        }
    }
    // After loop, reader is at EndElement of brush or parent if no children
}

void MaterialManager::parseItemChild(QXmlStreamReader& reader, MaterialData& materialData, MaterialWallPart* wallPart) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "item");
    QXmlStreamAttributes attrs = reader.attributes();
    quint16 itemId = toUShort(attrs.value("id"));
    int chance = toInt(attrs.value("chance"), 100);

    if (itemId != 0) {
        if (wallPart) {
            wallPart->items.append(MaterialWallPartItem(itemId, chance));
        } else {
            materialData.primaryItems.append(MaterialItem(itemId, chance));
        }
    } else {
        // qWarning() << "MaterialManager: Item tag with id=0 for brush" << materialData.id;
    }
    reader.skipCurrentElement(); // Consume the rest of this <item> element
}

void MaterialManager::parseBorderChild(QXmlStreamReader& reader, MaterialData& materialData) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "border");
    QXmlStreamAttributes attrs = reader.attributes();
    MaterialBorder border;
    border.align = attrs.value("align").toString().toLower();
    border.borderSetId = attrs.value("id").toString();
    border.toMaterialName = attrs.value("to").toString();
    border.groundEquivalentId = toUShort(attrs.value("ground_equivalent"));
    border.isSuper = toBool(attrs.value("super"));

    if (!border.borderSetId.isEmpty()) {
        materialData.borders.append(border);
    } else {
        qWarning() << "MaterialManager: Border tag missing 'id' attribute for brush" << materialData.id;
    }
    reader.skipCurrentElement();
}

void MaterialManager::parseFriendChild(QXmlStreamReader& reader, MaterialData& materialData) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "friend");
    QString name = reader.attributes().value("name").toString();
    if (!name.isEmpty()) {
        materialData.friendMaterials.append(name);
    } else {
         qWarning() << "MaterialManager: Friend tag missing 'name' attribute for brush" << materialData.id;
    }
    reader.skipCurrentElement();
}

void MaterialManager::parseOptionalChild(QXmlStreamReader& reader, MaterialData& materialData) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "optional");
    QString id = reader.attributes().value("id").toString();
    if (!id.isEmpty()) {
        materialData.optionalBorderSetIds.append(id);
    } else {
        qWarning() << "MaterialManager: Optional tag missing 'id' attribute for brush" << materialData.id;
    }
    reader.skipCurrentElement();
}

// New helper for <door> elements, as it's nested
void parseDoorChild(QXmlStreamReader& reader, MaterialWallPart& wallPart) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "door");
    QXmlStreamAttributes attrs = reader.attributes();
    MaterialDoor door;
    door.itemId = toUShort(attrs.value("id"));
    door.type = attrs.value("type").toString().toLower();
    door.isOpen = toBool(attrs.value("open"));
    door.isLocked = toBool(attrs.value("locked"));
    if (door.itemId != 0) {
        wallPart.doors.append(door);
    } else {
        qWarning() << "MaterialManager: Door tag with id=0 for wall type" << wallPart.type;
    }
    reader.skipCurrentElement();
}

void MaterialManager::parseWallChild(QXmlStreamReader& reader, MaterialData& materialData) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "wall");
    QString wallTypeAttr = reader.attributes().value("type").toString().toLower();
    if (wallTypeAttr.isEmpty()) {
        qWarning() << "MaterialManager: Wall tag missing 'type' attribute for brush" << materialData.id;
        reader.skipCurrentElement();
        return;
    }
    MaterialWallPart& part = materialData.getOrCreateWallPart(wallTypeAttr);

    while (reader.readNextStartElement()) {
        QString childName = reader.name().toString();
        if (childName == "item") {
            parseItemChild(reader, materialData, &part); // Pass wallPart context
        } else if (childName == "door") {
            parseDoorChild(reader, part); // Use new helper
        } else {
            qWarning() << "MaterialManager: Unknown child" << childName << "under wall type" << wallTypeAttr << "in brush" << materialData.id;
            reader.skipCurrentElement();
        }
    }
    // After loop, reader is at EndElement of wall or parent if no children
}

void MaterialManager::parseCompositeChild(QXmlStreamReader& reader, MaterialData& materialData) {
    Q_ASSERT(reader.isStartElement() && reader.name().toString() == "composite");
    MaterialComposite composite(toInt(reader.attributes().value("chance"), 100));

    while (reader.readNextStartElement()) {
        if (reader.name().toString() == "tile") {
            QXmlStreamAttributes attrs = reader.attributes();
            MaterialCompositeTile mct;
            mct.relativeX = toInt(attrs.value("x"));
            mct.relativeY = toInt(attrs.value("y"));
            mct.itemId = toUShort(attrs.value("id"));
            if (mct.itemId != 0) {
                composite.tiles.append(mct);
            } else {
                 qWarning() << "MaterialManager: Composite tile with id=0 for brush" << materialData.id;
            }
            reader.skipCurrentElement(); // Consume <tile ... />
        } else {
            qWarning() << "MaterialManager: Unknown child" << reader.name().toString() << "under composite in brush" << materialData.id;
            reader.skipCurrentElement();
        }
    }
    if (!composite.tiles.isEmpty()) {
        materialData.composites.append(composite);
    }
    // After loop, reader is at EndElement of composite or parent
}

const MaterialData* MaterialManager::getMaterial(const QString& id) const {
    return m_materialsById.value(id, nullptr);
}

} // namespace assets
} // namespace core
} // namespace RME
