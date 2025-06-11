#ifndef BATCHCOMMAND_H
#define BATCHCOMMAND_H

#include "appundocommand.h" // Using AppUndoCommand for consistency if specific features are needed
#include <QList>
#include <QUndoCommand> // Required for QUndoCommand list

class BatchCommand : public AppUndoCommand // Or directly QUndoCommand if no AppUndoCommand specific features are needed by BatchCommand itself
{
public:
    // Takes ownership of the commands in the list.
    explicit BatchCommand(Map* map, QList<QUndoCommand*> commands, const QString& text = "", QUndoCommand *parent = nullptr);
    ~BatchCommand() override;

    void undo() override;
    void redo() override;

    // A batch command could represent a macro. QUndoStack has built-in macro support.
    // This custom BatchCommand could be used if more specific control or merging is needed.
    // int id() const override { return QUndoCommand::id() + 1; } // Example for macros

    // Optional: Implement mergeWith if batches themselves can be merged.
    // bool mergeWith(const QUndoCommand *other) override;

private:
    QList<QUndoCommand*> m_commands; // Owns these commands
};

#endif // BATCHCOMMAND_H
