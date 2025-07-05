#ifndef RME_MAPWIDEOPERATIONCOMMAND_H
#define RME_MAPWIDEOPERATIONCOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include <QHash>
#include <QList>
#include <functional>
#include "core/Position.h"
#include "core/actions/CommandIds.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int MapWideOperationCommandId = toInt(CommandId::MapWideOperation);

/**
 * @brief Command for map-wide operations with progress tracking and undo support
 * 
 * This command handles large-scale map operations like borderization, randomization,
 * cleanup operations, and validation. It provides progress tracking for long-running
 * operations and proper undo/redo support by storing tile state changes.
 */
class MapWideOperationCommand : public BaseCommand {
public:
    enum class OperationType {
        BorderizeMap,
        RandomizeMap,
        ClearInvalidHouseTiles,
        ClearModifiedTileState,
        ValidateGrounds,
        BorderizeSelection,
        RandomizeSelection,
        GenerateEmptyGrounds,
        RemoveDuplicateGrounds
    };

    MapWideOperationCommand(
        OperationType operationType,
        RME::core::Map* map,
        RME::core::editor::EditorControllerInterface* controller,
        const QHash<QString, QVariant>& parameters = QHash<QString, QVariant>(),
        QUndoCommand* parent = nullptr
    );

    ~MapWideOperationCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return MapWideOperationCommandId; }
    
    // Progress tracking
    void setProgressCallback(std::function<void(int, const QString&)> callback);
    void setCancellationCallback(std::function<bool()> callback);
    
    // Results
    quint32 getProcessedTileCount() const { return m_processedTileCount; }
    quint32 getModifiedTileCount() const { return m_modifiedTileCount; }
    bool wasCancelled() const { return m_wasCancelled; }

private:
    OperationType m_operationType;
    RME::core::Map* m_map;
    QHash<QString, QVariant> m_parameters;
    
    // State backup for undo
    struct TileBackup {
        RME::core::Position position;
        QByteArray tileData; // Serialized tile state
    };
    QList<TileBackup> m_tileBackups;
    
    // Progress tracking
    std::function<void(int, const QString&)> m_progressCallback;
    std::function<bool()> m_cancellationCallback;
    quint32 m_processedTileCount = 0;
    quint32 m_modifiedTileCount = 0;
    bool m_wasCancelled = false;
    bool m_hasBeenExecuted = false;
    
    // Operation implementations
    void executeBorderizeMap();
    void executeRandomizeMap();
    void executeClearInvalidHouseTiles();
    void executeClearModifiedTileState();
    void executeValidateGrounds();
    void executeBorderizeSelection();
    void executeRandomizeSelection();
    void executeGenerateEmptyGrounds();
    void executeRemoveDuplicateGrounds();
    
    // Helper methods
    void backupTile(const RME::core::Position& pos);
    void restoreTile(const TileBackup& backup);
    bool shouldContinue();
    void updateProgress(int percentage, const QString& message);
    QString getOperationName() const;
    QList<RME::core::Position> getOperationPositions() const;
    
    // Operation helper methods
    bool applyBorderizationToTile(RME::core::Tile* tile, const RME::core::Position& pos, class MaterialManager* materialManager);
    bool applyRandomizationToTile(RME::core::Tile* tile, const RME::core::Position& pos, class MaterialManager* materialManager);
    RME::core::Tile* getNeighborTile(const RME::core::Position& pos, int direction) const;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_MAPWIDEOPERATIONCOMMAND_H