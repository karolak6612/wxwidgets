#include "core/actions/TileChangeCommand.h"
#include "core/Map.h"   // For Map methods like getTile, setTile
#include "core/Tile.h"  // For Tile::deepCopy()

// For setText for QUndoCommand
#include <QString>
// #include <QDebug> // For logging

namespace RME {
namespace core {
namespace actions {

TileChangeCommand::TileChangeCommand(
    RME::core::Map* map,
    const RME::core::Position& pos,
    std::unique_ptr<RME::core::Tile> newTileStateData, // This is the state TO BE APPLIED by redo()
    QUndoCommand* parent)
    : AppUndoCommand(map, parent), m_pos(pos), m_newTileStateData(std::move(newTileStateData)) {

    // Capture the state of the tile *before* this command is applied.
    const RME::core::Tile* currentTileOnMap = m_map->getTile(m_pos); // Assuming Map::getTile(Position)
    if (currentTileOnMap) {
        m_oldTileStateData = currentTileOnMap->deepCopy(); // Assuming Tile::deepCopy()
    } else {
        m_oldTileStateData = nullptr; // Represents that there was no tile (or an empty state)
    }

    // Set the command text for display in undo/redo views
    setText(QObject::tr("Change tile at (%1, %2, %3)")
                .arg(m_pos.x).arg(m_pos.y).arg(m_pos.z));
}

TileChangeCommand::~TileChangeCommand() {
    // std::unique_ptr members m_oldTileStateData and m_newTileStateData handle their own memory.
}

void TileChangeCommand::undo() {
    if (!m_map) return;

    // Apply the m_oldTileStateData to the map.
    // Map::setTile should handle replacing existing tile or creating new if needed.
    // If m_oldTileStateData is nullptr, it means the tile should be removed or made empty.
    if (m_oldTileStateData) {
        // Create a copy to place on map, command retains its m_oldTileStateData
        std::unique_ptr<Tile> tileToPlace = m_oldTileStateData->deepCopy();
        m_map->setTile(m_pos, std::move(tileToPlace)); // Assuming Map::setTile takes ownership or copies
    } else {
        m_map->removeTile(m_pos); // Assuming Map::removeTile or Map::setTile(pos, nullptr)
    }

    // Placeholder: emit m_map->notifyTileChanged(m_pos); or similar
    // Or if AppUndoCommand has a signal: emit commandExecuted({m_pos});
    // qCDebug(someLoggingCategory) << "TileChangeCommand undone at" << m_pos.x << m_pos.y << m_pos.z;
}

void TileChangeCommand::redo() {
    if (!m_map) return;

    // Apply the m_newTileStateData to the map.
    if (m_newTileStateData) {
        std::unique_ptr<Tile> tileToPlace = m_newTileStateData->deepCopy();
        m_map->setTile(m_pos, std::move(tileToPlace));
    } else {
        // This means the action was to delete the tile at m_pos
        m_map->removeTile(m_pos);
    }

    // Placeholder: emit m_map->notifyTileChanged(m_pos); or similar
    // qCDebug(someLoggingCategory) << "TileChangeCommand redone at" << m_pos.x << m_pos.y << m_pos.z;
}

// int TileChangeCommand::id() const {
//     return TileChangeCommandId; // Example for merging
// }

// bool TileChangeCommand::mergeWith(const QUndoCommand* other) {
//     // const TileChangeCommand* otherCmd = dynamic_cast<const TileChangeCommand*>(other);
//     // if (!otherCmd) return false;
//     // if (otherCmd->m_pos != this->m_pos) return false;
//     // Merge logic: e.g., this command's new state becomes the final new state.
//     // m_newTileStateData = otherCmd->m_newTileStateData->deepCopy(); // Or std::move if appropriate
//     // setText(...);
//     // return true;
//     return false; // Default no merge
// }

// QList<RME::core::Position> TileChangeCommand::getAffectedPositions() const {
//     return {m_pos};
// }

} // namespace actions
} // namespace core
} // namespace RME
