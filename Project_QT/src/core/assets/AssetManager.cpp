#include "AssetManager.h"
#include <QDebug>
#include <QFileInfo> // For path operations
#include <QCoreApplication> // For applicationDirPath fallback, already in .h but good for .cpp too

namespace RME {

struct AssetManager::AssetManagerData {
    ClientVersionManager clientVersionManager;
    ItemDatabase itemDatabase;
    CreatureDatabase creatureDatabase;
    SpriteManager spriteManager;

    const ClientProfile* currentClientProfile = nullptr; // Cached pointer
    QString currentDataPath;
};

AssetManager::AssetManager() : d(new AssetManagerData()) {}
AssetManager::~AssetManager() = default;

QString AssetManager::resolvePath(const QString& dataPath, const QString& relativeOrAbsolutePath) const {
    if (relativeOrAbsolutePath.isEmpty()) {
        return QString();
    }
    QFileInfo fileInfo(relativeOrAbsolutePath);
    if (fileInfo.isAbsolute()) {
        return QDir::toNativeSeparators(relativeOrAbsolutePath); // Normalize path separators
    }
    // If relative, resolve against dataPath
    return QDir::toNativeSeparators(QDir(dataPath).filePath(relativeOrAbsolutePath));
}

bool AssetManager::loadAllAssets(const QString& dataPath, const QString& clientVersionString) {
    d->currentDataPath = dataPath;
    qInfo() << "AssetManager: Starting to load all assets for client version" << clientVersionString << "from base path" << dataPath;

    // 1. Load Client Versions
    QString clientsXmlPath = resolvePath(dataPath, "clients.xml");
    if (!QFile::exists(clientsXmlPath)) {
        // Fallback search logic for clients.xml
        QString appPath = QCoreApplication::applicationDirPath();
        clientsXmlPath = resolvePath(QDir(appPath).filePath("data"), "XML/clients.xml"); // Check ./data/XML/
        if (!QFile::exists(clientsXmlPath)) {
             clientsXmlPath = resolvePath(appPath, "XML/clients.xml"); // Check ./XML/
             if (!QFile::exists(clientsXmlPath)) {
                clientsXmlPath = resolvePath(appPath, "clients.xml"); // Check ./ (often used in dev)
             }
        }
    }
    qInfo() << "AssetManager: Attempting to load clients.xml from:" << clientsXmlPath;
    if (!QFile::exists(clientsXmlPath) || !d->clientVersionManager.loadVersions(clientsXmlPath)) {
        qCritical() << "AssetManager: Failed to load client versions from clients.xml. Path tried:" << clientsXmlPath;
        return false;
    }

    d->currentClientProfile = d->clientVersionManager.getClientProfile(clientVersionString);
    if (!d->currentClientProfile) {
        qCritical() << "AssetManager: Client profile for version" << clientVersionString << "not found in clients.xml.";
        return false;
    }
    qInfo() << "AssetManager: Successfully loaded client profile for" << clientVersionString << ":" << d->currentClientProfile->name;

    // 2. Load Items (OTB and/or XML)
    const OtbVersion* otbInfo = d->clientVersionManager.getOtbVersionById(d->currentClientProfile->clientOtbmVersionId);
    QString otbPathToLoad;

    if (otbInfo) {
        // Try specific named OTB first (e.g. items_7.60.otb)
        otbPathToLoad = resolvePath(dataPath, QString("items_%1.otb").arg(otbInfo->name));
        if (!QFile::exists(otbPathToLoad)) {
            otbPathToLoad = resolvePath(dataPath, "items.otb"); // Fallback to generic items.otb
        }
    } else {
        qWarning() << "AssetManager: No specific OTB version info found for client profile" << clientVersionString
                   << "(OTB ID:" << d->currentClientProfile->clientOtbmVersionId << "). Trying generic items.otb.";
        otbPathToLoad = resolvePath(dataPath, "items.otb");
    }

    qInfo() << "AssetManager: Attempting to load items from OTB:" << otbPathToLoad;
    if (QFile::exists(otbPathToLoad)) {
        if (!d->itemDatabase.loadFromOTB(otbPathToLoad)) {
            qWarning() << "AssetManager: Failed to load items from OTB file:" << otbPathToLoad << "Continuing with items.xml if available.";
        }
    } else {
        qWarning() << "AssetManager: OTB file not found at" << otbPathToLoad;
    }

    QString itemsXmlPath = resolvePath(dataPath, "items.xml");
    qInfo() << "AssetManager: Attempting to load items from XML:" << itemsXmlPath;
    if (QFile::exists(itemsXmlPath)) {
        if (!d->itemDatabase.loadFromXML(itemsXmlPath)) {
            qWarning() << "AssetManager: Failed to load or append items from items.xml:" << itemsXmlPath;
        }
    } else {
        qInfo() << "AssetManager: items.xml not found at" << itemsXmlPath << "(this may be normal if OTB is primary).";
    }
    if(d->itemDatabase.getItemCount() == 0) {
        qCritical() << "AssetManager: Item database is empty after attempting OTB and XML load.";
        return false;
    }

    // 3. Load Creatures
    QString creaturesXmlPath = resolvePath(dataPath, "creatures.xml");
    qInfo() << "AssetManager: Attempting to load creatures from RME creatures.xml:" << creaturesXmlPath;
    if (QFile::exists(creaturesXmlPath)) {
        if (!d->creatureDatabase.loadFromXML(creaturesXmlPath)) {
            qWarning() << "AssetManager: Failed to load creatures from RME creatures.xml:" << creaturesXmlPath;
        }
    } else {
        qWarning() << "AssetManager: RME creatures.xml not found at" << creaturesXmlPath;
    }
    // Example for loading individual monster files (optional, adapt path as needed)
    QDir monsterDir(resolvePath(dataPath, "monsters"));
    if (monsterDir.exists()) {
        qInfo() << "AssetManager: Scanning for OT server monster XML files in:" << monsterDir.path();
        QStringList filters;
        filters << "*.xml";
        for (const QString& fileName : monsterDir.entryList(filters, QDir::Files)) {
            d->creatureDatabase.importFromOtServerXml(monsterDir.filePath(fileName));
        }
    }


    // 4. Load Sprites (DAT/SPR)
    QString otfiPath;
    if (!d->currentClientProfile->customOtfIndexPath.isEmpty()) {
        otfiPath = resolvePath(dataPath, d->currentClientProfile->customOtfIndexPath);
        qInfo() << "AssetManager: Client profile specifies OTFI path:" << otfiPath;
    } else {
        // Try conventional OTFI name if no custom path specified
        otfiPath = resolvePath(dataPath, QString("Tibia_%1.otfi").arg(d->currentClientProfile->versionString));
         qInfo() << "AssetManager: Attempting to load conventional OTFI (optional):" << otfiPath;
    }

    OtfiData loadedOtfiData;
    if (QFile::exists(otfiPath)) {
        if (!d->spriteManager.loadOtfi(otfiPath, loadedOtfiData)) { // SpriteManager stores its own copy of OtfiData internally
            qWarning() << "AssetManager: Failed to load OTFI file:" << otfiPath << "Continuing with default DAT/SPR paths.";
        } else {
             qInfo() << "AssetManager: OTFI loaded. Custom DAT:" << loadedOtfiData.customDatPath
                     << "Custom SPR:" << loadedOtfiData.customSprPath;
        }
    } else {
        qInfo() << "AssetManager: OTFI file not found at" << otfiPath << "(this is normal if not used).";
    }

    // SpriteManager's loadDatSpr will internally use its activeOtfiData if loadOtfi was successful.
    // We pass the original hints from client profile, SpriteManager will decide.
    QString datHintPath = resolvePath(dataPath, d->currentClientProfile->datPathHint);
    QString sprHintPath = resolvePath(dataPath, d->currentClientProfile->sprPathHint);

    qInfo() << "AssetManager: Attempting to load sprites using DAT hint:" << datHintPath << "SPR hint:" << sprHintPath
            << "with client profile:" << d->currentClientProfile->name;

    if (!d->spriteManager.loadDatSpr(datHintPath, sprHintPath, *d->currentClientProfile)) {
        qCritical() << "AssetManager: Failed to load sprites from DAT/SPR files.";
        return false;
    }

    qInfo() << "AssetManager: Successfully loaded all essential assets for client version" << clientVersionString;
    return true;
}

const ClientVersionManager& AssetManager::getClientVersionManager() const { return d->clientVersionManager; }
const ItemDatabase& AssetManager::getItemDatabase() const { return d->itemDatabase; }
const CreatureDatabase& AssetManager::getCreatureDatabase() const { return d->creatureDatabase; }
const SpriteManager& AssetManager::getSpriteManager() const { return d->spriteManager; }

const ClientProfile* AssetManager::getCurrentClientProfile() const { return d->currentClientProfile; }

const ItemData* AssetManager::getItemData(quint16 itemId) const {
    return d->itemDatabase.getItemData(itemId);
}
const CreatureData* AssetManager::getCreatureData(const QString& name) const {
    return d->creatureDatabase.getCreatureData(name);
}
const SpriteData* AssetManager::getSpriteData(quint32 spriteId) const {
    return d->spriteManager.getSpriteData(spriteId);
}


// --- IItemTypeProvider Implementation ---
QString AssetManager::getName(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->name : "Unknown";
}
QString AssetManager::getDescription(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->description : "";
}
quint32 AssetManager::getFlags(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? static_cast<quint32>(data->flags) : 0;
}
double AssetManager::getWeight(quint16 id, quint16 subtype) const {
    const ItemData* data = getItemData(id);
    if (data && data->serverID != 0) {
        return (data->hasFlag(ItemFlag::STACKABLE) && subtype > 0) ? data->weight * subtype : data->weight;
    }
    return 0.0;
}
bool AssetManager::isBlocking(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::BLOCK_SOLID) : true;
}
bool AssetManager::isProjectileBlocking(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::BLOCK_PROJECTILE) : true;
}
bool AssetManager::isPathBlocking(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::BLOCK_PATHFIND) : true;
}
bool AssetManager::isWalkable(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? !data->hasFlag(ItemFlag::BLOCK_PATHFIND) && !data->hasFlag(ItemFlag::WALL) : false;
}
bool AssetManager::isStackable(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::STACKABLE) : false;
}
bool AssetManager::isGround(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::GROUND : false;
}
bool AssetManager::isAlwaysOnTop(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::ALWAYSONTOP) : false;
}
bool AssetManager::isReadable(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::READABLE) : false;
}
bool AssetManager::isWriteable(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? (data->hasFlag(ItemFlag::READABLE) && data->maxReadWriteChars > 0) : false;
}
bool AssetManager::isFluidContainer(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::FLUID : false;
}
bool AssetManager::isSplash(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::SPLASH : false;
}
bool AssetManager::isMoveable(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::MOVEABLE) : false;
}
bool AssetManager::hasHeight(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->hasFlag(ItemFlag::HAS_HEIGHT) : false;
}
bool AssetManager::isContainer(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::CONTAINER : false;
}
bool AssetManager::isTeleport(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::TELEPORT : false;
}
bool AssetManager::isDoor(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::DOOR : false;
}
bool AssetManager::isPodium(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->group == ItemGroup::PODIUM : false;
}
bool AssetManager::isDepot(quint16 id) const {
    const ItemData* data = getItemData(id);
    return (data && data->serverID != 0) ? data->type == ItemType::TYPE_DEPOT : false;
}

} // namespace RME
