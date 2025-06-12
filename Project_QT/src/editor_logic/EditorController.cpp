#include "EditorController.h"
// Required full includes for implementation details (will be needed for command creation)
#include "core/map/Map.h"
#include "core/selection/SelectionManager.h"
#include "core/brush/BrushManagerService.h"
#include "core/brush/Brush.h"
#include "core/brush/BrushSettings.h"
#include "core/Position.h"
#include "core/waypoints/WaypointManager.h" // Added
#include "core/waypoints/Waypoint.h"       // Added
#include <QUndoStack>
#include "commands/BrushStrokeCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/AddWaypointCommand.h"   // Added
#include "commands/MoveWaypointCommand.h"  // Added
#include <QDebug> // For qWarning (optional)

namespace RME {

EditorController::EditorController(
    Map* map,
    QUndoStack* undoStack,
    SelectionManager* selectionManager,
    BrushManagerService* brushManagerService,
    RME::core::WaypointManager* waypointManager, // Added
    QObject* parent
) : QObject(parent),
    m_map(map),
    m_undoStack(undoStack),
    m_selectionManager(selectionManager),
    m_brushManagerService(brushManagerService),
    m_waypointManager(waypointManager) { // Added
    // TODO: Assert that pointers are not null
    Q_ASSERT(m_map);
    Q_ASSERT(m_undoStack);
    Q_ASSERT(m_selectionManager);
    Q_ASSERT(m_brushManagerService);
    Q_ASSERT(m_waypointManager);
}

EditorController::~EditorController() {
    // Destructor
}

void EditorController::applyBrushStroke(const QList<Position>& positions, const BrushSettings& settings, bool isEraseOperation) {
    if (!m_map || !m_undoStack || !m_brushManagerService || positions.isEmpty()) {
        if (positions.isEmpty()) {
             qWarning("EditorController::applyBrushStroke: Attempted to apply brush stroke with empty positions list.");
        }
        return;
    }

    // Assuming BrushSettings contains the name of the brush to be used.
    // This name is then used to retrieve the Brush instance from BrushManagerService.
    // This interaction detail depends on how BrushManagerService and BrushSettings are designed.
    Brush* currentBrush = m_brushManagerService->getBrush(settings.getName());

    if (!currentBrush) {
        qWarning("EditorController::applyBrushStroke: Could not find brush '%s'", qPrintable(settings.getName()));
        return;
    }

    m_undoStack->push(new RME_COMMANDS::BrushStrokeCommand(m_map, currentBrush, positions, settings, isEraseOperation));
}

void EditorController::deleteSelection() {
    if (!m_map || !m_undoStack || !m_selectionManager) {
        qWarning("EditorController::deleteSelection: Core component is null.");
        return;
    }
    // Check if selection is empty to avoid pushing an empty command
    if (m_selectionManager->getSelectedTiles().isEmpty()) {
        // Optionally provide user feedback e.g. via status bar
        // qInfo("EditorController::deleteSelection: Selection is empty, nothing to delete.");
        return;
    }
    m_undoStack->push(new RME_COMMANDS::DeleteCommand(m_map, m_selectionManager));
}

void EditorController::placeOrMoveWaypoint(const QString& name, const RME::core::Position& targetPos) {
    if (!m_map || !m_undoStack || !m_waypointManager) { // Check m_waypointManager
        qWarning("EditorController::placeOrMoveWaypoint: Core component (map, undoStack, or waypointManager) is null.");
        return;
    }
    if (name.isEmpty()) {
        qWarning("EditorController::placeOrMoveWaypoint: Waypoint name cannot be empty.");
        return;
    }
    // Assuming Position::isValid() checks for reasonable map coordinates, not just non-default.
    // Map::isPositionValid might be more appropriate if it checks against map boundaries.
    if (!m_map->isPositionValid(targetPos)) {
        qWarning("EditorController::placeOrMoveWaypoint: Target position %d,%d,%d is invalid for the current map.",
                 targetPos.x, targetPos.y, targetPos.z);
        return;
    }

    RME::core::Waypoint* existingWp = m_waypointManager->getWaypoint(name);

    if (existingWp) { // Waypoint with this name exists, so it's a move operation
        if (existingWp->position == targetPos) {
            // qInfo("EditorController::placeOrMoveWaypoint: Target position is same as current for waypoint '%s'. No action.", qPrintable(name));
            return; // No change needed
        }
        m_undoStack->push(new RME_COMMANDS::MoveWaypointCommand(
            m_waypointManager,
            name,
            existingWp->position,
            targetPos
        ));
    } else { // Waypoint with this name does not exist, so it's an add operation
        m_undoStack->push(new RME_COMMANDS::AddWaypointCommand(
            m_waypointManager,
            name,
            targetPos
        ));
    }
}

} // namespace RME
