#ifndef RME_SPAWN_MANAGER_H
#define RME_SPAWN_MANAGER_H

#include "SpawnData.h"
#include "core/Position.h"
#include <QSet>
#include <QList>

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
}
}

namespace RME {
namespace core {
namespace spawns {

/**
 * @brief Central spawn management class inspired by original wxWidgets Spawns class
 * 
 * Provides centralized spawn position tracking and management while using
 * SpawnData as the primary data structure.
 */
class SpawnManager {
public:
    explicit SpawnManager(RME::core::Map* map);
    ~SpawnManager() = default;

    // Prevent copying, allow moving
    SpawnManager(const SpawnManager&) = delete;
    SpawnManager& operator=(const SpawnManager&) = delete;
    SpawnManager(SpawnManager&&) = default;
    SpawnManager& operator=(SpawnManager&&) = default;

    // Spawn management (similar to original wxWidgets Spawns class)
    void addSpawn(const Position& pos, const SpawnData& spawnData);
    void removeSpawn(const Position& pos);
    bool hasSpawn(const Position& pos) const;
    
    // Position tracking (from original wxWidgets)
    const QSet<Position>& getSpawnPositions() const { return m_spawnPositions; }
    QList<Position> getAllSpawnPositions() const;
    
    // SpawnData access
    SpawnData* getSpawnAt(const Position& pos);
    const SpawnData* getSpawnAt(const Position& pos) const;
    
    // Bulk operations
    void clearAllSpawns();
    int getSpawnCount() const { return m_spawnPositions.size(); }
    
    // Selection management (from original wxWidgets)
    void selectSpawn(const Position& pos);
    void deselectSpawn(const Position& pos);
    void deselectAllSpawns();
    QList<Position> getSelectedSpawnPositions() const;
    
    // Utility methods
    QList<Position> getSpawnsInRadius(const Position& center, int radius) const;
    bool isPositionInAnySpawn(const Position& pos) const;

private:
    RME::core::Map* m_map; // Non-owning pointer to map context
    QSet<Position> m_spawnPositions; // Central position tracking (like wxWidgets)
    
    // Helper methods
    void updateTileSpawnData(const Position& pos, const SpawnData& spawnData);
    void clearTileSpawnData(const Position& pos);
};

} // namespace spawns
} // namespace core
} // namespace RME

#endif // RME_SPAWN_MANAGER_H