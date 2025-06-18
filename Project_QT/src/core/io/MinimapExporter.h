#ifndef RME_MINIMAP_EXPORTER_H
#define RME_MINIMAP_EXPORTER_H

#include <QString>
#include <QRect>
#include <QImage>
#include "core/Position.h"

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
    namespace selection { class SelectionManager; }
}
}

namespace RME {
namespace core {
namespace io {

/**
 * @brief Exports map data as minimap images
 * 
 * This class handles the export of map data to minimap format images.
 * It supports exporting entire maps, specific floors, or selected regions
 * as minimap images that can be used by Tibia clients or other tools.
 */
class MinimapExporter {
public:
    struct ExportOptions {
        int floor = -1;              // -1 for all floors, specific floor number otherwise
        bool includeCreatures = false;
        bool includeSpawns = false;
        bool includeHouses = true;
        bool includeWaypoints = false;
        QRect region = QRect();      // Empty rect for entire map
        int scale = 1;               // Scale factor for output image
        QString format = "png";      // Output image format
    };

    explicit MinimapExporter(RME::core::Map* map);
    ~MinimapExporter() = default;

    // Export methods
    bool exportMap(const QString& filename, const ExportOptions& options = ExportOptions());
    bool exportFloor(const QString& filename, int floor, const ExportOptions& options = ExportOptions());
    bool exportRegion(const QString& filename, const QRect& region, const ExportOptions& options = ExportOptions());
    bool exportSelection(const QString& filename, RME::core::selection::SelectionManager* selectionManager, const ExportOptions& options = ExportOptions());

    // Image generation
    QImage generateMapImage(const ExportOptions& options = ExportOptions());
    QImage generateFloorImage(int floor, const ExportOptions& options = ExportOptions());
    QImage generateRegionImage(const QRect& region, const ExportOptions& options = ExportOptions());

    // Utility methods
    QRect getMapBounds() const;
    QRect getFloorBounds(int floor) const;
    QSize getOutputSize(const QRect& region, int scale = 1) const;

    // Color mapping
    static QColor getTileColor(const RME::core::Tile* tile);
    static QColor getGroundColor(quint16 itemId);
    static QColor getWaterColor();
    static QColor getVoidColor();

private:
    RME::core::Map* m_map;

    // Helper methods
    QImage createBaseImage(const QRect& region, int scale) const;
    void renderTile(QImage& image, const RME::core::Position& pos, const RME::core::Tile* tile, const QRect& region, int scale) const;
    void renderCreatures(QImage& image, const QRect& region, int scale) const;
    void renderSpawns(QImage& image, const QRect& region, int scale) const;
    void renderHouses(QImage& image, const QRect& region, int scale) const;
    void renderWaypoints(QImage& image, const QRect& region, int scale) const;
    
    QRect calculateRegion(const ExportOptions& options) const;
    QString generateFilename(const QString& baseFilename, int floor) const;
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_MINIMAP_EXPORTER_H