# CMakeLists.txt for the editor_logic library

set(RME_EDITOR_LOGIC_LIB_SOURCES
    EditorController.cpp
    commands/AddCreatureCommand.cpp          # Added
    commands/AddWaypointCommand.cpp
    commands/BoundingBoxSelectCommand.cpp    # Added
    commands/BrushStrokeCommand.cpp
    commands/ClearSelectionCommand.cpp       # Added
    commands/DeleteCommand.cpp
    commands/DeleteSelectionCommand.cpp      # Added
    commands/MoveWaypointCommand.cpp
    commands/RecordAddRemoveItemCommand.cpp # Added
    commands/RecordModifyTileContentsCommand.cpp # Added
    commands/RecordSetGroundCommand.cpp     # Added
    commands/RecordSetSpawnCommand.cpp
    commands/RemoveCreatureCommand.cpp       # Added
    commands/SetHouseExitCommand.cpp
    commands/SetHouseTileCommand.cpp         # Added
)

set(RME_EDITOR_LOGIC_LIB_HEADERS
    EditorController.h
    commands/AddCreatureCommand.h            # Added
    commands/AddWaypointCommand.h
    commands/BoundingBoxSelectCommand.h      # Added
    commands/BrushStrokeCommand.h
    commands/ClearSelectionCommand.h         # Added
    commands/DeleteCommand.h
    commands/DeleteSelectionCommand.h        # Added
    commands/MoveWaypointCommand.h
    commands/RecordAddRemoveItemCommand.h   # Added
    commands/RecordModifyTileContentsCommand.h # Added
    commands/RecordSetGroundCommand.h       # Added
    commands/RecordSetSpawnCommand.h
    commands/RemoveCreatureCommand.h         # Added
    commands/SetHouseExitCommand.h           # Verified
    commands/SetHouseTileCommand.h           # Added
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
