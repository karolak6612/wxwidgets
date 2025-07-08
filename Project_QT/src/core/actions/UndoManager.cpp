#include "core/actions/UndoManager.h"
#include "core/actions/AppUndoCommand.h" // Needed for pushCommand argument type

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Constructs an UndoManager.
 * @param parent Parent QObject, if any.
 */
UndoManager::UndoManager(QObject* parent)
    : QObject(parent)
{
    // Connect QUndoStack signals to this manager's signals
    connect(&m_undoStack, &QUndoStack::canUndoChanged, this, &UndoManager::canUndoChanged);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, this, &UndoManager::canRedoChanged);
    // connect(&m_undoStack, &QUndoStack::indexChanged, this, &UndoManager::indexChanged); // Replaced by handleIndexChanged
    connect(&m_undoStack, &QUndoStack::cleanChanged, this, &UndoManager::cleanChanged);

    // Connect to the new private slot to handle index changes and emit detailed signals
    connect(&m_undoStack, &QUndoStack::indexChanged, this, &UndoManager::handleIndexChanged);
    // The commandStackChanged signal will now be emitted by handleIndexChanged.
    // If it was vital that commandStackChanged is emitted *by QUndoStack's signal directly*
    // for some subtle Qt reason, the old connection could be kept. But usually, emitting it
    // from our handler is fine.
    // connect(&m_undoStack, &QUndoStack::indexChanged, this, &UndoManager::commandStackChanged); // Now emitted by handleIndexChanged
}

/**
 * @brief Destroys the UndoManager.
 */
UndoManager::~UndoManager() {
    // m_undoStack is a member, cleaned up automatically.
    // Commands pushed to it are owned by it.
}

/**
 * @brief Pushes a command onto the undo stack.
 * @param command The command to push.
 * @see UndoManager::pushCommand()
 */
void UndoManager::pushCommand(AppUndoCommand* command) {
    // QUndoStack takes ownership of the command
    m_undoStack.push(command);
    // commandStackChanged will be emitted via indexChanged signal from QUndoStack
}

/**
 * @brief Undoes the last command.
 * @see UndoManager::undo()
 */
void UndoManager::undo() {
    m_undoStack.undo();
    // commandStackChanged will be emitted via indexChanged
}

/**
 * @brief Redoes the last undone command.
 * @see UndoManager::redo()
 */
void UndoManager::redo() {
    m_undoStack.redo();
    // commandStackChanged will be emitted via indexChanged
}

/**
 * @brief Checks if there are commands available to undo.
 * @return True if undo is available, false otherwise.
 * @see UndoManager::canUndo()
 */
bool UndoManager::canUndo() const {
    return m_undoStack.canUndo();
}

/**
 * @brief Checks if there are commands available to redo.
 * @return True if redo is available, false otherwise.
 * @see UndoManager::canRedo()
 */
bool UndoManager::canRedo() const {
    return m_undoStack.canRedo();
}

/**
 * @brief Gets the text for the current undo command.
 * @return QString The undo text.
 * @see UndoManager::undoText()
 */
QString UndoManager::undoText() const {
    return m_undoStack.undoText();
}

/**
 * @brief Gets the text for the current redo command.
 * @return QString The redo text.
 * @see UndoManager::redoText()
 */
QString UndoManager::redoText() const {
    return m_undoStack.redoText();
}

/**
 * @brief Gets the total number of commands on the stack.
 * @return int The command count.
 * @see UndoManager::count()
 */
int UndoManager::count() const {
    return m_undoStack.count();
}

/**
 * @brief Gets the current index in the command stack.
 * @return int The current index.
 * @see UndoManager::index()
 */
int UndoManager::index() const {
    return m_undoStack.index();
}

/**
 * @brief Gets the index of the command that marks the "clean" state.
 * @return int The clean index.
 * @see UndoManager::cleanIndex()
 */
int UndoManager::cleanIndex() const {
    return m_undoStack.cleanIndex();
}

/**
 * @brief Checks if the stack is in a "clean" state.
 * @return True if clean, false otherwise.
 * @see UndoManager::isClean()
 */
bool UndoManager::isClean() const {
    return m_undoStack.isClean();
}

/**
 * @brief Sets the maximum number of commands on the undo stack.
 * @param limit The undo limit.
 * @see UndoManager::setUndoLimit()
 */
void UndoManager::setUndoLimit(int limit) {
    m_undoStack.setUndoLimit(limit);
}

/**
 * @brief Gets the maximum number of commands on the undo stack.
 * @return int The undo limit.
 * @see UndoManager::undoLimit()
 */
int UndoManager::undoLimit() const {
    return m_undoStack.undoLimit();
}

/**
 * @brief Marks the current state of the command stack as clean.
 * @see UndoManager::setClean()
 */
void UndoManager::setClean() {
    m_undoStack.setClean();
    // commandStackChanged might not be strictly needed here, but cleanChanged signal covers it.
}

/**
 * @brief Clears the command stack.
 * @see UndoManager::clear()
 */
void UndoManager::clear() {
    m_undoStack.clear(); // Clears all commands
    // commandStackChanged will be emitted via indexChanged (likely to -1 or 0 through handleIndexChanged)
}

/**
 * @brief Handles the QUndoStack's indexChanged signal.
 *
 * This slot is responsible for emitting the UndoManager's own indexChanged
 * and commandStackChanged signals. It also inspects the current command
 * (if any) at the new index `idx` to emit `mapDataChanged` with the
 * affected positions. If the command is not an `AppUndoCommand` or if there's
 * no command at the index (e.g., stack cleared), `mapDataChanged` is emitted
 * with an empty list of positions.
 *
 * @param idx The new current index in the undo stack.
 */
void UndoManager::handleIndexChanged(int idx) {
    emit indexChanged(idx); // Maintain original indexChanged signal
    emit commandStackChanged(); // Consolidate commandStackChanged emission here

    const QUndoCommand* rawCmd = m_undoStack.command(idx);

    if (rawCmd) {
        const AppUndoCommand* appCmd = dynamic_cast<const AppUndoCommand*>(rawCmd);
        if (appCmd) {
            emit mapDataChanged(appCmd->getAffectedPositions());
        } else {
            // Command is not an AppUndoCommand, or cast failed.
            // Emit with empty list, signaling a general/unknown change area.
            emit mapDataChanged(QList<RME::core::Position>());
        }
    } else {
        // No command at the current index (e.g., stack cleared, idx is -1)
        // Emit with empty list.
        emit mapDataChanged(QList<RME::core::Position>());
    }
}

} // namespace actions
} // namespace core
} // namespace RME

// #include "UndoManager.moc" // Removed - Q_OBJECT is in header
