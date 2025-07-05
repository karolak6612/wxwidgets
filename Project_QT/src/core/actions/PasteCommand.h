#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include <QUndoCommand>
#include "core/clipboard/ClipboardData.h" // For ClipboardContent
#include "core/Position.h"

namespace RME {
class Map;

class PasteCommand : public QUndoCommand {
public:
    PasteCommand(
        Map* map,
        const Position& targetTopLeftPosition, // Top-left where paste begins
        const ClipboardContent& clipboardContent,
        const QString& text = "Paste",
        QUndoCommand* parent = nullptr
    );

    ~PasteCommand() override;

    void undo() override; // Removes the pasted elements
    void redo() override; // Performs the paste operation

private:
    Map* m_map; // Non-owning
    Position m_targetTopLeft;
    ClipboardContent m_pastedContent; // Data that was pasted

    // To support undo, we need to store what was on the tiles *before* pasting,
    // if we are merging. If we are replacing, we need to store the replaced content.
    // This can be complex. A common approach is for redo() to capture the "before" state
    // of the affected area, which undo() then restores.
    // For simplicity in this first pass, undo might just delete what was pasted,
    // assuming no complex merge that needs perfect original state restoration yet.
    // A more robust undo would store the state of all affected tiles before the paste.
    QList<Tile*> m_affectedTilesOriginalState; // Simplified: list of pointers to tiles that were modified.
                                              // A better way is to store QList<DeletedTileData> of what was there before paste.
    QList<Position> m_pastedTilePositions; // Store actual absolute positions where tiles were pasted/affected.
};

} // namespace RME
#endif // PASTECOMMAND_H
