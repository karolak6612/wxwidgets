#include "commands/SetHouseExitCommand.h"
#include "core/map/Map.h"           // For RME::Map
#include "core/houses/HouseData.h"  // For RME::HouseData
#include <QDebug>
#include <QObject> // For QObject::tr

namespace RME_COMMANDS {

SetHouseExitCommand::SetHouseExitCommand(
    RME::Map* map,
    uint32_t houseId,
    const RME::core::Position& oldExitPos,
    const RME::core::Position& newExitPos,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    m_houseId(houseId),
    m_oldExitPos(oldExitPos),
    m_newExitPos(newExitPos) {
    // Initial text is set in redo() as it reflects the action taken.
    // If newExitPos is invalid (e.g. 0,0,0 meaning clear exit), text should reflect that.
    // For now, assume newExitPos is a valid position.
}

void SetHouseExitCommand::undo() {
    if (!m_map) {
        qWarning("SetHouseExitCommand::undo: Map is null.");
        return;
    }
    RME::HouseData* house = m_map->getHouse(m_houseId);
    if (house) {
        // The HouseData::setEntryPoint method handles updating tile flags
        house->setEntryPoint(m_oldExitPos, m_map);
        setText(QObject::tr("Undo Set House Exit for House %1 to (%2,%3,%4)")
            .arg(m_houseId)
            .arg(m_oldExitPos.x)
            .arg(m_oldExitPos.y)
            .arg(m_oldExitPos.z));
    } else {
        qWarning("SetHouseExitCommand::undo: House ID %u not found.", m_houseId);
        setText(QObject::tr("Undo Set House Exit (House %1 not found)").arg(m_houseId));
        // If house doesn't exist, can't restore its exit. This implies an issue elsewhere
        // or that the house was deleted by an intervening command.
    }
}

void SetHouseExitCommand::redo() {
    if (!m_map) {
        qWarning("SetHouseExitCommand::redo: Map is null.");
        return;
    }
    RME::HouseData* house = m_map->getHouse(m_houseId);
    if (house) {
        // m_oldExitPos was the state *before* this command was first executed.
        // It's correctly stored by the constructor.
        // When redoing, we apply m_newExitPos.
        house->setEntryPoint(m_newExitPos, m_map);
        setText(QObject::tr("Set House Exit for House %1 to (%2,%3,%4)")
            .arg(m_houseId)
            .arg(m_newExitPos.x)
            .arg(m_newExitPos.y)
            .arg(m_newExitPos.z));
    } else {
        qWarning("SetHouseExitCommand::redo: House ID %u not found.", m_houseId);
        setText(QObject::tr("Set House Exit (House %1 not found)").arg(m_houseId));
        // If the house doesn't exist on redo, this command is effectively stale.
    }
}

} // namespace RME_COMMANDS
