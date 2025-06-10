#include "core/actions/AppUndoCommand.h"
#include "core/Map.h" // Include full Map definition

namespace RME {
namespace core {
namespace actions {

AppUndoCommand::AppUndoCommand(RME::core::Map* map, QUndoCommand* parent)
    : QUndoCommand(parent), m_map(map) {
    // m_map must not be null if commands rely on it.
    // Q_ASSERT(map); // Or handle null map appropriately
}

AppUndoCommand::~AppUndoCommand() {
    // Destructor
}

int AppUndoCommand::id() const {
    // Default implementation: no command merging by ID.
    // Derived classes can override this to return a specific ID
    // if they support merging (e.g., multiple brush strokes of the same type).
    return -1;
}

bool AppUndoCommand::mergeWith(const QUndoCommand* other) {
    // Default implementation: no merging.
    // Derived classes override this to implement merging logic
    // with commands of the same type (checked using id()).
    Q_UNUSED(other);
    return false;
}

// QList<RME::core::Position> AppUndoCommand::getAffectedPositions() const {
//     // Default: no specific positions. Derived classes should override.
//     return QList<RME::core::Position>();
// }

} // namespace actions
} // namespace core
} // namespace RME
