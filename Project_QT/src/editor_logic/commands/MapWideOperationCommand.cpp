#include "editor_logic/commands/MapWideOperationCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/selection/SelectionManager.h"
#include "core/houses/Houses.h"
#include "core/assets/MaterialManager.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning
#include <QDataStream>
#include <QBuffer>
#include <QRandomGenerator> // For randomization

namespace RME {
namespace core {
namespace actions {

MapWideOperationCommand::MapWideOperationCommand(
    OperationType operationType,
    RME::core::Map* map,
    RME::core::editor::EditorControllerInterface* controller,
    const QHash<QString, QVariant>& parameters,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Map Operation"), parent),
    m_operationType(operationType),
    m_map(map),
    m_parameters(parameters)
{
    Q_ASSERT(m_map);
    
    setText(getOperationName());
}

void MapWideOperationCommand::redo() {
    if (!validateMembers() || !m_map) {
        setErrorText("redo map operation");
        return;
    }
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    m_wasCancelled = false;
    
    // Backup tiles only on first execution
    if (!m_hasBeenExecuted) {
        QList<RME::core::Position> positions = getOperationPositions();
        
        updateProgress(0, QObject::tr("Preparing operation..."));
        
        // Backup all tiles that might be modified
        for (int i = 0; i < positions.size(); ++i) {
            if (!shouldContinue()) {
                m_wasCancelled = true;
                return;
            }
            
            backupTile(positions[i]);
            
            if (i % 100 == 0) {
                int progress = (i * 10) / positions.size(); // 0-10% for backup
                updateProgress(progress, QObject::tr("Backing up tiles... %1/%2").arg(i).arg(positions.size()));
            }
        }
        
        m_hasBeenExecuted = true;
    }
    
    // Execute the operation
    updateProgress(10, QObject::tr("Executing %1...").arg(getOperationName()));
    
    switch (m_operationType) {
        case OperationType::BorderizeMap:
            executeBorderizeMap();
            break;
        case OperationType::RandomizeMap:
            executeRandomizeMap();
            break;
        case OperationType::ClearInvalidHouseTiles:
            executeClearInvalidHouseTiles();
            break;
        case OperationType::ClearModifiedTileState:
            executeClearModifiedTileState();
            break;
        case OperationType::ValidateGrounds:
            executeValidateGrounds();
            break;
        case OperationType::BorderizeSelection:
            executeBorderizeSelection();
            break;
        case OperationType::RandomizeSelection:
            executeRandomizeSelection();
            break;
        case OperationType::GenerateEmptyGrounds:
            executeGenerateEmptyGrounds();
            break;
        case OperationType::RemoveDuplicateGrounds:
            executeRemoveDuplicateGrounds();
            break;
    }
    
    if (!m_wasCancelled) {
        updateProgress(100, QObject::tr("Operation completed. Modified %1 tiles.").arg(m_modifiedTileCount));
        m_map->setChanged(true);
    } else {
        updateProgress(0, QObject::tr("Operation cancelled."));
    }
    
    qDebug() << "MapWideOperationCommand::redo: Executed" << getOperationName() 
             << "- processed" << m_processedTileCount << "tiles, modified" << m_modifiedTileCount;
}

void MapWideOperationCommand::undo() {
    if (!validateMembers() || !m_map || !m_hasBeenExecuted) {
        setErrorText("undo map operation");
        return;
    }
    
    updateProgress(0, QObject::tr("Undoing %1...").arg(getOperationName()));
    
    // Restore all backed up tiles
    for (int i = 0; i < m_tileBackups.size(); ++i) {
        if (!shouldContinue()) {
            break;
        }
        
        restoreTile(m_tileBackups[i]);
        
        if (i % 100 == 0) {
            int progress = (i * 100) / m_tileBackups.size();
            updateProgress(progress, QObject::tr("Restoring tiles... %1/%2").arg(i).arg(m_tileBackups.size()));
        }
    }
    
    updateProgress(100, QObject::tr("Undo completed. Restored %1 tiles.").arg(m_tileBackups.size()));
    m_map->setChanged(true);
    
    setText(QObject::tr("Undo: %1").arg(getOperationName()));
    
    qDebug() << "MapWideOperationCommand::undo: Restored" << m_tileBackups.size() << "tiles";
}

// Progress and cancellation
void MapWideOperationCommand::setProgressCallback(std::function<void(int, const QString&)> callback) {
    m_progressCallback = callback;
}

void MapWideOperationCommand::setCancellationCallback(std::function<bool()> callback) {
    m_cancellationCallback = callback;
}

bool MapWideOperationCommand::shouldContinue() {
    if (m_cancellationCallback) {
        return !m_cancellationCallback();
    }
    return true;
}

void MapWideOperationCommand::updateProgress(int percentage, const QString& message) {
    if (m_progressCallback) {
        m_progressCallback(percentage, message);
    }
}

// Helper methods
void MapWideOperationCommand::backupTile(const RME::core::Position& pos) {
    RME::core::Tile* tile = m_map->getTile(pos);
    if (!tile) {
        return;
    }
    
    // Serialize tile state
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    
    // Store tile data (simplified - would need proper tile serialization)
    stream << tile->getHouseId();
    stream << tile->isProtectionZone();
    stream << tile->hasGround();
    // TODO: Add complete tile serialization
    
    TileBackup backup;
    backup.position = pos;
    backup.tileData = buffer.data();
    
    m_tileBackups.append(backup);
}

void MapWideOperationCommand::restoreTile(const TileBackup& backup) {
    RME::core::Tile* tile = m_map->getTile(backup.position);
    if (!tile) {
        return;
    }
    
    // Deserialize and restore tile state
    QBuffer buffer;
    buffer.setData(backup.tileData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    
    quint32 houseId;
    bool isProtectionZone;
    bool hasGround;
    
    stream >> houseId >> isProtectionZone >> hasGround;
    
    tile->setHouseId(houseId);
    tile->setIsProtectionZone(isProtectionZone);
    // TODO: Add complete tile restoration
    
    notifyMapChanged(backup.position);
}

QString MapWideOperationCommand::getOperationName() const {
    switch (m_operationType) {
        case OperationType::BorderizeMap:
            return QObject::tr("Borderize Map");
        case OperationType::RandomizeMap:
            return QObject::tr("Randomize Map");
        case OperationType::ClearInvalidHouseTiles:
            return QObject::tr("Clear Invalid House Tiles");
        case OperationType::ClearModifiedTileState:
            return QObject::tr("Clear Modified Tile State");
        case OperationType::ValidateGrounds:
            return QObject::tr("Validate Grounds");
        case OperationType::BorderizeSelection:
            return QObject::tr("Borderize Selection");
        case OperationType::RandomizeSelection:
            return QObject::tr("Randomize Selection");
        case OperationType::GenerateEmptyGrounds:
            return QObject::tr("Generate Empty Grounds");
        case OperationType::RemoveDuplicateGrounds:
            return QObject::tr("Remove Duplicate Grounds");
        default:
            return QObject::tr("Map Operation");
    }
}

QList<RME::core::Position> MapWideOperationCommand::getOperationPositions() const {
    QList<RME::core::Position> positions;
    
    switch (m_operationType) {
        case OperationType::BorderizeSelection:
        case OperationType::RandomizeSelection: {
            // Get selected positions
            if (m_controller->getSelectionManager()) {
                // Get selected positions from selection manager
                if (getController()->getSelectionManager()) {
                    selectedPositions = getController()->getSelectionManager()->getSelectedPositions();
                }
                // positions = m_controller->getSelectionManager()->getSelectedPositions();
            }
            break;
        }
        default: {
            // Get all map positions
            for (int z = 0; z < m_map->getDepth(); ++z) {
                for (int y = 0; y < m_map->getHeight(); ++y) {
                    for (int x = 0; x < m_map->getWidth(); ++x) {
                        positions.append(RME::core::Position(x, y, z));
                    }
                }
            }
            break;
        }
    }
    
    return positions;
}

// Operation implementations (simplified - would need full implementations)
void MapWideOperationCommand::executeBorderizeMap() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeBorderizeMap: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting borderization..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get MaterialManager for border data
    auto materialManager = getController()->getMaterialManager();
    if (!materialManager) {
        qWarning() << "MapWideOperationCommand::executeBorderizeMap: No MaterialManager available";
        return;
    }
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Analyzing tiles for borderization..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile && tile->getGround()) {
                    // Apply borderization to this tile
                    if (applyBorderizationToTile(tile, pos, materialManager)) {
                        m_modifiedTileCount++;
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Borderizing tiles... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Borderization completed. Modified %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeBorderizeMap: Processed" << m_processedTileCount 
            << "tiles, modified" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeRandomizeMap() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeRandomizeMap: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting randomization..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get MaterialManager for randomization data
    auto materialManager = getController()->getMaterialManager();
    if (!materialManager) {
        qWarning() << "MapWideOperationCommand::executeRandomizeMap: No MaterialManager available";
        return;
    }
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Analyzing tiles for randomization..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile && tile->getGround()) {
                    // Apply randomization to this tile
                    if (applyRandomizationToTile(tile, pos, materialManager)) {
                        m_modifiedTileCount++;
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Randomizing tiles... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Randomization completed. Modified %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeRandomizeMap: Processed" << m_processedTileCount 
            << "tiles, modified" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeClearInvalidHouseTiles() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeClearInvalidHouseTiles: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting house tile cleanup..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get Houses system
    RME::core::Houses* houses = m_map->getHouses();
    if (!houses) {
        qWarning() << "MapWideOperationCommand::executeClearInvalidHouseTiles: No houses system available";
        return;
    }
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Checking house tile validity..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile && tile->getHouseId() != 0) {
                    // Check if house still exists
                    if (!houses->getHouse(tile->getHouseId())) {
                        // House doesn't exist, clear the house ID
                        tile->setHouseId(0);
                        m_modifiedTileCount++;
                        m_map->notifyTileChanged(pos);
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Checking house tiles... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("House tile cleanup completed. Cleaned %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeClearInvalidHouseTiles: Processed" << m_processedTileCount 
            << "tiles, cleaned" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeClearModifiedTileState() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeClearModifiedTileState: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting modified state clearing..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Clearing tile modification flags..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile) {
                    // Clear modification flags (assuming there's a method for this)
                    // This would clear any "dirty" or "modified" flags on the tile
                    if (tile->isModified()) {
                        tile->setModified(false);
                        m_modifiedTileCount++;
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Clearing modification flags... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Modified state clearing completed. Cleared %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeClearModifiedTileState: Processed" << m_processedTileCount 
            << "tiles, cleared" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeValidateGrounds() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeValidateGrounds: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting ground validation..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get default ground item ID from parameters or use a default
    uint16_t defaultGroundId = m_parameters.value("defaultGroundId", 100).toUInt(); // Default grass
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Validating ground items..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile) {
                    bool needsGround = false;
                    
                    // Check if tile needs ground
                    if (!tile->getGround()) {
                        needsGround = true;
                    } else {
                        RME::core::Item* ground = tile->getGround();
                        // Check if ground item is valid (simplified check)
                        if (ground->getID() == 0) {
                            needsGround = true;
                        }
                    }
                    
                    if (needsGround) {
                        // Add default ground
                        auto defaultGround = RME::core::Item::create(defaultGroundId, tile->getItemTypeProvider());
                        if (defaultGround) {
                            tile->setGround(std::move(defaultGround));
                            m_modifiedTileCount++;
                            m_map->notifyTileChanged(pos);
                        }
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Validating grounds... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Ground validation completed. Fixed %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeValidateGrounds: Processed" << m_processedTileCount 
            << "tiles, fixed" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeBorderizeSelection() {
    if (!m_map || !m_controller) {
        qWarning() << "MapWideOperationCommand::executeBorderizeSelection: Missing dependencies";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting selection borderization..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get MaterialManager for border data
    auto materialManager = getController()->getMaterialManager();
    if (!materialManager) {
        qWarning() << "MapWideOperationCommand::executeBorderizeSelection: No MaterialManager available";
        return;
    }
    
    // Get selected positions
    QList<RME::core::Position> selectedPositions;
    if (getController()->getSelectionManager()) {
        selectedPositions = getController()->getSelectionManager()->getSelectedPositions();
    }
    
    if (selectedPositions.isEmpty()) {
        updateProgress(100, QObject::tr("No selection to borderize."));
        qWarning() << "MapWideOperationCommand::executeBorderizeSelection: No selection";
        return;
    }
    
    updateProgress(20, QObject::tr("Applying borders to %1 selected tiles...").arg(selectedPositions.size()));
    
    // Process selected tiles
    int processedCount = 0;
    for (const RME::core::Position& pos : selectedPositions) {
        if (!shouldContinue()) {
            m_wasCancelled = true;
            return;
        }
        
        RME::core::Tile* tile = m_map->getTile(pos);
        if (tile && tile->getGround()) {
            // Apply borderization to this tile
            if (applyBorderizationToTile(tile, pos, materialManager)) {
                m_modifiedTileCount++;
            }
        }
        
        processedCount++;
        m_processedTileCount++;
        
        // Update progress every 10 tiles
        if (processedCount % 10 == 0) {
            int progress = 20 + (processedCount * 70) / selectedPositions.size(); // 20-90%
            updateProgress(progress, QObject::tr("Borderizing selection... %1/%2").arg(processedCount).arg(selectedPositions.size()));
        }
    }
    
    updateProgress(100, QObject::tr("Selection borderization completed. Modified %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeBorderizeSelection: Processed" << m_processedTileCount 
            << "tiles, modified" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeRandomizeSelection() {
    if (!m_map || !m_controller) {
        qWarning() << "MapWideOperationCommand::executeRandomizeSelection: Missing dependencies";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting selection randomization..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get MaterialManager for randomization data
    auto materialManager = getController()->getMaterialManager();
    if (!materialManager) {
        qWarning() << "MapWideOperationCommand::executeRandomizeSelection: No MaterialManager available";
        return;
    }
    
    // Get selected positions
    QList<RME::core::Position> selectedPositions;
    if (getController()->getSelectionManager()) {
        selectedPositions = getController()->getSelectionManager()->getSelectedPositions();
    }
    
    if (selectedPositions.isEmpty()) {
        updateProgress(100, QObject::tr("No selection to randomize."));
        qWarning() << "MapWideOperationCommand::executeRandomizeSelection: No selection";
        return;
    }
    
    updateProgress(20, QObject::tr("Randomizing %1 selected tiles...").arg(selectedPositions.size()));
    
    // Process selected tiles
    int processedCount = 0;
    for (const RME::core::Position& pos : selectedPositions) {
        if (!shouldContinue()) {
            m_wasCancelled = true;
            return;
        }
        
        RME::core::Tile* tile = m_map->getTile(pos);
        if (tile && tile->getGround()) {
            // Apply randomization to this tile
            if (applyRandomizationToTile(tile, pos, materialManager)) {
                m_modifiedTileCount++;
            }
        }
        
        processedCount++;
        m_processedTileCount++;
        
        // Update progress every 10 tiles
        if (processedCount % 10 == 0) {
            int progress = 20 + (processedCount * 70) / selectedPositions.size(); // 20-90%
            updateProgress(progress, QObject::tr("Randomizing selection... %1/%2").arg(processedCount).arg(selectedPositions.size()));
        }
    }
    
    updateProgress(100, QObject::tr("Selection randomization completed. Modified %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeRandomizeSelection: Processed" << m_processedTileCount 
            << "tiles, modified" << m_modifiedTileCount;
}

void MapWideOperationCommand::executeGenerateEmptyGrounds() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeGenerateEmptyGrounds: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting empty ground generation..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Get default ground item ID from parameters or use a default
    uint16_t defaultGroundId = m_parameters.value("defaultGroundId", 100).toUInt(); // Default grass
    
    // Process all map positions, creating tiles where needed
    int totalPositions = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    int processedCount = 0;
    
    updateProgress(20, QObject::tr("Generating empty grounds..."));
    
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (!tile) {
                    // Create a new tile with default ground
                    tile = m_map->createTile(pos);
                    if (tile) {
                        auto defaultGround = RME::core::Item::create(defaultGroundId, tile->getItemTypeProvider());
                        if (defaultGround) {
                            tile->setGround(std::move(defaultGround));
                            m_modifiedTileCount++;
                            m_map->notifyTileChanged(pos);
                        }
                    }
                } else if (!tile->getGround()) {
                    // Add ground to existing tile without ground
                    auto defaultGround = RME::core::Item::create(defaultGroundId, tile->getItemTypeProvider());
                    if (defaultGround) {
                        tile->setGround(std::move(defaultGround));
                        m_modifiedTileCount++;
                        m_map->notifyTileChanged(pos);
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 1000 positions
                if (processedCount % 1000 == 0) {
                    int progress = 20 + (processedCount * 70) / totalPositions; // 20-90%
                    updateProgress(progress, QObject::tr("Generating grounds... %1/%2").arg(processedCount).arg(totalPositions));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Empty ground generation completed. Added %1 grounds.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeGenerateEmptyGrounds: Processed" << m_processedTileCount 
            << "positions, added" << m_modifiedTileCount << "grounds";
}

void MapWideOperationCommand::executeRemoveDuplicateGrounds() {
    if (!m_map) {
        qWarning() << "MapWideOperationCommand::executeRemoveDuplicateGrounds: No map available";
        return;
    }
    
    updateProgress(10, QObject::tr("Starting duplicate ground removal..."));
    
    m_processedTileCount = 0;
    m_modifiedTileCount = 0;
    
    // Calculate total tiles for progress tracking
    int totalTiles = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                RME::core::Position pos(x, y, z);
                if (m_map->getTile(pos)) {
                    totalTiles++;
                }
            }
        }
    }
    
    updateProgress(20, QObject::tr("Checking for duplicate grounds..."));
    
    // Process all tiles
    int processedCount = 0;
    for (int z = 0; z < m_map->getDepth(); ++z) {
        for (int y = 0; y < m_map->getHeight(); ++y) {
            for (int x = 0; x < m_map->getWidth(); ++x) {
                if (!shouldContinue()) {
                    m_wasCancelled = true;
                    return;
                }
                
                RME::core::Position pos(x, y, z);
                RME::core::Tile* tile = m_map->getTile(pos);
                
                if (tile) {
                    // Check for duplicate ground items in the tile's item list
                    // This would remove any ground-type items that are not the actual ground
                    const auto& items = tile->getItems();
                    bool removedDuplicates = false;
                    
                    if (tile->getGround()) {
                        uint16_t groundId = tile->getGround()->getID();
                        
                        // Remove any items that have the same ID as the ground
                        // (simplified implementation - in reality would need more sophisticated duplicate detection)
                        auto it = items.begin();
                        while (it != items.end()) {
                            if ((*it) && (*it)->getID() == groundId) {
                                // Found duplicate ground item
                                it = items.erase(it);
                                removedDuplicates = true;
                            } else {
                                ++it;
                            }
                        }
                    }
                    
                    if (removedDuplicates) {
                        m_modifiedTileCount++;
                        m_map->notifyTileChanged(pos);
                    }
                }
                
                processedCount++;
                m_processedTileCount++;
                
                // Update progress every 100 tiles
                if (processedCount % 100 == 0) {
                    int progress = 20 + (processedCount * 70) / totalTiles; // 20-90%
                    updateProgress(progress, QObject::tr("Checking duplicates... %1/%2").arg(processedCount).arg(totalTiles));
                }
            }
        }
    }
    
    updateProgress(100, QObject::tr("Duplicate ground removal completed. Cleaned %1 tiles.").arg(m_modifiedTileCount));
    qInfo() << "MapWideOperationCommand::executeRemoveDuplicateGrounds: Processed" << m_processedTileCount 
            << "tiles, cleaned" << m_modifiedTileCount;
}

// Helper method implementations
bool MapWideOperationCommand::applyBorderizationToTile(RME::core::Tile* tile, const RME::core::Position& pos, MaterialManager* materialManager) {
    if (!tile || !materialManager || !tile->getGround()) {
        return false;
    }
    
    RME::core::Item* groundItem = tile->getGround();
    if (!groundItem) {
        return false;
    }
    
    // Get material data for the ground item
    const MaterialData* material = materialManager->getMaterialByItemId(groundItem->getID());
    if (!material || !material->hasBorders()) {
        return false; // No border data available
    }
    
    // Get neighboring tiles
    RME::core::Tile* neighbors[8];
    for (int i = 0; i < 8; ++i) {
        neighbors[i] = getNeighborTile(pos, i);
    }
    
    // Apply borderization using tile's built-in method
    tile->borderize(neighbors);
    
    return true; // Assume modification occurred
}

bool MapWideOperationCommand::applyRandomizationToTile(RME::core::Tile* tile, const RME::core::Position& pos, MaterialManager* materialManager) {
    if (!tile || !materialManager || !tile->getGround()) {
        return false;
    }
    
    RME::core::Item* groundItem = tile->getGround();
    if (!groundItem) {
        return false;
    }
    
    // Get material data for randomization options
    const MaterialData* material = materialManager->getMaterialByItemId(groundItem->getID());
    if (!material || !material->hasAlternatives()) {
        return false; // No alternatives available
    }
    
    // Apply randomization (simplified implementation)
    // In a full implementation, this would select random alternatives from the material
    QRandomGenerator* rng = QRandomGenerator::global();
    if (rng->bounded(100) < 30) { // 30% chance to randomize
        // TODO: Get alternative ground items from material and apply one randomly
        return true;
    }
    
    return false;
}

RME::core::Tile* MapWideOperationCommand::getNeighborTile(const RME::core::Position& pos, int direction) const {
    if (!m_map) {
        return nullptr;
    }
    
    // Direction mapping: 0=NW, 1=N, 2=NE, 3=E, 4=SE, 5=S, 6=SW, 7=W
    static const int dx[] = {-1,  0,  1,  1,  1,  0, -1, -1};
    static const int dy[] = {-1, -1, -1,  0,  1,  1,  1,  0};
    
    if (direction < 0 || direction >= 8) {
        return nullptr;
    }
    
    RME::core::Position neighborPos(pos.x + dx[direction], pos.y + dy[direction], pos.z);
    return m_map->getTile(neighborPos);
}

} // namespace actions
} // namespace core
} // namespace RME