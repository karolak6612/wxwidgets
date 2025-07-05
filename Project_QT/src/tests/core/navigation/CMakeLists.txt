# CMakeLists.txt for Project_QT/src/tests/core/navigation

cmake_minimum_required(VERSION 3.16)
project(RMECoreNavigationTests)

set(CMAKE_AUTOMOC ON)

# Find necessary Qt packages
find_package(Qt6 COMPONENTS Core Test REQUIRED)

# Define the test executable for navigation tests (currently just WaypointData)
add_executable(rme_core_navigation_tests
    TestWaypointData.cpp
)

# Link against Qt and RME core libraries
# RME_CORE_LIBRARY_TARGET_NAME should be defined by the root CMakeLists.txt or passed down.
# It refers to the target name of the main core library (e.g., rme_core_lib)
if(NOT DEFINED RME_CORE_LIBRARY_TARGET_NAME)
    set(RME_CORE_LIBRARY_TARGET_NAME rme_core_lib) # Fallback, but should be globally defined
    message(WARNING "RME_CORE_LIBRARY_TARGET_NAME was not defined. Using default 'rme_core_lib'. Ensure this is correct.")
endif()

if(NOT TARGET ${RME_CORE_LIBRARY_TARGET_NAME})
    message(FATAL_ERROR "RME_CORE_LIBRARY_TARGET_NAME (${RME_CORE_LIBRARY_TARGET_NAME}) is not a valid target. It must be created by Project_QT/src/core/CMakeLists.txt.")
endif()

target_link_libraries(rme_core_navigation_tests
    PRIVATE
    Qt6::Core
    Qt6::Test
    ${RME_CORE_LIBRARY_TARGET_NAME}
)

# Add the test to CTest
include(CTest)
qt_add_test(rme_core_navigation_tests_run rme_core_navigation_tests)
