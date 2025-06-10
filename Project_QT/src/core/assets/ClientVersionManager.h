#ifndef RME_CLIENT_VERSION_MANAGER_H
#define RME_CLIENT_VERSION_MANAGER_H

#include "ClientProfile.h"
#include <QList>
#include <QMap>
#include <QString>
#include <QScopedPointer> // For PIMPL or internal data

class QXmlStreamReader; // Forward declaration

namespace RME {

class ClientVersionManager {
public:
    ClientVersionManager();
    ~ClientVersionManager();

    bool loadVersions(const QString& filePath); // Loads from clients.xml

    const QList<ClientProfile>& getClientProfiles() const;
    const ClientProfile* getClientProfile(const QString& versionString) const;
    const ClientProfile* getDefaultClientProfile() const; // e.g., first loaded or a marked default

    const QList<OtbVersion>& getOtbVersions() const;
    const OtbVersion* getOtbVersionById(quint32 id) const;
    const OtbVersion* getOtbVersionByName(const QString& name) const;


private:
    // Internal methods for parsing different sections of clients.xml
    void parseOtbsSection(QXmlStreamReader& xml);
    void parseClientsSection(QXmlStreamReader& xml);
    void parseClientNode(QXmlStreamReader& xml, ClientProfile& profile);
    void parseSignaturesNode(QXmlStreamReader& xml, ClientProfile& profile);
    void parseDatSprNode(QXmlStreamReader& xml, const QString& type, QMap<QString, QByteArray>& signatureMap);
    void parseExtensionsNode(QXmlStreamReader& xml, ClientProfile& profile);
    DatFormat datFormatFromString(const QString& formatStr) const;


    struct ClientVersionManagerData; // PIMPL for private members
    QScopedPointer<ClientVersionManagerData> d;
};

} // namespace RME

#endif // RME_CLIENT_VERSION_MANAGER_H
