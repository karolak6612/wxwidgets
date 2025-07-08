#ifndef RME_ADDWAYPOINTCOMMAND_H
#define RME_ADDWAYPOINTCOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include "core/Position.h" // RME::core::Position
#include "core/actions/CommandIds.h"
#include <memory> // For std::unique_ptr

// Forward declarations
namespace RME { namespace core { namespace waypoints { // Added ::waypoints
    class WaypointManager;
    struct Waypoint; // The actual Waypoint struct/class
}}}

namespace RME {
namespace core {
namespace actions {

constexpr int AddWaypointCommandId = toInt(CommandId::AddWaypoint);

class AddWaypointCommand : public RME::editor_logic::commands::BaseCommand { // Added full namespace
public:
    AddWaypointCommand(
        RME::core::waypoints::WaypointManager* waypointManager, // Corrected namespace
        const QString& waypointName,
        const RME::Position& position, // Corrected namespace
        RME::core::editor::EditorControllerInterface* editorController, // Corrected namespace for consistency from previous fixes
        QUndoCommand* parent = nullptr
    );
    ~AddWaypointCommand() override = default; // unique_ptr for m_replacedWaypoint handled automatically

    void undo() override;
    void redo() override;

    int id() const override { return AddWaypointCommandId; }
    // bool mergeWith(const QUndoCommand* command) override; // Merging add likely not needed

private:
    RME::core::waypoints::WaypointManager* m_waypointManager; // Corrected namespace
    QString m_waypointName;
    RME::Position m_position; // Corrected namespace

    // To handle cases where an existing waypoint with the same name might be replaced by addWaypoint
    // and needs to be restored on undo.
    std::unique_ptr<RME::core::waypoints::Waypoint> m_replacedWaypoint; // Corrected namespace
    bool m_wasReplacement = false;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_ADDWAYPOINTCOMMAND_H
