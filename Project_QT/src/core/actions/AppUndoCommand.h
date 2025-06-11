#ifndef RME_APP_UNDO_COMMAND_H
#define RME_APP_UNDO_COMMAND_H

#include <QUndoCommand> // From QtWidgets module
#include <QList>        // For QList<Position> for signals
#include "core/Position.h" // For RME::core::Position

// Forward declaration
namespace RME {
namespace core {
    class Map; // Forward declare Map
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Base class for all undoable commands in the application.
 *
 * AppUndoCommand extends QUndoCommand to provide a common interface for commands
 * that operate on the RME::core::Map or other application components.
 * It includes functionality for tracking affected map positions and potentially
 * merging commands.
 */
class AppUndoCommand : public QUndoCommand {
    // Q_OBJECT // Not strictly necessary for QUndoCommand unless it has its own signals/slots
public:
    /**
     * @brief Constructs an AppUndoCommand.
     * @param map Pointer to the map instance the command will operate on.
     * @param parent Parent command, if any.
     */
    explicit AppUndoCommand(RME::core::Map* map, QUndoCommand* parent = nullptr);
    /**
     * @brief Destroys the AppUndoCommand.
     */
    ~AppUndoCommand() override;

    /**
     * @brief Returns a unique ID for the command type.
     *
     * This ID is used by QUndoStack to determine if commands can be merged.
     * Commands with the same non-negative ID and that are consecutive in the stack
     * can be merged if mergeWith() returns true.
     * @return int The command ID. Default is -1 (no merging).
     */
    int id() const override;
    /**
     * @brief Attempts to merge this command with a subsequent command.
     *
     * This method is called by QUndoStack when a new command is pushed and has the
     * same ID as the top command. If merging is possible (e.g., two consecutive
     * paint operations on the same tile), this method should perform the merge and
     * return true.
     * @param other The command to merge with.
     * @return bool True if merging was successful, false otherwise.
     */
    bool mergeWith(const QUndoCommand* other) override;

    /**
     * @brief Retrieves a list of map positions affected by this command.
     *
     * This method is used to identify which parts of the map need to be redrawn
     * or updated after the command is executed or undone.
     * @return QList<RME::core::Position> A list of affected positions.
     */
    virtual QList<RME::core::Position> getAffectedPositions() const;

    /**
     * @brief Estimates the memory cost of this command.
     *
     * QUndoStack can use this information if its undo limit is interpreted
     * as a memory limit. The cost should be an estimate of the command's
     * memory footprint in bytes. A cost of 0 is ignored by QUndoStack,
     * and a cost of -1 makes the command un-undoable via the limit.
     * Derived classes should override this to provide a more accurate cost.
     *
     * @return int The estimated memory cost of the command. Must be >= 1.
     */
    int cost() const override;

signals:
    // Example signal if commands were QObjects, but typically QUndoStack signals are used.
    // void commandExecuted(const QList<RME::core::Position>& affectedPositions);

protected:
    RME::core::Map* m_map; // Non-owning pointer to the map instance
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_APP_UNDO_COMMAND_H
