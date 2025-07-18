# CMakeLists.txt for the core library of RME-Qt6

# Define the library sources
set(RME_CORE_LIB_SOURCES
    # CORE-01 files
    Position.cpp
    Item.cpp          # Uses IItemTypeProvider.h
    ItemTypeProvider.cpp  # Concrete implementation of IItemTypeProvider
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
    
    # Utility files
    utils/ResourcePathManager.cpp
    utils/ProgressTracker.cpp
    map/Map.cpp

    # CORE-06 settings system files
    settings/AppSettings.cpp

    # CORE-BRUSH-FRAMEWORK files
    brush/Brush.cpp
    brush/BrushManagerService.cpp
    
    # Service architecture files
    services/ClientDataService.cpp
    services/WindowManagerService.cpp
    services/ApplicationSettingsService.cpp
    services/ServiceContainer.cpp
    services/BrushPaletteService.cpp
    services/BrushStateService.cpp
    services/EditorStateService.cpp
    services/LightCalculatorService.cpp

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
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/House.h     # Added
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/Houses.h    # Added
)

set(CORE_HOUSES_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/HouseData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/House.cpp     # Added
    ${CMAKE_CURRENT_SOURCE_DIR}/houses/Houses.cpp    # Added
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_HOUSES_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_HOUSES_HDRS})

# Spawn System
set(CORE_SPAWNS_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/spawns/Spawn.h
)

set(CORE_SPAWNS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/spawns/Spawn.cpp
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

# Data Transfer Objects
set(CORE_DATA_TRANSFER_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/data_transfer/TileData.h
)
set(CORE_DATA_TRANSFER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/data_transfer/TileData.cpp
)
list(APPEND RME_CORE_LIB_SOURCES ${CORE_DATA_TRANSFER_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_DATA_TRANSFER_HDRS})

# Waypoint System (LOGIC-04 definitions, distinct from navigation/WaypointData.cpp)
set(CORE_WAYPOINTS_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/waypoints/Waypoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/waypoints/WaypointManager.h
)
# WaypointManager.cpp is already in RME_CORE_LIB_SOURCES
list(APPEND RME_CORE_LIB_HEADERS ${CORE_WAYPOINTS_HDRS})

# Specific Brush Logics
set(CORE_BRUSHES_LOGIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/RawBrush.h
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/EraserBrush.h
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/HouseBrush.h # New line
    # Add other specific brush headers here in future tasks
)

set(CORE_BRUSHES_LOGIC_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/RawBrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/EraserBrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/brush/HouseBrush.cpp # New line
    # Add other specific brush sources here in future tasks
)

list(APPEND RME_CORE_LIB_SOURCES ${CORE_BRUSHES_LOGIC_SRCS})
list(APPEND RME_CORE_LIB_HEADERS ${CORE_BRUSHES_LOGIC_HDRS}) # Assuming RME_CORE_LIB_HEADERS is used

add_library(rme_core_lib STATIC ${RME_CORE_LIB_SOURCES})

# Public include directories for this library
target_include_directories(rme_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/core/ and its subdirectories
)

# Add subdirectories for components with their own CMakeLists.txt
add_subdirectory(lighting) # RENDER-04 Lighting System

# Link against Qt6 Core and Widgets
target_link_libraries(rme_core_lib PRIVATE
    Qt6::Core
    Qt6::Widgets
)
