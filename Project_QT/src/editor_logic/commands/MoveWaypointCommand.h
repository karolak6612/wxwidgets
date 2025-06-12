#ifndef RME_MOVEWAYPOINTCOMMAND_H
#define RME_MOVEWAYPOINTCOMMAND_H

#include <QUndoCommand>
#include <QString>
#include "core/Position.h" // RME::core::Position

// Forward declarations
namespace RME { namespace core {
    class WaypointManager;
}}

namespace RME_COMMANDS {

const int MoveWaypointCommandId = 1004; // Unique ID for this command type

class MoveWaypointCommand : public QUndoCommand {
public:
    MoveWaypointCommand(
        RME::core::WaypointManager* waypointManager,
        const QString& waypointName,
        const RME::core::Position& oldPosition,
        const RME::core::Position& newPosition,
        QUndoCommand* parent = nullptr
    );
    ~MoveWaypointCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return MoveWaypointCommandId; }
    bool mergeWith(const QUndoCommand* command) override;

private:
    RME::core::WaypointManager* m_waypointManager;
    QString m_waypointName;
    RME::core::Position m_oldPosition;
    RME::core::Position m_newPosition; // This is the target position for this command's redo
                                     // If merged, this becomes the final new position of the sequence.
};

} // namespace RME_COMMANDS
#endif // RME_MOVEWAYPOINTCOMMAND_H
