# CMakeLists.txt for the core library of RME-Qt6

# Define the library sources
set(RME_CORE_LIB_SOURCES
    # CORE-01 files
    Position.cpp
    Item.cpp          # Uses IItemTypeProvider.h
    Creature.cpp      # Stub
    Spawn.cpp         # Stub
    Tile.cpp          # Uses Item.h, Creature.h, Spawn.h, Position.h

    # CORE-02 asset loading files
    assets/ClientVersionManager.cpp
    assets/ItemDatabase.cpp
    assets/CreatureDatabase.cpp
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

    # LOGIC-04 Waypoint System files
    waypoints/WaypointManager.cpp

    # NET-01 Network Protocol & IO Handlers
    network/NetworkMessage.cpp
    network/MapProtocolCodec.cpp
    io/BinaryNode.cpp
    io/NodeFileReadHandle.cpp
    io/MemoryNodeFileReadHandle.cpp
    io/NodeFileWriteHandle.cpp
    io/MemoryNodeFileWriteHandle.cpp
)

add_library(rme_core_lib STATIC ${RME_CORE_LIB_SOURCES})

# Public include directories for this library
# This allows other targets linking against rme_core_lib to find its headers
# e.g., #include "Position.h", #include "assets/AssetManager.h", #include "settings/AppSettings.h"
target_include_directories(rme_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/core/ and its subdirectories
)

# Link against Qt6 Core as QSettings, QXmlStreamReader, QFile, QDataStream, QString, QList, QMap, QVariant etc. are used.
# QVector is also in Core. QDebug is in Core.
target_link_libraries(rme_core_lib PRIVATE Qt6::Core)

# Add other dependencies if necessary
