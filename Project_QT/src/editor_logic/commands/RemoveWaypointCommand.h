#ifndef RME_REMOVEWAYPOINT_COMMAND_H
#define RME_REMOVEWAYPOINT_COMMAND_H

#include "BaseCommand.h"
#include <QString>
#include <memory>
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

constexpr int RemoveWaypointCommandId = toInt(CommandId::RemoveWaypoint);

class RemoveWaypointCommand : public BaseCommand {
public:
    RemoveWaypointCommand(
        RME::core::waypoints::WaypointManager* manager,
        const QString& waypointName,
        QUndoCommand* parent = nullptr
    );
    ~RemoveWaypointCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RemoveWaypointCommandId; }

private:
    RME::core::waypoints::WaypointManager* m_waypointManager;
    QString m_waypointName;
    RME::core::Position m_waypointPosition;
    std::unique_ptr<RME::core::waypoints::Waypoint> m_removedWaypoint;
    bool m_waypointExisted = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_REMOVEWAYPOINT_COMMAND_H