#include "actions/appundocommand.h"
#include "map/map.h" // Include the full definition for Map

AppUndoCommand::AppUndoCommand(Map* map, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_map(map),
      m_creation_timestamp(QDateTime::currentMSecsSinceEpoch()) // Initialize timestamp
{
    // m_map should not be null, but an assertion could be added here if desired.
    // Q_ASSERT(map);
}

AppUndoCommand::~AppUndoCommand()
{
    // No specific cleanup needed for m_map as it's a non-owning pointer.
}

Map* AppUndoCommand::getMap() const { return m_map; }

qint64 AppUndoCommand::creationTimestamp() const { return m_creation_timestamp; }

// Default implementation returns an empty list.
QList<Position> AppUndoCommand::getChangedPositions() const
{
    return QList<Position>();
}
// Implementations for other common functions would go here.
