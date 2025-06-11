#ifndef SELECTIONCOMMAND_H
#define SELECTIONCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "SelectionManager.h" // For SelectionManager::SelectionChange (consider moving SelectionChange to its own header or forward declaring if it becomes an issue)
                             // For now, including SelectionManager.h is acceptable for accessing SelectionChange.

// Forward declarations
class Map; // Assuming Map class is needed, e.g. for context or validation, though not directly used in this command's core logic
namespace RME { // Ensure this is the correct namespace

class SelectionCommand : public QUndoCommand {
public:
    // Constructor takes the list of changes from SelectionManager
    SelectionCommand(
        SelectionManager* selectionManager, // To update m_selectedTiles
        Map* map, // Context, if needed by map elements, though primarily changes are on elements themselves
        const QList<SelectionManager::SelectionChange>& changes,
        const QString& text, // Undo text
        QUndoCommand* parent = nullptr
    );

    ~SelectionCommand() override;

    void undo() override;
    void redo() override;

private:
    void applyChanges(bool applyCurrentState); // Helper to apply either previous or current state

    SelectionManager* m_selectionManager; // Non-owning, to update its internal state (m_selectedTiles)
    Map* m_map;                           // Non-owning, context if needed
    QList<SelectionManager::SelectionChange> m_changes; // Store a copy of the changes

    // To correctly manage m_selectedTiles in SelectionManager, we might need to know
    // the state of m_selectedTiles before and after this command.
    // However, a simpler approach is to have redo/undo in SelectionCommand
    // also call public methods on SelectionManager to update its m_selectedTiles if necessary,
    // or m_selectedTiles is purely derived from Tile::hasSelectedElements() after changes.
    // For now, SelectionCommand will focus on applying setSelected to elements.
    // SelectionManager's m_selectedTiles update will be refined if needed.
};

} // namespace RME
#endif // SELECTIONCOMMAND_H
