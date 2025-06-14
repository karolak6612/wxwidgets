#include "editor_logic/commands/SetHouseTileCommand.h"
#include "core/houses/House.h"
#include "core/Tile.h"
#include "core/map/Map.h" // For controller->getMap()->notifyTileChanged
#include "core/editor/EditorControllerInterface.h"

#include <QObject> // For tr()
#include <QDebug>  // For Q_ASSERT, qWarning

namespace RME {
namespace editor_logic {
namespace commands {

SetHouseTileCommand::SetHouseTileCommand(
    RME::core::houses::House* house,
    RME::core::Tile* tile,
    bool assignToHouse,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_house(house),
    m_tile(tile),
    m_controller(controller),
    m_assignToHouse(assignToHouse),
    m_originalTileHouseId(0),
    m_originalTilePZStatus(false),
    m_wasTileInThisHouseListBeforeRedo(false) // Initialized, though not strictly used in current undo logic
{
    Q_ASSERT(m_house);
    Q_ASSERT(m_tile);
    Q_ASSERT(m_controller);
    m_tilePos = m_tile->getPosition(); // Store position for safety
    // Text set in redo
}

void SetHouseTileCommand::redo() {
    if (!m_house || !m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("SetHouseTileCommand::redo: Invalid members.");
        setText(QObject::tr("Set House Tile (Error)"));
        return;
    }

    // Capture state before this action
    m_originalTileHouseId = m_tile->getHouseId();
    m_originalTilePZStatus = m_tile->isProtectionZone();
    // m_wasTileInThisHouseListBeforeRedo = m_house->hasTilePosition(m_tilePos); // Could be used for more complex undo logic

    RME::core::Map* map = m_controller->getMap();

    if (m_assignToHouse) {
        if (m_originalTileHouseId != 0 && m_originalTileHouseId != m_house->getId()) {
            qWarning("SetHouseTileCommand: Tile at (%s) belonged to house %u, reassigning to house %u.")
                .arg(m_tilePos.toString()).arg(m_originalTileHouseId).arg(m_house->getId());
            // NOTE: This command does not manage unlinking from the *old* house's m_tilePositions list.
            // That would require access to the Houses manager or the old House object.
            // The HouseBrush should ideally handle such complex reassignments by generating multiple commands
            // or a more sophisticated command if a tile is "stolen" from another house.
        }

        m_house->linkTile(m_tile); // Sets tile's house ID to m_house->getId(), adds pos to m_house's list.
        m_tile->setIsProtectionZone(true); // Assigning to house makes it a PZ.
        setText(QObject::tr("Assign tile (%1) to house %2").arg(m_tilePos.toString()).arg(m_house->getId()));
    } else { // Unassign from this house
        if (m_originalTileHouseId == m_house->getId()) {
            m_house->unlinkTile(m_tile); // Sets tile's house ID to 0, PZ to false, removes from m_house's list.
            setText(QObject::tr("Unassign tile (%1) from house %2").arg(m_tilePos.toString()).arg(m_house->getId()));
        } else {
            // Tile doesn't belong to this house, or is already unassigned.
            setText(QObject::tr("Unassign tile (%1) from house %2 (no change for this house)").arg(m_tilePos.toString()).arg(m_house->getId()));
            // No actual change made by this command's redo in this case for *this* house.
            // The tile's state (m_originalTileHouseId, m_originalTilePZStatus) is preserved for undo.
        }
    }
    map->notifyTileChanged(m_tilePos);
}

void SetHouseTileCommand::undo() {
    if (!m_house || !m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("SetHouseTileCommand::undo: Invalid members.");
        return;
    }
    RME::core::Map* map = m_controller->getMap();

    // Revert tile's house ID and PZ status to its original state *before this command's redo action*
    m_tile->setHouseId(m_originalTileHouseId);
    m_tile->setIsProtectionZone(m_originalTilePZStatus);

    // Adjust m_house's internal list of tile positions based on what redo did.
    if (m_assignToHouse) {
        // Redo assigned tile to m_house. Undo should remove it from m_house's list.
        m_house->removeTilePosition(m_tilePos);
    } else {
        // Redo unassigned tile from m_house (it must have belonged to m_house before for redo to act).
        if (m_originalTileHouseId == m_house->getId()) { // This condition ensures redo actually unlinked from THIS house
             m_house->addTilePosition(m_tilePos);
        }
    }
    // Note: If m_originalTileHouseId pointed to *another* house, restoring that link fully
    // would require getting that House object and calling its linkTile, or more complex state storage.
    // This command primarily focuses on the state change of m_tile with respect to m_house.

    map->notifyTileChanged(m_tilePos);
    // Construct a meaningful undo text
    QString originalActionText;
    if (m_assignToHouse) {
        originalActionText = QObject::tr("Assign tile (%1) to house %2").arg(m_tilePos.toString()).arg(m_house->getId());
    } else {
        if (m_originalTileHouseId == m_house->getId()) { // If redo actually did unassign from this house
            originalActionText = QObject::tr("Unassign tile (%1) from house %2").arg(m_tilePos.toString()).arg(m_house->getId());
        } else {
            originalActionText = QObject::tr("Unassign tile (%1) from house %2 (no change for this house)").arg(m_tilePos.toString()).arg(m_house->getId());
        }
    }
    setText(QObject::tr("Undo: ") + originalActionText);
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
