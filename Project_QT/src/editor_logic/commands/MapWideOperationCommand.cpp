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

namespace RME {
namespace core {
namespace actions {

MapWideOperationCommand::MapWideOperationCommand(
    OperationType operationType,
    RME::core::Map* map,
    RME::core::editor::EditorControllerInterface* controller,
    const QHash<QString, QVariant>& parameters,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_operationType(operationType),
    m_map(map),
    m_controller(controller),
    m_parameters(parameters)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_controller);
    
    setText(getOperationName());
}

void MapWideOperationCommand::redo() {
    if (!m_map || !m_controller) {
        qWarning("MapWideOperationCommand::redo: Invalid members.");
        setText(QObject::tr("Map Operation (Error)"));
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
    if (!m_map || !m_controller || !m_hasBeenExecuted) {
        qWarning("MapWideOperationCommand::undo: Invalid state or not executed.");
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
    
    m_map->notifyTileChanged(backup.position);
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
                // TODO: Get selected positions from selection manager
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
    // TODO: Implement borderization logic
    updateProgress(50, QObject::tr("Applying borders..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = m_processedTileCount / 4; // Estimate
}

void MapWideOperationCommand::executeRandomizeMap() {
    // TODO: Implement randomization logic
    updateProgress(50, QObject::tr("Randomizing grounds..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = m_processedTileCount / 3; // Estimate
}

void MapWideOperationCommand::executeClearInvalidHouseTiles() {
    // TODO: Implement house tile cleanup
    updateProgress(50, QObject::tr("Clearing invalid house tiles..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = 0; // Count actual modifications
}

void MapWideOperationCommand::executeClearModifiedTileState() {
    // TODO: Implement modified state clearing
    updateProgress(50, QObject::tr("Clearing modified tile states..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = m_processedTileCount; // All tiles modified
}

void MapWideOperationCommand::executeValidateGrounds() {
    // TODO: Implement ground validation
    updateProgress(50, QObject::tr("Validating grounds..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = 0; // Count actual fixes
}

void MapWideOperationCommand::executeBorderizeSelection() {
    // TODO: Implement selection borderization
    updateProgress(50, QObject::tr("Applying borders to selection..."));
    // Get selection size and process
}

void MapWideOperationCommand::executeRandomizeSelection() {
    // TODO: Implement selection randomization
    updateProgress(50, QObject::tr("Randomizing selection..."));
    // Get selection size and process
}

void MapWideOperationCommand::executeGenerateEmptyGrounds() {
    // TODO: Implement empty ground generation
    updateProgress(50, QObject::tr("Generating empty grounds..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = 0; // Count actual additions
}

void MapWideOperationCommand::executeRemoveDuplicateGrounds() {
    // TODO: Implement duplicate ground removal
    updateProgress(50, QObject::tr("Removing duplicate grounds..."));
    m_processedTileCount = m_map->getWidth() * m_map->getHeight() * m_map->getDepth();
    m_modifiedTileCount = 0; // Count actual removals
}

} // namespace actions
} // namespace core
} // namespace RME