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

class AppUndoCommand : public QUndoCommand {
    // Q_OBJECT // Not strictly necessary for QUndoCommand unless it has its own signals/slots
public:
    // Constructor might take a pointer to the map or other relevant services
    explicit AppUndoCommand(RME::core::Map* map, QUndoCommand* parent = nullptr);
    ~AppUndoCommand() override;

    // Optional: For command compression (merging)
    // Returns a unique ID for the command type. Default is -1 (no merging).
    int id() const override;
    // Try to merge this command with 'other'. Return true if merged.
    bool mergeWith(const QUndoCommand* other) override;

    // Optional: For providing a list of affected areas for UI updates
    virtual QList<RME::core::Position> getAffectedPositions() const;

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
