#ifndef RME_ICLIENTDATASERVICE_H
#define RME_ICLIENTDATASERVICE_H

#include <QObject>
#include <QString>

namespace RME {
namespace core {
namespace assets {
    class ClientVersion;
    class ItemDatabase;
    class CreatureDatabase;
    class MaterialManager;
    class AssetManager;
}

namespace sprites {
    class SpriteManager;
}

/**
 * @brief Interface for client data management service
 * 
 * This interface defines the contract for managing client data including
 * client version, item database, sprite manager, material manager, and creature database.
 */
class IClientDataService : public QObject {
    Q_OBJECT

public:
    virtual ~IClientDataService() = default;

    // Client version management
    virtual bool loadClientVersion(const QString& versionId) = 0;
    virtual void unloadClientVersion() = 0;
    
    // Data access
    virtual assets::ClientVersion* getClientVersion() const = 0;
    virtual assets::ItemDatabase* getItemDatabase() const = 0;
    virtual sprites::SpriteManager* getSpriteManager() const = 0;
    virtual assets::MaterialManager* getMaterialManager() const = 0;
    virtual assets::CreatureDatabase* getCreatureDatabase() const = 0;
    virtual assets::AssetManager* getAssetManager() const = 0;
    
    // Status
    virtual bool isClientVersionLoaded() const = 0;
    virtual QString getCurrentVersionId() const = 0;

signals:
    void clientVersionChanged(assets::ClientVersion* version);
    void clientVersionLoaded(const QString& versionId);
    void clientVersionUnloaded();
    void dataLoadingProgress(int percentage, const QString& message);
};

} // namespace core
} // namespace RME

#endif // RME_ICLIENTDATASERVICE_H