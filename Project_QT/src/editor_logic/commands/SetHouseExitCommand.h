#ifndef RME_SETHOUSEEXITCOMMAND_H
#define RME_SETHOUSEEXITCOMMAND_H

#include <QUndoCommand>
#include <QString>
#include "core/Position.h"

// Forward declarations
namespace RME {
namespace core {
    class Map; // For notifications via controller
    namespace editor { class EditorControllerInterface; }
    namespace houses { class House; }
}
}

namespace RME_COMMANDS {

const int SetHouseExitCommandId = 1015; // Choose a unique ID

class SetHouseExitCommand : public QUndoCommand {
public:
    SetHouseExitCommand(
        RME::core::houses::House* house,
        const RME::core::Position& newExitPos, // New exit. Invalid position means clear exit.
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~SetHouseExitCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return SetHouseExitCommandId; }

    // For testing
    const RME::core::houses::House* getHouse() const { return m_house; }
    const RME::core::Position& getNewExitPosition() const { return m_newExitPos; }
    const RME::core::Position& getOldExitPosition() const { return m_oldExitPos; }

private:
    RME::core::houses::House* m_house; // Non-owning
    RME::core::editor::EditorControllerInterface* m_controller;

    RME::core::Position m_newExitPos; // Target exit position for redo()
    RME::core::Position m_oldExitPos; // Original exit position, for undo()
};

} // namespace RME_COMMANDS
#endif // RME_SETHOUSEEXITCOMMAND_H
