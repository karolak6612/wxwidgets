#ifndef RME_RECORDMODIFYTILECONTENTSCOMMAND_H
#define RME_RECORDMODIFYTILECONTENTSCOMMAND_H

#include <QUndoCommand>
#include <memory>      // For std::unique_ptr
#include <vector>      // For std::vector
#include <QString>     // For command text

#include "core/Position.h" // For RME::core::Position

// Forward declarations
namespace RME {
namespace core {
    class Item;
    class Tile;
    class Spawn;
    class Creature;
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RecordModifyTileContentsCommandId = toInt(CommandId::RecordModifyTileContents);

class RecordModifyTileContentsCommand : public QUndoCommand {
public:
    RecordModifyTileContentsCommand(
        RME::core::Tile* tile,
        RME::core::editor::EditorControllerInterface* controller,
        // Data that was on the tile *before* the brush cleared it
        std::unique_ptr<RME::core::Item> previouslyExistingGround,
        std::vector<std::unique_ptr<RME::core::Item>> previouslyExistingItems,
        std::unique_ptr<RME::core::Spawn> previouslyExistingSpawn,
        std::unique_ptr<RME::core::Creature> previouslyExistingCreature,
        QUndoCommand* parent = nullptr
    );

    ~RecordModifyTileContentsCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RecordModifyTileContentsCommandId; }

    // Getters for testing state captured by command
    const RME::core::Item* getStoredOldGround() const { return m_undoneGround.get(); }
    const std::vector<std::unique_ptr<RME::core::Item>>& getStoredOldItems() const { return m_undoneItems; }
    const RME::core::Spawn* getStoredOldSpawn() const { return m_undoneSpawn.get(); }
    const RME::core::Creature* getStoredOldCreature() const { return m_undoneCreature.get(); }

private:
    RME::core::Tile* m_tile;
    RME::core::editor::EditorControllerInterface* m_controller;
    RME::core::Position m_tilePosition;

    // These store the state of things that were cleared by the brush's action
    std::unique_ptr<RME::core::Item> m_undoneGround;      // Ground that was cleared
    std::vector<std::unique_ptr<RME::core::Item>> m_undoneItems; // Items that were cleared
    std::unique_ptr<RME::core::Spawn> m_undoneSpawn;      // Spawn that was cleared
    std::unique_ptr<RME::core::Creature> m_undoneCreature;  // Creature that was cleared

    // Flags to indicate if the brush intended to clear these elements.
    // Set in constructor based on whether the unique_ptrs/vector are populated.
    bool m_didClearGround;
    bool m_didClearItems; // True if m_undoneItems is not empty
    bool m_didClearSpawn;
    bool m_didClearCreature;

    QString m_commandText;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_RECORDMODIFYTILECONTENTSCOMMAND_H
