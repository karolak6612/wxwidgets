#ifndef RME_CLIENT_VERSION_MANAGER_H
#define RME_CLIENT_VERSION_MANAGER_H

#include "ClientProfile.h" // This now includes OtbVersionInfo, ClientProfile, etc.
#include <QList>
#include <QString>
#include <QScopedPointer> // For PIMPL

// Forward declarations
class QXmlStreamReader;

namespace RME {
namespace Assets { // Consistent namespacing

class ClientVersionManager {
public:
    ClientVersionManager();
    ~ClientVersionManager(); // Required for QScopedPointer to PIMPL

    // Loads client and OTB configurations from the specified clients.xml file path.
    // Returns true on success, false on failure. Use getLastError() for details.
    bool loadVersions(const QString& clientsXmlPath);

    // Client Profile Accessors
    const QList<ClientProfile>& getClientProfiles() const;
    const ClientProfile* getClientProfileByNumericVersion(quint16 version) const;
    const ClientProfile* getClientProfileByVersionString(const QString& versionString) const;
    const ClientProfile* getClientProfileByName(const QString& name) const; // Name from <client name="...">
    const ClientProfile* getDefaultClientProfile() const; // e.g., first valid one or a specially marked one

    // OTB Version Info Accessors
    const QList<OtbVersionInfo>& getOtbVersionInfos() const;
    const OtbVersionInfo* getOtbVersionInfoByName(const QString& name) const;
    // If lookup by client ID is needed, it can be implemented by iterating OtbVersionInfos

    // Persistence for user-specific client path configurations
    // These would typically save/load a small JSON or INI file mapping client versions
    // to their user-confirmed disk paths (populating ClientProfile::userConfiguredClientPath).
    bool saveUserClientPaths(const QString& saveFilePath) const;
    bool loadUserClientPaths(const QString& loadFilePath);
    // Applies loaded user paths to the m_clientProfiles list
    void applyUserPathsToProfiles();


    // Returns the last error message, if any.
    QString getLastError() const;

private:
    // Private helper methods for parsing XML (to be implemented in .cpp)
    // Their exact signatures might evolve based on .cpp implementation needs.
    void parseOtbVersionInfoSection(QXmlStreamReader& xml); // Parses <otbs>
    void parseSingleOtbVersionInfo(QXmlStreamReader& xml);  // Parses an <otb> entry

    void parseClientsSection(QXmlStreamReader& xml);        // Parses <clients>
    void parseSingleClientProfile(QXmlStreamReader& xml);   // Parses a <client> entry
    void parseClientSignatures(QXmlStreamReader& xml, ClientProfile& profile); // Parses <dat> entries for a client
    void parseClientMapVersions(QXmlStreamReader& xml, ClientProfile& profile); // Parses <mapversion> entries
    void parseClientExtensions(QXmlStreamReader& xml, ClientProfile& profile); // Parses <extension> entries

    // Helper to convert version string like "7.60" to numeric 760
    quint16 versionStringToNumeric(const QString& versionString) const;
    // Helper to convert string to DatFormat
    DatFormat datFormatFromString(const QString& formatStr) const;


    // PIMPL (Private Implementation)
    // Declare the data structure here, define it in the .cpp file.
    struct ClientVersionManagerData;
    QScopedPointer<ClientVersionManagerData> d;
};

} // namespace Assets
} // namespace RME

#endif // RME_CLIENT_VERSION_MANAGER_H
