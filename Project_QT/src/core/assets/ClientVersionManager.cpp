#include "ClientVersionManager.h"
#include "ClientProfile.h" // For DatFormat enum, OtbVersionInfo, ClientProfile structs

#include <QFile>
#include <QXmlStreamReader>
#include <QDebug> // For error reporting during development
#include <QRegularExpression> // For version string to numeric conversion
#include <QJsonDocument> // For saving/loading user paths
#include <QJsonObject>   // For saving/loading user paths
#include <QJsonArray>    // For saving/loading user paths
#include <QDir>          // For path operations with user paths JSON

// Define the private data structure for PIMPL
struct RME::Assets::ClientVersionManager::ClientVersionManagerData {
    QList<RME::Assets::ClientProfile> clientProfiles;
    QList<RME::Assets::OtbVersionInfo> otbVersionInfos;
    QString lastError;

    // For quick lookups, populated after parsing
    QMap<quint16, const RME::Assets::ClientProfile*> profileByNumericVersion;
    QMap<QString, const RME::Assets::ClientProfile*> profileByVersionString;
    QMap<QString, const RME::Assets::ClientProfile*> profileByName;
    QMap<QString, const RME::Assets::OtbVersionInfo*> otbInfoByName;

    // To store loaded user paths before applying them or during saving
    // Key: versionString (e.g., "7.60"), Value: path string
    QMap<QString, QString> userConfiguredPaths;
};

namespace RME {
namespace Assets {

ClientVersionManager::ClientVersionManager() : d(new ClientVersionManagerData) {
}

ClientVersionManager::~ClientVersionManager() {
    // QScopedPointer handles deletion of d
}

bool ClientVersionManager::loadVersions(const QString& clientsXmlPath) {
    d->clientProfiles.clear();
    d->otbVersionInfos.clear();
    d->profileByNumericVersion.clear();
    d->profileByVersionString.clear();
    d->profileByName.clear();
    d->otbInfoByName.clear();
    d->lastError.clear();

    QFile file(clientsXmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        d->lastError = QString("Failed to open clients.xml: %1 (Path: %2)").arg(file.errorString()).arg(clientsXmlPath);
        qWarning() << d->lastError;
        return false;
    }

    QXmlStreamReader xml(&file);

    // Explicitly find the root <client_config> element first.
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("client_config")) {
        d->lastError = "clients.xml is not valid: Missing root <client_config> element.";
        qWarning() << d->lastError;
        return false;
    }

    // Iterate through children of the root <client_config> element
    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("otbs")) {
            parseOtbVersionInfoSection(xml); // Consumes the <otbs> element
        } else if (xml.name() == QLatin1String("clients")) { // The list of clients is under <clients>
            parseClientsSection(xml);        // Consumes the <clients> element (list of profiles)
        } else {
            xml.skipCurrentElement(); // Skip any other unexpected elements under root like <conversions>
        }
    }

    if (xml.hasError()) {
        d->lastError = QString("XML parsing error in %1: %2 at line %3, column %4.")
                           .arg(clientsXmlPath, xml.errorString(),
                                QString::number(xml.lineNumber()),
                                QString::number(xml.columnNumber()));
        qWarning() << d->lastError;
        // Clear partially parsed data if error occurs
        d->clientProfiles.clear();
        d->otbVersionInfos.clear();
        return false;
    }

    // Populate lookup maps
    for (const auto& otbInfo : d->otbVersionInfos) {
        d->otbInfoByName.insert(otbInfo.name, &otbInfo);
    }
    for (const auto& profile : d->clientProfiles) {
        d->profileByNumericVersion.insert(profile.numericVersion, &profile);
        d->profileByVersionString.insert(profile.versionString, &profile);
        d->profileByName.insert(profile.name, &profile);
    }

    // Load and apply user-configured paths
    // Determine path for client_custom_paths.json relative to clients.xml
    QFileInfo clientsXmlInfo(clientsXmlPath);
    QString userPathsFile = clientsXmlInfo.dir().filePath("client_custom_paths.json");
    if (loadUserClientPaths(userPathsFile)) { // Loads into d->userConfiguredPaths
        applyUserPathsToProfiles(); // Applies them to d->clientProfiles
    } else {
        qInfo() << "No custom client paths loaded or error during loading from:" << userPathsFile;
        // Not necessarily a fatal error for loadVersions itself.
    }

    return d->lastError.isEmpty() && (!d->clientProfiles.isEmpty() || !d->otbVersionInfos.isEmpty());
}

void ClientVersionManager::parseOtbVersionInfoSection(QXmlStreamReader& xml) {
    // Assumes xml is at the start of <otbs>
    if (!xml.isStartElement() || xml.name() != QLatin1String("otbs")) return;

    while (xml.readNextStartElement()) { // Iterates over <otb> children
        if (xml.name() == QLatin1String("otb")) {
            parseSingleOtbVersionInfo(xml); // Consumes the <otb> element
        } else {
            xml.skipCurrentElement(); // Skip other unexpected elements within <otbs>
        }
    }
    // After the loop, xml should be at the EndElement of <otbs>
}

void ClientVersionManager::parseSingleOtbVersionInfo(QXmlStreamReader& xml) {
    // Assumes xml is at the start of <otb>
    // XML example: <otb client="7.60" version="1" id="3"/>
    OtbVersionInfo otbInfo;
    QString clientVersionStr = xml.attributes().value("client").toString();
    otbInfo.name = clientVersionStr; // Use the 'client' attribute as the reference name
    otbInfo.formatVersionMajor = xml.attributes().value("version").toUInt(); // This is the OTB format version (1, 2, 3)

    // Store the original ID from XML, might be useful.
    otbInfo.xmlOriginalId = xml.attributes().value("id").toUInt();

    // Parse Minor and Build from clientVersionStr (e.g., "7.60")
    QStringList versionParts = clientVersionStr.split('.');
    if (!versionParts.isEmpty()) {
        otbInfo.parsedClientMajor = versionParts[0].toUShort();
        if (versionParts.size() > 1) {
            QString minorPart = versionParts[1];
            minorPart.remove(QRegularExpression("[^\\d]")); // Remove non-digits
            otbInfo.parsedClientMinor = minorPart.toUShort();
        }
    }
    // No direct 'desc' or 'build' attributes for OtbVersionInfo in this XML structure for <otb>
    // The 'clientIDs' list is also not applicable here as each <otb> is for one client version string.

    xml.skipCurrentElement(); // Consume the rest of the <otb> element as it has no children in the provided XML

    d->otbVersionInfos.append(otbInfo);
    qDebug() << "Parsed OTB Info:" << otbInfo.name << "Major:" << otbInfo.formatVersionMajor << "XML ID:" << otbInfo.xmlOriginalId;
}

void ClientVersionManager::parseClientsSection(QXmlStreamReader& xml) {
    // Assumes xml is at the start of <clients> (the list of client profiles)
    if (!xml.isStartElement() || xml.name() != QLatin1String("clients")) return;

    while (xml.readNextStartElement()) { // Iterates over <client> children
        if (xml.name() == QLatin1String("client")) {
            parseSingleClientProfile(xml); // Consumes the <client> element
        } else {
            xml.skipCurrentElement(); // Skip other unexpected elements
        }
    }
    // After the loop, xml should be at the EndElement of <clients> (list)
}

void ClientVersionManager::parseSingleClientProfile(QXmlStreamReader& xml) {
    // Assumes xml is at the start of <client>
    // XML Example: <client name="7.6" otb="7.60" visible="true" data_directory="760">
    ClientProfile profile;
    profile.name = xml.attributes().value("name").toString(); // This is "7.6"
    profile.otbNameReference = xml.attributes().value("otb").toString(); // This is "7.60"
    profile.clientDataPathHint = xml.attributes().value("data_directory").toString();
    // versionString in ClientProfile should be the one people identify with, e.g. "7.6" or "7.60".
    // The 'name' attribute ("7.6") seems like a good candidate for versionString.
    // Let's use 'name' for profile.versionString and derive numeric from it.
    profile.versionString = profile.name;
    profile.numericVersion = versionStringToNumeric(profile.versionString);
    // picPathHint is not directly in <client> attributes, might be a convention (e.g. version.pic inside data_directory)

    // Default visibility, can be overridden by attribute
    profile.visibleInUI = (xml.attributes().value("visible").toString() == "true");
    profile.isDefaultChoice = (xml.attributes().value("default").toString() == "true");


    while (xml.readNextStartElement()) { // Children of <client>
        if (xml.name() == QLatin1String("otbm")) {
            // XML: <otbm version="1"/>
            MapVersionSupportInfo mvInfo;
            mvInfo.otbmVersion = xml.attributes().value("version").toUShort();
            mvInfo.clientVersionNumeric = profile.numericVersion; // Parent profile's version
            // mvInfo.description is not in the XML for this tag.
            profile.supportedMapVersions.append(mvInfo);
            xml.skipCurrentElement();
        } else if (xml.name() == QLatin1String("extensions")) {
            // XML: <extensions from="7.6" to="7.6"/>
            profile.extensions.insert("from", xml.attributes().value("from").toString());
            profile.extensions.insert("to", xml.attributes().value("to").toString());
            xml.skipCurrentElement();
        } else if (xml.name() == QLatin1String("data")) {
            // XML: <data format="7.55" dat="0x..." spr="0x..."/>
            ClientSignature sig;
            // 'type' for ClientSignature could be the client version string itself or derived.
            // For now, let's use the profile's main version string.
            sig.type = profile.versionString;
            sig.datSignatureHex = xml.attributes().value("dat").toString();
            sig.sprSignatureHex = xml.attributes().value("spr").toString();
            // The 'format' attribute here is key for datFormatFromString
            sig.format = datFormatFromString(xml.attributes().value("format").toString());
            profile.signatures.append(sig);
            xml.skipCurrentElement();
        } else if (xml.name() == QLatin1String("fucked_up_charges")) {
            profile.extensions.insert("fucked_up_charges", "true");
            xml.skipCurrentElement();
        } else {
            xml.skipCurrentElement(); // Skip other unknown children
        }
    }
    // After loop, xml is at EndElement of <client>
    d->clientProfiles.append(profile);
    qDebug() << "Parsed Client Profile:" << profile.name << "Version:" << profile.versionString;
}

// These helper functions are now effectively integrated into parseSingleClientProfile
// due to the flatter XML structure under <client>.
// Keeping the functions but they will not be called from parseSingleClientProfile in the new structure.
// They might be useful if the XML structure for these items changes back to nested.
void ClientVersionManager::parseClientSignatures(QXmlStreamReader& xml, ClientProfile& profile) {
    Q_UNUSED(xml); Q_UNUSED(profile);
    qWarning() << "parseClientSignatures called but XML structure is flat. Parsing handled in parseSingleClientProfile.";
    // Original logic for <datsignatures><signature .../></datsignatures>
}

void ClientVersionManager::parseClientMapVersions(QXmlStreamReader& xml, ClientProfile& profile) {
    Q_UNUSED(xml); Q_UNUSED(profile);
    qWarning() << "parseClientMapVersions called but XML structure is flat. Parsing handled in parseSingleClientProfile.";
    // Original logic for <mapversions><mapversion .../></mapversions>
}

void ClientVersionManager::parseClientExtensions(QXmlStreamReader& xml, ClientProfile& profile) {
    Q_UNUSED(xml); Q_UNUSED(profile);
    qWarning() << "parseClientExtensions called but XML structure is flat. Parsing handled in parseSingleClientProfile.";
    // Original logic for <extensions><extension key="" value=""/></extensions>
}

quint16 ClientVersionManager::versionStringToNumeric(const QString& versionString) const {
    QStringList parts = versionString.split('.');
    quint16 numericVersion = 0;
    if (!parts.isEmpty()) {
        numericVersion = parts[0].toUShort() * 100;
        if (parts.size() > 1) {
            QString minorPart = parts[1];
            minorPart.remove(QRegularExpression("[^\\d]")); // Remove non-digits like 'b' in "98b"
            numericVersion += minorPart.toUShort();
        }
    }
    return numericVersion;
}

DatFormat ClientVersionManager::datFormatFromString(const QString& formatStr) const {
    // Based on the values seen in the provided clients.xml <data format="X">
    // and the previous CVM.cpp's partial implementation.
    if (formatStr.isEmpty() || formatStr == "default") return DatFormat::UNKNOWN; // Or a sensible default like V_760

    // Exact matches from ClientProfile.h enum names (without "V_")
    // This requires XML "format" attribute to be like "7.4", "7.55", "7.8", "8.6", "9.6", "10.10", "10.57" etc.
    // The enum names are V_740, V_755, V_780_792, V_860_862, V_960_963, V_1010, V_1050_1057

    // Attempting to map common XML values to enum values
    if (formatStr == "7.4") return DatFormat::V_740; // XML has "7.4", enum V_740
    if (formatStr == "7.55") return DatFormat::V_755;
    if (formatStr == "7.6") return DatFormat::V_760; // XML has no direct "7.6" format, but often implies V_755 or V_760
    if (formatStr == "7.7" || formatStr == "7.70") return DatFormat::V_770;
    if (formatStr == "7.8" || formatStr == "7.8-7.92") return DatFormat::V_780_792; // XML has "7.8"
    if (formatStr == "8.0-8.1" || formatStr == "8.00" || formatStr == "8.10" || formatStr == "8.11") return DatFormat::V_800_801;
    if (formatStr == "8.10-8.11") return DatFormat::V_810_811; // More specific than above
    if (formatStr == "8.20") return DatFormat::V_820;
    if (formatStr == "8.3" || formatStr == "8.30") return DatFormat::V_830;
    if (formatStr == "8.4" || formatStr == "8.40-8.42") return DatFormat::V_840_842;
    if (formatStr == "8.5" || formatStr == "8.50-8.54") return DatFormat::V_850_854;
    if (formatStr == "8.55-8.57") return DatFormat::V_855_857;
    if (formatStr == "8.6" || formatStr == "8.60-8.62") return DatFormat::V_860_862; // XML has "8.6"
    if (formatStr == "8.7" || formatStr == "8.70-8.73") return DatFormat::V_870_873;
    if (formatStr == "9.0" || formatStr == "9.00") return DatFormat::V_900;
    if (formatStr == "9.1" || formatStr == "9.10") return DatFormat::V_910;
    if (formatStr == "9.4-9.46" || formatStr == "9.40" || formatStr == "9.46") return DatFormat::V_940_946;
    if (formatStr == "9.5-9.54" || formatStr == "9.54") return DatFormat::V_950_954;
    if (formatStr == "9.6" || formatStr == "9.60-9.63") return DatFormat::V_960_963; // XML has "9.6"
    if (formatStr == "9.70") return DatFormat::V_970;
    if (formatStr == "9.80-9.86" || formatStr == "9.86") return DatFormat::V_980_986;
    if (formatStr == "10.00-10.01" || formatStr == "10.00") return DatFormat::V_1000_1001;
    if (formatStr == "10.10") return DatFormat::V_1010; // XML has "10.10"
    if (formatStr == "10.20") return DatFormat::V_1020;
    if (formatStr == "10.30-10.38" || formatStr == "10.30") return DatFormat::V_1030_1038;
    if (formatStr == "10.41") return DatFormat::V_1041;
    if (formatStr == "10.50-10.57" || formatStr == "10.57") return DatFormat::V_1050_1057; // XML has "10.57"
    if (formatStr == "10.61-10.62") return DatFormat::V_1061_1062;
    if (formatStr == "10.70-10.74") return DatFormat::V_1070_1074;
    if (formatStr == "10.75-10.77") return DatFormat::V_1075_1077;
    if (formatStr == "10.80") return DatFormat::V_1080;
    if (formatStr == "10.90-10.94") return DatFormat::V_1090_1094;
    if (formatStr == "10.95-10.99" || formatStr == "10.9x" || formatStr == "10.98") return DatFormat::V_1095_1099;
    if (formatStr == "11.00+") return DatFormat::V_1100_PLUS;
    if (formatStr == "custom") return DatFormat::CUSTOM;

    // Fallback for enum names like "V_760" if used directly in XML format attribute
    // (My ClientProfile.h has enum values like V_760, V_770)
    // This is a bit redundant if the above covers all actual XML values.
    if (formatStr == "V_600") return DatFormat::V_600;
    if (formatStr == "V_700") return DatFormat::V_700;
    if (formatStr == "V_740") return DatFormat::V_740;
    if (formatStr == "V_750") return DatFormat::V_750;
    if (formatStr == "V_755") return DatFormat::V_755;
    if (formatStr == "V_760") return DatFormat::V_760;
    if (formatStr == "V_770") return DatFormat::V_770;
    if (formatStr == "V_780_792") return DatFormat::V_780_792;
    if (formatStr == "V_800_801") return DatFormat::V_800_801;
    if (formatStr == "V_810_811") return DatFormat::V_810_811;
    if (formatStr == "V_820") return DatFormat::V_820;
    if (formatStr == "V_830") return DatFormat::V_830;
    if (formatStr == "V_840_842") return DatFormat::V_840_842;
    if (formatStr == "V_850_854") return DatFormat::V_850_854;
    if (formatStr == "V_855_857") return DatFormat::V_855_857;
    if (formatStr == "V_860_862") return DatFormat::V_860_862;
    if (formatStr == "V_870_873") return DatFormat::V_870_873;
    if (formatStr == "V_900") return DatFormat::V_900;
    if (formatStr == "V_910") return DatFormat::V_910;
    if (formatStr == "V_940_946") return DatFormat::V_940_946;
    if (formatStr == "V_950_954") return DatFormat::V_950_954;
    if (format_str == "V_960_963") return DatFormat::V_960_963; // Typo: format_str -> formatStr
    if (formatStr == "V_970") return DatFormat::V_970;
    if (formatStr == "V_980_986") return DatFormat::V_980_986;
    if (formatStr == "V_1000_1001") return DatFormat::V_1000_1001;
    if (formatStr == "V_1010") return DatFormat::V_1010;
    if (formatStr == "V_1020") return DatFormat::V_1020;
    if (formatStr == "V_1030_1038") return DatFormat::V_1030_1038;
    if (formatStr == "V_1041") return DatFormat::V_1041;
    if (formatStr == "V_1050_1057") return DatFormat::V_1050_1057;
    if (formatStr == "V_1061_1062") return DatFormat::V_1061_1062;
    if (formatStr == "V_1070_1074") return DatFormat::V_1070_1074;
    if (formatStr == "V_1075_1077") return DatFormat::V_1075_1077;
    if (formatStr == "V_1080") return DatFormat::V_1080;
    if (formatStr == "V_1090_1094") return DatFormat::V_1090_1094;
    if (formatStr == "V_1095_1099") return DatFormat::V_1095_1099;
    if (formatStr == "V_1100_PLUS") return DatFormat::V_1100_PLUS;
    if (formatStr == "CUSTOM") return DatFormat::CUSTOM;

    qWarning() << "Unknown DatFormat string in XML:" << formatStr << "- defaulting to UNKNOWN.";
    return DatFormat::UNKNOWN;
}

const QList<ClientProfile>& ClientVersionManager::getClientProfiles() const {
    return d->clientProfiles;
}

const ClientProfile* ClientVersionManager::getClientProfileByNumericVersion(quint16 version) const {
    return d->profileByNumericVersion.value(version, nullptr);
}

const ClientProfile* ClientVersionManager::getClientProfileByVersionString(const QString& versionString) const {
     return d->profileByVersionString.value(versionString, nullptr);
}

const ClientProfile* ClientVersionManager::getClientProfileByName(const QString& name) const {
    return d->profileByName.value(name, nullptr);
}

const ClientProfile* ClientVersionManager::getDefaultClientProfile() const {
    if (!d->clientProfiles.isEmpty()) {
        // A more robust "default" might be one explicitly marked in XML or the highest valid version.
        // For now, just the first one.
        return &d->clientProfiles.first();
    }
    return nullptr;
}

const QList<OtbVersionInfo>& ClientVersionManager::getOtbVersionInfos() const {
    return d->otbVersionInfos;
}

const OtbVersionInfo* ClientVersionManager::getOtbVersionInfoByName(const QString& name) const {
    return d->otbInfoByName.value(name, nullptr);
}

bool ClientVersionManager::saveUserClientPaths(const QString& saveFilePath) const {
    QJsonArray clientsArray;
    for (const auto& pair : d->userConfiguredPaths.toStdMap()) { // Iterate QMap
        QJsonObject profileObject;
        profileObject[QStringLiteral("versionString")] = pair.first;
        profileObject[QStringLiteral("path")] = pair.second;
        clientsArray.append(profileObject);
    }
    // Also save paths directly set on profiles if d->userConfiguredPaths isn't the sole source
    for (const ClientProfile& profile : d->clientProfiles) {
        if (!profile.userConfiguredClientPath.isEmpty() && !d->userConfiguredPaths.contains(profile.versionString)) {
             QJsonObject profileObject;
             profileObject[QStringLiteral("versionString")] = profile.versionString;
             profileObject[QStringLiteral("path")] = profile.userConfiguredClientPath;
             clientsArray.append(profileObject); // Avoid duplicates if logic changes
        }
    }


    QJsonDocument doc(clientsArray);
    QFile jsonFile(saveFilePath);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        d->lastError = QString("Failed to open client paths file for writing: %1").arg(jsonFile.errorString());
        qWarning() << d->lastError;
        return false;
    }

    jsonFile.write(doc.toJson(QJsonDocument::Indented));
    jsonFile.close();
    return true;
}

bool ClientVersionManager::loadUserClientPaths(const QString& loadFilePath) {
    QFile jsonFile(loadFilePath);
    if (!jsonFile.exists()) {
        qInfo() << "Client custom paths file not found (this is okay):" << loadFilePath;
        return true; // Not an error if file doesn't exist.
    }
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        d->lastError = QString("Failed to open client paths file for reading: %1").arg(jsonFile.errorString());
        qWarning() << d->lastError;
        return false;
    }

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        d->lastError = QString("Failed to parse client paths JSON: %1").arg(parseError.errorString());
        qWarning() << d->lastError;
        return false;
    }
    if (!doc.isArray()) {
        d->lastError = "Client paths JSON root is not an array.";
        qWarning() << d->lastError;
        return false;
    }

    d->userConfiguredPaths.clear();
    QJsonArray clientsArray = doc.array();
    for (const QJsonValue& val : clientsArray) {
        QJsonObject obj = val.toObject();
        if (obj.contains("versionString") && obj.contains("path")) {
            d->userConfiguredPaths.insert(obj["versionString"].toString(), obj["path"].toString());
        }
    }
    qInfo() << "Loaded" << d->userConfiguredPaths.size() << "user configured client paths from" << loadFilePath;
    return true;
}

void ClientVersionManager::applyUserPathsToProfiles() {
    if (d->userConfiguredPaths.isEmpty()) return;

    for (ClientProfile& profile : d->clientProfiles) { // Must be non-const iteration
        if (d->userConfiguredPaths.contains(profile.versionString)) {
            profile.userConfiguredClientPath = d->userConfiguredPaths.value(profile.versionString);
            // TODO: Add logic to validate this path and set profile.pathsAreValid
            // For now, just applying the string.
            qDebug() << "Applied user path to" << profile.versionString << ":" << profile.userConfiguredClientPath;
        }
    }
}

QString ClientVersionManager::getLastError() const {
    return d->lastError;
}

} // namespace Assets
} // namespace RME
