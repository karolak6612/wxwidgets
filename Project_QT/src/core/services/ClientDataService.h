#ifndef RME_CLIENTDATASERVICE_H
#define RME_CLIENTDATASERVICE_H

#include "IClientDataService.h"
#include <QObject>
#include <QString>
#include <memory>

namespace RME {
namespace core {
namespace assets {
    class ClientVersion;
    class ItemDatabase;
    class CreatureDatabase;
    class MaterialManager;
    class AssetManager;
    class ClientVersionManager;
}

namespace sprites {
    class SpriteManager;
}

/**
 * @brief Service for managing client data including version, databases, and managers
 * 
 * This service centralizes access to all client-related data and provides
 * a unified interface for loading/unloading client versions and accessing
 * the various data managers.
 */
class ClientDataService : public IClientDataService
{
    Q_OBJECT

public:
    explicit ClientDataService(QObject* parent = nullptr);
    ~ClientDataService();

    // IClientDataService implementation
    bool loadClientVersion(const QString& versionId) override;
    void unloadClientVersion() override;
    
    assets::ClientVersion* getClientVersion() const override;
    assets::ItemDatabase* getItemDatabase() const override;
    sprites::SpriteManager* getSpriteManager() const override;
    assets::MaterialManager* getMaterialManager() const override;
    assets::CreatureDatabase* getCreatureDatabase() const override;
    assets::AssetManager* getAssetManager() const override;
    
    bool isClientVersionLoaded() const override;
    QString getCurrentVersionId() const override;

private slots:
    void onClientVersionLoadingProgress(int percentage, const QString& message);
    void onClientVersionLoaded();
    void onClientVersionLoadFailed(const QString& error);

private:
    void initializeManagers();
    void cleanupManagers();

private:
    assets::ClientVersionManager* m_versionManager;
    assets::AssetManager* m_assetManager;
    assets::ItemDatabase* m_itemDatabase;
    sprites::SpriteManager* m_spriteManager;
    assets::MaterialManager* m_materialManager;
    assets::CreatureDatabase* m_creatureDatabase;
    assets::ClientVersion* m_currentVersion;
    QString m_currentVersionId;
    bool m_isLoaded;
};

} // namespace core
} // namespace RME

#endif // RME_CLIENTDATASERVICE_H