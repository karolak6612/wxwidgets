#include "actions/changetilecommand.h"
#include "map/map.h"
#include <QDebug> // For warnings/info
#include "settings/AppSettings.h" // Assuming a global settings provider (from CORE-06)

// Static member initialization
bool ChangeTileCommand::s_group_actions_enabled = false; // Default: disabled
int ChangeTileCommand::s_stacking_delay_ms = 1000;   // Default: 1 second

// Assuming Tile has a copy constructor: Tile(const Tile& other);
// Assuming Map has:
//   const Tile* Map::getTile(const Position& pos) const;
//   void Map::setTile(const Position& pos, std::unique_ptr<Tile> tile);

ChangeTileCommand::ChangeTileCommand(Map* map, const Position& pos, std::unique_ptr<Tile> new_tile_data, QUndoCommand *parent)
    : AppUndoCommand(map, parent),
      m_position(pos),
      m_new_tile_data(std::move(new_tile_data)),
      m_first_execution(true)
{
    // Fetch initial settings values when the first command is created or on demand
    // This is a simplified approach; typically this would be part of an app initialization
    // or fetched from a settings manager instance.
    // For now, let's assume they are updated externally via the static setters.
    // Settings* settings = Settings::getInstance(); // Example
    // if (settings) {
    //     s_group_actions_enabled = settings->getBool("Editor/GroupActions", false);
    //     s_stacking_delay_ms = settings->getInt("Editor/StackingDelayMs", 1000);
    // }


    if (m_new_tile_data) {
        setText(QString("Change tile at (%1, %2, %3)").arg(pos.x()).arg(pos.y()).arg(pos.z()));
    } else {
        setText(QString("Clear tile at (%1, %2, %3)").arg(pos.x()).arg(pos.y()).arg(pos.z()));
    }
}

ChangeTileCommand::~ChangeTileCommand() { }

void ChangeTileCommand::setGroupActions(bool enabled) {
    s_group_actions_enabled = enabled;
}

void ChangeTileCommand::setStackingDelay(int ms) {
    s_stacking_delay_ms = ms;
}

// Define a unique ID for this command type, used by QUndoStack
int ChangeTileCommand::id() const {
    // A common way to generate IDs is to use an enum or a sequence.
    // For simplicity, returning a constant.
    // Ensure this ID is unique among commands that can be merged or need distinction.
    return 1001;
}

bool ChangeTileCommand::mergeWith(const QUndoCommand *other) {
    if (!s_group_actions_enabled) {
        return false;
    }

    // QUndoStack calls this->mergeWith(other) if this->id() == other->id().
    // So, we don't need to check id() here again if it's positive and same.
    // If id() is -1 for both, it also calls mergeWith.
    const ChangeTileCommand *otherCmd = dynamic_cast<const ChangeTileCommand *>(other);
    if (!otherCmd) {
        return false; // Not a ChangeTileCommand
    }

    // Check if the command is for the same position
    if (otherCmd->m_position != this->m_position) {
        return false;
    }

    // Check stacking delay. 'this' is the older command, 'other' is the newer one.
    if ((otherCmd->creationTimestamp() - this->creationTimestamp()) >= s_stacking_delay_ms) {
        return false; // Too much time has passed
    }

    // If all conditions met, merge 'otherCmd' into 'this'.
    // The 'm_old_tile_data' of 'this' (the first command in a potential series) is the one that matters.
    // The 'm_new_tile_data' of 'this' should become the 'm_new_tile_data' of 'otherCmd'.
    if (otherCmd->m_new_tile_data) {
        this->m_new_tile_data = std::make_unique<Tile>(*otherCmd->m_new_tile_data);
    } else {
        this->m_new_tile_data.reset();
    }

    // Update text to reflect it's a merged command, if desired.
    setText(QString("Update tile at (%1, %2, %3) (merged)").arg(m_position.x()).arg(m_position.y()).arg(m_position.z()));

    // qInfo() << "Merged ChangeTileCommand at" << this->m_position;
    return true;
}

QList<Position> ChangeTileCommand::getChangedPositions() const
{
    return {m_position}; // Return a list containing the single position that changed.
}

// undo() and redo() implementations use standard tile state restoration
// (Copied from previous step for completeness if running this subtask standalone)
void ChangeTileCommand::undo()
{
    if (!m_map) {
        qWarning("ChangeTileCommand::undo(): Map is null.");
        return;
    }
    const Tile* current_tile_on_map = m_map->getTile(m_position);
    std::unique_ptr<Tile> temp_tile_for_redo;
    if (current_tile_on_map) {
        temp_tile_for_redo = std::make_unique<Tile>(*current_tile_on_map);
    }
    if (m_old_tile_data) {
        m_map->setTile(m_position, std::make_unique<Tile>(*m_old_tile_data));
    } else {
        m_map->setTile(m_position, nullptr);
    }
    m_new_tile_data = std::move(temp_tile_for_redo);
}

void ChangeTileCommand::redo()
{
    if (!m_map) {
        qWarning("ChangeTileCommand::redo(): Map is null.");
        return;
    }
    if (m_first_execution) {
        const Tile* tile_currently_on_map = m_map->getTile(m_position);
        if (tile_currently_on_map) {
            m_old_tile_data = std::make_unique<Tile>(*tile_currently_on_map);
        } else {
            m_old_tile_data.reset();
        }
        m_first_execution = false;
    } else {
        const Tile* tile_currently_on_map = m_map->getTile(m_position);
        if (tile_currently_on_map) {
            m_old_tile_data = std::make_unique<Tile>(*tile_currently_on_map);
        } else {
            m_old_tile_data.reset();
        }
    }
    if (m_new_tile_data) {
        m_map->setTile(m_position, std::make_unique<Tile>(*m_new_tile_data));
    } else {
        m_map->setTile(m_position, nullptr);
    }
}
