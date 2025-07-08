#include "ClientDataService.h"
#include "core/assets/AssetManager.h"
#include "core/assets/ClientVersionManager.h"
#include "core/assets/ItemDatabase.h"
#include "core/assets/CreatureDatabase.h"
#include "core/assets/MaterialManager.h"
#include "core/sprites/SpriteManager.h"
#include <QDebug>

namespace RME {
namespace core {

ClientDataService::ClientDataService(QObject* parent)
    : IClientDataService(parent)
    , m_versionManager(nullptr)
    , m_assetManager(nullptr)
    , m_itemDatabase(nullptr)
    , m_spriteManager(nullptr)
    , m_materialManager(nullptr)
    , m_creatureDatabase(nullptr)
    , m_currentVersion(nullptr)
    , m_isLoaded(false)
{
    initializeManagers();
}

ClientDataService::~ClientDataService()
{
    cleanupManagers();
}

void ClientDataService::initializeManagers()
{
    // Create the version manager
    m_versionManager = new assets::ClientVersionManager(this);
    
    // Create the asset manager
    m_assetManager = new assets::AssetManager(this);
    
    // Create individual managers
    m_itemDatabase = new assets::ItemDatabase(this);
    m_spriteManager = new sprites::SpriteManager(this);
    m_materialManager = new assets::MaterialManager(this);
    m_creatureDatabase = new assets::CreatureDatabase(this);
    
    // Connect signals for loading progress
    connect(m_versionManager, &assets::ClientVersionManager::loadingProgress,
            this, &ClientDataService::onClientVersionLoadingProgress);
    connect(m_versionManager, &assets::ClientVersionManager::versionLoaded,
            this, &ClientDataService::onClientVersionLoaded);
    connect(m_versionManager, &assets::ClientVersionManager::loadFailed,
            this, &ClientDataService::onClientVersionLoadFailed);
}

void ClientDataService::cleanupManagers()
{
    unloadClientVersion();
    
    // Managers will be deleted automatically by Qt's parent-child system
    m_versionManager = nullptr;
    m_assetManager = nullptr;
    m_itemDatabase = nullptr;
    m_spriteManager = nullptr;
    m_materialManager = nullptr;
    m_creatureDatabase = nullptr;
}

bool ClientDataService::loadClientVersion(const QString& versionId)
{
    if (m_isLoaded && m_currentVersionId == versionId) {
        qDebug() << "ClientDataService: Version" << versionId << "is already loaded";
        return true;
    }
    
    // Unload current version if any
    if (m_isLoaded) {
        unloadClientVersion();
    }
    
    qDebug() << "ClientDataService: Loading client version" << versionId;
    
    // Load the client version through the version manager
    if (!m_versionManager->loadVersion(versionId)) {
        qWarning() << "ClientDataService: Failed to load client version" << versionId;
        return false;
    }
    
    m_currentVersion = m_versionManager->getCurrentVersion();
    if (!m_currentVersion) {
        qWarning() << "ClientDataService: Version manager returned null version";
        return false;
    }
    
    // Load data into individual managers
    if (!m_itemDatabase->loadFromVersion(m_currentVersion)) {
        qWarning() << "ClientDataService: Failed to load item database";
        unloadClientVersion();
        return false;
    }
    
    if (!m_spriteManager->loadFromVersion(m_currentVersion)) {
        qWarning() << "ClientDataService: Failed to load sprite manager";
        unloadClientVersion();
        return false;
    }
    
    if (!m_materialManager->loadFromVersion(m_currentVersion)) {
        qWarning() << "ClientDataService: Failed to load material manager";
        unloadClientVersion();
        return false;
    }
    
    if (!m_creatureDatabase->loadFromVersion(m_currentVersion)) {
        qWarning() << "ClientDataService: Failed to load creature database";
        unloadClientVersion();
        return false;
    }
    
    m_currentVersionId = versionId;
    m_isLoaded = true;
    
    emit clientVersionLoaded(versionId);
    emit clientVersionChanged(m_currentVersion);
    
    qDebug() << "ClientDataService: Successfully loaded client version" << versionId;
    return true;
}

void ClientDataService::unloadClientVersion()
{
    if (!m_isLoaded) {
        return;
    }
    
    qDebug() << "ClientDataService: Unloading client version" << m_currentVersionId;
    
    // Unload data from individual managers
    if (m_creatureDatabase) {
        m_creatureDatabase->unload();
    }
    
    if (m_materialManager) {
        m_materialManager->unload();
    }
    
    if (m_spriteManager) {
        m_spriteManager->unload();
    }
    
    if (m_itemDatabase) {
        m_itemDatabase->unload();
    }
    
    // Unload from version manager
    if (m_versionManager) {
        m_versionManager->unloadVersion();
    }
    
    m_currentVersion = nullptr;
    m_currentVersionId.clear();
    m_isLoaded = false;
    
    emit clientVersionUnloaded();
    emit clientVersionChanged(nullptr);
}

assets::ClientVersion* ClientDataService::getClientVersion() const
{
    return m_currentVersion;
}

assets::ItemDatabase* ClientDataService::getItemDatabase() const
{
    return m_itemDatabase;
}

sprites::SpriteManager* ClientDataService::getSpriteManager() const
{
    return m_spriteManager;
}

assets::MaterialManager* ClientDataService::getMaterialManager() const
{
    return m_materialManager;
}

assets::CreatureDatabase* ClientDataService::getCreatureDatabase() const
{
    return m_creatureDatabase;
}

assets::AssetManager* ClientDataService::getAssetManager() const
{
    return m_assetManager;
}

bool ClientDataService::isClientVersionLoaded() const
{
    return m_isLoaded;
}

QString ClientDataService::getCurrentVersionId() const
{
    return m_currentVersionId;
}

void ClientDataService::onClientVersionLoadingProgress(int percentage, const QString& message)
{
    emit dataLoadingProgress(percentage, message);
}

void ClientDataService::onClientVersionLoaded()
{
    qDebug() << "ClientDataService: Received version loaded signal";
}

void ClientDataService::onClientVersionLoadFailed(const QString& error)
{
    qWarning() << "ClientDataService: Version load failed:" << error;
    unloadClientVersion();
}

} // namespace core
} // namespace RME

// #include "ClientDataService.moc" // Removed - Q_OBJECT is in header