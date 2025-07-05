#ifndef RME_CORE_WORLD_TOWNMANAGER_H
#define RME_CORE_WORLD_TOWNMANAGER_H

#include "TownData.h"
#include <QHash>
#include <QList>
#include <QString>
#include <memory>

// Forward declarations
namespace RME {
namespace core {
    class Map;
}
}

namespace RME {
namespace core {
namespace world {

/**
 * @brief Central town management class inspired by original wxWidgets Towns class
 * 
 * Provides centralized town storage, lookup, and management functionality
 * similar to the original wxWidgets Towns class but with modern C++ design.
 */
class TownManager {
public:
    explicit TownManager(RME::core::Map* map = nullptr);
    ~TownManager() = default;

    // Prevent copying, allow moving
    TownManager(const TownManager&) = delete;
    TownManager& operator=(const TownManager&) = delete;
    TownManager(TownManager&&) = default;
    TownManager& operator=(TownManager&&) = default;

    // Town management (similar to original wxWidgets Towns class)
    bool addTown(const TownData& townData);
    bool removeTown(uint32_t townId);
    bool removeTown(const QString& townName);
    
    // Town lookup (from original wxWidgets)
    TownData* getTown(uint32_t townId);
    const TownData* getTown(uint32_t townId) const;
    TownData* getTown(const QString& townName);
    const TownData* getTown(const QString& townName) const;
    
    // ID management (from original wxWidgets)
    uint32_t getEmptyTownID() const;
    bool isTownIdValid(uint32_t townId) const;
    
    // Bulk operations
    void clearAllTowns();
    int getTownCount() const { return m_townsById.size(); }
    
    // Iterator interface (from original wxWidgets)
    QList<TownData*> getAllTowns();
    QList<const TownData*> getAllTowns() const;
    QList<uint32_t> getAllTownIds() const;
    QList<QString> getAllTownNames() const;
    
    // Validation and utility
    bool validateTownData(const TownData& townData) const;
    QString getDescription() const;

private:
    RME::core::Map* m_map; // Non-owning pointer to map context
    
    // Efficient storage for lookups (like original wxWidgets std::map)
    QHash<uint32_t, TownData> m_townsById;     // Primary storage by ID
    QHash<QString, uint32_t> m_townsByName;   // Name-to-ID lookup cache
    
    // Helper methods
    void updateNameCache();
    QString normalizeTownName(const QString& name) const;
};

} // namespace world
} // namespace core
} // namespace RME

#endif // RME_CORE_WORLD_TOWNMANAGER_H