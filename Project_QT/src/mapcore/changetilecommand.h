#ifndef CHANGETILECOMMAND_H
#define CHANGETILECOMMAND_H

#include "appundocommand.h"
#include "position.h" // From CORE-01
#include "tile.h"     // From CORE-01
#include <memory>     // For std::unique_ptr
#include <QList> // For QList<Position> return type

class Map; // Forward declaration

class ChangeTileCommand : public AppUndoCommand
{
public:
    // ... constructor, destructor, undo, redo, id, mergeWith, static setters ...
    ChangeTileCommand(Map* map, const Position& pos, std::unique_ptr<Tile> new_tile_data, QUndoCommand *parent = nullptr);
    ~ChangeTileCommand() override;

    void undo() override;
    void redo() override;

    int id() const override; // Used by QUndoStack for potential merging
    bool mergeWith(const QUndoCommand *other) override;

    QList<Position> getChangedPositions() const override;

    static void setGroupActions(bool enabled);
    static void setStackingDelay(int ms);

private:
    // ... existing members ...
    Position m_position;
    std::unique_ptr<Tile> m_new_tile_data; // The tile state to be applied on redo
    std::unique_ptr<Tile> m_old_tile_data; // The tile state that was on the map before redo

    bool m_first_execution; // To handle storing old_tile_data only on first redo

    // Static members for merging configuration
    static bool s_group_actions_enabled;
    static int s_stacking_delay_ms;
};

#endif // CHANGETILECOMMAND_H
