# CMakeLists.txt for core action tests (undo/redo)

find_package(Qt6 REQUIRED COMPONENTS Test Core Gui) # Gui is included as Map might need it via rme_core_lib

add_executable(TestCore_Actions_UndoRedo test_undoredo.cpp)

# Public include for rme_core_lib is src/core. Headers in actions subdir are found via "actions/header.h"
# Tests are in src/tests/core/actions. To find rme_core_lib headers:
# target_include_directories(TestCore_Actions_UndoRedo PRIVATE ../../) would make #include "core/actions/header.h" possible
# However, rme_core_lib already exports its include directory as "src/core"
# So includes like "actions/appundocommand.h" should work if CMakeLists for rme_core_lib is set up correctly.
# And for "map/map.h", "Tile.h" etc.

target_link_libraries(TestCore_Actions_UndoRedo PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui # Added because rme_core_lib links to Widgets, which pulls Gui. Better to be explicit.
    rme_core_lib
)

add_test(NAME TestCore_Actions_UndoRedo COMMAND TestCore_Actions_UndoRedo)
