#ifndef RME_BRUSHSTROKECOMMAND_H
#define RME_BRUSHSTROKECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QSet> // For QSet<RME::Position>
#include <memory> // For std::unique_ptr
#include "core/Position.h"
#include "core/brush/BrushSettings.h" // Assuming RME::BrushSettings
// Forward declarations
namespace RME {
    class Map;
    class Brush;
    class Tile;
}

namespace RME_COMMANDS { // Or RME::Commands, or just RME

const int BrushStrokeCommandId = 1001; // Unique ID for this command type

class BrushStrokeCommand : public QUndoCommand {
public:
    BrushStrokeCommand(
        RME::Map* map,
        RME::Brush* brush,
        const QList<RME::Position>& positions,
        const RME::BrushSettings& settings,
        bool isErase,
        QUndoCommand* parent = nullptr
    );
    ~BrushStrokeCommand() override;

    void undo() override;
    void redo() override;

    int id() const override { return BrushStrokeCommandId; }
    // Basic merge: only merge identical consecutive brush strokes (same brush, same settings, erase flag)
    // More advanced merging could consider proximity of positions.
    bool mergeWith(const QUndoCommand* command) override;

private:
    RME::Map* m_map;
    RME::Brush* m_brush; // Non-owning, managed by BrushManagerService
    QList<RME::Position> m_positions;
    RME::BrushSettings m_settings;
    bool m_isErase;

    // Stores the state of tiles *before* redo() modified them.
    // Key: Position, Value: unique_ptr to the original Tile (or nullptr if tile was created)
    QMap<RME::Position, std::unique_ptr<RME::Tile>> m_originalTiles;
    // Stores whether a tile was newly created by this command's redo()
    QSet<RME::Position> m_createdTiles;
};

} // namespace RME_COMMANDS
#endif // RME_BRUSHSTROKECOMMAND_H
