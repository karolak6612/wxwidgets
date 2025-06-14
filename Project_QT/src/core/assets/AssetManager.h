#ifndef RME_ASSET_MANAGER_H
#define RME_ASSET_MANAGER_H

#include "ClientVersionManager.h"
#include "ItemDatabase.h"
#include "CreatureDatabase.h"
#include "../sprites/SpriteManager.h" // SpriteManager is in sprites subdir
#include "../IItemTypeProvider.h"     // For ItemDatabase to provide to Item instances

#include <QString>
#include <QScopedPointer>
#include <QDir> // For path operations
#include <QCoreApplication> // For applicationDirPath fallback

namespace RME {

// Forward declare data types for accessors
struct ClientProfile;
namespace assets { // Assuming ItemData, CreatureData, SpriteData are in RME::core::assets
    class ItemData;
    class CreatureData;
    class SpriteData;
    class MaterialManager; // Forward declare for getMaterialManager()
    class MaterialData;
} // namespace assets


// The AssetManager will also implement IItemTypeProvider for the new Item/Tile classes
class AssetManager : public IItemTypeProvider {
public:
    // Singleton access or service locator pattern can be used.
    // For now, allow direct instantiation or make it a global/service.
    // Let's assume it's instantiated and managed by the application.
    AssetManager();
    ~AssetManager();

    // Attempts to load all assets for a given client version.
    // dataPath is the root directory where client data (e.g., Tibia.dat, Tibia.spr, items.otb, clients.xml) can be found.
    // clientVersionString is like "10.98", "7.72", etc.
    // Returns true if all essential assets were loaded successfully.
    bool loadAllAssets(const QString& dataPath, const QString& clientVersionString);

    // Accessors for the loaded data managers
    const ClientVersionManager& getClientVersionManager() const;
    const ItemDatabase& getItemDatabase() const;
    const CreatureDatabase& getCreatureDatabase() const;
    const SpriteManager& getSpriteManager() const;
    const RME::core::assets::MaterialManager& getMaterialManager() const; // New accessor

    // Convenience accessors for specific data
    const ClientProfile* getCurrentClientProfile() const;
    const assets::ItemData* getItemData(quint16 itemId) const; // From ItemDatabase, use namespaced type
    const assets::CreatureData* getCreatureData(const QString& name) const; // From CreatureDatabase, use namespaced type
    const assets::SpriteData* getSpriteData(quint32 spriteId) const; // From SpriteManager, use namespaced type
    const assets::MaterialData* getMaterialData(const QString& id) const; // Delegates to MaterialManager

    // --- IItemTypeProvider Implementation ---
    // These methods will delegate to the ItemDatabase instance.
    QString getName(quint16 id) const override;
    QString getDescription(quint16 id) const override;
    quint32 getFlags(quint16 id) const override; // ItemData::flags
    double getWeight(quint16 id, quint16 subtype) const override;
    bool isBlocking(quint16 id) const override;
    bool isProjectileBlocking(quint16 id) const override;
    bool isPathBlocking(quint16 id) const override;
    bool isWalkable(quint16 id) const override;
    bool isStackable(quint16 id) const override;
    bool isGround(quint16 id) const override;
    bool isAlwaysOnTop(quint16 id) const override;
    bool isReadable(quint16 id) const override;
    bool isWriteable(quint16 id) const override;
    bool isFluidContainer(quint16 id) const override;
    bool isSplash(quint16 id) const override;
    bool isMoveable(quint16 id) const override;
    bool hasHeight(quint16 id) const override;
    bool isContainer(quint16 id) const override;
    bool isTeleport(quint16 id) const override;
    bool isDoor(quint16 id) const override;
    bool isPodium(quint16 id) const override;
    bool isDepot(quint16 id) const override;
    // --- End IItemTypeProvider Implementation ---

private:
    QString resolvePath(const QString& dataPath, const QString& relativeOrAbsolutePath) const;

    struct AssetManagerData; // PIMPL
    QScopedPointer<AssetManagerData> d;
};

} // namespace RME

#endif // RME_ASSET_MANAGER_H
