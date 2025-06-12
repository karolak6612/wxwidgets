#include "EditorController.h"
// Required full includes for implementation details (will be needed for command creation)
#include "core/map/Map.h"
#include "core/selection/SelectionManager.h"
#include "core/brush/BrushManagerService.h"
#include "core/brush/Brush.h"
#include "core/brush/BrushSettings.h"
#include "core/Position.h"
#include <QUndoStack>
#include "commands/BrushStrokeCommand.h"
#include "commands/DeleteCommand.h"      // Added DeleteCommand
#include <QDebug> // For qWarning (optional)

namespace RME {

EditorController::EditorController(
    Map* map,
    QUndoStack* undoStack,
    SelectionManager* selectionManager,
    BrushManagerService* brushManagerService,
    QObject* parent
) : QObject(parent),
    m_map(map),
    m_undoStack(undoStack),
    m_selectionManager(selectionManager),
    m_brushManagerService(brushManagerService) {
    // TODO: Assert that pointers are not null
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

} // namespace RME
