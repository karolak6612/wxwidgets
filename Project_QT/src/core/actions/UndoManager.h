#ifndef RME_UNDO_MANAGER_H
#define RME_UNDO_MANAGER_H

#include <QObject>
#include <QUndoStack> // From QtWidgets module
// AppUndoCommand is needed for pushCommand
// #include "core/actions/AppUndoCommand.h" // Include if not just forward declaring

// Forward declarations
namespace RME {
namespace core {
    class Map; // Forward declare Map
    namespace actions {
        class AppUndoCommand; // Forward declare AppUndoCommand
    }
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace actions {

class UndoManager : public QObject {
    Q_OBJECT
public:
    // Map pointer is stored for context but not owned by UndoManager itself.
    // It might be passed to commands if they need it and don't get it from elsewhere.
    explicit UndoManager(RME::core::Map* mapContext, QObject* parent = nullptr);
    ~UndoManager() override;

    void pushCommand(AppUndoCommand* command); // Takes ownership if QUndoStack does by default (it does)

    // Wrappers for QUndoStack methods
    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;
    int count() const;
    int index() const;
    int cleanIndex() const;
    bool isClean() const;

    void setUndoLimit(int limit);
    int undoLimit() const;

public slots:
    void undo();
    void redo();
    void setClean(); // Marks current stack state as clean
    void clear();    // Clears the stack

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void indexChanged(int idx);
    void cleanChanged(bool isClean);
    // Signal to notify that the map state has effectively changed due to an undo/redo action.
    // Commands themselves might also provide more granular change info.
    void commandStackChanged();

private:
    // No need to store m_mapContext if commands get it directly or AppUndoCommand base does.
    // RME::core::Map* m_mapContext;
    QUndoStack m_undoStack; // Owned QUndoStack instance
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_UNDO_MANAGER_H
