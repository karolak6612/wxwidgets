#ifndef APPUNDOCOMMAND_H
#define APPUNDOCOMMAND_H

#include <QUndoCommand>
// #include <QObject> // Required for signals/slots - Not using QObject for commands now
#include <QDateTime>
#include <QList>   // For QList<Position>
#include "Position.h" // Ensure Position is known

// Forward declaration
class Map;

/**
 * @brief Base class for all application-specific undo commands.
 *
 * AppUndoCommand extends QUndoCommand to provide common functionalities
 * for commands within Remere's Map Editor, such as access to the Map object
 * and a creation timestamp.
 */
class AppUndoCommand : public QUndoCommand // No QObject inheritance here for simplicity now
{
public:
    /**
     * @brief Constructs an AppUndoCommand.
     * @param map Pointer to the main Map object. Commands interact with this map. Non-owning.
     * @param parent Optional parent QUndoCommand for macro commands.
     */
    explicit AppUndoCommand(Map* map, QUndoCommand *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~AppUndoCommand() override;

    /**
     * @brief Gets the Map object associated with this command.
     * @return Pointer to the Map object.
     */
    Map* getMap() const;

    /**
     * @brief Gets the creation timestamp of this command.
     * @return Timestamp in milliseconds since epoch when the command was created.
     */
    qint64 creationTimestamp() const;

    /**
     * @brief Returns an ID for the command type.
     * Used by QUndoStack to determine if commands can be merged.
     * Default is -1 (not mergeable by type unless other also returns -1).
     * @return Integer ID for the command type.
     */
    int id() const override { return -1; }

    /**
     * @brief Retrieves a list of map positions affected by this command.
     * This method should be overridden by concrete commands to report the specific
     * areas they modified, allowing for optimized UI updates.
     * @return A QList of Position objects. Default implementation returns an empty list.
     */
    virtual QList<Position> getChangedPositions() const;

protected:
    Map* m_map; ///< Non-owning pointer to the map data.
    qint64 m_creation_timestamp; ///< Timestamp of command creation.
};

#endif // APPUNDOCOMMAND_H
