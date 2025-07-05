#ifndef RME_RECORDSETGROUNDCOMMAND_H
#define RME_RECORDSETGROUNDCOMMAND_H

#include <QUndoCommand>
#include <memory> // For std::unique_ptr
#include <QString>  // For command text

#include "core/Position.h" // For RME::core::Position

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

namespace RME_COMMANDS { // Consistent namespace

// Choose a unique ID, assuming RecordSetSpawnCommandId was 1006
const int RecordSetGroundCommandId = 1007;

class RecordSetGroundCommand : public QUndoCommand {
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
    RME::core::editor::EditorControllerInterface* m_controller;
    RME::core::Position m_tilePosition;
    QString m_commandTextBase;
};

} // namespace RME_COMMANDS
#endif // RME_RECORDSETGROUNDCOMMAND_H
