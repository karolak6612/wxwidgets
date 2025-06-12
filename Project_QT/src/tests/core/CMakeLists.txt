# CMakeLists.txt for all core library unit tests

# Qt6 Test is essential. Core and Gui might be needed by linked rme_core_lib
# or if tests directly use QCoreApplication, QFile, QImage etc.
# Sub-CMakeLists in assets/ and sprites/ will call find_package for their specific needs too.
find_package(Qt6 REQUIRED COMPONENTS Test Core Gui)

# Tests for original core data structures (Position, Item, Tile)
add_executable(TestCore_Position TestPosition.cpp)
target_link_libraries(TestCore_Position PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Position COMMAND TestCore_Position)

# MockItemTypeProvider.h is included by TestItem.cpp and TestTile.cpp,
# it doesn't need to be listed in add_executable unless it has Q_OBJECT macros.
add_executable(TestCore_Item TestItem.cpp)
target_link_libraries(TestCore_Item PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Item COMMAND TestCore_Item)

add_executable(TestCore_Tile TestTile.cpp)
target_link_libraries(TestCore_Tile PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Tile COMMAND TestCore_Tile)


# Add subdirectories for categorized core tests
add_subdirectory(assets)
add_subdirectory(sprites)
add_subdirectory(map)
add_subdirectory(settings) # New line for AppSettings tests
add_subdirectory(actions)
add_subdirectory(io)
add_subdirectory(creatures)
add_subdirectory(navigation) # Added for CORE-11 WaypointData tests
add_subdirectory(spawns)
add_subdirectory(items) # New line for derived item tests
add_subdirectory(world) # New line for world (town) tests
add_subdirectory(brush) # New line for brush logic tests
add_subdirectory(waypoints) # New line for WaypointManager tests

# --- Mock Objects (Common for core tests if needed, or defined per test) ---
# These mocks are specific to the tests in this directory or its subdirectories.
set(TEST_CORE_MOCKS_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/MockMapElements.h
)
set(TEST_CORE_MOCKS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/MockMapElements.cpp
)

# --- Test Executable: test_selectionmanager ---
add_executable(test_selectionmanager
    selection/test_selectionmanager.cpp
    ${TEST_CORE_MOCKS_SRCS} # Compile mocks with the test
)

# Include directories for the test executable:
target_include_directories(test_selectionmanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_SOURCE_DIR}/Project_QT/src/core # To find headers of the library being tested
    ${CMAKE_CURRENT_BINARY_DIR} # For MOC outputs from this test
)

# Link test_selectionmanager against Qt Core, Qt Test, and rme_core_lib
target_link_libraries(test_selectionmanager PRIVATE
    Qt6::Core
    Qt6::Test
    rme_core_lib
)

# Enable automatic MOC, UIC, RCC handling for Qt
set_target_properties(test_selectionmanager PROPERTIES AUTOMOC ON)

# Add the test to CTest
add_test(NAME test_selectionmanager COMMAND test_selectionmanager)

# --- Test Executable: test_clipboardmanager ---
add_executable(test_clipboardmanager
    clipboard/test_clipboardmanager.cpp
    ${TEST_CORE_MOCKS_SRCS} # Compile mocks with the test
)

target_include_directories(test_clipboardmanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_SOURCE_DIR}/Project_QT/src/core # To find headers of the library being tested
    ${CMAKE_CURRENT_BINARY_DIR} # For MOC outputs from this test
)

target_link_libraries(test_clipboardmanager PRIVATE
    Qt6::Core
    Qt6::Test
    rme_core_lib
)

set_target_properties(test_clipboardmanager PROPERTIES AUTOMOC ON)
add_test(NAME test_clipboardmanager COMMAND test_clipboardmanager)

# --- House System Tests ---
add_executable(test_housesystem
    houses/test_housesystem.cpp
    ${TEST_CORE_MOCKS_SRCS} # Mocks are used for tiles within the real Map
)

target_include_directories(test_housesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_SOURCE_DIR}/Project_QT/src/core
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test_housesystem PRIVATE
    Qt6::Core
    Qt6::Test
    rme_core_lib
)

set_target_properties(test_housesystem PROPERTIES AUTOMOC ON)
add_test(NAME test_housesystem COMMAND test_housesystem)


# CTest include is managed at Project_QT/src/tests/CMakeLists.txt.
# add_test() commands register tests with CTest framework.
