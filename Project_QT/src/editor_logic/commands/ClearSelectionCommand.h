#ifndef RME_CLEARSELECTIONCOMMAND_H
#define RME_CLEARSELECTIONCOMMAND_H

#include "BaseCommand.h"
#include <QList>
#include <memory>      // For std::unique_ptr if Tile objects were owned, but here raw Tile*
#include <QString>     // For command text
#include "core/actions/CommandIds.h"

// Forward declarations
namespace RME {
namespace core {
    class Tile; // Used in QList<RME::core::Tile*>
    namespace selection { class SelectionManager; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int ClearSelectionCommandId = toInt(CommandId::ClearSelection);

class ClearSelectionCommand : public BaseCommand {
public:
    ClearSelectionCommand(
        RME::core::selection::SelectionManager* selectionManager,
        QUndoCommand* parent = nullptr
    );

    ~ClearSelectionCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return ClearSelectionCommandId; }
    // bool mergeWith(const QUndoCommand *other) override; // Optional, can be false for now

    // For testing: to check what was captured
    const QList<RME::core::Tile*>& getOldSelectedTiles() const { return m_oldSelectedTiles; }

private:
    RME::core::selection::SelectionManager* m_selectionManager;
    QList<RME::core::Tile*> m_oldSelectedTiles; // Stores pointers to tiles that were selected
                                                // Assumes Tile objects have stable lifetime managed by Map
    bool m_hadSelectionToClear; // To optimize text and potentially redo/undo calls
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_CLEARSELECTIONCOMMAND_H
