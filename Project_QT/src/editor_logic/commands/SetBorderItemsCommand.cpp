#include "editor_logic/commands/SetBorderItemsCommand.h"
#include "core/Tile.h"
#include "core/Item.h" // For Item::create
#include "core/assets/AssetManager.h" // For getting AssetManager
#include "core/editor/EditorControllerInterface.h"
#include "core/map/Map.h" // For notifyTileChanged via controller
#include <QObject> // for tr()
#include <QDebug>

namespace RME {
namespace editor_logic {
namespace commands {

SetBorderItemsCommand::SetBorderItemsCommand(
    RME::core::Tile* tile,
    const QList<uint16_t>& oldBorderItemIds,
    const QList<uint16_t>& newBorderItemIds,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : QUndoCommand(parent),
    m_tile(tile),
    m_oldBorderItemIds(oldBorderItemIds),
    m_newBorderItemIds(newBorderItemIds),
    m_controller(controller),
    m_assetManager(nullptr)
{
    Q_ASSERT(m_tile);
    Q_ASSERT(m_controller);
    m_assetManager = m_controller->getAssetManager(); // Get AssetManager from controller
    Q_ASSERT(m_assetManager); // AssetManager is crucial for creating items
    m_tilePosition = m_tile->getPosition();

    setText(QObject::tr("Set Border Items at (%1,%2,%3)")
                .arg(m_tilePosition.x)
                .arg(m_tilePosition.y)
                .arg(m_tilePosition.z));
}

void SetBorderItemsCommand::redo() {
    if (!m_tile || !m_controller || !m_assetManager) {
        qWarning("SetBorderItemsCommand::redo: Tile, controller, or assetManager is null.");
        return;
    }

    // 1. Remove old border items (one of each ID specified)
    // This simple loop removes one of each ID. If there were multiple old items of the same ID,
    // and newBorderItemIds also has that ID, this might not perfectly revert to the "exact" previous state
    // if specific instances mattered beyond just the set of IDs.
    for (uint16_t itemId : m_oldBorderItemIds) {
        // Assuming Tile::removeItemByID removes one instance of the item if found.
        // This is a simplified approach. A more robust way might need to identify
        // specific "border" items if they have a flag or are of a certain type.
        if (!m_tile->removeItemById(itemId)) { // Changed to removeItemById
            qDebug("SetBorderItemsCommand::redo: Did not find old border item ID %d to remove on tile %s.",
                   itemId, qUtf8Printable(m_tilePosition.toString()));
        }
    }

    // 2. Add new border items
    for (uint16_t itemId : m_newBorderItemIds) {
        auto newItem = RME::core::Item::create(itemId, m_assetManager);
        if (newItem) {
            m_tile->addItem(std::move(newItem));
        } else {
            qWarning("SetBorderItemsCommand::redo: Failed to create new border item with ID %d for tile %s.",
                     itemId, qUtf8Printable(m_tilePosition.toString()));
        }
    }
    m_controller->notifyTileChanged(m_tilePosition);
}

void SetBorderItemsCommand::undo() {
    if (!m_tile || !m_controller || !m_assetManager) {
        qWarning("SetBorderItemsCommand::undo: Tile, controller, or assetManager is null.");
        return;
    }

    // 1. Remove new border items (that were added in redo)
    for (uint16_t itemId : m_newBorderItemIds) {
        if (!m_tile->removeItemById(itemId)) { // Changed to removeItemById
             qDebug("SetBorderItemsCommand::undo: Did not find new border item ID %d to remove (that was added by redo) on tile %s.",
                    itemId, qUtf8Printable(m_tilePosition.toString()));
        }
    }

    // 2. Add old border items back
    for (uint16_t itemId : m_oldBorderItemIds) {
        auto oldItem = RME::core::Item::create(itemId, m_assetManager);
        if (oldItem) {
            m_tile->addItem(std::move(oldItem));
        } else {
            qWarning("SetBorderItemsCommand::undo: Failed to create old border item with ID %d to re-add on tile %s.",
                     itemId, qUtf8Printable(m_tilePosition.toString()));
        }
    }
    m_controller->notifyTileChanged(m_tilePosition);
}

} // namespace commands
} // namespace editor_logic
} // namespace RME
