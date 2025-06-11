#include "ClientVersionManager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug> // For error reporting
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace RME {

// PIMPL data structure
struct ClientVersionManager::ClientVersionManagerData {
    QList<ClientProfile> clientProfiles;
    QList<OtbVersion> otbVersions;
    QMap<QString, const ClientProfile*> profileByVersionString; // Fast lookup
    QMap<quint32, const OtbVersion*> otbById;
    QMap<QString, const OtbVersion*> otbByName;
};

ClientVersionManager::ClientVersionManager() : d(new ClientVersionManagerData()) {}
ClientVersionManager::~ClientVersionManager() = default; // QScopedPointer handles deletion

bool ClientVersionManager::loadVersions(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ClientVersionManager: Could not open clients.xml file:" << filePath;
        return false;
    }

    d->clientProfiles.clear();
    d->otbVersions.clear();
    d->profileByVersionString.clear();
    d->otbById.clear();
    d->otbByName.clear();

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("clients")) { // Root element
                // This is the root, we need to find the children <otbs> and the list <clients>
                // QXmlStreamReader naturally iterates into children when readNext() is called.
                // This structure is a bit confusing: root is <clients>, then <otbs>, then another <clients> for the list.
                // The loop structure will handle finding these children.
            } else if (xml.name() == QLatin1String("otbs")) {
                parseOtbsSection(xml);
            } else if (xml.name() == QLatin1String("client")) { // This is a client entry, should be inside the second <clients> tag
                 // This case implies the structure might be <clients><client/><client/></clients>
                 // If there's a top-level <clients> that IS the list, this needs adjustment.
                 // Based on typical XML, the second "clients" (plural) inside the main "clients" (plural) is odd.
                 // Assuming it's <clients_root><otbs>...</otbs><clients_list_tag>...</clients_list_tag></clients_root>
                 // For now, let's assume the initial loop structure is okay and parseClientsSection is called for the list tag.
                 // If the list tag is also "clients", the first xml.name()=="clients" check needs to be more specific
                 // or ensure parseClientsSection is robust enough.
                 // The provided XML structure in YAML was: <clients><otbs>...</otbs><clients>...</clients></clients>
                 // This means the outer "clients" is the root. The inner "clients" is the list.
                 // The current logic should be okay if the first if(xml.name() == "clients") just continues.
                 // The second time it encounters "clients" (the list), it should fall into parseClientsSection.
                 // This was a misinterpretation, the second if should be `xml.name() == "clients_list_tag_name"`
                 // Let's assume the list tag is also called "clients" as per the YAML example.
                parseClientsSection(xml); // This should be called if xml.name() is the *list* of clients.
            }
        }
    }

    // Correction: The outer loop should find the main "clients" (root).
    // Inside that, it should specifically look for "otbs" and then the *second* "clients" (list of client profiles).
    // Resetting and re-parsing with a more explicit structure:
    file.seek(0); // Reset file pointer to reread
    xml.setDevice(&file); // Re-associate reader

    d->clientProfiles.clear(); // Clear again due to re-parse
    d->otbVersions.clear();

    if (xml.readNextStartElement() && xml.name() == QLatin1String("clients")) { // Root element
        while(xml.readNextStartElement()){ // Iterate through children of root <clients>
            if(xml.name() == QLatin1String("otbs")){
                parseOtbsSection(xml);
            } else if (xml.name() == QLatin1String("clients")){ // This is the list of <client> nodes
                parseClientsSection(xml);
            } else {
                xml.skipCurrentElement(); // Skip other elements under root <clients>
            }
        }
    } else {
         qWarning() << "ClientVersionManager: Could not find root <clients> element in" << filePath;
         return false;
    }


    if (xml.hasError()) {
        qWarning() << "ClientVersionManager: XML parsing error in" << filePath << ":" << xml.errorString();
        return false;
    }

    // Populate lookup maps
    // Need to use a different loop variable name to avoid shadowing if this code is moved
    for (int i = 0; i < d->clientProfiles.size(); ++i) {
        // Ensure profile is mutable to set userSetClientPath later
        ClientProfile& profile = d->clientProfiles[i];
        d->profileByVersionString.insert(profile.versionString, &profile);
    }
    for (int i = 0; i < d->otbVersions.size(); ++i) {
        d->otbById.insert(d->otbVersions[i].clientID, &d->otbVersions[i]);
        d->otbByName.insert(d->otbVersions[i].name, &d->otbVersions[i]);
    }

    // Load custom client paths after parsing clients.xml and populating profiles
    QString clientPathsJsonPath;
    QFileInfo clientsXmlInfo(filePath); // filePath is the path to clients.xml
    clientPathsJsonPath = clientsXmlInfo.dir().filePath("client_custom_paths.json");
    // TODO: Consider making client_custom_paths.json location configurable or also checking user config dirs.

    qInfo() << "ClientVersionManager: Attempting to load custom client paths from:" << clientPathsJsonPath;
    if (!loadClientPaths(clientPathsJsonPath)) {
        qWarning() << "ClientVersionManager: Failed to load or parse custom client paths from" << clientPathsJsonPath << ". Proceeding without them.";
        // This is not a fatal error for loadVersions itself.
    }

    qInfo() << "ClientVersionManager: Successfully loaded" << d->clientProfiles.size() << "client profiles and"
            << d->otbVersions.size() << "OTB versions from" << filePath;
    return !d->clientProfiles.isEmpty(); // Consider it successful if at least one profile loaded
}

void ClientVersionManager::parseOtbsSection(QXmlStreamReader& xml) {
    // Assumes xml is currently at the <otbs> start element
    while (xml.readNextStartElement()) { // Iterates over <otb> children
        if (xml.name() == QLatin1String("otb")) {
            OtbVersion otb;
            otb.name = xml.attributes().value("name").toString();
            otb.formatVersion = xml.attributes().value("format_version").toUInt();
            otb.clientID = xml.attributes().value("id").toUInt();
            otb.description = xml.attributes().value("desc").toString();
            d->otbVersions.append(otb);
            xml.skipCurrentElement(); // Consume the rest of the <otb> element
        } else {
            xml.skipCurrentElement(); // Skip other elements within <otbs>
        }
    }
    // After the loop, xml should be at the EndElement of <otbs>
}

void ClientVersionManager::parseClientsSection(QXmlStreamReader& xml) {
    // Assumes xml is currently at the <clients> (list) start element
    while (xml.readNextStartElement()) { // Iterates over <client> children
        if (xml.name() == QLatin1String("client")) {
            ClientProfile profile;
            profile.versionString = xml.attributes().value("version").toString();
            profile.name = xml.attributes().value("desc").toString();
            profile.clientOtbmVersionId = xml.attributes().value("otb").toUInt();

            profile.datPathHint = xml.attributes().value("dat").toString();
            profile.sprPathHint = xml.attributes().value("spr").toString();
            profile.picPathHint = xml.attributes().value("pic").toString();

            parseClientNode(xml, profile); // Parses children of <client>
            d->clientProfiles.append(profile);
            // parseClientNode consumes the <client> element, so no skipCurrentElement here
        } else {
            xml.skipCurrentElement(); // Skip other elements within the <clients> list tag
        }
    }
    // After the loop, xml should be at the EndElement of the <clients> list tag
}


void ClientVersionManager::parseClientNode(QXmlStreamReader& xml, ClientProfile& profile) {
    // Assumes xml is currently at the <client> start element
    while (xml.readNextStartElement()) { // Iterates over children of <client>
        if (xml.name() == QLatin1String("otbm")) {
            QStringList versions = xml.readElementText().split(',', Qt::SkipEmptyParts);
            for (const QString& v : versions) {
                profile.supportedOtbmVersions.append(v.toUInt());
            }
        } else if (xml.name() == QLatin1String("paths")) {
            profile.datPathHint = xml.attributes().value("dat", profile.datPathHint).toString();
            profile.sprPathHint = xml.attributes().value("spr", profile.sprPathHint).toString();
            profile.picPathHint = xml.attributes().value("pic", profile.picPathHint).toString();
            xml.skipCurrentElement(); // Consume the rest of the <paths> element
        } else if (xml.name() == QLatin1String("signatures")) {
            parseSignaturesNode(xml, profile); // This will consume the <signatures> element
        } else if (xml.name() == QLatin1String("extensions")) {
            parseExtensionsNode(xml, profile); // This will consume the <extensions> element
        } else if (xml.name() == QLatin1String("otfi")) {
             profile.customOtfIndexPath = xml.readElementText();
        } else {
            xml.skipCurrentElement();
        }
    }
    // After the loop, xml should be at the EndElement of <client>
}

void ClientVersionManager::parseSignaturesNode(QXmlStreamReader& xml, ClientProfile& profile) {
    // Assumes xml is currently at the <signatures> start element
    profile.datFormat = datFormatFromString(xml.attributes().value("dat_format").toString());

    while (xml.readNextStartElement()) { // Iterates over <dat> or <spr> children
        if (xml.name() == QLatin1String("dat")) {
            parseDatSprNode(xml, "dat", profile.datSignatures); // Consumes <dat>
        } else if (xml.name() == QLatin1String("spr")) {
            parseDatSprNode(xml, "spr", profile.sprSignatures); // Consumes <spr>
        } else {
            xml.skipCurrentElement();
        }
    }
    // After the loop, xml should be at the EndElement of <signatures>
}

void ClientVersionManager::parseDatSprNode(QXmlStreamReader& xml, const QString& type, QMap<QString, QByteArray>& signatureMap) {
    // Assumes xml is currently at the <dat> or <spr> start element
    QString key = xml.attributes().value("key").toString();
    QString signatureHex = xml.readElementText(); // Consumes the element
    if (!key.isEmpty() && !signatureHex.isEmpty()) {
        signatureMap.insert(key, QByteArray::fromHex(signatureHex.toLatin1()));
    }
}

void ClientVersionManager::parseExtensionsNode(QXmlStreamReader& xml, ClientProfile& profile) {
    // Assumes xml is currently at the <extensions> start element
    profile.extendedSprites = xml.attributes().value("extended").toString() == "true";
    profile.transparentSprites = xml.attributes().value("transparent").toString() == "true";
    profile.frameDurations = xml.attributes().value("frame_durations").toString() == "true";
    profile.patternZ = xml.attributes().value("pattern_z").toString() == "true";
    profile.looktypeU16 = xml.attributes().value("u16_looktype").toString() == "true";
    profile.actionInstancing = xml.attributes().value("action_instancing").toString() == "true";
    xml.skipCurrentElement(); // Consume the rest of the <extensions> element
}


DatFormat ClientVersionManager::datFormatFromString(const QString& formatStr) const {
    // This is a simplified mapping. The original RME has a more complex one.
    // Refer to `wxwidgets/client_version.cpp` `ClientVersion::StringToDatFormat`
    if (formatStr.isEmpty() || formatStr == "default") return DatFormat::V_760;
    if (formatStr == "7.55") return DatFormat::V_755;
    if (formatStr == "7.7" || formatStr == "7.70") return DatFormat::V_770;
    if (formatStr == "7.8-7.92") return DatFormat::V_780_792;
    if (formatStr == "8.0-8.1") return DatFormat::V_800_801; // Approximation
    if (formatStr == "8.10-8.11") return DatFormat::V_810_811;
    if (formatStr == "8.20") return DatFormat::V_820;
    if (formatStr == "8.30") return DatFormat::V_830;
    if (formatStr == "8.40-8.42") return DatFormat::V_840_842;
    if (formatStr == "8.50-8.54") return DatFormat::V_850_854;
    if (formatStr == "8.55-8.57") return DatFormat::V_855_857;
    if (formatStr == "8.60-8.62") return DatFormat::V_860_862;
    if (formatStr == "8.70-8.73") return DatFormat::V_870_873;
    if (formatStr == "9.0") return DatFormat::V_900;
    if (formatStr == "9.1") return DatFormat::V_910;
    if (formatStr == "9.4-9.46") return DatFormat::V_940_946;
    if (formatStr == "9.5-9.54") return DatFormat::V_950_954;
    if (formatStr == "9.6-9.63") return DatFormat::V_960_963;
    if (formatStr == "9.70") return DatFormat::V_970;
    if (formatStr == "9.80-9.86") return DatFormat::V_980_986;
    if (formatStr == "10.00-10.01") return DatFormat::V_1000_1001;
    if (formatStr == "10.10") return DatFormat::V_1010;
    if (formatStr == "10.20") return DatFormat::V_1020;
    if (formatStr == "10.30-10.38") return DatFormat::V_1030_1038;
    if (formatStr == "10.41") return DatFormat::V_1041;
    if (formatStr == "10.50-10.57") return DatFormat::V_1050_1057;
    if (formatStr == "10.61-10.62") return DatFormat::V_1061_1062;
    if (formatStr == "10.70-10.74") return DatFormat::V_1070_1074;
    if (formatStr == "10.75-10.77") return DatFormat::V_1075_1077;
    if (formatStr == "10.80") return DatFormat::V_1080;
    if (formatStr == "10.90-10.94") return DatFormat::V_1090_1094;
    if (formatStr == "10.95-10.99" || formatStr == "10.9x") return DatFormat::V_1095_1099;
    if (formatStr == "11.00+") return DatFormat::V_1100_PLUS;
    if (formatStr == "custom") return DatFormat::CUSTOM;

    qWarning() << "ClientVersionManager: Unknown dat_format string:" << formatStr << "Defaulting to UNKNOWN.";
    return DatFormat::UNKNOWN;
}


const QList<ClientProfile>& ClientVersionManager::getClientProfiles() const {
    return d->clientProfiles;
}

const ClientProfile* ClientVersionManager::getClientProfile(const QString& versionString) const {
    return d->profileByVersionString.value(versionString, nullptr);
}

const ClientProfile* ClientVersionManager::getDefaultClientProfile() const {
    return d->clientProfiles.isEmpty() ? nullptr : &d->clientProfiles.first();
}

const QList<OtbVersion>& ClientVersionManager::getOtbVersions() const {
    return d->otbVersions;
}

const OtbVersion* ClientVersionManager::getOtbVersionById(quint32 id) const {
    return d->otbById.value(id, nullptr);
}

const OtbVersion* ClientVersionManager::getOtbVersionByName(const QString& name) const {
    return d->otbByName.value(name, nullptr);
}

bool ClientVersionManager::saveClientPaths(const QString& saveFilePath) const {
    QJsonArray clientsArray;
    for (const ClientProfile& profile : d->clientProfiles) {
        if (!profile.userSetClientPath.isEmpty()) {
            QJsonObject profileObject;
            profileObject[QStringLiteral("id")] = profile.versionString; // Use versionString as the persistent ID
            profileObject[QStringLiteral("path")] = profile.userSetClientPath;
            clientsArray.append(profileObject);
        }
    }

    QJsonDocument doc(clientsArray);
    QFile jsonFile(saveFilePath);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "ClientVersionManager: Failed to open client paths file for writing:" << saveFilePath;
        return false;
    }

    qint64 bytesWritten = jsonFile.write(doc.toJson(QJsonDocument::Indented));
    jsonFile.close();

    if (bytesWritten == -1) {
        qWarning() << "ClientVersionManager: Failed to write to client paths file:" << saveFilePath;
        return false;
    }
    return true;
}

bool ClientVersionManager::loadClientPaths(const QString& loadFilePath) {
    QFile jsonFile(loadFilePath);
    if (!jsonFile.exists()) {
        qInfo() << "ClientVersionManager: Client paths file not found, no custom paths loaded:" << loadFilePath;
        return true; // Not an error if the file doesn't exist, just no custom paths.
    }
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ClientVersionManager: Failed to open client paths file for reading:" << loadFilePath;
        return false;
    }

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "ClientVersionManager: Failed to parse client paths JSON from" << loadFilePath << "Error:" << parseError.errorString();
        return false;
    }

    if (!doc.isArray()) {
        qWarning() << "ClientVersionManager: Client paths JSON root is not an array in" << loadFilePath;
        return false;
    }

    QJsonArray clientsArray = doc.array();
    for (const QJsonValue& val : clientsArray) {
        if (!val.isObject()) {
            qWarning() << "ClientVersionManager: Found non-object value in client paths array in" << loadFilePath;
            continue;
        }
        QJsonObject profileObject = val.toObject();
        if (profileObject.contains(QStringLiteral("id")) && profileObject.contains(QStringLiteral("path"))) {
            QString versionId = profileObject[QStringLiteral("id")].toString();
            QString path = profileObject[QStringLiteral("path")].toString();

            // Find the ClientProfile by versionString (which we stored as 'id')
            // Important: Need to modify the actual ClientProfile in d->clientProfiles, not a copy.
            ClientProfile* profile = nullptr;
            for (ClientProfile& p : d->clientProfiles) { // Iterate by reference
                if (p.versionString == versionId) {
                    profile = &p;
                    break;
                }
            }

            if (profile) {
                profile->userSetClientPath = path;
                qDebug() << "ClientVersionManager: Loaded custom path for" << versionId << ":" << path;
            } else {
                qWarning() << "ClientVersionManager: Could not find client profile for id '" << versionId << "' when loading custom paths from" << loadFilePath;
            }
        } else {
            qWarning() << "ClientVersionManager: Client path entry missing 'id' or 'path' in" << loadFilePath;
        }
    }
    return true;
}

} // namespace RME
