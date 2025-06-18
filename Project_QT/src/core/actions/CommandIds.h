#ifndef RME_COMMAND_IDS_H
#define RME_COMMAND_IDS_H

namespace RME {
namespace core {
namespace actions {

/**
 * @brief Centralized registry of command IDs for undo/redo system
 * 
 * These IDs are used by QUndoCommand::id() for command merging.
 * Commands with the same ID can potentially be merged together.
 */
enum class CommandId : int {
    // Base command (should not be used for merging)
    BaseCommand = 0,
    
    // Core tile operations
    TileChange = 100,
    TileModifyContents = 101,
    TileSetGround = 102,
    TileSetSpawn = 103,
    TileAddRemoveItem = 104,
    RecordSetSpawn = 105,
    RecordSetGround = 106,
    RecordModifyTileContents = 107,
    RecordAddRemoveItem = 108,
    
    // Brush operations
    BrushStroke = 200,
    
    // Selection operations
    BoundingBoxSelect = 300,
    ClearSelection = 301,
    DeleteSelection = 302,
    Delete = 303,
    
    // House operations
    SetHouseExit = 400,
    SetHouseTile = 401,
    CreateHouse = 402,
    RemoveHouse = 403,
    ModifyHouseProperties = 404,
    
    // Spawn operations
    AddSpawn = 500,
    RemoveSpawn = 501,
    UpdateSpawn = 502,
    
    // Creature operations
    AddCreature = 510,
    RemoveCreature = 511,
    
    // Map-wide operations
    MapWideOperation = 600,
    ImportMap = 601,
    ExportMinimap = 602,
    
    // Waypoint operations
    AddWaypoint = 700,
    MoveWaypoint = 701,
    RemoveWaypoint = 602,
    RenameWaypoint = 603,
    
    // Batch operations
    BatchCommand = 900,
    ChangeSet = 901
};

/**
 * @brief Convert CommandId enum to int for QUndoCommand::id()
 */
constexpr int toInt(CommandId id) {
    return static_cast<int>(id);
}

} // namespace actions
} // namespace core
} // namespace RME

#endif // RME_COMMAND_IDS_H