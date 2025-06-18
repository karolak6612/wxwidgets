#include "TownManager.h"
#include "core/map/Map.h"
#include <QDebug>
#include <algorithm>

namespace RME {
namespace core {
namespace world {

TownManager::TownManager(RME::core::Map* map)
    : m_map(map)
{
}

// Town management (similar to original wxWidgets Towns class)
bool TownManager::addTown(const TownData& townData) {
    if (!validateTownData(townData)) {
        qWarning("TownManager::addTown: Invalid town data provided.");
        return false;
    }
    
    if (m_townsById.contains(townData.id)) {
        qWarning("TownManager::addTown: Town with ID %u already exists.").arg(townData.id);
        return false;
    }
    
    QString normalizedName = normalizeTownName(townData.name);
    if (m_townsByName.contains(normalizedName)) {
        qWarning("TownManager::addTown: Town with name '%s' already exists.").arg(townData.name);
        return false;
    }
    
    // Add to primary storage
    m_townsById.insert(townData.id, townData);
    
    // Update name cache
    m_townsByName.insert(normalizedName, townData.id);
    
    return true;
}

bool TownManager::removeTown(uint32_t townId) {
    auto it = m_townsById.find(townId);
    if (it == m_townsById.end()) {
        return false; // Town not found
    }
    
    // Remove from name cache
    QString normalizedName = normalizeTownName(it.value().name);
    m_townsByName.remove(normalizedName);
    
    // Remove from primary storage
    m_townsById.erase(it);
    
    return true;
}

bool TownManager::removeTown(const QString& townName) {
    TownData* town = getTown(townName);
    if (!town) {
        return false;
    }
    return removeTown(town->id);
}

// Town lookup (from original wxWidgets)
TownData* TownManager::getTown(uint32_t townId) {
    auto it = m_townsById.find(townId);
    return (it != m_townsById.end()) ? &it.value() : nullptr;
}

const TownData* TownManager::getTown(uint32_t townId) const {
    auto it = m_townsById.constFind(townId);
    return (it != m_townsById.constEnd()) ? &it.value() : nullptr;
}

TownData* TownManager::getTown(const QString& townName) {
    QString normalizedName = normalizeTownName(townName);
    auto it = m_townsByName.find(normalizedName);
    if (it != m_townsByName.end()) {
        return getTown(it.value()); // Get by ID
    }
    return nullptr;
}

const TownData* TownManager::getTown(const QString& townName) const {
    QString normalizedName = normalizeTownName(townName);
    auto it = m_townsByName.constFind(normalizedName);
    if (it != m_townsByName.constEnd()) {
        return getTown(it.value()); // Get by ID
    }
    return nullptr;
}

// ID management (from original wxWidgets)
uint32_t TownManager::getEmptyTownID() const {
    if (m_townsById.isEmpty()) {
        return 1; // Start from 1, not 0
    }
    
    // Find the first unused ID starting from 1
    for (uint32_t id = 1; id < UINT32_MAX; ++id) {
        if (!m_townsById.contains(id)) {
            return id;
        }
    }
    
    qWarning("TownManager::getEmptyTownID: No available town IDs (this should never happen).");
    return 0; // Error case
}

bool TownManager::isTownIdValid(uint32_t townId) const {
    return townId > 0 && m_townsById.contains(townId);
}

// Bulk operations
void TownManager::clearAllTowns() {
    m_townsById.clear();
    m_townsByName.clear();
}

// Iterator interface (from original wxWidgets)
QList<TownData*> TownManager::getAllTowns() {
    QList<TownData*> result;
    result.reserve(m_townsById.size());
    for (auto it = m_townsById.begin(); it != m_townsById.end(); ++it) {
        result.append(&it.value());
    }
    return result;
}

QList<const TownData*> TownManager::getAllTowns() const {
    QList<const TownData*> result;
    result.reserve(m_townsById.size());
    for (auto it = m_townsById.constBegin(); it != m_townsById.constEnd(); ++it) {
        result.append(&it.value());
    }
    return result;
}

QList<uint32_t> TownManager::getAllTownIds() const {
    return m_townsById.keys();
}

QList<QString> TownManager::getAllTownNames() const {
    QList<QString> result;
    result.reserve(m_townsById.size());
    for (auto it = m_townsById.constBegin(); it != m_townsById.constEnd(); ++it) {
        result.append(it.value().name);
    }
    return result;
}

// Validation and utility
bool TownManager::validateTownData(const TownData& townData) const {
    if (townData.id == 0) {
        qWarning("TownManager::validateTownData: Town ID cannot be 0.");
        return false;
    }
    
    if (townData.name.isEmpty()) {
        qWarning("TownManager::validateTownData: Town name cannot be empty.");
        return false;
    }
    
    if (!townData.templePosition.isValid()) {
        qWarning("TownManager::validateTownData: Town temple position is invalid.");
        return false;
    }
    
    return true;
}

QString TownManager::getDescription() const {
    return QString("TownManager: %1 towns loaded").arg(m_townsById.size());
}

// Helper methods
void TownManager::updateNameCache() {
    m_townsByName.clear();
    for (auto it = m_townsById.constBegin(); it != m_townsById.constEnd(); ++it) {
        QString normalizedName = normalizeTownName(it.value().name);
        m_townsByName.insert(normalizedName, it.key());
    }
}

QString TownManager::normalizeTownName(const QString& name) const {
    return name.trimmed().toLower(); // Case-insensitive, trimmed
}

} // namespace world
} // namespace core
} // namespace RME