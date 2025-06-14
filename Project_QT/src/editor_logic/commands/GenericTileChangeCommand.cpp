#include "editor_logic/commands/GenericTileChangeCommand.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h" // For deepCopying items
#include "core/creatures/Creature.h" // For deepCopying creatures
#include "core/assets/SpawnData.h" // For SpawnDataRef (RME::core::assets::SpawnData)
#include "core/editor/EditorControllerInterface.h"
// AssetManager might not be directly needed here if Tile::deepCopy and Item::create handle it via Tile's provider
// #include "core/assets/AssetManager.h"

#include <QObject> // for tr()
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

GenericTileChangeCommand::GenericTileChangeCommand(
    RME::core::Map* map,
    const RME::core::Position& tilePos,
    std::unique_ptr<RME::core::Tile> oldTileState,
    std::unique_ptr<RME::core::Tile> newTileState,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_map(map),
    m_tilePosition(tilePos),
    m_oldTileState(std::move(oldTileState)),
    m_newTileState(std::move(newTileState)),
    m_controller(controller)
{
    Q_ASSERT(m_map);
    Q_ASSERT(m_controller);
    // It's possible for one of the states to be null if a tile is being created or completely cleared (though less likely for generic change)
    Q_ASSERT(m_oldTileState || m_newTileState);

    setText(QObject::tr("Modify Tile at (%1,%2,%3)")
                .arg(m_tilePosition.x)
                .arg(m_tilePosition.y)
                .arg(m_tilePosition.z));
}

bool GenericTileChangeCommand::applyState(const RME::core::Tile* stateToApply) {
    if (!m_map || !m_controller) {
        qWarning("GenericTileChangeCommand::applyState: Map or controller is null.");
        return false;
    }

    bool tileWasJustCreatedByGetOrCreate = false;
    // Use getOrCreateTile to ensure the tile exists on the map.
    // If stateToApply is nullptr, this means we intend to clear the tile, making it equivalent to a new empty tile.
    // If the tile didn't exist and stateToApply is nullptr, getOrCreateTile will create it, then we clear it.
    RME::core::Tile* liveTile = m_map->getOrCreateTile(m_tilePosition, tileWasJustCreatedByGetOrCreate);
    if (!liveTile) {
        qWarning("GenericTileChangeCommand::applyState: Could not get or create live tile at %s.", qUtf8Printable(m_tilePosition.toString()));
        return false;
    }

    if (!stateToApply) { // Applying a "null" state effectively means clearing the tile to its default state
        liveTile->setGround(nullptr);
        liveTile->clearItems();
        liveTile->setCreature(nullptr);
        liveTile->setSpawnDataRef(nullptr);
        liveTile->setMapFlags(RME::TileMapFlag::NO_FLAGS); // Reset map flags
        // liveTile->setStateFlags(RME::TileStateFlag::NO_FLAGS); // Be careful with transient state flags
        liveTile->setHouseId(0);
        // Note: Does not remove tile from house's internal list if it was a house tile.
        // A more robust "clear" for a house tile would involve EditorController.unassignTileFromHouse or similar.
        // This command assumes it's restoring a state that was *previously* null or fully cleared.
    } else {
        // Apply state from stateToApply (which is a deep copy) to liveTile
        liveTile->setGround(stateToApply->getGround() ? stateToApply->getGround()->deepCopy() : nullptr);

        liveTile->clearItems(); // Clear existing items before adding new ones
        for (const auto& item_ptr : stateToApply->getItems()) {
            if (item_ptr) {
                liveTile->addItem(item_ptr->deepCopy());
            }
        }

        liveTile->setCreature(stateToApply->getCreature() ? stateToApply->getCreature()->deepCopy() : nullptr);

        // SpawnDataRef is a raw pointer; the SpawnData objects are owned by the Map.
        // Tile::deepCopy should have copied the pointer value. We restore that pointer value.
        // The pointed-to SpawnData must still be valid in the map's list.
        liveTile->setSpawnDataRef(stateToApply->getSpawnDataRef());

        liveTile->setMapFlags(stateToApply->getMapFlags());
        // Do not copy all stateFlags directly, as some (like SELECTED, MODIFIED) are transient.
        // Only copy persistent state-like flags if any, or let Tile::update handle them.
        // For example, if BLOCKING is a state flag cached by Tile::update(), don't set it here directly.
        liveTile->setHouseId(stateToApply->getHouseId());
        // TODO: Consider if other properties like m_isHouseExit need copying if they are part of Tile state.
        // For now, assuming HouseId and MapFlags are the main non-object states.
    }

    liveTile->update(); // Recalculate derived states like blocking, has_table etc.
    m_controller->notifyTileChanged(m_tilePosition);
    return true;
}

void GenericTileChangeCommand::redo() {
    if (!applyState(m_newTileState.get())) {
         qWarning("GenericTileChangeCommand::redo failed to apply new state for tile at %s.", qUtf8Printable(m_tilePosition.toString()));
    }
}

void GenericTileChangeCommand::undo() {
    if (!applyState(m_oldTileState.get())) {
         qWarning("GenericTileChangeCommand::undo failed to apply old state for tile at %s.", qUtf8Printable(m_tilePosition.toString()));
    }
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
