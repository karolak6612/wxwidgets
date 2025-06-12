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
    rme_core_lib # For Base Brush, Map, Tile, Item, AssetManager access etc.
    # rme_editor_logic_lib # Not strictly needed if MockEditorController.h defines its own
                           # minimal interface or includes the real one from editor_logic,
                           # and the test links rme_core_lib which provides BrushSettings etc.
                           # If EditorControllerInterface.h is in rme_editor_logic_lib and included
                           # by MockEditorController.h, then this link would be necessary.
                           # For now, assuming this test is self-contained with mocks or
                           # that rme_core_lib provides enough (like BrushSettings).
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
