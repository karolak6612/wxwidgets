#ifndef RME_CREATEHOUSECOMMAND_H
#define RME_CREATEHOUSECOMMAND_H

#include "BaseCommand.h"
#include <QString>
#include "core/Position.h"
#include "core/actions/CommandIds.h"
#include <QtGlobal> // For quint32

// Forward declarations
namespace RME {
namespace core {
    namespace houses { class Houses; class HouseData; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace actions {

constexpr int CreateHouseCommandId = toInt(CommandId::CreateHouse);

class CreateHouseCommand : public BaseCommand {
public:
    CreateHouseCommand(
        const QString& houseName,
        const RME::core::Position& entryPoint,
        quint32 townId,
        quint32 rent,
        bool isGuildhall,
        RME::core::houses::Houses* housesManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~CreateHouseCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return CreateHouseCommandId; }
    
    // Get the created house ID (valid after first redo)
    quint32 getCreatedHouseId() const { return m_createdHouseId; }

private:
    QString m_houseName;
    RME::core::Position m_entryPoint;
    quint32 m_townId;
    quint32 m_rent;
    bool m_isGuildhall;
    
    RME::core::houses::Houses* m_housesManager;
    RME::core::editor::EditorControllerInterface* m_controller;
    
    // State tracking
    quint32 m_createdHouseId = 0; // Set during first redo
    bool m_hasBeenExecuted = false;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_CREATEHOUSECOMMAND_H