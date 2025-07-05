#ifndef RME_RECORDSETGROUNDCOMMAND_H
#define RME_RECORDSETGROUNDCOMMAND_H

#include "BaseCommand.h"
#include <memory> // For std::unique_ptr
#include <QString>  // For command text

#include "core/Position.h" // For RME::core::Position
#include "core/actions/CommandIds.h"

// Forward declare RME::core::Item and RME::core::Tile
namespace RME {
namespace core {
    class Item;
    class Tile;
    namespace editor { // Forward declare EditorControllerInterface
        class EditorControllerInterface;
    }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RecordSetGroundCommandId = toInt(CommandId::RecordSetGround);

class RecordSetGroundCommand : public BaseCommand {
public:
    RecordSetGroundCommand(
        RME::core::Tile* tile,
        std::unique_ptr<RME::core::Item> newGround, // The state *after* the change
        std::unique_ptr<RME::core::Item> oldGround, // The state *before* the change
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );
    ~RecordSetGroundCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RecordSetGroundCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Optional, can be added later

private:
    RME::core::Tile* m_tile;
    std::unique_ptr<RME::core::Item> m_groundStateForRedo; // Stores the state to apply on redo
    std::unique_ptr<RME::core::Item> m_groundStateForUndo; // Stores the state to apply on undo
    RME::core::Position m_tilePosition;
    QString m_commandTextBase;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_RECORDSETGROUNDCOMMAND_H
