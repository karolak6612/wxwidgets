#ifndef RME_SETHOUSETILECOMMAND_H
#define RME_SETHOUSETILECOMMAND_H

#include <QUndoCommand>
#include <QString>
#include "core/Position.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    namespace editor { class EditorControllerInterface; }
    namespace houses { class House; }
}
}

namespace RME {
namespace editor_logic {
namespace commands {

const int SetHouseTileCommandId = 1014; // Choose a unique ID

class SetHouseTileCommand : public QUndoCommand {
public:
    SetHouseTileCommand(
        RME::core::houses::House* house,
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
    RME::core::houses::House* m_house; // Non-owning
    RME::core::Tile* m_tile;       // Non-owning, for direct interaction
    RME::core::Position m_tilePos; // Store position for notifications, as tile ptr might change if map reallocates
    RME::core::editor::EditorControllerInterface* m_controller;

    bool m_assignToHouse; // The action this command performs

    // State before redo()
    quint32 m_originalTileHouseId;
    bool m_originalTilePZStatus;
    bool m_wasTileInThisHouseListBeforeRedo; // Was m_tilePos in m_house->getTilePositions() before redo?
                                          // (Not critically needed if undo logic is self-contained based on m_assignToHouse)
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_SETHOUSETILECOMMAND_H
