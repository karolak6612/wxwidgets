#ifndef RME_RENAMEWAYPOINT_COMMAND_H
#define RME_RENAMEWAYPOINT_COMMAND_H

#include <QUndoCommand>
#include <QString>
#include "core/Position.h"
#include "core/actions/CommandIds.h"

// Forward declarations
namespace RME {
namespace core {
namespace waypoints {
    class WaypointManager;
    class Waypoint;
}
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RenameWaypointCommandId = toInt(CommandId::RenameWaypoint);

class RenameWaypointCommand : public QUndoCommand {
public:
    RenameWaypointCommand(
        RME::core::waypoints::WaypointManager* manager,
        const QString& oldName,
        const QString& newName,
        QUndoCommand* parent = nullptr
    );
    ~RenameWaypointCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RenameWaypointCommandId; }

private:
    RME::core::waypoints::WaypointManager* m_waypointManager;
    QString m_oldName;
    QString m_newName;
    RME::core::Position m_waypointPosition;
    bool m_oldWaypointExisted = false;
    bool m_newNameConflicted = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_RENAMEWAYPOINT_COMMAND_H