#include "editor_logic/commands/ClearSelectionCommand.h"
#include "core/selection/SelectionManager.h"
#include "core/Tile.h" // For RME::core::Tile pointer usage, though not strictly needed for method calls if QList is opaque

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT

namespace RME {
namespace core {
namespace actions {

ClearSelectionCommand::ClearSelectionCommand(
    RME::core::selection::SelectionManager* selectionManager,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_selectionManager(selectionManager),
    m_hadSelectionToClear(false)
{
    Q_ASSERT(m_selectionManager);
    // Text will be set in redo()
}

void ClearSelectionCommand::redo() {
    // Capture the state *before* this command makes changes
    // This is crucial if redo() is the first action that performs the change.
    if (m_oldSelectedTiles.isEmpty() && !m_selectionManager->getCurrentSelectedTilesList().isEmpty()) {
        // If getSelectedTiles() from SelectionManager (from previous turn) returns QSet<Tile*>:
        // m_oldSelectedTiles = QList<RME::core::Tile*>(m_selectionManager->getSelectedTiles().begin(), m_selectionManager->getSelectedTiles().end());
        // Using the new method:
        m_oldSelectedTiles = m_selectionManager->getCurrentSelectedTilesList();
    }

    m_hadSelectionToClear = !m_oldSelectedTiles.isEmpty();

    if (m_hadSelectionToClear || m_selectionManager->isSelectionChangeActive()) {
        // If selection change is active (e.g. from bounding box start),
        // this clear is part of that, so it should proceed even if current selection seems empty to this command's snapshot.
        // However, the plan is EditorController calls this *before* starting bounding box if Ctrl not held.
        // So, isSelectionChangeActive() check might not be relevant here.
    }

    // Perform the action: clear the selection
    // Check live state before clearing, in case another command already cleared it (e.g. if this command was merged or delayed)
    if (!m_selectionManager->getCurrentSelectedTilesList().isEmpty()) {
         m_selectionManager->clearSelectionInternal();
         setText(QObject::tr("Clear Selection (%1 tiles)").arg(m_oldSelectedTiles.size()));
    } else {
         // If m_oldSelectedTiles was populated, but current selection is already empty,
         // it means something else cleared it. This command effectively does nothing then,
         // but it should reflect that it intended to clear what *was* there.
         if (m_hadSelectionToClear) {
             setText(QObject::tr("Clear Selection (already cleared - %1 tiles were selected)").arg(m_oldSelectedTiles.size()));
         } else {
             setText(QObject::tr("Clear Selection (nothing selected)"));
         }
    }
}

void ClearSelectionCommand::undo() {
    if (!m_hadSelectionToClear) {
        // If nothing was cleared by redo(), then undo does nothing.
        setText(QObject::tr("Undo Clear Selection (nothing to restore)"));
        return;
    }
    // Restore the previous selection state
    m_selectionManager->setSelectedTilesInternal(m_oldSelectedTiles);
    setText(QObject::tr("Undo Clear Selection (restored %1 tiles)").arg(m_oldSelectedTiles.size()));
}


} // namespace actions
} // namespace core
} // namespace RME
