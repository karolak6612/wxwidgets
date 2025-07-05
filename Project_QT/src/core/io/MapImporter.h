#ifndef RME_MAP_IMPORTER_H
#define RME_MAP_IMPORTER_H

#include <QString>
#include <QHash>
#include <QVariant>
#include "core/Position.h"

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
    namespace editor { class EditorControllerInterface; }
    namespace utils { class ProgressTracker; }
}
}

namespace RME {
namespace core {
namespace io {

/**
 * @brief Imports map data from various formats
 * 
 * This class handles the import of map data from OTBM files and other
 * supported formats. It provides options for merging imported data
 * with existing maps, applying offsets, and handling conflicts.
 */
class MapImporter {
public:
    enum class ImportMode {
        Replace,        // Replace existing tiles
        Merge,          // Merge with existing tiles
        AddOnly,        // Only add to empty tiles
        SkipExisting    // Skip tiles that already exist
    };

    struct ImportOptions {
        RME::core::Position offset = RME::core::Position(0, 0, 0);
        ImportMode mode = ImportMode::Merge;
        bool importHouses = true;
        bool importSpawns = true;
        bool importWaypoints = true;
        bool importCreatures = false;
        bool validateAfterImport = true;
        bool showProgressDialog = true;
        QHash<QString, QVariant> customOptions;
    };

    explicit MapImporter(RME::core::Map* targetMap, RME::core::editor::EditorControllerInterface* controller);
    ~MapImporter() = default;

    // Import methods
    bool importFromFile(const QString& filename, const ImportOptions& options = ImportOptions());
    bool importFromOTBM(const QString& filename, const ImportOptions& options = ImportOptions());
    bool importFromMap(RME::core::Map* sourceMap, const ImportOptions& options = ImportOptions());

    // Progress tracking
    void setProgressTracker(RME::core::utils::ProgressTracker* tracker);

    // Statistics
    struct ImportStatistics {
        quint32 tilesProcessed = 0;
        quint32 tilesImported = 0;
        quint32 tilesSkipped = 0;
        quint32 tilesReplaced = 0;
        quint32 housesImported = 0;
        quint32 spawnsImported = 0;
        quint32 waypointsImported = 0;
        quint32 creaturesImported = 0;
        QString errorMessage;
        bool success = false;
    };

    ImportStatistics getLastImportStatistics() const { return m_lastStatistics; }

private:
    RME::core::Map* m_targetMap;
    RME::core::editor::EditorControllerInterface* m_controller;
    RME::core::utils::ProgressTracker* m_progressTracker = nullptr;
    ImportStatistics m_lastStatistics;

    // Import implementation
    bool performImport(RME::core::Map* sourceMap, const ImportOptions& options);
    bool importTile(const RME::core::Position& sourcePos, const RME::core::Position& targetPos, 
                   const RME::core::Tile* sourceTile, const ImportOptions& options);
    bool importHouses(RME::core::Map* sourceMap, const ImportOptions& options);
    bool importSpawns(RME::core::Map* sourceMap, const ImportOptions& options);
    bool importWaypoints(RME::core::Map* sourceMap, const ImportOptions& options);
    bool importCreatures(RME::core::Map* sourceMap, const ImportOptions& options);

    // Helper methods
    bool shouldImportTile(const RME::core::Position& targetPos, const RME::core::Tile* sourceTile, const ImportOptions& options) const;
    RME::core::Position applyOffset(const RME::core::Position& pos, const RME::core::Position& offset) const;
    bool isValidTargetPosition(const RME::core::Position& pos) const;
    void updateProgress(const QString& message);
    void resetStatistics();
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_MAP_IMPORTER_H