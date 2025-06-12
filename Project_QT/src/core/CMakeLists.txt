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
    navigation/WaypointData.cpp

    # CORE-02 & CORE-14 asset loading files
    assets/ClientVersionManager.cpp
    assets/ItemDatabase.cpp
    assets/CreatureDatabase.cpp
    assets/MaterialData.cpp         # Added for CORE-14
    assets/MaterialManager.cpp      # Manages material definitions from XML
    assets/AssetManager.cpp         # Uses all managers, IItemTypeProvider.h
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
    actions/appundocommand.cpp
    actions/changetilecommand.cpp
    actions/batchcommand.cpp

    # Network files that were part of original RME_CORE_LIB_SOURCES
    network/NetworkMessage.cpp
    network/MapProtocolCodec.cpp
)

# Selection system
set(CORE_SELECTION_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/selection/SelectionManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/selection/SelectionCommand.h
)

set(CORE_SELECTION_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/selection/SelectionManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/selection/SelectionCommand.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_SELECTION_SRCS})
# Headers are made available via target_include_directories for CORE_SELECTION_HDRS.

# Clipboard system
set(CORE_CLIPBOARD_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/ClipboardData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/ClipboardManager.h
)

set(CORE_CLIPBOARD_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/ClipboardData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/ClipboardManager.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_CLIPBOARD_SRCS})
# Define RME_CORE_LIB_HEADERS if it's intended to be a comprehensive list, otherwise headers are found via include_directories
set(RME_CORE_LIB_HEADERS ${CORE_SELECTION_HDRS} ${CORE_CLIPBOARD_HDRS})
# Note: RME_CORE_LIB_HEADERS is defined here but not explicitly used by add_library or target_sources.
# It's good for organization or if other CMake scripts consume it.
# Primary mechanism for header availability is target_include_directories.

# Actions (Undo/Redo Commands)
set(CORE_ACTIONS_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/appundocommand.h # Already existed, ensure it's part of this logical group
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/changetilecommand.h # Already existed
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/batchcommand.h # Already existed
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/DeleteCommand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/PasteCommand.h
)

set(CORE_ACTIONS_SRCS
    # appundocommand.cpp, changetilecommand.cpp, batchcommand.cpp are already in RME_CORE_LIB_SOURCES directly
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/DeleteCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/PasteCommand.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_ACTIONS_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_ACTIONS_HDRS}) # Add new action headers to the list

# House System
set(CORE_HOUSES_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/HouseData.h
)

set(CORE_HOUSES_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/HouseData.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_HOUSES_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_HOUSES_HDRS})

# Spawn System
set(CORE_SPAWNS_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/spawns/SpawnData.h
)

set(CORE_SPAWNS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/spawns/SpawnData.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_SPAWNS_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_SPAWNS_HDRS})

# Derived Item System (Complex Items)
set(CORE_ITEMS_DERIVED_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/items/ContainerItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/items/TeleportItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/items/DoorItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/items/DepotItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/items/PodiumItem.h
)

set(CORE_ITEMS_DERIVED_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/items/ContainerItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/TeleportItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/DoorItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/DepotItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/PodiumItem.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_ITEMS_DERIVED_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_ITEMS_DERIVED_HDRS})

# Town System (World Data)
set(CORE_WORLD_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/world/TownData.h
)

set(CORE_WORLD_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/world/TownData.cpp
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_WORLD_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_WORLD_HDRS})

# Specific Brush Logics
set(CORE_BRUSHES_LOGIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/RawBrush.h
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/EraserBrush.h # New line
    # Add other specific brush headers here in future tasks
)

set(CORE_BRUSHES_LOGIC_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/RawBrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/EraserBrush.cpp # New line
    # Add other specific brush sources here in future tasks
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_BRUSHES_LOGIC_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_BRUSHES_LOGIC_HDRS}) # Assuming RME_CORE_LIB_HEADERS is used

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
