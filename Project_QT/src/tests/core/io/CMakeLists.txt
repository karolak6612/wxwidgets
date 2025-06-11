cmake_minimum_required(VERSION 3.16)
project(RMECoreIOTests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find necessary Qt packages
find_package(Qt6 COMPONENTS Core Test REQUIRED)

# Include directories
# Assumes RME_SOURCE_DIR is set by a higher-level CMakeLists.txt
# to point to Project_QT directory.
include_directories(${RME_SOURCE_DIR}/src) # Provides access to core headers

# Define the test executable
add_executable(rme_core_io_tests
    TestOtbmMapIO.cpp
    # Add other test files here as they are created
)

# Link against Qt and RME core libraries
# RME_CORE_LIBRARY should be a target defined in a higher-level CMakeLists.txt
# that represents the compiled library for your core module.
target_link_libraries(rme_core_io_tests
    PRIVATE
    Qt6::Core
    Qt6::Test
    ${RME_CORE_LIBRARY} # Link against your main core library
)

# Add the test to CTest
include(CTest)
qt_add_test(rme_core_io_tests rme_core_io_tests)
