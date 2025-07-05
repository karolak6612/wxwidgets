#ifndef RME_BRUSHSTROKECOMMAND_H
#define RME_BRUSHSTROKECOMMAND_H

#include "BaseCommand.h"
#include <QList>
#include <QMap>
#include <QSet> // For QSet<RME::Position>
#include <memory> // For std::unique_ptr
#include "core/Position.h"
#include "core/brush/BrushSettings.h" // Assuming RME::BrushSettings
#include "core/actions/CommandIds.h"
// Forward declarations
namespace RME {
namespace core {
    namespace map { class Map; }
    class Brush;
    class Tile;
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int BrushStrokeCommandId = toInt(CommandId::BrushStroke);

class BrushStrokeCommand : public BaseCommand {
public:
    BrushStrokeCommand(
        RME::core::map::Map* map,
        RME::core::Brush* brush,
        const QList<RME::core::Position>& positions,
        const RME::core::BrushSettings& settings,
        bool isErase,
        RME::core::editor::EditorControllerInterface* controller,
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
    RME::core::map::Map* m_map;
    RME::core::Brush* m_brush; // Non-owning, managed by BrushManagerService
    QList<RME::core::Position> m_positions;
    RME::core::BrushSettings m_settings;
    bool m_isErase;

    // Stores the state of tiles *before* redo() modified them.
    // Key: Position, Value: unique_ptr to the original Tile (or nullptr if tile was created)
    QMap<RME::core::Position, std::unique_ptr<RME::core::Tile>> m_originalTiles;
    // Stores whether a tile was newly created by this command's redo()
    QSet<RME::core::Position> m_createdTiles;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_BRUSHSTROKECOMMAND_H
