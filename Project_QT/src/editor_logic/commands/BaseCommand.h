#ifndef RME_EDITOR_LOGIC_COMMANDS_BASECOMMAND_H
#define RME_EDITOR_LOGIC_COMMANDS_BASECOMMAND_H

#include <QUndoCommand>
#include <QString>
#include <QDebug>
#include "core/Position.h"

namespace RME {
namespace core {
    class EditorControllerInterface;
}
}

namespace RME {
namespace editor_logic {
namespace commands {

/**
 * @brief Base class for all editor commands to eliminate duplicate code
 * 
 * This class provides common functionality that was duplicated across
 * 40+ command files, including:
 * - Common constructor patterns
 * - Validation patterns
 * - Map notification patterns
 * - Debug logging patterns
 * - Text generation patterns
 * 
 * By inheriting from this class, command implementations can focus on
 * their specific logic while getting common functionality for free.
 */
class BaseCommand : public QUndoCommand {
public:
    /**
     * @brief Construct a new Base Command object
     * 
     * @param controller Editor controller interface (must not be null)
     * @param description Command description for undo/redo text
     * @param parent Parent command for command grouping
     */
    explicit BaseCommand(
        RME::core::EditorControllerInterface* controller,
        const QString& description = QString(),
        QUndoCommand* parent = nullptr
    );

protected:
    /**
     * @brief Validate that required members are not null
     * 
     * @return true if all required members are valid
     * @return false if any required member is null
     */
    bool validateMembers() const;
    
    /**
     * @brief Notify the map that a tile has changed
     * 
     * @param position Position of the changed tile
     */
    void notifyMapChanged(const Position& position);
    
    /**
     * @brief Log a redo operation for debugging
     * 
     * @param action Description of the action being performed
     * @param position Position where the action is being performed
     * @param additionalInfo Additional information to log (optional)
     */
    void logRedo(const QString& action, const Position& position, const QString& additionalInfo = QString());
    
    /**
     * @brief Log an undo operation for debugging
     * 
     * @param action Description of the action being undone
     * @param position Position where the action is being undone
     */
    void logUndo(const QString& action, const Position& position);
    
    /**
     * @brief Set the undo text for this command
     * 
     * @param originalAction Description of the original action
     * @param position Position where the action was performed
     */
    void setUndoText(const QString& originalAction, const Position& position);
    
    /**
     * @brief Set error text when command fails
     * 
     * @param operation Description of the failed operation
     */
    void setErrorText(const QString& operation);
    
    /**
     * @brief Get the editor controller
     * 
     * @return RME::core::EditorControllerInterface* Controller interface
     */
    RME::core::EditorControllerInterface* getController() const { return m_controller; }

private:
    RME::core::EditorControllerInterface* m_controller;
};

} // namespace commands
} // namespace editor_logic
} // namespace RME

#endif // RME_EDITOR_LOGIC_COMMANDS_BASECOMMAND_H