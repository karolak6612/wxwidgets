#ifndef RME_SETBORDERITEMSCOMMAND_H
#define RME_SETBORDERITEMSCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h"
#include <QList>
#include <cstdint> // For uint16_t

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    // class Item; // Not directly stored as full objects, only IDs
    namespace editor {
        class EditorControllerInterface;
    }
    namespace assets {
        class AssetManager; // For creating items
    }
} // namespace core

namespace editor_logic {
namespace commands {

class SetBorderItemsCommand : public QUndoCommand {
public:
    SetBorderItemsCommand(
        RME::core::Tile* tile,
        const QList<uint16_t>& oldBorderItemIds,
        const QList<uint16_t>& newBorderItemIds,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~SetBorderItemsCommand() override = default;

    void undo() override;
    void redo() override;

private:
    RME::core::Tile* m_tile;
    QList<uint16_t> m_oldBorderItemIds;
    QList<uint16_t> m_newBorderItemIds;
    RME::core::editor::EditorControllerInterface* m_controller;
    RME::core::assets::AssetManager* m_assetManager; // To create items from IDs
    RME::core::Position m_tilePosition;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME
#endif // RME_SETBORDERITEMSCOMMAND_H
