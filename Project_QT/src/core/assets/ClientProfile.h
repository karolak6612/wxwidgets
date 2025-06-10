#ifndef RME_CLIENT_PROFILE_H
#define RME_CLIENT_PROFILE_H

#include <QString>
#include <QList>
#include <QMap> // For signatures, extensions

namespace RME {

// Defines the format of a .dat sprite metadata file
enum class DatFormat {
    UNKNOWN,
    V_600,   // Example for very old versions
    V_700,
    V_740,
    V_750,
    V_755,
    V_760,
    V_770,
    V_780_792, // Shared format for 7.80-7.92
    V_800_801,
    V_810_811,
    V_820,
    V_830,
    V_840_842,
    V_850_854,
    V_855_857,
    V_860_862,
    V_870_873,
    V_900,
    V_910,
    V_940_946,
    V_950_954,
    V_960_963,
    V_970,
    V_980_986,
    V_1000_1001,
    V_1010,
    V_1020,
    V_1030_1038,
    V_1041,
    V_1050_1057,
    V_1061_1062,
    V_1070_1074,
    V_1075_1077,
    V_1080,
    V_1090_1094,
    V_1095_1099, // Or just 109X
    V_1100_PLUS, // Generic for 11.00+ if structure stabilizes
    CUSTOM // For OTFI defined formats
};

// Represents an OTB (Open Tibia Binary item file) version configuration
struct OtbVersion {
    QString name;           // e.g., "7.60", "8.60", "Custom"
    quint32 formatVersion;  // OTB format version (1, 2, 3, etc.)
    quint32 clientID;       // Associated client version ID (e.g., 760, 860)
    QString description;    // Optional description
};

// Holds data for a single Tibia client version profile
struct ClientProfile {
    QString name;                 // User-friendly name, e.g., "Tibia 10.98"
    QString versionString;        // String representation from XML, e.g. "10.98"
    quint32 clientOtbmVersionId;  // OTB version ID this client primarily uses (from clients.xml <client otb="X">)

    // Paths are hints and might need to be resolved against a base data path
    QString datPathHint;
    QString sprPathHint;
    QString picPathHint; // For character previews, if used by editor

    // File signatures (hex strings or byte arrays)
    QMap<QString, QByteArray> datSignatures; // e.g., "760.dat" -> QByteArray::fromHex("...")
    QMap<QString, QByteArray> sprSignatures;

    DatFormat datFormat = DatFormat::UNKNOWN; // Determined from signature or version

    QList<quint32> supportedOtbmVersions; // List of OTBM version IDs this client can handle

    // Extensions (from <extensions> tag in clients.xml)
    bool extendedSprites = false;     // Supports extended .spr file format
    bool transparentSprites = false;  // Supports transparency in sprites
    bool frameDurations = false;      // Supports frame durations for animations
    bool patternZ = false;            // Supports pattern Z offset
    bool looktypeU16 = false;         // Outfit looktypes are uint16 instead of uint8
    bool actionInstancing = false;    // Related to how actions are handled/instanced

    QString customOtfIndexPath;       // Path to a custom .otfi file, if specified

    // Helper to get a signature string for display or logging
    QString getDatSignatureString(const QString& key) const {
        return QString(datSignatures.value(key).toHex());
    }
    QString getSprSignatureString(const QString& key) const {
        return QString(sprSignatures.value(key).toHex());
    }
};

} // namespace RME

#endif // RME_CLIENT_PROFILE_H
