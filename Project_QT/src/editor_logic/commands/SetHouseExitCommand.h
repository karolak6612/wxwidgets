#ifndef RME_SETHOUSEEXITCOMMAND_H
#define RME_SETHOUSEEXITCOMMAND_H

#include <QUndoCommand>
#include <QString>
#include "core/Position.h" // RME::core::Position
#include <cstdint>        // For uint32_t

// Forward declarations
namespace RME { // RME::Map is directly in RME namespace
    class Map;
}
// Forward declare RME::core::Position if not fully included by "core/Position.h" above (it is)
// namespace RME { namespace core { class Position; }}


namespace RME_COMMANDS {

const int SetHouseExitCommandId = 1005; // Unique ID for this command type

class SetHouseExitCommand : public QUndoCommand {
public:
    SetHouseExitCommand(
        RME::Map* map,
        uint32_t houseId,
        const RME::core::Position& oldExitPos, // Position of the exit *before* this command
        const RME::core::Position& newExitPos, // New position for the exit
        QUndoCommand* parent = nullptr
    );
    ~SetHouseExitCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return SetHouseExitCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Optional for merging consecutive exit sets

private:
    RME::Map* m_map;
    uint32_t m_houseId;
    RME::core::Position m_oldExitPos; // Stored for undo
    RME::core::Position m_newExitPos; // Stored for redo
};

} // namespace RME_COMMANDS
#endif // RME_SETHOUSEEXITCOMMAND_H
