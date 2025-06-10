#ifndef RME_TILE_CHANGE_COMMAND_H
#define RME_TILE_CHANGE_COMMAND_H

#include "core/actions/AppUndoCommand.h"
#include "core/Position.h"
#include "core/Tile.h" // For std::unique_ptr<Tile>
#include <memory>      // For std::unique_ptr
#include <QList>       // For getAffectedPositions

// Forward declaration
// namespace RME { namespace core { class Map; class Tile; }} // Already included Tile

namespace RME {
namespace core {
namespace actions {

// Command ID for merging (optional)
// enum { TileChangeCommandId = 1 }; // Example

class TileChangeCommand : public AppUndoCommand {
public:
    // newTileStateData is the state the tile at 'pos' WILL BECOME after this command is redone.
    // The constructor will capture the CURRENT state at 'pos' as the 'old' state.
    TileChangeCommand(RME::core::Map* map,
                      const RME::core::Position& pos,
                      std::unique_ptr<RME::core::Tile> newTileStateData, // Can be nullptr if deleting tile
                      QUndoCommand* parent = nullptr);
    ~TileChangeCommand() override;

    void undo() override;
    void redo() override;

    // int id() const override; // For merging
    // bool mergeWith(const QUndoCommand* other) override; // For merging

    // Optional: For reporting affected area
    QList<RME::core::Position> getAffectedPositions() const override;

private:
    RME::core::Position m_pos;
    std::unique_ptr<RME::core::Tile> m_oldTileStateData; // State of tile at m_pos BEFORE change
    std::unique_ptr<RME::core::Tile> m_newTileStateData; // State of tile at m_pos AFTER change
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_TILE_CHANGE_COMMAND_H
