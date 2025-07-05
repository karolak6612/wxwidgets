#include "SpawnManager.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include <QDebug>

namespace RME {
namespace core {
namespace spawns {

SpawnManager::SpawnManager(RME::core::Map* map)
    : m_map(map)
{
}

// Spawn management (similar to original wxWidgets Spawns class)
void SpawnManager::addSpawn(const Position& pos, const Spawn& spawn) {
    if (!m_map) {
        qWarning("SpawnManager::addSpawn: Map pointer is null.");
        return;
    }
    
    // Add to position tracking
    m_spawnPositions.insert(pos);
    
    // Add to map's spawn list
    m_map->addSpawn(spawn);
    
    // Update tile with spawn data
    updateTileSpawn(pos, spawn);
}

void SpawnManager::removeSpawn(const Position& pos) {
    if (!m_map) {
        qWarning("SpawnManager::removeSpawn: Map pointer is null.");
        return;
    }
    
    if (!m_spawnPositions.contains(pos)) {
        return; // No spawn at this position
    }
    
    // Remove from position tracking
    m_spawnPositions.remove(pos);
    
    // Remove from map's spawn list
    m_map->removeSpawnAt(pos);
    
    // Clear tile spawn data
    clearTileSpawn(pos);
}

bool SpawnManager::hasSpawn(const Position& pos) const {
    return m_spawnPositions.contains(pos);
}

// Position tracking (from original wxWidgets)
QList<Position> SpawnManager::getAllSpawnPositions() const {
    return m_spawnPositions.values();
}

// Spawn access
Spawn* SpawnManager::getSpawn(const Position& pos) {
    if (!m_map) {
        return nullptr;
    }
    return m_map->getSpawnAt(pos);
}

const Spawn* SpawnManager::getSpawn(const Position& pos) const {
    if (!m_map) {
        return nullptr;
    }
    return m_map->getSpawnAt(pos);
}

// Bulk operations
void SpawnManager::clearAllSpawns() {
    if (!m_map) {
        return;
    }
    
    // Clear tile spawn data for all spawn positions
    for (const Position& pos : m_spawnPositions) {
        clearTileSpawn(pos);
    }
    
    // Clear position tracking
    m_spawnPositions.clear();
    
    // Clear map's spawn list
    m_map->clearSpawns();
}

// Selection management (from original wxWidgets)
void SpawnManager::selectSpawn(const Position& pos) {
    Spawn* spawn = getSpawn(pos);
    if (spawn) {
        spawn->select();
    }
}

void SpawnManager::deselectSpawn(const Position& pos) {
    Spawn* spawn = getSpawn(pos);
    if (spawn) {
        spawn->deselect();
    }
}

void SpawnManager::deselectAllSpawns() {
    for (const Position& pos : m_spawnPositions) {
        deselectSpawn(pos);
    }
}

QList<Position> SpawnManager::getSelectedSpawnPositions() const {
    QList<Position> selected;
    for (const Position& pos : m_spawnPositions) {
        const Spawn* spawn = getSpawn(pos);
        if (spawn && spawn->isSelected()) {
            selected.append(pos);
        }
    }
    return selected;
}

// Utility methods
QList<Position> SpawnManager::getSpawnsInRadius(const Position& center, int radius) const {
    QList<Position> spawnsInRadius;
    
    for (const Position& spawnPos : m_spawnPositions) {
        // Check if spawn is within radius of center (Manhattan distance)
        int dx = abs(spawnPos.x() - center.x());
        int dy = abs(spawnPos.y() - center.y());
        int dz = abs(spawnPos.z() - center.z());
        
        if (dz == 0 && dx <= radius && dy <= radius) {
            spawnsInRadius.append(spawnPos);
        }
    }
    
    return spawnsInRadius;
}

bool SpawnManager::isPositionInAnySpawn(const Position& pos) const {
    for (const Position& spawnPos : m_spawnPositions) {
        const Spawn* spawn = getSpawn(spawnPos);
        if (spawn && spawn->containsPosition(pos)) {
            return true;
        }
    }
    return false;
}

// Helper methods
void SpawnManager::updateTileSpawn(const Position& pos, const Spawn& spawn) {
    if (!m_map) {
        return;
    }
    
    Tile* tile = m_map->getTile(pos);
    if (tile) {
        tile->setSpawn(spawn);
        m_map->notifyTileChanged(pos);
    }
}

void SpawnManager::clearTileSpawn(const Position& pos) {
    if (!m_map) {
        return;
    }
    
    Tile* tile = m_map->getTile(pos);
    if (tile) {
        tile->clearSpawn();
        m_map->notifyTileChanged(pos);
    }
}

} // namespace spawns
} // namespace core
} // namespace RME