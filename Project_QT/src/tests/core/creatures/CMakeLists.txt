# CMakeLists.txt for Project_QT/src/tests/core/creatures

cmake_minimum_required(VERSION 3.16)
project(RMECoreCreaturesTests)

set(CMAKE_AUTOMOC ON)
# CMAKE_AUTORCC and CMAKE_AUTOUIC might not be needed if no .qrc or .ui files here

# Find necessary Qt packages (already found by parent, but good practice for standalone understanding)
find_package(Qt6 COMPONENTS Core Test REQUIRED)

# Include directories
# Parent (tests/core/CMakeLists.txt or tests/CMakeLists.txt) should ensure
# that ${RME_SOURCE_DIR}/src is an include directory to find core headers like:
# "core/creatures/Creature.h"
# "core/assets/CreatureData.h"
# If not, it needs to be added here or in a higher-level test CMake file.
# Assuming include_directories(${RME_SOURCE_DIR}/src) is already effective from a parent.

# Define the test executable for creature tests
add_executable(rme_core_creatures_tests
    TestOutfit.cpp
    TestCreature.cpp
)

# Link against Qt and RME core libraries
# RME_CORE_LIBRARY should be a target defined in a higher-level CMakeLists.txt
# (e.g. Project_QT/src/core/CMakeLists.txt creates rme_core_lib)
if(NOT TARGET ${RME_CORE_LIBRARY_TARGET_NAME})
    message(FATAL_ERROR "RME_CORE_LIBRARY_TARGET_NAME (${RME_CORE_LIBRARY_TARGET_NAME}) is not defined or not a target. It must be created by src/core/CMakeLists.txt.")
endif()

target_link_libraries(rme_core_creatures_tests
    PRIVATE
    Qt6::Core
    Qt6::Test
    ${RME_CORE_LIBRARY_TARGET_NAME} # Link against your main core library
)

# Add the test to CTest
include(CTest)
# The name "rme_core_creatures_tests_run" is just a unique name for CTest internal tracking.
# The actual executable is rme_core_creatures_tests.
qt_add_test(rme_core_creatures_tests_run rme_core_creatures_tests)
