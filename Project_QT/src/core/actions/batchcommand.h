#ifndef BATCHCOMMAND_H
#define BATCHCOMMAND_H

#include "actions/appundocommand.h" // Using AppUndoCommand for consistency if specific features are needed
#include <QList>
#include <QUndoCommand> // Required for QUndoCommand list

/**
 * @brief Command for grouping multiple QUndoCommands into a single undo/redo operation.
 *
 * BatchCommand takes a list of other QUndoCommand instances and treats them
 * as a single atomic operation. Undoing a BatchCommand will undo all its
 * child commands in reverse order, and redoing it will redo them in their
 * original order. This command takes ownership of the child commands.
 */
class BatchCommand : public AppUndoCommand // Or directly QUndoCommand if no AppUndoCommand specific features are needed by BatchCommand itself
{
public:
    /**
     * @brief Constructs a BatchCommand.
     * @param map Pointer to the Map object. Passed to AppUndoCommand base.
     * @param commands A QList of QUndoCommand pointers to be grouped. BatchCommand takes ownership of these commands.
     * @param text Optional descriptive text for this batch operation. If empty, a generic text is generated.
     * @param parent Optional parent QUndoCommand.
     */
    explicit BatchCommand(Map* map, QList<QUndoCommand*> commands, const QString& text = "", QUndoCommand *parent = nullptr);

    /**
     * @brief Destructor. Deletes all child commands owned by this BatchCommand.
     */
    ~BatchCommand() override;

    /**
     * @brief Undoes all child commands in reverse order of their initial execution.
     */
    void undo() override;

    /**
     * @brief Redoes all child commands in their original execution order.
     */
    void redo() override;

    // A batch command could represent a macro. QUndoStack has built-in macro support.
    // This custom BatchCommand could be used if more specific control or merging is needed.
    // int id() const override { return QUndoCommand::id() + 1; } // Example for macros

    // Optional: Implement mergeWith if batches themselves can be merged.
    // bool mergeWith(const QUndoCommand *other) override;

private:
    QList<QUndoCommand*> m_commands; ///< List of child commands. BatchCommand owns these.
};

#endif // BATCHCOMMAND_H
