#ifndef RME_BOUNDINGBOXSELECTCOMMAND_H
#define RME_BOUNDINGBOXSELECTCOMMAND_H

#include "BaseCommand.h"
#include <QList>
#include <QSet> // For efficient unique union
#include <QString>     // For command text
#include "core/actions/CommandIds.h"

// Forward declarations
namespace RME {
namespace core {
    class Tile;
    namespace selection { class SelectionManager; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int BoundingBoxSelectCommandId = toInt(CommandId::BoundingBoxSelect);

class BoundingBoxSelectCommand : public BaseCommand {
public:
    BoundingBoxSelectCommand(
        RME::core::selection::SelectionManager* selectionManager,
        const QList<RME::core::Tile*>& calculatedTilesInBox,
        bool isAdditive, // True if Ctrl was held (add to current selection)
        const QList<RME::core::Tile*>& selectionStateBeforeThisCommand,
        QUndoCommand* parent = nullptr
    );

    ~BoundingBoxSelectCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return BoundingBoxSelectCommandId; }
    // bool mergeWith(const QUndoCommand *other) override; // Optional

    // For testing
    const QList<RME::core::Tile*>& getSelectionStateBefore() const { return m_selectionStateBeforeCommand; }
    const QList<RME::core::Tile*>& getSelectionStateAfter() const { return m_selectionStateAfterCommand; }
    const QList<RME::core::Tile*>& getCalculatedTilesInBox() const { return m_calculatedTilesInBox; }
    bool getIsAdditive() const { return m_isAdditive; }

private:
    RME::core::selection::SelectionManager* m_selectionManager;
    QList<RME::core::Tile*> m_calculatedTilesInBox; // Tiles identified within the drawn rectangle
    bool m_isAdditive;

    QList<RME::core::Tile*> m_selectionStateBeforeCommand; // Full list of selected tiles before this command ran
    QList<RME::core::Tile*> m_selectionStateAfterCommand;  // Full list of selected tiles after this command's redo

    bool m_firstRun;
};

} // namespace actions
} // namespace core
} // namespace RME
#endif // RME_BOUNDINGBOXSELECTCOMMAND_H
