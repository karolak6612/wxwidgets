# CMakeLists.txt for core world data unit tests (e.g., TownData)

find_package(Qt6 REQUIRED COMPONENTS Test Core)

add_executable(TestCoreWorld_TownData TestTownData.cpp)

target_link_libraries(TestCoreWorld_TownData PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # Link against the main core library
)

# Enable automatic MOC, UIC, RCC handling for Qt
set_target_properties(TestCoreWorld_TownData PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreWorld_TownData COMMAND TestCoreWorld_TownData)
