#include "batchcommand.h"
#include "map.h" // For AppUndoCommand constructor if BatchCommand derives from it

// Constructor takes ownership of the commands.
BatchCommand::BatchCommand(Map* map, QList<QUndoCommand*> commands, const QString& text, QUndoCommand *parent)
    : AppUndoCommand(map, parent), m_commands(commands) // Pass map to AppUndoCommand constructor
{
    if (!text.isEmpty()) {
        setText(text);
    } else {
        // Try to derive text from child commands if possible, or set a generic one.
        if (!m_commands.isEmpty() && m_commands.first()) {
            setText(QString("Grouped: %1...").arg(m_commands.first()->text()));
        } else {
            setText("Grouped Action");
        }
    }
}

BatchCommand::~BatchCommand()
{
    // Delete all commands in the list since BatchCommand takes ownership.
    qDeleteAll(m_commands);
    m_commands.clear();
}

void BatchCommand::undo()
{
    // For undo, commands are undone in reverse order of execution.
    for (int i = m_commands.size() - 1; i >= 0; --i) {
        if (m_commands.at(i)) {
            m_commands.at(i)->undo();
        }
    }
}

void BatchCommand::redo()
{
    // For redo, commands are redone in their original order.
    for (QUndoCommand *command : m_commands) {
        if (command) {
            command->redo();
        }
    }
}

// Optional mergeWith implementation for BatchCommand
/*
bool BatchCommand::mergeWith(const QUndoCommand *other)
{
    const BatchCommand *otherBatch = dynamic_cast<const BatchCommand *>(other);
    if (!otherBatch)
        return false;

    // Example: Only merge if this batch is not too large yet
    if (m_commands.size() > 10) // Arbitrary limit
        return false;

    // Add all commands from the other batch to this one.
    // The other batch will be destroyed by the QUndoStack if merge returns true,
    // so we need to be careful with ownership if commands were not heap-allocated
    // or if QUndoStack doesn't manage them after merge.
    // Assuming QUndoCommands are always heap allocated and QUndoStack manages them.

    // Steal commands from otherBatch.
    // This is tricky because otherBatch->m_commands will be cleared by its destructor.
    // A better approach for QUndoCommand's mergeWith is that 'other' is usually destroyed.
    // So we'd typically create new commands or re-parent.
    // However, if BatchCommand owns its QUndoCommands, we can take them.

    // Let's assume for simplicity in this example, we just copy pointers,
    // and the `other` BatchCommand is responsible for not deleting them if merge is successful.
    // QUndoStack usually deletes the 'other' command if mergeWith returns true.
    // So, if 'other' is a BatchCommand, its destructor would delete its children.
    // This means we must take ownership properly.

    // A common pattern for mergeWith is that 'this' command modifies itself to include 'other'.
    // 'other' is then obsolete.

    for (QUndoCommand* cmd : otherBatch->m_commands) {
        // We need to take ownership. If otherBatch->m_commands were std::vector<std::unique_ptr<QUndoCommand>>,
        // we could std::move them. With QList<QUndoCommand*>, and if otherBatch will delete them,
        // this is unsafe unless otherBatch can be told not to delete them.
        // For now, let's assume QUndoStack handles the 'other' command's lifetime.
        // If 'other' is deleted, its m_commands are deleted.
        // So, we cannot just copy pointers if 'other' is a BatchCommand that cleans up its children.
        // A safer approach for simple BatchCommand merge:
        // If the other command is also a BatchCommand, append its children to ours.
        // The QUndoStack will delete the 'other' command. If 'other' is a BatchCommand,
        // its destructor will delete its children. This is problematic.

        // The most robust way for QUndoCommand::mergeWith is that 'this' command becomes
        // a new command that encompasses 'this' and 'other'.
        // Or, if 'other' is a simple command, 'this' (BatchCommand) adds it.

        // For this example, let's say we are merging another BatchCommand's children into this one.
        // The `other` BatchCommand would need a way to release ownership of its commands.
        // This is getting complex and depends on specific ownership model.

        // Simpler: If `other` is NOT a BatchCommand, add it to our list.
        // If `other` IS a BatchCommand, append its children (this requires `otherBatch` to expose its children or a way to take them).

        // For now, let's keep it simple: BatchCommands don't merge with other BatchCommands by default,
        // but they could merge with simple commands if it makes sense.
        // This example won't implement a complex merge.
        // If merging is important, usually one would check command IDs and specific conditions.
    }
    // setText(new combined text);
    return false; // Placeholder
}
*/
