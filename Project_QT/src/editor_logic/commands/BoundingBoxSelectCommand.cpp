#include "editor_logic/commands/BoundingBoxSelectCommand.h"
#include "core/selection/SelectionManager.h"
#include "core/Tile.h" // For RME::core::Tile pointer usage

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT
#include <QSet>    // For efficient union of tile lists

namespace RME {
namespace core {
namespace actions {

BoundingBoxSelectCommand::BoundingBoxSelectCommand(
    RME::SelectionManager* selectionManager, // Corrected namespace
    const QList<RME::core::Tile*>& calculatedTilesInBox,
    bool isAdditive,
    const QList<RME::core::Tile*>& selectionStateBeforeThisCommand,
    QUndoCommand* parent
) : RME::editor_logic::commands::BaseCommand(nullptr, QString(), parent), // Corrected BaseCommand namespace
    m_selectionManager(selectionManager),
    m_calculatedTilesInBox(calculatedTilesInBox),
    m_isAdditive(isAdditive),
    m_selectionStateBeforeCommand(selectionStateBeforeThisCommand),
    m_firstRun(true)
{
    if (!m_selectionManager) {
        qWarning("BoundingBoxSelectCommand: Initialization with null selection manager.");
        setErrorText("Bounding Box Selection");
        return;
    }
    // Command text will be set in redo()
}

void BoundingBoxSelectCommand::redo() {
    if (m_firstRun) {
        // Calculate the final selection state this command will apply
        if (m_isAdditive) {
            // Combine selectionBefore with calculatedTilesInBox, ensuring uniqueness
            QSet<RME::core::Tile*> combinedSelectionSet = QSet<RME::core::Tile*>::fromList(m_selectionStateBeforeCommand);
            for (RME::core::Tile* tile : m_calculatedTilesInBox) {
                if (tile) combinedSelectionSet.insert(tile);
            }
            // Convert QSet back to QList. Note: QSet iteration order is undefined.
            // For consistency in m_selectionStateAfterCommand if order matters to other parts,
            // consider sorting if necessary, though SelectionManager uses QSet internally for m_selectedTiles.
            m_selectionStateAfterCommand.clear();
            for(RME::core::Tile* tile : combinedSelectionSet) {
                m_selectionStateAfterCommand.append(tile);
            }
            // Optional: sort m_selectionStateAfterCommand if a consistent order is beneficial.
            // std::sort(m_selectionStateAfterCommand.begin(), m_selectionStateAfterCommand.end(), ...);

        } else {
            // Replace current selection with only the tiles in the box
            m_selectionStateAfterCommand = m_calculatedTilesInBox;
        }
        m_firstRun = false;
    }

    // Apply the calculated "after" state
    m_selectionManager->setSelectedTilesInternal(m_selectionStateAfterCommand);

    // Set command text based on what happened
    if (m_isAdditive) {
        int addedCount = 0;
        QSet<RME::core::Tile*> beforeSet = QSet<RME::core::Tile*>::fromList(m_selectionStateBeforeCommand);
        for (RME::core::Tile* tile : m_selectionStateAfterCommand) {
            if (!beforeSet.contains(tile)) {
                addedCount++;
            }
        }
        if (addedCount > 0) {
            setText(QObject::tr("Add %1 tile(s) to selection via bounding box").arg(addedCount));
        } else {
             // This can happen if all calculatedTilesInBox were already in selectionStateBeforeCommand
            setText(QObject::tr("Bounding Box Selection (no change)"));
        }
    } else {
        setText(QObject::tr("Select %1 tile(s) via bounding box").arg(m_selectionStateAfterCommand.size()));
    }
}

void BoundingBoxSelectCommand::undo() {
    // Restore the selection state that existed before this command ran
    m_selectionManager->setSelectedTilesInternal(m_selectionStateBeforeCommand);
    // Text for undo is usually set by QUndoStack or by re-calling setText if needed.
    // For clarity, we can set it here too.
    setText(QObject::tr("Undo Bounding Box Selection"));
}

} // namespace actions
} // namespace core
} // namespace RME
