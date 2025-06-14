#include "editor_logic/commands/RecordModifyTileContentsCommand.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/Spawn.h"
#include "core/Creature.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h" // For map->notifyTileChanged
#include "core/assets/AssetManager.h" // For item names etc. (potentially)
#include "core/assets/ItemDatabase.h"

#include <QObject> // For tr()
#include <QDebug>  // For qWarning, Q_ASSERT
#include <sstream> // For std::ostringstream

namespace RME {
namespace editor_logic {
namespace commands {

RecordModifyTileContentsCommand::RecordModifyTileContentsCommand(
    RME::core::Tile* tile,
    RME::core::editor::EditorControllerInterface* controller,
    std::unique_ptr<RME::core::Item> previouslyExistingGround,
    std::vector<std::unique_ptr<RME::core::Item>> previouslyExistingItems,
    std::unique_ptr<RME::core::Spawn> previouslyExistingSpawn,
    std::unique_ptr<RME::core::Creature> previouslyExistingCreature,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_tile(tile),
    m_controller(controller)
{
    Q_ASSERT(m_tile);
    Q_ASSERT(m_controller);

    m_tilePosition = m_tile->getPosition();

    // Take ownership of the passed-in previous states. These are deep copies made by the brush.
    m_undoneGround = std::move(previouslyExistingGround);
    m_undoneItems = std::move(previouslyExistingItems);
    m_undoneSpawn = std::move(previouslyExistingSpawn);
    m_undoneCreature = std::move(previouslyExistingCreature);

    m_didClearGround = (m_undoneGround != nullptr);
    m_didClearItems = !m_undoneItems.empty();
    m_didClearSpawn = (m_undoneSpawn != nullptr);
    m_didClearCreature = (m_undoneCreature != nullptr);

    // Generate command text
    std::ostringstream description;
    description << "Erase Tile Contents at (" << m_tilePosition.x << "," << m_tilePosition.y << "," << m_tilePosition.z << "):";
    if (m_didClearGround) description << " Ground,";
    if (m_didClearItems) description << " " << m_undoneItems.size() << " Item(s),";
    if (m_didClearSpawn) description << " Spawn,";
    if (m_didClearCreature) description << " Creature,";

    QString desc = QString::fromStdString(description.str());
    // Remove trailing comma if any
    if (desc.endsWith(',')) desc.chop(1);
    setText(desc);
}

void RecordModifyTileContentsCommand::undo() {
    if (!m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("RecordModifyTileContentsCommand::undo: Critical objects missing.");
        return;
    }

    // Restore elements in a logical order (e.g., ground first)
    if (m_didClearGround && m_undoneGround) {
        m_tile->setGround(m_undoneGround->deepCopy()); // Add back a copy
    }

    if (m_didClearItems) {
        for (const auto& item_ptr : m_undoneItems) {
            if (item_ptr) m_tile->addItem(item_ptr->deepCopy()); // Add back copies
        }
    }

    if (m_didClearSpawn && m_undoneSpawn) {
        m_tile->setSpawn(m_undoneSpawn->deepCopy()); // Add back a copy
    }

    if (m_didClearCreature && m_undoneCreature) {
        m_tile->setCreature(m_undoneCreature->deepCopy()); // Add back a copy
    }

    m_controller->getMap()->notifyTileChanged(m_tilePosition);
}

void RecordModifyTileContentsCommand::redo() {
    if (!m_tile || !m_controller || !m_controller->getMap()) {
        qWarning("RecordModifyTileContentsCommand::redo: Critical objects missing.");
        return;
    }

    // The brush has already performed the clear operations on the tile when this command was created.
    // For redo, we need to ensure the tile is cleared of these specific elements again.
    // This means the brush must have done its job, and redo is about re-applying that specific clearing.

    if (m_didClearGround) {
        // We assume if m_didClearGround is true, the intention was to remove the ground that was m_undoneGround.
        // If another command changed the ground in between, this might clear the wrong ground.
        // This command is simpler if it assumes it's only re-clearing what was there.
        // More robust: if m_undoneGround exists, clear current ground if it matches m_undoneGround by ID?
        // For now: if the command was for clearing ground, redo clears current ground.
        m_tile->setGround(nullptr);
    }

    if (m_didClearItems) {
        // Remove items that correspond to those in m_undoneItems.
        // This requires identifying them on the tile. Using IDs is most common.
        std::vector<RME::core::Item*> itemsToRemoveOnTile;
        for (const auto& originally_cleared_item_ptr : m_undoneItems) {
            if (!originally_cleared_item_ptr) continue;
            // Find this item on the tile. For simplicity, find by ID. Topmost if multiple.
            RME::core::Item* itemOnTile = m_tile->getTopItemByID(originally_cleared_item_ptr->getID());
            if(itemOnTile) {
                 bool alreadyMarked = false;
                 for(RME::core::Item* markedItem : itemsToRemoveOnTile) {
                     if(markedItem == itemOnTile) {
                         alreadyMarked = true;
                         break;
                     }
                 }
                 if(!alreadyMarked) itemsToRemoveOnTile.push_back(itemOnTile);
            }
        }
        for(RME::core::Item* itemToRemove : itemsToRemoveOnTile) {
            m_tile->removeItem(itemToRemove, true /*destroy*/);
        }
    }

    if (m_didClearSpawn) {
        m_tile->setSpawn(nullptr);
    }

    if (m_didClearCreature) {
        m_tile->setCreature(nullptr); // Assuming Tile::setCreature(nullptr) is the way to clear.
    }

    m_controller->getMap()->notifyTileChanged(m_tilePosition);
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
