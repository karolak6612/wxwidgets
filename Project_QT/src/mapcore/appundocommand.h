#ifndef APPUNDOCOMMAND_H
#define APPUNDOCOMMAND_H

#include <QUndoCommand>
// #include <QObject> // Required for signals/slots - Not using QObject for commands now
#include <QDateTime>
#include <QList>   // For QList<Position>
#include "position.h" // Ensure Position is known

// Forward declaration
class Map;

// AppUndoCommand must inherit from QObject for signals.
// QUndoCommand is not a QObject, so AppUndoCommand must multi-inherit or QUndoCommand must be a QObject.
// QUndoCommand is NOT a QObject. This is a common issue.
// Solutions:
// 1. The QUndoStack itself is a QObject and emits signals like indexChanged(). Listen to those.
// 2. Commands themselves don't emit signals directly. Instead, they provide data about changes,
//    and the code that calls undo/redo or listens to QUndoStack signals queries this data.
// 3. A wrapper system or a notification manager.

// Given QUndoCommand is not a QObject, commands themselves cannot directly emit Qt signals
// unless they are also QObjects AND managed in a way that Qt's meta-object system can work with them
// (e.g. if they are parented to a QObject). Pushing raw QObject-derived commands onto QUndoStack owned by non-QObject
// can be tricky for signal/slot connections if not handled carefully.

// Let's choose a simpler path first: The command can hold the changed region data,
// and the code reacting to QUndoStack::indexChanged() can query the command.

// New plan for this step:
// - Add a virtual method to AppUndoCommand: `virtual QList<Position> getChangedPositions() const;`
// - Concrete commands will implement this to return the positions they affected.
// - The UI/Map View will connect to `QUndoStack::indexChanged(int idx)`, then get the command
//   via `stack->command(idx)` and call `getChangedPositions()` on it.

class AppUndoCommand : public QUndoCommand // No QObject inheritance here for simplicity now
{
public:
    explicit AppUndoCommand(Map* map, QUndoCommand *parent = nullptr);
    ~AppUndoCommand() override;

    Map* getMap() const;
    qint64 creationTimestamp() const;
    virtual int id() const override { return -1; }

    // Method to be overridden by concrete commands to report what changed.
    virtual QList<Position> getChangedPositions() const;

protected:
    Map* m_map; // Pointer to the map, not owning.
    qint64 m_creation_timestamp;
};

#endif // APPUNDOCOMMAND_H
