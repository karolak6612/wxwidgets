#ifndef RME_REMOVEHOUSECOMMAND_H
#define RME_REMOVEHOUSECOMMAND_H

#include <QUndoCommand>
#include <QString>
#include <QList>
#include "core/Position.h"
#include "core/actions/CommandIds.h"
#include "core/houses/HouseData.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    namespace houses { class Houses; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int RemoveHouseCommandId = toInt(CommandId::RemoveHouse);

class RemoveHouseCommand : public QUndoCommand {
public:
    RemoveHouseCommand(
        quint32 houseId,
        RME::core::houses::Houses* housesManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~RemoveHouseCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return RemoveHouseCommandId; }

private:
    quint32 m_houseId;
    RME::core::houses::Houses* m_housesManager;
    RME::core::editor::EditorControllerInterface* m_controller;
    
    // State backup for undo
    RME::core::houses::HouseData m_backupHouseData;
    QList<RME::core::Position> m_backupTilePositions;
    bool m_hasBackup = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_REMOVEHOUSECOMMAND_H