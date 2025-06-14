#ifndef RME_CLIENT_PROFILE_H
#define RME_CLIENT_PROFILE_H

#include <QString>
#include <QList>
#include <QMap>
#include <QByteArray> // For actual signature data
#include <cstdint>    // For quint16, quint32

// Using a namespace RME::Assets to organize asset-related classes/structs
namespace RME {
namespace Assets {

// Defines the format of a .dat sprite metadata file
// (Kept from the existing file, seems useful)
enum class DatFormat {
    UNKNOWN, V_600, V_700, V_740, V_750, V_755, V_760, V_770,
    V_780_792, V_800_801, V_810_811, V_820, V_830, V_840_842,
    V_850_854, V_855_857, V_860_862, V_870_873, V_900, V_910,
    V_940_946, V_950_954, V_960_963, V_970, V_980_986, V_1000_1001,
    V_1010, V_1020, V_1030_1038, V_1041, V_1050_1057, V_1061_1062,
    V_1070_1074, V_1075_1077, V_1080, V_1090_1094, V_1095_1099,
    V_1100_PLUS, CUSTOM
};

// Represents an OTB (Open Tibia Binary item file) version, parsed from <otbs><otb client="version" version="X" id="Y"/> in clients.xml
// This will be stored by ClientVersionManager
struct OtbVersionInfo {
    QString name;                 // Unique name for this OTB configuration, from 'client' attribute (e.g., "7.60", "8.00")
    quint32 formatVersionMajor = 0; // OTB format version (1, 2, or 3), from 'version' attribute
    quint32 xmlOriginalId = 0;      // Original 'id' attribute from XML, for reference

    // Parsed from the 'client' attribute string for potential detailed matching
    quint16 parsedClientMajor = 0;
    quint16 parsedClientMinor = 0;
    // description and clientIDs list are not directly available from the new XML structure for <otb>
    // QString description;
    // QList<quint16> clientIDs;
};

// Represents a DAT/SPR signature pair, parsed from <client><dat> in clients.xml
struct ClientSignature {
    QString type; // e.g., "7.60", "8.60", also used as key in ClientProfile::signatures
    // Store signatures as hex strings as they appear in XML, convert to QByteArray on load for comparison
    QString datSignatureHex;
    QString sprSignatureHex;
    // Resolved DatFormat for this signature
    DatFormat format = DatFormat::UNKNOWN;
};

// Represents a supported map version, parsed from <client><mapversion> in clients.xml
struct MapVersionSupportInfo {
    quint16 otbmVersion = 0;
    quint16 clientVersionNumeric = 0; // e.g., 770, 860
    QString description;
};

// Holds data for a single Tibia client version profile, parsed from <client> in clients.xml
struct ClientProfile {
    QString name;                 // User-friendly name, e.g., "Tibia 7.6" (from <client name="...">)
    QString versionString;        // String representation from XML, e.g. "7.6", "8.00" (from <client name="..."> attribute)
    quint16 numericVersion = 0;   // Numeric version, e.g., 760 (derived from versionString)

    QString otbNameReference;     // Name of the OTB configuration to use (references an OtbVersionInfo by its name, e.g. "7.60")

    QList<ClientSignature> signatures; // List of DAT/SPR signatures for this client

    // Data paths - hints from clients.xml
    QString clientDataPathHint; // From 'data_directory' attribute (e.g., "760")
    QString picPathHint;        // Conventionally 'version.pic' within the clientDataPathHint, not a direct XML attribute.

    // Resolved absolute paths after validation
    QString resolvedDatPath;
    QString resolvedSprPath;
    QString resolvedPicPath;
    bool    pathsAreValid = false;

    QList<MapVersionSupportInfo> supportedMapVersions;

    // Storing extensions. For <extensions from="X" to="Y"/>, map will contain {"from": "X", "to": "Y"}
    // For tags like <fucked_up_charges/>, map will contain {"fucked_up_charges": "true"}
    QMap<QString, QString> extensions;

    // UI related flags
    bool visibleInUI = true;
    bool isDefaultChoice = false;

    // Path set by user in application settings, overriding any auto-detection
    QString userConfiguredClientPath;

    // Helper to get a specific signature by type string (e.g., "7.60")
    // Note: 'type' in ClientSignature is currently set to profile's versionString.
    // This helper might need adjustment if 'type' needs to be more specific from XML.
    const ClientSignature* getSignatureByType(const QString& type) const {
        for (const auto& sig : signatures) {
            if (sig.type == type) {
                return &sig;
            }
        }
        return nullptr;
    }

    // Helper to get a specific extension value
    QString getExtension(const QString& key, const QString& defaultValue = QString()) const {
        return extensions.value(key, defaultValue);
    }
};

} // namespace Assets
} // namespace RME

#endif // RME_CLIENT_PROFILE_H
