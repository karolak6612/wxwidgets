#ifndef RME_SETHOUSETILECOMMAND_H
#define RME_SETHOUSETILECOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include "core/Position.h"
#include "core/actions/CommandIds.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    namespace editor { class EditorControllerInterface; }
    namespace houses { class Houses; class HouseData; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int SetHouseTileCommandId = toInt(CommandId::SetHouseTile);

class SetHouseTileCommand : public BaseCommand {
public:
    SetHouseTileCommand(
        quint32 houseId,
        RME::core::Tile* tile,
        bool assignToHouse, // true to assign tile to house, false to unassign
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~SetHouseTileCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return SetHouseTileCommandId; }

private:
    quint32 m_houseId; // House ID to assign to
    RME::core::Tile* m_tile;       // Non-owning, for direct interaction
    RME::core::Position m_tilePos; // Store position for notifications, as tile ptr might change if map reallocates

    bool m_assignToHouse; // The action this command performs

    // State before redo()
    quint32 m_originalTileHouseId;
    bool m_originalTilePZStatus;
    bool m_wasTileInThisHouseListBeforeRedo; // Was m_tilePos in m_house->getTilePositions() before redo?
                                          // (Not critically needed if undo logic is self-contained based on m_assignToHouse)
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_SETHOUSETILECOMMAND_H
