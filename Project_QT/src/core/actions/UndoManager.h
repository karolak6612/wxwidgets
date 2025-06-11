#ifndef RME_UNDO_MANAGER_H
#define RME_UNDO_MANAGER_H

#include <QObject>
#include <QUndoStack> // From QtWidgets module
// AppUndoCommand is needed for pushCommand
// #include "core/actions/AppUndoCommand.h" // Include if not just forward declaring

// Forward declarations
namespace RME {
namespace core {
    // class Map; // Forward declare Map - Removed as no longer directly needed by UndoManager.h
    namespace actions {
        class AppUndoCommand; // Forward declare AppUndoCommand
    }
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Manages the undo/redo stack for the application.
 *
 * UndoManager is a wrapper around QUndoStack, providing a centralized place
 * to handle command execution, undo, redo, and related signals. It integrates
 * with AppUndoCommand to manage application-specific commands.
 */
class UndoManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs an UndoManager.
     * @param parent Parent QObject, if any.
     */
    explicit UndoManager(QObject* parent = nullptr);
    /**
     * @brief Destroys the UndoManager.
     */
    ~UndoManager() override;

    /**
     * @brief Pushes a command onto the undo stack.
     *
     * The UndoManager takes ownership of the command.
     * @param command The command to push.
     */
    void pushCommand(AppUndoCommand* command);

    // Wrappers for QUndoStack methods
    /** @brief Checks if there are commands available to undo. @return True if undo is available, false otherwise. */
    bool canUndo() const;
    /** @brief Checks if there are commands available to redo. @return True if redo is available, false otherwise. */
    bool canRedo() const;
    /** @brief Gets the text for the current undo command. @return QString The undo text. */
    QString undoText() const;
    /** @brief Gets the text for the current redo command. @return QString The redo text. */
    QString redoText() const;
    /** @brief Gets the total number of commands on the stack. @return int The command count. */
    int count() const;
    /** @brief Gets the current index in the command stack. @return int The current index. */
    int index() const;
    /** @brief Gets the index of the command that marks the "clean" state. @return int The clean index. */
    int cleanIndex() const;
    /** @brief Checks if the stack is in a "clean" state (no unsaved changes). @return True if clean, false otherwise. */
    bool isClean() const;

    /**
     * @brief Sets the maximum number of commands on the undo stack.
     * @param limit The undo limit.
     */
    void setUndoLimit(int limit);
    /**
     * @brief Gets the maximum number of commands on the undo stack.
     * @return int The undo limit.
     */
    int undoLimit() const;

public slots:
    /** @brief Undoes the last command. */
    void undo();
    /** @brief Redoes the last undone command. */
    void redo();
    /** @brief Marks the current state of the command stack as clean. */
    void setClean();
    /** @brief Clears the command stack. */
    void clear();

signals:
    /** @brief Emitted when the availability of the undo action changes. @param canUndo True if undo is available. */
    void canUndoChanged(bool canUndo);
    /** @brief Emitted when the availability of the redo action changes. @param canRedo True if redo is available. */
    void canRedoChanged(bool canRedo);
    /** @brief Emitted when the current command index changes. @param idx The new index. */
    void indexChanged(int idx);
    /** @brief Emitted when the clean state of the stack changes. @param isClean True if the stack is clean. */
    void cleanChanged(bool isClean);
    /**
     * @brief Emitted when the command stack changes, e.g., after a command is pushed, undone, or redone.
     * This signal can be used to trigger UI updates or other reactions to changes in the undo history.
     */
    void commandStackChanged();
    /**
     * @brief Emitted when map data has changed due to an undo or redo operation.
     *
     * This signal provides a list of specific positions on the map that were
     * affected by the command that was just undone or redone. This allows for
     * targeted UI updates (e.g., redrawing only specific tiles or regions).
     * If the list is empty, it may indicate a general change or that specific
     * positions could not be determined.
     * @param affectedPositions A list of RME::core::Position objects.
     */
    void mapDataChanged(const QList<RME::core::Position>& affectedPositions);

private slots:
    /**
     * @brief Handles the QUndoStack's indexChanged signal.
     *
     * This slot is responsible for emitting the UndoManager's own indexChanged
     * and commandStackChanged signals. It also inspects the current command
     * to emit mapDataChanged with the affected positions.
     * @param idx The new current index in the undo stack.
     */
    void handleIndexChanged(int idx);

private:
    // No need to store m_mapContext if commands get it directly or AppUndoCommand base does.
    // RME::core::Map* m_mapContext;
    QUndoStack m_undoStack; // Owned QUndoStack instance
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_UNDO_MANAGER_H
