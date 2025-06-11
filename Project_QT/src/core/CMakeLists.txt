# CMakeLists.txt for the core library of RME-Qt6

# Define the library sources
set(RME_CORE_LIB_SOURCES
    # CORE-01 files
    Position.cpp
    Item.cpp          # Uses IItemTypeProvider.h
    # Creature.cpp from CORE-01 was a stub or basic version, actual in creatures subdir.
    # That top-level core/Creature.cpp might need to be removed or reconciled if it exists and conflicts.
    Spawn.cpp         # Stub
    Tile.cpp          # Uses Item.h, Creature.h, Spawn.h, Position.h

    # CORE-08 files
    creatures/Creature.cpp # Added for CORE-08

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

    # LOGIC-04 Waypoint System files
    waypoints/WaypointManager.cpp

    # NET-01 Network Protocol & IO Handlers (from CORE-07)
    network/NetworkMessage.cpp
    network/MapProtocolCodec.cpp
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
)

add_library(rme_core_lib STATIC ${RME_CORE_LIB_SOURCES})

# Public include directories for this library
# This allows other targets linking against rme_core_lib to find its headers
# e.g., #include "Position.h", #include "assets/AssetManager.h", #include "settings/AppSettings.h"
# It implicitly adds the current source dir and its subdirectories if headers are structured like core/subdir/header.h
target_include_directories(rme_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/core/ and its subdirectories like core/creatures/
)

# Link against Qt6 Core as QSettings, QXmlStreamReader, QFile, QDataStream, QString, QList, QMap, QVariant etc. are used.
# QVector is also in Core. QDebug is in Core.
target_link_libraries(rme_core_lib PRIVATE
    Qt6::Core
    Qt6::Widgets # Added for QUndoStack, QUndoCommand, QObject signals/slots in UndoManager
)

# Add other dependencies if necessary
