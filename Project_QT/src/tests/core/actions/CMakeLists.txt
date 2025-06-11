cmake_minimum_required(VERSION 3.16)
project(RMECoreActionsTests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find necessary Qt packages
find_package(Qt6 COMPONENTS Core Test REQUIRED)

# Include directories
# Assuming RME_CORE_LIBRARY_INCLUDE_DIRS is set by a higher-level CMakeLists.txt
# and points to Project_QT/src (or wherever core headers are)
# If not, specify directly: include_directories(../../../..) # Adjust path as needed
include_directories(${RME_SOURCE_DIR}/src) # Provides access to core headers

# Define the test executable
add_executable(rme_core_actions_tests
    TestUndoManager.cpp
    TestTileChangeCommand.cpp
    TestChangeSetCommand.cpp
    # Add other test files here as they are created
)

# Link against Qt and RME core libraries
# RME_CORE_LIBRARY should be a target defined in a higher-level CMakeLists.txt
# that represents the compiled library for your core module.
target_link_libraries(rme_core_actions_tests
    PRIVATE
    Qt6::Core
    Qt6::Test
    ${RME_CORE_LIBRARY} # Link against your main core library
)

# Add the test to CTest
include(CTest)
qt_add_test(rme_core_actions_tests rme_core_actions_tests)
