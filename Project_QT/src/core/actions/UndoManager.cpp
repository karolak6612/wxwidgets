#include "core/actions/UndoManager.h"
#include "core/actions/AppUndoCommand.h" // Needed for pushCommand argument type
// #include "core/Map.h" // Not directly needed if m_mapContext is removed

namespace RME {
namespace core {
namespace actions {

UndoManager::UndoManager(RME::core::Map* mapContext, QObject* parent)
    : QObject(parent)
    // m_mapContext(mapContext) // Not storing mapContext if commands handle it
{
    // Connect QUndoStack signals to this manager's signals
    connect(&m_undoStack, &QUndoStack::canUndoChanged, this, &UndoManager::canUndoChanged);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, this, &UndoManager::canRedoChanged);
    connect(&m_undoStack, &QUndoStack::indexChanged, this, &UndoManager::indexChanged);
    connect(&m_undoStack, &QUndoStack::cleanChanged, this, &UndoManager::cleanChanged);

    // Emit commandStackChanged whenever a command is pushed, undone, or redone.
    // indexChanged is a good proxy for this.
    connect(&m_undoStack, &QUndoStack::indexChanged, this, &UndoManager::commandStackChanged);
}

UndoManager::~UndoManager() {
    // m_undoStack is a member, cleaned up automatically.
    // Commands pushed to it are owned by it.
}

void UndoManager::pushCommand(AppUndoCommand* command) {
    // QUndoStack takes ownership of the command
    m_undoStack.push(command);
    // commandStackChanged will be emitted via indexChanged signal from QUndoStack
}

void UndoManager::undo() {
    m_undoStack.undo();
    // commandStackChanged will be emitted via indexChanged
}

void UndoManager::redo() {
    m_undoStack.redo();
    // commandStackChanged will be emitted via indexChanged
}

bool UndoManager::canUndo() const {
    return m_undoStack.canUndo();
}

bool UndoManager::canRedo() const {
    return m_undoStack.canRedo();
}

QString UndoManager::undoText() const {
    return m_undoStack.undoText();
}

QString UndoManager::redoText() const {
    return m_undoStack.redoText();
}

int UndoManager::count() const {
    return m_undoStack.count();
}

int UndoManager::index() const {
    return m_undoStack.index();
}

int UndoManager::cleanIndex() const {
    return m_undoStack.cleanIndex();
}

bool UndoManager::isClean() const {
    return m_undoStack.isClean();
}

void UndoManager::setUndoLimit(int limit) {
    m_undoStack.setUndoLimit(limit);
}

int UndoManager::undoLimit() const {
    return m_undoStack.undoLimit();
}

void UndoManager::setClean() {
    m_undoStack.setClean();
    // commandStackChanged might not be strictly needed here, but cleanChanged signal covers it.
}

void UndoManager::clear() {
    m_undoStack.clear(); // Clears all commands
    // commandStackChanged will be emitted via indexChanged (likely to -1 or 0)
}

} // namespace actions
} // namespace core
} // namespace RME
