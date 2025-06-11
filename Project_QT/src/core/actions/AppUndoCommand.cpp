#include "core/actions/AppUndoCommand.h"
#include "core/Map.h" // Include full Map definition

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Constructs an AppUndoCommand.
 * @param map Pointer to the map instance the command will operate on.
 * @param parent Parent command, if any.
 */
AppUndoCommand::AppUndoCommand(RME::core::Map* map, QUndoCommand* parent)
    : QUndoCommand(parent), m_map(map) {
    // m_map must not be null if commands rely on it.
    // Q_ASSERT(map); // Or handle null map appropriately
}

/**
 * @brief Destroys the AppUndoCommand.
 */
AppUndoCommand::~AppUndoCommand() {
    // Destructor
}

/**
 * @brief Returns a unique ID for the command type.
 * @return int The command ID. Default is -1 (no merging).
 * @see AppUndoCommand::id()
 */
int AppUndoCommand::id() const {
    // Default implementation: no command merging by ID.
    // Derived classes can override this to return a specific ID
    // if they support merging (e.g., multiple brush strokes of the same type).
    return -1;
}

/**
 * @brief Attempts to merge this command with a subsequent command.
 * @param other The command to merge with.
 * @return bool True if merging was successful, false otherwise.
 * @see AppUndoCommand::mergeWith()
 */
bool AppUndoCommand::mergeWith(const QUndoCommand* other) {
    // Default implementation: no merging.
    // Derived classes override this to implement merging logic
    // with commands of the same type (checked using id()).
    Q_UNUSED(other);
    return false;
}

/**
 * @brief Retrieves a list of map positions affected by this command.
 * @return QList<RME::core::Position> A list of affected positions.
 *         Default implementation returns an empty list.
 * @see AppUndoCommand::getAffectedPositions()
 */
QList<RME::core::Position> AppUndoCommand::getAffectedPositions() const {
    // Default: no specific positions. Derived classes should override.
    return QList<RME::core::Position>();
}

/**
 * @brief Estimates the memory cost of this command.
 * @return int The base cost, defaults to 1. Derived classes should override.
 * @see AppUndoCommand::cost()
 */
int AppUndoCommand::cost() const {
    // Default cost for a command. QUndoStack requires cost >= 1.
    // Derived classes should provide a more accurate estimate of their memory footprint.
    return 1;
}

} // namespace actions
} // namespace core
} // namespace RME
