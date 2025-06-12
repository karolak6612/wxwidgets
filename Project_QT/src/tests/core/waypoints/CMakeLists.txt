# CMakeLists.txt for core waypoint unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core)

# Test for WaypointManager
add_executable(TestCoreWaypoints_Manager TestWaypointManager.cpp)

target_link_libraries(TestCoreWaypoints_Manager PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # For Map, Tile, Waypoint, etc.
)

# Enable automatic MOC, UIC, RCC handling for Qt
# Required because TestWaypointManager uses Q_OBJECT
set_target_properties(TestCoreWaypoints_Manager PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreWaypoints_Manager COMMAND TestCoreWaypoints_Manager)
