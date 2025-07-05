#include "commands/BrushStrokeCommand.h"
#include "core/map/Map.h"        // RME::Map
#include "core/Tile.h"           // RME::Tile
#include "core/brush/Brush.h"    // RME::Brush
#include <QDebug>                // For qWarning (optional)
#include <QString>               // For QObject::tr and QString::fromStdString

namespace RME {
namespace core {
namespace actions {

BrushStrokeCommand::BrushStrokeCommand(
    RME::core::map::Map* map,
    RME::core::Brush* brush,
    const QList<RME::core::Position>& positions,
    const RME::core::BrushSettings& settings,
    bool isErase,
    RME::core::editor::EditorControllerInterface* controller,
    QUndoCommand* parent
) : BaseCommand(controller, QObject::tr("Brush Stroke"), parent),
    m_map(map),
    m_brush(brush),
    m_positions(positions),
    m_settings(settings),
    m_isErase(isErase) {
    // setText will be set in redo() the first time it's called
}

BrushStrokeCommand::~BrushStrokeCommand() {
    // m_originalTiles unique_ptrs will auto-delete.
}

void BrushStrokeCommand::undo() {
    if (!validateMembers() || !m_map) {
        setErrorText("undo brush stroke");
        return;
    }

    for (auto it = m_originalTiles.begin(); it != m_originalTiles.end(); ++it) {
        const RME::core::Position& pos = it.key();
        std::unique_ptr<RME::core::Tile>& originalTileState = it.value();

        if (m_createdTiles.contains(pos)) {
            // Tile was created by redo, so undo should remove it by making it null in map.
            // Tile was created by redo, so undo should remove it by making it null in map.
            m_map->setTile(pos, nullptr);
        } else if (originalTileState) {
            // Tile existed, restore its state.
            m_map->setTile(pos, std::move(originalTileState));
        } else {
            // Tile existed but was originally null (empty), and redo modified it.
            // This case could happen if an empty tile (nullptr in map) was painted on,
            // and redo stored nullptr as its original state. Undo should restore it to nullptr.
             m_map->setTile(pos, nullptr);
        }
        notifyMapChanged(pos);
    }
    // m_originalTiles now contains invalidated unique_ptrs (or nullptrs if they were taken). It will be repopulated by redo().
    // m_createdTiles also needs to be cleared as the state it tracked is undone.
    m_createdTiles.clear();
    // setText(QObject::tr("Undo %1").arg(m_isErase ? "Erase" : "Draw")); // Text is usually set by redo
}

void BrushStrokeCommand::redo() {
    if (!validateMembers() || !m_map || !m_brush) {
        setErrorText("redo brush stroke");
        return;
    }

    m_originalTiles.clear();
    m_createdTiles.clear();

    bool firstOp = true;
    for (const RME::core::Position& pos : m_positions) {
        bool tileWasJustCreatedByGetOrCreate = false;
        // getOrCreateTile might modify tileWasJustCreatedByGetOrCreate to true
        RME::core::Tile* tile = m_map->getOrCreateTile(pos, tileWasJustCreatedByGetOrCreate);

        if (!tile) {
            qWarning("BrushStrokeCommand::redo(): Failed to get or create tile at %d,%d,%d", pos.x, pos.y, pos.z);
            continue;
        }

        if (tileWasJustCreatedByGetOrCreate) {
            m_createdTiles.insert(pos);
            m_originalTiles.insert(pos, nullptr); // Original state was non-existent (nullptr)
        } else {
            // Tile existed, store a deep copy of its state BEFORE modification
            m_originalTiles.insert(pos, tile->deepCopy());
        }

        if (m_isErase) {
            m_brush->undraw(m_map, tile, &m_settings); // Pass settings to undraw as well
        } else {
            m_brush->draw(m_map, tile, &m_settings);
        }
        notifyMapChanged(pos);

        if (firstOp) {
             setText(QObject::tr("%1 %2").arg(m_isErase ? "Erase" : "Draw").arg(m_brush->getName()));
             firstOp = false;
        }
    }
    if (m_positions.isEmpty() && firstOp) {
         setText(QObject::tr("%1 (empty)").arg(m_isErase ? "Erase" : "Draw"));
    }
}

bool BrushStrokeCommand::mergeWith(const QUndoCommand* command) {
    if (command->id() != this->id()) {
        return false;
    }
    const BrushStrokeCommand* nextCommand = static_cast<const BrushStrokeCommand*>(command);

    // Check if brush settings are comparable. If BrushSettings doesn't have operator==,
    // this part of the condition needs to be removed or adapted.
    // For now, assuming BrushSettings has operator== implemented.
    if (m_brush == nextCommand->m_brush &&
        m_settings == nextCommand->m_settings &&
        m_isErase == nextCommand->m_isErase) {

        m_positions.append(nextCommand->m_positions);
        // setText(QObject::tr("%1 %2 (merged)").arg(m_isErase ? "Erase" : "Draw").arg(QString::fromStdString(m_brush->getName())));
        // The text will be updated by the next redo() call to reflect the full operation name.
        // QUndoStack usually calls redo() on the merged command.
        return true;
    }
    return false;
}

} // namespace actions
} // namespace core
} // namespace RME
