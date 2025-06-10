# CMakeLists.txt for the core library of RME-Qt6

# Define the library sources
set(RME_CORE_LIB_SOURCES
    # Original CORE-01 files
    Position.cpp
    Item.cpp          # Uses IItemTypeProvider.h
    Creature.cpp      # Stub
    Spawn.cpp         # Stub
    Tile.cpp          # Uses Item.h, Creature.h, Spawn.h, Position.h

    # New CORE-02 asset loading files
    assets/ClientVersionManager.cpp
    assets/ItemDatabase.cpp
    assets/CreatureDatabase.cpp
    assets/AssetManager.cpp         # Uses all managers, IItemTypeProvider.h
    sprites/SpriteManager.cpp
)

add_library(rme_core_lib STATIC ${RME_CORE_LIB_SOURCES})

# Public include directories for this library
# This allows other targets linking against rme_core_lib to find its headers
# e.g., #include "Position.h", #include "assets/AssetManager.h", #include "sprites/SpriteManager.h"
target_include_directories(rme_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/core/ and its subdirectories
)

# Link against Qt6 Core as QXmlStreamReader, QFile, QDataStream, QString, QList, QMap, QVariant etc. are used.
target_link_libraries(rme_core_lib PRIVATE Qt6::Core)

# Add other dependencies if necessary
