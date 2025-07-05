#include "BaseCommand.h"
#include "core/editor/EditorControllerInterface.h"
#include <QObject>

namespace RME {
namespace editor_logic {
namespace commands {

BaseCommand::BaseCommand(
    RME::core::EditorControllerInterface* controller,
    const QString& description,
    QUndoCommand* parent
) : QUndoCommand(parent)
  , m_controller(controller)
{
    if (!m_controller) {
        qWarning("BaseCommand: Initialization with null controller.");
        setText("Invalid Command");
        return;
    }
    
    if (!description.isEmpty()) {
        setText(description);
    }
}

bool BaseCommand::validateMembers() const {
    if (!m_controller) {
        qWarning("BaseCommand::validateMembers: Controller is null.");
        return false;
    }
    
    return true;
}

void BaseCommand::notifyMapChanged(const Position& position) {
    if (!validateMembers()) {
        return;
    }
    
    auto map = m_controller->getMap();
    if (map) {
        map->notifyTileChanged(position);
        map->setChanged(true);
    }
}

void BaseCommand::logRedo(const QString& action, const Position& position, const QString& additionalInfo) {
    QString logMessage = QString("%1 at %2").arg(action, position.toString());
    
    if (!additionalInfo.isEmpty()) {
        logMessage += QString(" - %1").arg(additionalInfo);
    }
    
    qDebug() << QString("%1::redo:").arg(metaObject()->className()) << logMessage;
}

void BaseCommand::logUndo(const QString& action, const Position& position) {
    qDebug() << QString("%1::undo:").arg(metaObject()->className()) 
             << QString("Undoing %1 at %2").arg(action, position.toString());
}

void BaseCommand::setUndoText(const QString& originalAction, const Position& position) {
    setText(QObject::tr("Undo: %1 at (%2, %3, %4)")
                .arg(originalAction)
                .arg(position.x())
                .arg(position.y())
                .arg(position.z()));
}

void BaseCommand::setErrorText(const QString& operation) {
    setText(QObject::tr("%1 (Error)").arg(operation));
}

} // namespace commands
} // namespace editor_logic
} // namespace RME