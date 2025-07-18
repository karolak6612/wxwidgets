#ifndef RME_ADDWAYPOINTCOMMAND_H
#define RME_ADDWAYPOINTCOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include "core/Position.h" // RME::core::Position
#include "core/actions/CommandIds.h"
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME { namespace core {
    class WaypointManager;
    struct Waypoint; // The actual Waypoint struct/class
}}

namespace RME {
namespace core {
namespace actions {

constexpr int AddWaypointCommandId = toInt(CommandId::AddWaypoint);

class AddWaypointCommand : public BaseCommand {
public:
    AddWaypointCommand(
        RME::core::WaypointManager* waypointManager,
        const QString& waypointName,
        const RME::core::Position& position,
        RME::core::EditorControllerInterface* editorController,
        QUndoCommand* parent = nullptr
    );
    ~AddWaypointCommand() override = default; // unique_ptr for m_replacedWaypoint handled automatically

    void undo() override;
    void redo() override;

    int id() const override { return AddWaypointCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Merging add likely not needed

private:
    RME::core::WaypointManager* m_waypointManager;
    QString m_waypointName;
    RME::core::Position m_position;

    // To handle cases where an existing waypoint with the same name might be replaced by addWaypoint
    // and needs to be restored on undo.
    std::unique_ptr<RME::core::Waypoint> m_replacedWaypoint;
    bool m_wasReplacement = false;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_ADDWAYPOINTCOMMAND_H
