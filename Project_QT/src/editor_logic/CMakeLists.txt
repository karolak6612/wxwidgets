# CMakeLists.txt for the editor_logic library

set(RME_EDITOR_LOGIC_LIB_SOURCES
    EditorController.cpp
    commands/BrushStrokeCommand.cpp
    commands/DeleteCommand.cpp
    commands/AddWaypointCommand.cpp
    commands/MoveWaypointCommand.cpp
    commands/SetHouseExitCommand.cpp
    commands/RecordSetSpawnCommand.cpp
    commands/RecordSetGroundCommand.cpp     # Added
    commands/RecordAddRemoveItemCommand.cpp # Added
    commands/RecordModifyTileContentsCommand.cpp # Added
)

set(RME_EDITOR_LOGIC_LIB_HEADERS
    EditorController.h
    commands/BrushStrokeCommand.h
    commands/DeleteCommand.h
    commands/AddWaypointCommand.h
    commands/MoveWaypointCommand.h
    commands/SetHouseExitCommand.h
    commands/RecordSetSpawnCommand.h
    commands/RecordSetGroundCommand.h       # Added
    commands/RecordAddRemoveItemCommand.h   # Added
    commands/RecordModifyTileContentsCommand.h # Added
)
# Note: RME_EDITOR_LOGIC_LIB_HEADERS is for organization/completeness,
# actual header availability is via target_include_directories.

add_library(rme_editor_logic_lib STATIC
    ${RME_EDITOR_LOGIC_LIB_SOURCES}
    # Headers are not typically listed directly in add_library for static/shared libs
    # unless they are non-installable headers that need to be part of the target's sources for IDEs.
    # It's more common to list only .cpp files.
)

# Public include directories for this library
# This makes headers within Project_QT/src/editor_logic/ available
# to other targets that link against rme_editor_logic_lib.
target_include_directories(rme_editor_logic_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against necessary libraries (e.g., rme_core_lib and Qt Core)
target_link_libraries(rme_editor_logic_lib PRIVATE
    Qt6::Core
    rme_core_lib # Depends on core data structures and services
)
