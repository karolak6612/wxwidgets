# CMakeLists.txt for core brush logic unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core)

# MockEditorController.h is included by TestRawBrush.cpp
# It does not need to be listed in add_executable unless it has Q_OBJECT macros or its own .cpp.
# TestRawBrush.cpp also includes MockItemTypeProvider.h from tests/core/
# Listing MockEditorController.h here for clarity or if it ever becomes more complex.
add_executable(TestCoreBrush_RawBrush TestRawBrush.cpp MockEditorController.h)

target_link_libraries(TestCoreBrush_RawBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib         # For Base Brush, Map, Tile, Item, AssetManager access etc.
    rme_editor_logic_lib # Added: RawBrush uses commands from editor_logic
)

# Ensure tests can find headers from the main src/core directory
# This is typically handled by rme_core_lib's PUBLIC include directories
# target_include_directories(TestCoreBrush_RawBrush PRIVATE
#     ${CMAKE_PROJECT_SOURCE_DIR}/Project_QT/src # If headers like core/Position.h are needed directly
# )


# Enable automatic MOC, UIC, RCC handling for Qt
# Required because TestRawBrush uses Q_OBJECT
set_target_properties(TestCoreBrush_RawBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_RawBrush COMMAND TestCoreBrush_RawBrush)

# Test for EraserBrush
# MockEditorController.h is included by TestEraserBrush.cpp
add_executable(TestCoreBrush_EraserBrush TestEraserBrush.cpp)

target_link_libraries(TestCoreBrush_EraserBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_EraserBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_EraserBrush COMMAND TestCoreBrush_EraserBrush)

# Test for HouseBrush
# MockEditorController.h is included by TestHouseBrush.cpp
add_executable(TestCoreBrush_HouseBrush TestHouseBrush.cpp)

target_link_libraries(TestCoreBrush_HouseBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_HouseBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_HouseBrush COMMAND TestCoreBrush_HouseBrush)

# Test for CarpetBrush
# MockEditorController.h is included by TestCarpetBrush.cpp
add_executable(TestCoreBrush_CarpetBrush TestCarpetBrush.cpp)

target_link_libraries(TestCoreBrush_CarpetBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_CarpetBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_CarpetBrush COMMAND TestCoreBrush_CarpetBrush)

# Test for TableBrush
add_executable(TestCoreBrush_TableBrush TestTableBrush.cpp)

target_link_libraries(TestCoreBrush_TableBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_TableBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_TableBrush COMMAND TestCoreBrush_TableBrush)

# Test for WallBrush
add_executable(TestCoreBrush_WallBrush TestWallBrush.cpp)

target_link_libraries(TestCoreBrush_WallBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_WallBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_WallBrush COMMAND TestCoreBrush_WallBrush)

# Test for DoodadBrush
add_executable(TestCoreBrush_DoodadBrush TestDoodadBrush.cpp)

target_link_libraries(TestCoreBrush_DoodadBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
)

set_target_properties(TestCoreBrush_DoodadBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_DoodadBrush COMMAND TestCoreBrush_DoodadBrush)

# Test for SpawnBrush
add_executable(TestCoreBrush_SpawnBrush TestSpawnBrush.cpp)

target_link_libraries(TestCoreBrush_SpawnBrush PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib
    rme_editor_logic_lib # For RecordSetSpawnCommand if SpawnBrush::apply uses it directly
)

set_target_properties(TestCoreBrush_SpawnBrush PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreBrush_SpawnBrush COMMAND TestCoreBrush_SpawnBrush)
