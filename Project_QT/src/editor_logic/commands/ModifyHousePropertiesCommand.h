#ifndef RME_MODIFYHOUSEPROPERTIESCOMMAND_H
#define RME_MODIFYHOUSEPROPERTIESCOMMAND_H

#include <QUndoCommand>
#include <QString>
#include <QVariant>
#include <QHash>
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

constexpr int ModifyHousePropertiesCommandId = toInt(CommandId::ModifyHouseProperties);

class ModifyHousePropertiesCommand : public QUndoCommand {
public:
    ModifyHousePropertiesCommand(
        quint32 houseId,
        const QHash<QString, QVariant>& newProperties,
        RME::core::houses::Houses* housesManager,
        RME::core::editor::EditorControllerInterface* controller,
        QUndoCommand* parent = nullptr
    );

    ~ModifyHousePropertiesCommand() override = default;

    void undo() override;
    void redo() override;

    int id() const override { return ModifyHousePropertiesCommandId; }
    
    // For merging consecutive property changes
    bool mergeWith(const QUndoCommand* other) override;

private:
    quint32 m_houseId;
    QHash<QString, QVariant> m_newProperties;
    QHash<QString, QVariant> m_oldProperties;
    
    RME::core::houses::Houses* m_housesManager;
    RME::core::editor::EditorControllerInterface* m_controller;
    
    bool m_hasBackup = false;
    
    // Helper methods
    void backupCurrentProperties();
    void applyProperties(const QHash<QString, QVariant>& properties);
    QString generateCommandText() const;
};

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_MODIFYHOUSEPROPERTIESCOMMAND_H