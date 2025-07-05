#ifndef RME_MOVEWAYPOINTCOMMAND_H
#define RME_MOVEWAYPOINTCOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include "core/Position.h" // RME::core::Position
#include "core/actions/CommandIds.h"

// Forward declarations
namespace RME { namespace core {
    class WaypointManager;
    namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {
namespace actions {

constexpr int MoveWaypointCommandId = toInt(CommandId::MoveWaypoint);

class MoveWaypointCommand : public BaseCommand {
public:
    MoveWaypointCommand(
        RME::core::WaypointManager* waypointManager,
        const QString& waypointName,
        const RME::core::Position& oldPosition,
        const RME::core::Position& newPosition,
        RME::core::EditorControllerInterface* controller,
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

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_MOVEWAYPOINTCOMMAND_H
