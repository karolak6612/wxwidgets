#include "editor_logic/commands/SetHouseTileCommand.h"
#include "core/houses/Houses.h"
#include "core/Tile.h"
#include "core/map/Map.h" // For controller->getMap()->notifyTileChanged
#include "core/editor/EditorControllerInterface.h"
#include "core/items/DoorItem.h" // For door ID management

#include <QObject> // For tr()
#include <QDebug>  // For Q_ASSERT, qWarning

namespace RME {
namespace core {
namespace actions {

SetHouseTileCommand::SetHouseTileCommand(
    quint32 houseId,
    RME::core::Tile* tile,
    bool assignToHouse,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Set House Tile"), parent),
    m_houseId(houseId),
    m_tile(tile),
    m_assignToHouse(assignToHouse),
    m_originalTileHouseId(0),
    m_originalTilePZStatus(false),
    m_wasTileInThisHouseListBeforeRedo(false) // Initialized, though not strictly used in current undo logic
{
    Q_ASSERT(m_houseId > 0 || !m_assignToHouse); // House ID should be valid for assignment
    Q_ASSERT(m_tile);
    m_tilePos = m_tile->getPosition(); // Store position for safety
    // Text set in redo
}

void SetHouseTileCommand::redo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("redo house tile operation");
        return;
    }
    
    // Get houses manager and house data
    auto* housesManager = getController()->getHousesManager();
    if (!housesManager) {
        setErrorText("redo house tile operation - no houses manager");
        return;
    }

    // Capture state before this action
    m_originalTileHouseId = m_tile->getHouseId();
    m_originalTilePZStatus = m_tile->isProtectionZone();

    if (m_assignToHouse) {
        if (m_originalTileHouseId != 0 && m_originalTileHouseId != m_houseId) {
            qWarning("SetHouseTileCommand: Tile at (%s) belonged to house %u, reassigning to house %u.")
                .arg(m_tilePos.toString()).arg(m_originalTileHouseId).arg(m_houseId);
            // NOTE: This command does not manage unlinking from the *old* house's m_tilePositions list.
            // That would require access to the Houses manager or the old House object.
            // The HouseBrush should ideally handle such complex reassignments by generating multiple commands
            // or a more sophisticated command if a tile is "stolen" from another house.
        }

        // Set tile's house ID and protection zone
        m_tile->setHouseId(m_houseId);
        m_tile->setIsProtectionZone(true); // Assigning to house makes it a PZ.
        
        // Link tile to house via Houses manager
        if (getController() && getController()->getHousesManager()) {
            getController()->getHousesManager()->linkTileToHouse(m_houseId, m_tilePos);
        }
        
        // Handle AppSettings-based features
        const auto& appSettings = getController()->getAppSettings();
        
        // Remove moveable items if setting is enabled
        if (appSettings.isHouseBrushRemoveItemsEnabled()) {
            auto& items = m_tile->getItems();
            for (auto it = items.begin(); it != items.end(); ) {
                if ((*it) && (*it)->isMoveable()) {
                    it = items.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        // Auto-assign door IDs if setting is enabled
        if (appSettings.isAutoAssignDoorIDEnabled()) {
            auto& items = m_tile->getItems();
            for (auto& item : items) {
                if (item && item->isDoor()) {
                    // Cast to DoorItem to access door-specific methods
                    if (auto* doorItem = dynamic_cast<RME::core::items::DoorItem*>(item.get())) {
                        if (doorItem->getDoorId() == 0 || m_originalTileHouseId != 0) {
                            // Get empty door ID from Houses manager
                            quint8 newDoorId = getController()->getHousesManager()->getEmptyDoorID(m_houseId);
                            doorItem->setDoorId(newDoorId);
                        }
                    }
                }
            }
        }
        
        setText(QObject::tr("Assign tile (%1) to house %2").arg(m_tilePos.toString()).arg(m_houseId));
    } else { // Unassign from this house
        if (m_originalTileHouseId == m_houseId) {
            // Clear tile's house ID and protection zone
            m_tile->setHouseId(0);
            m_tile->setIsProtectionZone(false);
            
            // Unlink tile from house via Houses manager
            if (getController() && getController()->getHousesManager()) {
                getController()->getHousesManager()->unlinkTileFromHouse(m_houseId, m_tilePos);
            }
            
            // Handle AppSettings-based features for erase mode
            const auto& appSettings = getController()->getAppSettings();
            
            // Clear door IDs if setting is enabled
            if (appSettings.isAutoAssignDoorIDEnabled()) {
                auto& items = m_tile->getItems();
                for (auto& item : items) {
                    if (item && item->isDoor()) {
                        // Cast to DoorItem to access door-specific methods
                        if (auto* doorItem = dynamic_cast<RME::core::items::DoorItem*>(item.get())) {
                            doorItem->setDoorId(0);
                        }
                    }
                }
            }
            
            setText(QObject::tr("Unassign tile (%1) from house %2").arg(m_tilePos.toString()).arg(m_houseId));
        } else {
            // Tile doesn't belong to this house, or is already unassigned.
            setText(QObject::tr("Unassign tile (%1) from house %2 (no change for this house)").arg(m_tilePos.toString()).arg(m_houseId));
            // No actual change made by this command's redo in this case for *this* house.
            // The tile's state (m_originalTileHouseId, m_originalTilePZStatus) is preserved for undo.
        }
    }
    map->notifyTileChanged(m_tilePos);
}

void SetHouseTileCommand::undo() {
    if (!validateMembers() || !m_tile) {
        setErrorText("undo house tile operation");
        return;
    }

    // Revert tile's house ID and PZ status to its original state *before this command's redo action*
    m_tile->setHouseId(m_originalTileHouseId);
    m_tile->setIsProtectionZone(m_originalTilePZStatus);

    // Adjust house's internal list of tile positions based on what redo did.
    if (getController()->getHousesManager()) {
        if (m_assignToHouse) {
            // Redo assigned tile to house. Undo should remove it from house's list.
            getController()->getHousesManager()->unlinkTileFromHouse(m_houseId, m_tilePos);
            
            // If tile originally belonged to another house, restore that link
            if (m_originalTileHouseId != 0 && m_originalTileHouseId != m_houseId) {
                getController()->getHousesManager()->linkTileToHouse(m_originalTileHouseId, m_tilePos);
            }
        } else {
            // Redo unassigned tile from house (it must have belonged to this house before for redo to act).
            if (m_originalTileHouseId == m_houseId) { // This condition ensures redo actually unlinked from THIS house
                getController()->getHousesManager()->linkTileToHouse(m_houseId, m_tilePos);
            }
        }
    }

    notifyMapChanged(m_tilePos);
    // Construct a meaningful undo text
    QString originalActionText;
    if (m_assignToHouse) {
        originalActionText = QObject::tr("Assign tile (%1) to house %2").arg(m_tilePos.toString()).arg(m_houseId);
    } else {
        if (m_originalTileHouseId == m_houseId) { // If redo actually did unassign from this house
            originalActionText = QObject::tr("Unassign tile (%1) from house %2").arg(m_tilePos.toString()).arg(m_houseId);
        } else {
            originalActionText = QObject::tr("Unassign tile (%1) from house %2 (no change for this house)").arg(m_tilePos.toString()).arg(m_houseId);
        }
    }
    setText(QObject::tr("Undo: ") + originalActionText);
}

} // namespace actions
} // namespace core
} // namespace RME
