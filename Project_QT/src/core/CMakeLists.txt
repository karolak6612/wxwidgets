# CMakeLists.txt for the core library of RME-Qt6

# Define the library sources
set(RME_CORE_LIB_SOURCES
    # CORE-01 files
    Position.cpp
    Item.cpp          # Uses IItemTypeProvider.h
    Spawn.cpp         # Stub
    Tile.cpp          # Uses Item.h, Creature.h, Spawn.h, Position.h

    # CORE-08 files
    creatures/Creature.cpp

    # CORE-11 files
    navigation/WaypointData.cpp # Added for CORE-11

    # CORE-02 asset loading files
    assets/ClientVersionManager.cpp
    assets/ItemDatabase.cpp
    assets/CreatureDatabase.cpp
    assets/AssetManager.cpp         # Uses all managers, IItemTypeProvider.h
    assets/MaterialManager.cpp      # Manages material definitions from XML
    sprites/SpriteManager.cpp

    # CORE-03 map structure files
    map/Floor.cpp
    map/QTreeNode.cpp
    map/BaseMap.cpp
    map/MapIterator.cpp
    map/Map.cpp

    # CORE-06 settings system files
    settings/AppSettings.cpp

    # CORE-BRUSH-FRAMEWORK files
    brush/Brush.cpp
    brush/BrushManagerService.cpp

    # LOGIC-04 Waypoint System files (This is WaypointManager, distinct from WaypointData)
    waypoints/WaypointManager.cpp

    # IO Handlers (from CORE-07)
    io/BinaryNode.cpp
    io/NodeFileReadHandle.cpp
    io/MemoryNodeFileReadHandle.cpp
    io/NodeFileWriteHandle.cpp
    io/MemoryNodeFileWriteHandle.cpp
    io/DiskNodeFileReadHandle.cpp
    io/DiskNodeFileWriteHandle.cpp
    io/OtbmMapIO.cpp

    # CORE-04 Action & History System
    actions/appundocommand.cpp # Note: These might be misnamed based on previous CORE-04 work (AppUndoCommand.cpp etc)
    actions/changetilecommand.cpp
    actions/batchcommand.cpp

    # Network files that were part of original RME_CORE_LIB_SOURCES, likely from NET-01/02 if they touched these
    # These might belong to a separate network library later.
    network/NetworkMessage.cpp
    network/MapProtocolCodec.cpp
)

add_library(rme_core_lib STATIC ${RME_CORE_LIB_SOURCES})

# Public include directories for this library
target_include_directories(rme_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/core/ and its subdirectories
)

# Link against Qt6 Core and Widgets
target_link_libraries(rme_core_lib PRIVATE
    Qt6::Core
    Qt6::Widgets
)
